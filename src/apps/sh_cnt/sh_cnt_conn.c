//
// Created by bearh on 09.12.18.
//

#include "sh_cnt_conn.h"

#include <assert.h>
#include <string.h>

#include <openthread/coap.h>
#include <openthread/thread.h>

#include "sh_cnt_display.h"
#include "sh_cnt_mot.h"
#include "../../lib/conn/humi_conn.h"
#include "../../lib/mcbor/mcbor_dec.h"
#include "../../lib/mcbor/mcbor_enc.h"
#include "../../lib/timer/humi_timer.h"

#define SERVICE_TYPE_KEY "type"
#define SERVICE_TYPE     "shcnt"
#define SD_PAYLOAD_MAX_SIZE 64
#define SD_MIN_DELAY        500
#define SD_MAX_DELAY        1500

#define SH_PAYLOAD_MAX_SIZE 128
#define SH_VALUE_KEY  "val"
#define SH_VALUE_UP   "up"
#define SH_VALUE_DOWN "down"
#define SH_VALUE_STOP "stop"

static void sd_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info);
static void sh_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info);

static otCoapResource sd_resource = {
        .mUriPath = "sd",
        .mHandler = sd_handler,
        .mContext = NULL,
};

static otCoapResource sh_resources[] = {
        {
                .mUriPath = "dr1",
                .mHandler = sh_handler,
                .mContext = (void *)0,
        },
        {
                .mUriPath = "dr2",
                .mHandler = sh_handler,
                .mContext = (void *)1,
        },
};

static struct {
    otCoapHeader  req_header;
    otMessageInfo message_info;
} sd_rsp_data;
static humi_timer_t sd_timer;

static void coap_init(void)
{
    otError error;

    error = otCoapStart(humi_conn_get_instance(), OT_DEFAULT_COAP_PORT);
    assert(error == OT_ERROR_NONE);

    //otCoapSetDefaultHandler(humi_conn_get_instance(), coap_def_handler, NULL);

    for (int i = 0; i < sizeof(sh_resources) / sizeof(sh_resources[0]); i++)
    {
        error = otCoapAddResource(humi_conn_get_instance(), &sh_resources[i]);
        assert (error == OT_ERROR_NONE);
    }

    error = otCoapAddResource(humi_conn_get_instance(), &sd_resource);
    assert (error == OT_ERROR_NONE);
}

static size_t create_sd_response_payload(uint8_t *buffer, size_t buffer_size)
{
    int num_services = sizeof(sh_resources) / sizeof(sh_resources[0]);

    mcbor_enc_t cbor;
    mcbor_enc_init(buffer, buffer_size, &cbor);

    if (mcbor_enc_map(&cbor, num_services) != MCBOR_ERR_SUCCESS) return 0;

    for (int i = 0; i < num_services; i++)
    {
        // Key (service name)
        if (mcbor_enc_text(&cbor, sh_resources[i].mUriPath) != MCBOR_ERR_SUCCESS) return 0;

        // Service description
        if (mcbor_enc_map(&cbor, 1) != MCBOR_ERR_SUCCESS) return 0;
        if (mcbor_enc_text(&cbor, SERVICE_TYPE_KEY) != MCBOR_ERR_SUCCESS) return 0;
        if (mcbor_enc_text(&cbor, SERVICE_TYPE) != MCBOR_ERR_SUCCESS) return 0;
    }

    return mcbor_get_size(&cbor);
}

static void sd_response_send(void *context)
{
    (void)context;
    otError              error        = OT_ERROR_NONE;
    otCoapHeader        *req_header   = &sd_rsp_data.req_header;
    const otMessageInfo *message_info = &sd_rsp_data.message_info;
    otCoapHeader         header;
    otMessage           *response;

    uint8_t payload[SD_PAYLOAD_MAX_SIZE];
    size_t  payload_size;

    otCoapHeaderInit(&header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);
    otCoapHeaderSetToken(&header, otCoapHeaderGetToken(req_header), otCoapHeaderGetTokenLength(req_header));
    otCoapHeaderAppendContentFormatOption(&header, OT_COAP_OPTION_CONTENT_FORMAT_CBOR);
    (void)otCoapHeaderSetPayloadMarker(&header);

    response = otCoapNewMessage(humi_conn_get_instance(), &header, NULL);
    if (response == NULL)
    {
        goto exit;
    }

    payload_size = create_sd_response_payload(payload, sizeof(payload));
    assert(payload_size > 0);

    error = otMessageAppend(response, payload, payload_size);
    if (error != OT_ERROR_NONE)
    {
        goto exit;
    }

    error = otCoapSendResponse(humi_conn_get_instance(), response, message_info);

exit:
    if (error != OT_ERROR_NONE && response != NULL)
    {
        otMessageFree(response);
    }
}

