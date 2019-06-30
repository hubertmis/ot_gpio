//
// Created by bearh on 09.12.18.
//

#include "humi_conn.h"

#include <assert.h>
#include <string.h>

#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/joiner.h>
#include <openthread/link.h>
#include <openthread/thread.h>
#include <openthread/tasklet.h>
#include "openthread-system.h"

#define TX_POWER 8

#define MULTICAST_PREFIX 0xff
#define MULTICAST_SCOPE_MASK 0x0f
#define MESH_LOCAL_SCOPE 3

#define PP_DEFAULT 60000
#define PP_FAST    500

#ifndef VENDOR_NAME
#define VENDOR_NAME "humi"
#endif

#ifndef VENDOR_MODEL
#define VENDOR_MODEL "ot_gpio"
#endif

#ifndef SW_VERSION
#define SW_VERSION "0.0.1"
#endif

#ifndef VENDOR_DATA
#define VENDOR_DATA NULL
#endif


static otInstance *ot_instance;
static int fast_pp_req;

static void join_callback(otError error, void *context)
{
    (void)context;

    if (error == OT_ERROR_NONE)
    {
        humi_conn_start();
    }
    else
    {
        // TODO: Retry join? After some time? Or report to higher layer?
        humi_conn_join();
    }
}

void humi_conn_init(bool sed) {
    otError error;
    struct otIp6Address site_local_all_nodes_addr = {.mFields = {.m8 = 
        {0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01} }};

    otSysInit(0, NULL);

    ot_instance = otInstanceInitSingle();
    assert(ot_instance != NULL);

    error = otPlatRadioSetTransmitPower(ot_instance, TX_POWER);
    assert(error == OT_ERROR_NONE);

#ifndef PSKD
    error = otLinkSetPanId(ot_instance, 0x7277);
    assert(error == OT_ERROR_NONE);
    error = otLinkSetChannel(ot_instance, 19);
    assert(error == OT_ERROR_NONE);
#endif // !PSKD

    otLinkModeConfig link_mode_config;
    memset(&link_mode_config, 0, sizeof(link_mode_config));

    if (sed) {
        link_mode_config.mRxOnWhenIdle       = false;
        link_mode_config.mSecureDataRequests = true;
        link_mode_config.mDeviceType         = false;
        link_mode_config.mNetworkData        = false;

        otLinkSetPollPeriod(humi_conn_get_instance(), PP_DEFAULT);
    } else {
        link_mode_config.mRxOnWhenIdle       = true;
        link_mode_config.mSecureDataRequests = true;
        link_mode_config.mDeviceType         = true;
        link_mode_config.mNetworkData        = true;
    }

    error = otThreadSetLinkMode(ot_instance, link_mode_config);
    assert(error == OT_ERROR_NONE);

    error = otIp6SubscribeMulticastAddress(ot_instance, &site_local_all_nodes_addr);
    assert(error == OT_ERROR_NONE);
}

void humi_conn_process(void)
{
    otTaskletsProcess(ot_instance);
    otSysProcessDrivers(ot_instance);
}

bool humi_conn_is_pending(void)
{
    return otTaskletsArePending(ot_instance);
}

bool humi_conn_is_commissioned(void)
{
    // TODO: Verify if this commissioning verification method works as expected
    otPanId pan_id = otLinkGetPanId(ot_instance);
    return pan_id != 0xffff;
}

void humi_conn_start(void)
{
    otError error;

    if (humi_conn_is_commissioned())
    {
        error = otIp6SetEnabled(ot_instance, true);
        assert(error == OT_ERROR_NONE);

        error = otThreadSetEnabled(ot_instance, true);
        assert(error == OT_ERROR_NONE);
    }
    else
    {
        humi_conn_join();
    }
}

void humi_conn_stop(void)
{
    otError error;

    error = otThreadSetEnabled(ot_instance, false);
    assert(error == OT_ERROR_NONE);

    error = otIp6SetEnabled(ot_instance, false);
    assert(error == OT_ERROR_NONE);
}

void humi_conn_join(void)
{
#ifdef PSKD
    otError error;

    error = otIp6SetEnabled(ot_instance, true);
    assert(error == OT_ERROR_NONE);

    error = otJoinerStart(ot_instance, PSKD, NULL, VENDOR_NAME, VENDOR_MODEL, SW_VERSION, VENDOR_DATA, join_callback, NULL);
    assert(error == OT_ERROR_NONE);
#else
    assert(false);
#endif //PSKD
}

void humi_conn_decommission(void)
{
#ifdef PSKD
    otInstanceFactoryReset(ot_instance);
#endif //PSKD
}

otInstance *humi_conn_get_instance(void)
{
    return ot_instance;
}

bool humi_conn_is_addr_local(const otIp6Address *addr)
{
    const otMeshLocalPrefix *ml_prefix;

    if ((addr->mFields.m8[0] == MULTICAST_PREFIX) && ((addr->mFields.m8[1] & MULTICAST_SCOPE_MASK) <= MESH_LOCAL_SCOPE))
    {
        // Local multicast address
        return true;
    }

    ml_prefix = otThreadGetMeshLocalPrefix(ot_instance);

    if (0 == memcmp(addr, ml_prefix, sizeof(*ml_prefix)))
    {
        // Mesh local unicast address
        return true;
    }

    return false;
}

void humi_conn_fast_poll_period(void)
{
    if (fast_pp_req++ == 0)
    {
        otLinkSetPollPeriod(humi_conn_get_instance(), PP_FAST);
    }

    assert(fast_pp_req >= 0);
}

void humi_conn_normal_poll_period(void)
{
    if (--fast_pp_req == 0)
    {
        otLinkSetPollPeriod(humi_conn_get_instance(), PP_DEFAULT);
    }

    assert(fast_pp_req >= 0);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}
#pragma clang diagnostic pop

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);
}
#pragma clang diagnostic pop
#endif

