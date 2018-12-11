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
#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 7

static const char * const zone_names[NUM_ZONES] = {
        "dr1",
        "dr2",
        "dr3",
        "k2",
        "br",
        "o",
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

static void sd_response_handler(void                *context,
                                otCoapHeader        *header,
                                otMessage           *message,
                                const otMessageInfo *message_info,
                                otError              result)
{
    (void)context;

    pp_default();

    if (result != OT_ERROR_NONE)
    {
        return;
    }

    // TODO: Verify message payload
    (void)header;
    (void)message;

    // TODO: Fill appropriate addresses based on payload
    zone_addresses[0] = message_info->mPeerAddr;
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

    pp_default();

#if 0
    if (result != OT_ERROR_NONE)
    {
        return;
    }

    // TODO: Verify message payload
    (void)header;
    (void)message;

    // TODO: Fill appropriate addresses based on payload
    zone_addresses[0] = message_info->mPeerAddr;
#endif
}


static void req_zone(req_t req, int zone)
{
    // TODO: Get dst name and address
    (void)zone;
    const uint8_t *zone_name = zone_names[0];
    otIp6Address  *zone_addr = &zone_addresses[0];

    otError       error = OT_ERROR_NONE;
    otCoapHeader  header;
    otMessage   * request;
    otMessageInfo message_info;

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

    // TODO: Create CBOR encoder
    const uint8_t payload[] = {
            0xa1,
            0x63, 'v', 'a', 'l',
    };

    error = otMessageAppend(request, payload, sizeof(payload));
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }

    const uint8_t *value;
    uint8_t        value_len;
    const uint8_t  value_up[] = {0x62, 'u', 'p'};
    const uint8_t  value_down[] = {0x64, 'd', 'o', 'w', 'n'};
    const uint8_t  value_stop[] = {0x64, 's', 't', 'o', 'p'};

    switch (req)
    {
        case REQ_UP:
            value     = value_up;
            value_len = sizeof(value_up);
            break;

        case REQ_DOWN:
            value     = value_down;
            value_len = sizeof(value_down);
            break;

        case REQ_STOP:
            value     = value_stop;
            value_len = sizeof(value_stop);
            break;
    }

    error = otMessageAppend(request, value, value_len);
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }

    memset(&message_info, 0, sizeof(message_info));
    message_info.mInterfaceId = OT_NETIF_INTERFACE_ID_THREAD;
    message_info.mPeerPort    = OT_DEFAULT_COAP_PORT;
    message_info.mPeerAddr    = *zone_addr;

    error = otCoapSendRequest(humi_conn_get_instance(), request, &message_info, zone_response_handler, NULL);

    pp_fast();

    exit:
    if ((error != OT_ERROR_NONE) && (request != NULL))
    {
        otMessageFree(request);
    }
}

static void req_zones(req_t req, uint32_t zone_mask)
{
    // TODO: Iterate over zones
    (void)zone_mask;
    req_zone(req, 0);
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
