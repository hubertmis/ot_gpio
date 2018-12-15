//
// Created by bearh on 02.12.18.
//

#include "sh_rmt_conn.h"

#include <assert.h>
#include <memory.h>
#include <stddef.h>

#include <openthread/coap.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/thread.h>

#include "sh_rmt_anim.h"
#include "../../lib/conn/humi_conn.h"
#include "../../lib/mcbor/mcbor_dec.h"
#include "../../lib/mcbor/mcbor_enc.h"
#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 7

#define REQ_ZONE_ACTION_KEY  "val"
#define REQ_ZONE_ACTION_UP   "up"
#define REQ_ZONE_ACTION_DOWN "down"
#define REQ_ZONE_ACTION_STOP "stop"

#define SD_PAYLOAD_MAX_SIZE 128
#define SH_PAYLOAD_MAX_SIZE 64

#define SER_TYPE_KEY "type"
#define SER_TYPE_VAL "shcnt"

static const char * const zone_names[NUM_ZONES] = {
        "dr1",
        "dr2",
        "dr3",
        "k2",
        "br",
        "lr",
        "k1",
};

static otIp6Address zone_addresses[NUM_ZONES];

typedef enum
{
    REQ_UP,
    REQ_DOWN,
    REQ_STOP,
} req_t;

// Polling period

#define PP_DEFAULT 60000
#define PP_FAST    500

static void pp_default(void)
{
    // TODO decrement a counter
    otLinkSetPollPeriod(humi_conn_get_instance(), PP_DEFAULT);
}

static void pp_fast(void)
{
    // TODO increment a counter
    otLinkSetPollPeriod(humi_conn_get_instance(), PP_FAST);
}

// Service discovery

#define SD_RERUN_DELAY 120000

static humi_timer_t sd_timer;

static void sd_start_timer(void);

static mcbor_err_t process_and_skip_resource(const mcbor_dec_t *mcbor_dec, const void **resource_key, const otIp6Address *peer_addr)
{
    const void *item = *resource_key;
    mcbor_err_t err;
    mcbor_err_t result;
    bool        skip = false;

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
                                otCoapHeader        *header,
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

    if (otCoapHeaderGetCode(header) != OT_COAP_CODE_CONTENT)
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
    otCoapHeader  header;
    otMessage   * request;
    otMessageInfo message_info;

    otCoapHeaderInit(&header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_GET);
    otCoapHeaderGenerateToken(&header, 2);
    (void)otCoapHeaderAppendUriPathOptions(&header, "sd");

    request = otCoapNewMessage(humi_conn_get_instance(), &header, NULL);
    if (request == NULL)
    {
        goto exit;
    }

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

    // TODO: transmit a multicast SD message
    sd_start_timer();
}

static void sd_start_timer(void)
{
    sd_timer.target_time = humi_timer_get_target_from_delay(SD_RERUN_DELAY);
    sd_timer.callback    = sd_rerun;
    sd_timer.context     = NULL;

    humi_timer_gen_add(&sd_timer);
}

static void sd_init(void)
{
    sd_request();
    sd_start_timer();
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
}

static void zone_response_handler(void                *context,
                                  otCoapHeader        *header,
                                  otMessage           *message,
                                  const otMessageInfo *message_info,
                                  otError              result)
{
    (void)context;
    (void)header;
    (void)message;
    (void)message_info;
    (void)result;

    pp_default();
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

static void req_zone(req_t req, int zone)
{
    const uint8_t *zone_name = zone_names[zone];
    otIp6Address  *zone_addr = &zone_addresses[zone];

    otError       error = OT_ERROR_NONE;
    otCoapHeader  header;
    otMessage   * request;
    otMessageInfo message_info;

    uint8_t payload[SH_PAYLOAD_MAX_SIZE];
    size_t  payload_size;

    if (otIp6IsAddressUnspecified(zone_addr))
    {
        goto exit;
    }

    otCoapHeaderInit(&header, OT_COAP_TYPE_CONFIRMABLE, OT_COAP_CODE_PUT);
    otCoapHeaderGenerateToken(&header, 2);
    (void)otCoapHeaderAppendUriPathOptions(&header, zone_name);
    (void)otCoapHeaderAppendContentFormatOption(&header, OT_COAP_OPTION_CONTENT_FORMAT_CBOR);
    (void)otCoapHeaderSetPayloadMarker(&header);

    request = otCoapNewMessage(humi_conn_get_instance(), &header, NULL);
    if (request == NULL)
    {
        goto exit;
    }

    payload_size = create_req_zone_payload(req, payload, sizeof(payload));
    assert(payload_size > 0);

    error = otMessageAppend(request, payload, payload_size);
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }

    memset(&message_info, 0, sizeof(message_info));
    message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
    message_info.mPeerAddr    = *zone_addr;

    error = otCoapSendRequest(humi_conn_get_instance(), request, &message_info, zone_response_handler, NULL);

    if (error == OT_ERROR_NONE)
    {
        pp_fast();
    }

    exit:
    if ((error != OT_ERROR_NONE) && (request != NULL))
    {
        otMessageFree(request);
    }
}

static void req_zones(req_t req, uint32_t zone_mask)
{
    for (int i = 0; i < NUM_ZONES; i++)
    {
        if (zone_mask & (1 << i)) {
            req_zone(req, i);
        }
    }
}


void sh_rmt_conn_init(void)
{
    otError error;

    humi_conn_init(true);

    error = otSetStateChangedCallback(humi_conn_get_instance(), ot_state_changed, NULL);
    assert(error == OT_ERROR_NONE);

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

void sh_rmt_conn_up(uint32_t zone_mask) {
    req_zones(REQ_UP, zone_mask);
}

void sh_rmt_conn_down(uint32_t zone_mask) {
    req_zones(REQ_DOWN, zone_mask);
}

void sh_rmt_conn_stop(uint32_t zone_mask) {
    req_zones(REQ_STOP, zone_mask);
}
