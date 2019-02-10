//
// Created by bearh on 02.12.18.
//

#include "sh_rmt_conn.h"

#include <assert.h>
#include <memory.h>
#include <stddef.h>

#include <openthread/coap.h>
#ifdef COAP_PSK
#include <openthread/coap_secure.h>
#endif // COAP_PSK
#include <openthread/ip6.h>
#include <openthread/joiner.h>
#include <openthread/link.h>
#include <openthread/thread.h>

#include "sh_rmt_anim.h"
#include "../../lib/conn/humi_conn.h"
#include "../../lib/mcbor/mcbor_dec.h"
#include "../../lib/mcbor/mcbor_enc.h"
#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 6

#define REQ_ZONE_ACTION_KEY  "val"
#define REQ_ZONE_ACTION_UP   "up"
#define REQ_ZONE_ACTION_DOWN "down"
#define REQ_ZONE_ACTION_STOP "stop"

#define SD_PAYLOAD_MAX_SIZE 128
#define SH_PAYLOAD_MAX_SIZE 64

#define SER_TYPE_KEY "type"
#define SER_TYPE_VAL "shcnt"

#ifdef COAP_PSK
static const char coap_psk[] = COAP_PSK;
static const char coap_cli_id[] = "def";
#endif // COAP_PSK

static const char * const zone_names[NUM_ZONES] = {
        "dr2",
        "dr3",
        "k",
        "br",
        "lr",
        "dr1",
};

static otIp6Address zone_addresses[NUM_ZONES];

typedef enum
{
    REQ_UP,
    REQ_DOWN,
    REQ_STOP,
} req_t;

static struct {
    otIp6Address *zone_address;
    req_t         req;
    int           zone;
} cur_req;

static struct {
    uint32_t zone_mask;
    req_t    req;
} req_mask;

static void req_next(void);

// Polling period

static void pp_default(void)
{
    humi_conn_normal_poll_period();
}

static void pp_fast(void)
{
    humi_conn_fast_poll_period();
}

// Service discovery

#define SD_RERUN_DELAY 600000
#define SD_MIN_SEARCH_DELAY 5000
#define SD_MAX_SEARCH_DELAY 80000

static uint32_t     sd_last_delay;
static humi_timer_t sd_timer;

static void sd_start_timer(void);
static void sd_reset_timer(void);

static mcbor_err_t process_and_skip_resource(const mcbor_dec_t *mcbor_dec, const void **resource_key, const otIp6Address *peer_addr)
{
    const void *item = *resource_key;
    mcbor_err_t err;
    mcbor_err_t result = MCBOR_ERR_SUCCESS;
    bool        skip;

    const char *res_name;
    size_t      res_name_len;

    err = mcbor_dec_get_text(mcbor_dec, item, &res_name, &res_name_len);

    if (err == MCBOR_ERR_NOT_FOUND)
    {
        skip = true;
    }
    else if (err != MCBOR_ERR_SUCCESS)
    {
        return err;
    }
    else
    {
        skip = true;

        for (int i = 0; i < NUM_ZONES; i++)
        {
            const void *desc;
            int         desc_pairs;

            const char *key;
            size_t      key_len;
            const char *value;
            size_t      value_len;

            if (!mcbor_dec_is_text_equal_to_str(res_name, res_name_len, zone_names[i]))
            {
                continue;
            }

            skip = false;

            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip key
            if (err != MCBOR_ERR_SUCCESS) return err;

            err = mcbor_dec_get_map(mcbor_dec, item, &desc, &desc_pairs);
            if (err == MCBOR_ERR_NOT_FOUND)
            {
                err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip value
                if (err != MCBOR_ERR_SUCCESS) return err;
                continue;
            }
            else if ((err != MCBOR_ERR_SUCCESS) && (err != MCBOR_ERR_DATA_END))
            {
                return err;
            }
            else
            {
                result = err;
            }

            for (int j = 0; j < desc_pairs; j++)
            {
                // Get key
                err = mcbor_dec_get_text(mcbor_dec, desc, &key, &key_len);
                if (err == MCBOR_ERR_NOT_FOUND)
                {
                    err = mcbor_dec_skip_item(mcbor_dec, &desc); // Skip key
                    if (err != MCBOR_ERR_SUCCESS) return err;
                    err = mcbor_dec_skip_item(mcbor_dec, &desc); // Skip value
                    if (err != MCBOR_ERR_SUCCESS) return err;

                    continue;
                }
                if ((err != MCBOR_ERR_SUCCESS) )
                {
                    return err;
                }

                err = mcbor_dec_skip_item(mcbor_dec, &desc); // Skip key
                if (err != MCBOR_ERR_SUCCESS) return err;

                // Get value
                err = mcbor_dec_get_text(mcbor_dec, desc, &value, &value_len);
                if (err == MCBOR_ERR_NOT_FOUND)
                {
                    err = mcbor_dec_skip_item(mcbor_dec, &desc); // Skip value
                    if (err != MCBOR_ERR_SUCCESS) return err;

                    continue;
                }
                if ((err != MCBOR_ERR_SUCCESS) && (err != MCBOR_ERR_DATA_END))
                {
                    return err;
                }

                if (mcbor_dec_is_text_equal_to_str(key, key_len, SER_TYPE_KEY) &&
                    mcbor_dec_is_text_equal_to_str(value, value_len, SER_TYPE_VAL))
                {
                    zone_addresses[i] = *peer_addr;
                }

                err = mcbor_dec_skip_item(mcbor_dec, &desc); // Skip value
                if (err != MCBOR_ERR_SUCCESS) return err;
            }

            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip value
            if (err != MCBOR_ERR_SUCCESS) return err;
            break;
        }
    }

    if (skip)
    {
        err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip key
        if (err != MCBOR_ERR_SUCCESS) return err;
        err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip value
        if (err != MCBOR_ERR_SUCCESS) return err;
    }

    *resource_key = item;
    return result;
}

