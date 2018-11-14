
#include <assert.h>
#include <stdint.h>

#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include "openthread-system.h"

#include "sh_cnt_rly.h"

#define SWITCH_DELAY 500
#define RLY_CNT 4
static int m_next_rly;

static uint32_t time_now(void)
{
    return otPlatAlarmMilliGetNow();
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

    sh_cnt_rly_init();

    while (1)
    {
        now = time_now();

        if (now > last_switch + SWITCH_DELAY)
        {
            last_switch = now;

            sh_cnt_rly_toggle(m_next_rly);
            m_next_rly = (m_next_rly + 1) % RLY_CNT;
        }

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
