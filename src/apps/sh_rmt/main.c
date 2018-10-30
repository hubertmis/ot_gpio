
#include <assert.h>
#include <stdint.h>

#include <openthread/tasklet.h>
#include "openthread-system.h"

#include "sh_rmt_btn.h"
#include "sh_rmt_led.h"

void sh_rmt_btn_evt(sh_rmt_btn_idx_t idx)
{
    (void)idx;
    // TODO: process button event
    sh_rmt_led_toggle((sh_rmt_led_idx_t)idx);
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

int main(int argc, char *argv[])
{
    otInstance *instance;

    otSysInit(argc, argv);

    instance = otInstanceInitSingle();
    assert(instance != NULL);

    sh_rmt_btn_init();
    sh_rmt_led_init();

    while (1)
    {
        sh_rmt_btn_process();
        otTaskletsProcess(instance);
        otSysProcessDrivers(instance);
    }

    return 0;
}

/*
 * Provide, if required an "otPlatLog()" function
 */
#if OPENTHREAD_CONFIG_LOG_OUTPUT == OPENTHREAD_CONFIG_LOG_OUTPUT_APP
void otPlatLog(otLogLevel aLogLevel, otLogRegion aLogRegion, const char *aFormat, ...)
{
    OT_UNUSED_VARIABLE(aLogLevel);
    OT_UNUSED_VARIABLE(aLogRegion);
    OT_UNUSED_VARIABLE(aFormat);
}
#endif