static void sd_response_handler(void                *context,
                                otMessage           *message,
                                const otMessageInfo *message_info,
                                otError              result)
{
    char payload[SD_PAYLOAD_MAX_SIZE];
    int payload_len;

    (void)context;

    pp_default();

    if (result != OT_ERROR_NONE)
    {
        return;
    }

    if (otCoapMessageGetCode(message) != OT_COAP_CODE_CONTENT)
    {
        return;
    }

    if (!humi_conn_is_addr_local(&message_info->mPeerAddr))
    {
        // Allow only local traffic, because there is no encryption at the moment
        return;
    }

    payload_len = otMessageRead(message, otMessageGetOffset(message), payload, sizeof(payload));

    mcbor_iter_t   iter;
    mcbor_dec_t    mcbor_dec;
    const void    *top_map_item;
    int            pairs;

    mcbor_dec_init(payload, (size_t)payload_len, &mcbor_dec);
    mcbor_dec_iter_init(&mcbor_dec, &iter);

    for (mcbor_err_t err = mcbor_dec_iter_map(&mcbor_dec, &iter, &top_map_item, &pairs);
         err == MCBOR_ERR_SUCCESS;
         err = mcbor_dec_iter_map(&mcbor_dec, &iter, &top_map_item, &pairs))
    {
        mcbor_err_t mcbor_err;

        for (int i = 0; i < pairs; i++)
        {
            mcbor_err = process_and_skip_resource(&mcbor_dec, &top_map_item, &message_info->mPeerAddr);
            if (mcbor_err != MCBOR_ERR_SUCCESS) break;
        }
    }
}