static void sd_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info)
{
    (void)context;
    (void)message;

    if (otCoapHeaderGetType(header) != OT_COAP_TYPE_NON_CONFIRMABLE)
    {
        return;
    }

    if (otCoapHeaderGetCode(header) != OT_COAP_CODE_GET)
    {
        return;
    }

    if (!humi_conn_is_addr_local(&message_info->mPeerAddr))
    {
        // Allow only local traffic, because there is no encryption at the moment
        return;
    }

    // TODO: Filter traffic using payload

    int delay = SD_MIN_DELAY + (rand() % (SD_MAX_DELAY - SD_MIN_DELAY));
    sd_rsp_data.req_header   = *header;
    sd_rsp_data.message_info = *message_info;
    sd_timer.target_time     = humi_timer_get_target_from_delay(delay);
    sd_timer.callback        = sd_response_send;
    sd_timer.context         = NULL;
    humi_timer_gen_add(&sd_timer);
}

static bool is_cbor_text_equal_to_str(const uint8_t *cbor_text, size_t cbor_text_len, char *str)
{
    return mcbor_dec_is_text_equal_to_str(cbor_text, cbor_text_len, str);
}

static void sh_response_send(otCoapHeader *req_header, const otMessageInfo *message_info)
{
    otError      error = OT_ERROR_NONE;
    otCoapHeader header;
    otMessage   *response;

    otCoapHeaderInit(&header, OT_COAP_TYPE_ACKNOWLEDGMENT, OT_COAP_CODE_CHANGED);
    otCoapHeaderSetMessageId(&header, otCoapHeaderGetMessageId(req_header));
    otCoapHeaderSetToken(&header, otCoapHeaderGetToken(req_header), otCoapHeaderGetTokenLength(req_header));

    response = otCoapNewMessage(humi_conn_get_instance(), &header, NULL);
    if (response == NULL)
    {
        goto exit;
    }

    error = otCoapSendResponse(humi_conn_get_instance(), response, message_info);

exit:
    if (error != OT_ERROR_NONE && response != NULL)
    {
        otMessageFree(response);
    }
}

static mcbor_err_t process_and_skip_resource(int i, const mcbor_dec_t *mcbor_dec, const void *resource_map, int pairs)
{
    const void *item = resource_map;
    mcbor_err_t err;

    for (int j = 0; j < pairs; j++)
    {
        const char *key;
        size_t      key_len;

        const char *value;
        size_t      value_len;

        err = mcbor_dec_get_text(mcbor_dec, item, &key, &key_len);

        if ((err == MCBOR_ERR_NOT_FOUND) || !is_cbor_text_equal_to_str(key, key_len, SH_VALUE_KEY))
        {
            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip key
            if (err != MCBOR_ERR_SUCCESS) return err;
            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip value
            if (err != MCBOR_ERR_SUCCESS) return err;
        }
        else
        {
            if (err != MCBOR_ERR_SUCCESS) return err;

            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip key
            if (err != MCBOR_ERR_SUCCESS) return err;

            err = mcbor_dec_get_text(mcbor_dec, item, &value, &value_len);
            if (err != MCBOR_ERR_SUCCESS) return err;

            // Process type value
            if (is_cbor_text_equal_to_str(value, value_len, SH_VALUE_UP))
            {
                sh_cnt_mot_up(i);
            }
            else if (is_cbor_text_equal_to_str(value, value_len, SH_VALUE_DOWN))
            {
                sh_cnt_mot_down(i);
            }
            else if (is_cbor_text_equal_to_str(value, value_len, SH_VALUE_STOP))
            {
                sh_cnt_mot_stop(i);
            }

            err = mcbor_dec_skip_item(mcbor_dec, &item); // Skip value
            if (err != MCBOR_ERR_SUCCESS) return err;
        }
    }
}

static void sh_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info)
{
    char payload[SH_PAYLOAD_MAX_SIZE];
    int payload_len;

    int resource = (int)context;

    if (otCoapHeaderGetCode(header) != OT_COAP_CODE_PUT)
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

    mcbor_dec_init(payload, payload_len, &mcbor_dec);
    mcbor_dec_iter_init(&mcbor_dec, &iter);

    for (mcbor_err_t err = mcbor_dec_iter_map(&mcbor_dec, &iter, &top_map_item, &pairs);
         err == MCBOR_ERR_SUCCESS;
         err = mcbor_dec_iter_map(&mcbor_dec, &iter, &top_map_item, &pairs))
    {
        mcbor_err_t mcbor_err;
        mcbor_err = process_and_skip_resource(resource, &mcbor_dec, top_map_item, pairs);
        if (mcbor_err != MCBOR_ERR_SUCCESS) break;
    }

    sh_response_send(header, message_info);
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
                sh_cnt_display_connecting();
                break;

            case OT_DEVICE_ROLE_CHILD:
            case OT_DEVICE_ROLE_ROUTER:
            case OT_DEVICE_ROLE_LEADER:
                sh_cnt_display_connected();
                break;
        }
    }
}


void sh_cnt_conn_init(void)
{
    otError error;

    humi_conn_init(false);

    error = otSetStateChangedCallback(humi_conn_get_instance(), ot_state_changed, NULL);
    assert(error == OT_ERROR_NONE);

    coap_init();

    humi_conn_start();
}

void sh_cnt_conn_process(void)
{
    humi_conn_process();
}

bool sh_cnt_conn_is_pending(void)
{
    return humi_conn_is_pending();
}

