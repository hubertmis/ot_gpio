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

static void sd_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info);
static void sh_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info);

static otCoapResource sd_resource = {
        .mUriPath = "sd",
        .mHandler = sd_handler,
        .mContext = NULL,
};

static otCoapResource sh_resource = {
        .mUriPath = "dr1",
        .mHandler = sh_handler,
        .mContext = NULL,
};

static void coap_init(void)
{
    otError error;

    error = otCoapStart(humi_conn_get_instance(), OT_DEFAULT_COAP_PORT);
    assert(error == OT_ERROR_NONE);

    //otCoapSetDefaultHandler(humi_conn_get_instance(), coap_def_handler, NULL);

    error = otCoapAddResource(humi_conn_get_instance(), &sh_resource);
    assert (error == OT_ERROR_NONE);

    error = otCoapAddResource(humi_conn_get_instance(), &sd_resource);
    assert (error == OT_ERROR_NONE);
}

static void sd_response_send(otCoapHeader *req_header, const otMessageInfo *message_info)
{
    otError      error = OT_ERROR_NONE;
    otCoapHeader header;
    otMessage   *response;

    otCoapHeaderInit(&header, OT_COAP_TYPE_NON_CONFIRMABLE, OT_COAP_CODE_CONTENT);
    otCoapHeaderSetToken(&header, otCoapHeaderGetToken(req_header), otCoapHeaderGetTokenLength(req_header));
    otCoapHeaderAppendContentFormatOption(&header, OT_COAP_OPTION_CONTENT_FORMAT_CBOR);
    (void)otCoapHeaderSetPayloadMarker(&header);

    response = otCoapNewMessage(humi_conn_get_instance(), &header, NULL);
    if (response == NULL)
    {
        goto exit;
    }

    // TODO: Create CBOR encoder
    const uint8_t payload[] = {
            0xa2,
              0x63, 'd', 'r', '1',
              0xa1,
                0x64, 't', 'y', 'p', 'e',
                0x65, 's', 'h', 'c', 'n', 't',
              0x63, 'd', 'r', '2',
              0xa1,
                0x64, 't', 'y', 'p', 'e',
                0x65, 's', 'h', 'c', 'n', 't',
    };
    error = otMessageAppend(response, payload, sizeof(payload));
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

    sd_response_send(header, message_info);
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

/**
 * Search for given string in given memory.
 *
 * @param mem       Pointer to haystack. Must be zero-terminated.
 * @param mem_size  Size of the haystack.
 * @param str       Pointer to string (needle) searched in the haystack.
 * @return          If given string was found in the haystack.
 */
static bool is_str_in_mem(const void *mem, size_t mem_size, uint8_t *str)
{
    const void *first_char;
    const void *mem_left = mem;
    size_t      mem_left_size = mem_size;

    while (NULL != (first_char = memchr(mem_left, str[0], mem_left_size)))
    {
        if (NULL != strstr(first_char, str))
        {
            return true;
        }

        mem_left = (const uint8_t *)first_char + 1;
        mem_left_size = mem_size - (mem_left - mem);
    }

    return false;
}

static void sh_handler(void *context, otCoapHeader *header, otMessage *message, const otMessageInfo *message_info)
{
    (void)context;
#define PAYLOAD_SIZE 128
    char payload[PAYLOAD_SIZE];
    uint8_t payload_len;

    if (otCoapHeaderGetCode(header) != OT_COAP_CODE_PUT)
    {
        return;
    }

    if (!humi_conn_is_addr_local(&message_info->mPeerAddr))
    {
        // Allow only local traffic, because there is no encryption at the moment
        return;
    }

    payload_len = otMessageRead(message, 0, payload, sizeof(payload)-1);
    payload[payload_len] = '\0';

    // TODO: parse CBOR and find resource and value
    if (is_str_in_mem(payload, payload_len, "up"))
    {
        sh_cnt_mot_up(0);
    }
    else if (is_str_in_mem(payload, payload_len, "down"))
    {
        sh_cnt_mot_down(0);
    }
    else if (is_str_in_mem(payload, payload_len, "stop"))
    {
        sh_cnt_mot_stop(0);
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