static void sd_request(void)
{
    otError       error = OT_ERROR_NONE;
    otMessage   * request;
    otMessageInfo message_info;

    request = otCoapNewMessage(humi_conn_get_instance(), NULL);
    if (request == NULL)
    {
        goto exit;
    }

    otCoapMessageInit(request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapMessageGenerateToken(request, 2);
    (void)otCoapMessageAppendUriPathOptions(request, "sd");

    memset(&message_info, 0, sizeof(message_info));
    message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
    (void)otIp6AddressFromString("ff03::1", &message_info.mPeerAddr);

    error = otCoapSendRequest(humi_conn_get_instance(), request, &message_info, sd_response_handler, NULL);

    pp_fast();

exit:
    if ((error != OT_ERROR_NONE) && (request != NULL))
    {
        otMessageFree(request);
    }
}

static void sd_rerun(void *context)
{
    (void)context;

    sd_request();
    sd_start_timer();
}

static void sd_start_timer(void)
{
    bool     all_services_known = true;
    uint32_t delay;

    for (int i = 0; i < NUM_ZONES; i++)
    {
        if (otIp6IsAddressUnspecified(&zone_addresses[i]))
        {
            all_services_known = false;
            break;
        }
    }

    if (all_services_known)
    {
        delay = SD_RERUN_DELAY;
    }
    else
    {
        delay = sd_last_delay * 2;

        if (delay > SD_MAX_SEARCH_DELAY)
        {
            delay = SD_MAX_SEARCH_DELAY;
        }
    }

    sd_last_delay = delay;

    sd_timer.target_time = humi_timer_get_target_from_delay(delay);
    sd_timer.callback    = sd_rerun;
    sd_timer.context     = NULL;

    humi_timer_gen_add(&sd_timer);
}

static void sd_reset_timer(void)
{
    sd_request();

    sd_last_delay = SD_MIN_SEARCH_DELAY / 2;
    sd_start_timer();
}

static void sd_remove_zone_addr(const otIp6Address *zone_addr)
{
    for (int i = 0; i < NUM_ZONES; i++)
    {
        if (otIp6IsAddressEqual(&zone_addresses[i], zone_addr))
        {
            memset(&zone_addresses[i], 0, sizeof(zone_addresses[i]));
        }
    }

    sd_reset_timer();
}

static void sd_init(void)
{
    sd_reset_timer();
}

static void ot_state_changed(otChangedFlags flags, void *context)
{
    (void)context;

    if (flags & OT_CHANGED_THREAD_ROLE)
    {
        otDeviceRole ot_dev_role = otThreadGetDeviceRole(humi_conn_get_instance());

        switch (ot_dev_role)
        {
            case OT_DEVICE_ROLE_DISABLED:
            case OT_DEVICE_ROLE_DETACHED:
                sh_rmt_anim_connecting();
                break;

            case OT_DEVICE_ROLE_CHILD:
            case OT_DEVICE_ROLE_ROUTER:
            case OT_DEVICE_ROLE_LEADER:
                sh_rmt_anim_connected();

                sd_init();
                break;
        }
    }

    if (flags & OT_CHANGED_JOINER_STATE)
    {
        otJoinerState j_st = otJoinerGetState(humi_conn_get_instance());

        switch (j_st)
        {
            case OT_JOINER_STATE_DISCOVER:
                sh_rmt_anim_discovery();
                break;

            case OT_JOINER_STATE_CONNECT:
            case OT_JOINER_STATE_CONNECTED:
            case OT_JOINER_STATE_ENTRUST:
                sh_rmt_anim_commissioning();
                break;

            case OT_JOINER_STATE_IDLE:
            case OT_JOINER_STATE_JOINED:
                sh_rmt_anim_commissioning_idle();
                break;
        }
    }
}

static void zone_response_handler(void                *context,
                                  otMessage           *message,
                                  const otMessageInfo *message_info,
                                  otError              result)
{
    (void)context;
    (void)message;

    if (result != OT_ERROR_NONE)
    {
        sd_remove_zone_addr(&message_info->mPeerAddr);
    }

#ifdef COAP_PSK
    otCoapSecureDisconnect(humi_conn_get_instance());
#else // COAP_PSK
    pp_default();
#endif // COAP_PSK

    req_next();
}

static size_t create_req_zone_payload(req_t req, uint8_t *buffer, size_t buffer_size)
{
    char *action_value;

    mcbor_enc_t cbor;
    mcbor_enc_init(buffer, buffer_size, &cbor);

    if (mcbor_enc_map(&cbor, 1) != MCBOR_ERR_SUCCESS) return 0;

    if (mcbor_enc_text(&cbor, REQ_ZONE_ACTION_KEY) != MCBOR_ERR_SUCCESS) return 0;

    switch (req)
    {
        case REQ_UP:
            action_value = REQ_ZONE_ACTION_UP;
            break;

        case REQ_DOWN:
            action_value = REQ_ZONE_ACTION_DOWN;
            break;

        case REQ_STOP:
            action_value = REQ_ZONE_ACTION_STOP;
            break;

        default:
            assert(false);
    }

    if (mcbor_enc_text(&cbor, action_value) != MCBOR_ERR_SUCCESS) return 0;

    return mcbor_get_size(&cbor);
}

static bool send_zone_request(req_t req, int zone, const otIp6Address *zone_addr)
{
    const char   *zone_name = zone_names[zone];

    otError    error = OT_ERROR_NONE;
    otMessage *request = NULL;
#ifndef COAP_PSK
    otMessageInfo message_info;
#endif // COAP_PSK

    uint8_t payload[SH_PAYLOAD_MAX_SIZE];
    size_t  payload_size;

    request = otCoapNewMessage(humi_conn_get_instance(), NULL);
    if (request == NULL)
    {
        goto exit;
    }

    otCoapMessageInit(request, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_PUT);
    otCoapMessageGenerateToken(request, 2);
    (void)otCoapMessageAppendUriPathOptions(request, zone_name);
    (void)otCoapMessageAppendContentFormatOption(request, OT_COAP_OPTION_CONTENT_FORMAT_CBOR);
    (void)otCoapMessageSetPayloadMarker(request);

    payload_size = create_req_zone_payload(req, payload, sizeof(payload));
    assert(payload_size > 0);
    assert(payload_size <= UINT16_MAX);

    error = otMessageAppend(request, payload, (uint16_t)payload_size);
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }

#ifdef COAP_PSK
    error = otCoapSecureSendRequest(humi_conn_get_instance(), request, zone_response_handler, NULL);
#else // COAP_PSK
    memset(&message_info, 0, sizeof(message_info));
    message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
    message_info.mPeerAddr    = *zone_addr;

    error = otCoapSendRequest(humi_conn_get_instance(), request, &message_info, zone_response_handler, NULL);
#endif // COAP_PSK

exit:
    if ((error != OT_ERROR_NONE) && (request != NULL))
    {
        otMessageFree(request);
    }

    return error == OT_ERROR_NONE;
}

