//
// Created by bearh on 09.12.18.
//

#include "humi_conn.h"

#include <assert.h>
#include <string.h>

#include <openthread/instance.h>
#include <openthread/ip6.h>
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


static otInstance *ot_instance;
static int fast_pp_req;

void humi_conn_init(bool sed) {
    otError error;

    otSysInit(0, NULL);

    ot_instance = otInstanceInitSingle();
    assert(ot_instance != NULL);

    error = otPlatRadioSetTransmitPower(ot_instance, TX_POWER);
    assert(error == OT_ERROR_NONE);

    // TODO: Get Pan Id and channel via commissioning
    error = otLinkSetPanId(ot_instance, 0x7277);
    assert(error == OT_ERROR_NONE);
    error = otLinkSetChannel(ot_instance, 19);
    assert(error == OT_ERROR_NONE);

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

void humi_conn_start(void)
{
    otError error;

    error = otIp6SetEnabled(ot_instance, true);
    assert(error == OT_ERROR_NONE);

    error = otThreadSetEnabled(ot_instance, true);
    assert(error == OT_ERROR_NONE);
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

