
#include <assert.h>
#include <stdint.h>

#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include "openthread-system.h"

#include "sh_rmt_btn.h"
#include "sh_rmt_led.h"

#define SWITCH_DELAY 500
#define LED_CNT 12
static int m_next_led;

static uint32_t time_now(void)
{
    return otPlatAlarmMilliGetNow();
}

void sh_rmt_btn_evt(sh_rmt_btn_idx_t idx)
{
    // TODO: Process event correctly
    sh_rmt_led_toggle((sh_rmt_led_idx_t)idx);
}

void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}

int main(int argc, char *argv[])
{
    uint32_t last_switch = time_now();
    uint32_t now;

    otInstance *instance;

    otSysInit(argc, argv);

    instance = otInstanceInitSingle();
    assert(instance != NULL);

    sh_rmt_btn_init();
    sh_rmt_led_init();

    while (1)
    {
        now = time_now();

        if (now > last_switch + SWITCH_DELAY)
        {
            last_switch = now;

            sh_rmt_led_toggle((sh_rmt_led_idx_t)m_next_led/2);
            m_next_led = (m_next_led + 1) % (LED_CNT * 2);
        }

        //sh_rmt_btn_process();

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