void secure_req_conn_handler(bool connected, void *context)
{
    (void)context;

    if (connected)
    {
        send_zone_request(cur_req.req, cur_req.zone, cur_req.zone_address);
    }
    else
    {
        pp_default();
        sd_remove_zone_addr(cur_req.zone_address);
    }
}

static void req_zone(req_t req, int zone)
{
    otIp6Address *zone_addr = &zone_addresses[zone];

#ifdef COAP_PSK
    otError    error;
    otSockAddr sockAddr = {
            .mAddress = *zone_addr,
            .mPort    = OT_DEFAULT_COAP_SECURE_PORT,
            .mScopeId = OT_NETIF_INTERFACE_ID_THREAD,
    };
#endif // COAP_PSK

    if (otIp6IsAddressUnspecified(zone_addr))
    {
        sd_reset_timer();
        goto exit;
    }

#ifdef COAP_PSK
    cur_req.req          = req;
    cur_req.zone         = zone;
    cur_req.zone_address = zone_addr;

    error = otCoapSecureConnect(humi_conn_get_instance(), &sockAddr, secure_req_conn_handler, NULL);
    if (error != OT_ERROR_NONE)
    {
        sd_remove_zone_addr(zone_addr);
        goto exit;
    }
    else
    {
        pp_fast();
    }
#else // COAP_PSK
    bool result = send_zone_request(req, zone, zone_addr);
    if (result)
    {
        pp_fast();
    }
#endif // COAP_PSK

exit:
    return;
}

static void req_next(void)
{
    if (req_mask.zone_mask)
    {
        for (int i = 0; i < NUM_ZONES; i++)
        {
            uint32_t cur_mask = (1 << i);

            if (req_mask.zone_mask & cur_mask)
            {
                req_zone(req_mask.req, i);
                req_mask.zone_mask &= ~cur_mask;
            }
        }
    }
}

static void req_zones(req_t req, uint32_t zone_mask)
{
    req_mask.req       = req;
    req_mask.zone_mask = zone_mask;

    req_next();
}


void sh_rmt_conn_init(void)
{
    otError error;

    humi_conn_init(true);

    error = otSetStateChangedCallback(humi_conn_get_instance(), ot_state_changed, NULL);
    assert(error == OT_ERROR_NONE);

#ifdef COAP_PSK
    error = otCoapSecureSetPsk(humi_conn_get_instance(), coap_psk, strlen(coap_psk), coap_cli_id, strlen(coap_cli_id));
    assert(error == OT_ERROR_NONE);

    otCoapSecureSetSslAuthMode(humi_conn_get_instance(), true);

    error = otCoapSecureStart(humi_conn_get_instance(), OT_DEFAULT_COAP_SECURE_PORT, NULL);
    assert(error == OT_ERROR_NONE);
#endif // COAP_PSK

    error = otCoapStart(humi_conn_get_instance(), OT_DEFAULT_COAP_PORT);
    assert(error == OT_ERROR_NONE);

    humi_conn_start();
}

void sh_rmt_conn_process(void)
{
    humi_conn_process();
}

bool sh_rmt_conn_is_pending(void)
{
    return humi_conn_is_pending();
}

void sh_rmt_conn_reset(void)
{
    humi_conn_stop();
    humi_conn_decommission();
    humi_conn_start();
}

void sh_rmt_conn_up(uint32_t zone_mask) {
    req_zones(REQ_UP, zone_mask);
}

void sh_rmt_conn_down(uint32_t zone_mask) {
    req_zones(REQ_DOWN, zone_mask);
}

void sh_rmt_conn_stop(uint32_t zone_mask) {
    req_zones(REQ_STOP, zone_mask);
}
