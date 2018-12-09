
#include <assert.h>
#include <stdint.h>

#include <cmsis_gcc.h>
#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include "openthread-system.h"

#include "sh_cnt_btn.h"
#include "sh_cnt_display.h"
#include "sh_cnt_mot.h"
#include "../../lib/timer/humi_timer.h"

#include "sh_cnt_rly.h"

#define SWITCH_DELAY 500
#define RLY_CNT 4
static int m_next_rly;

static uint32_t time_now(void)
{
    return otPlatAlarmMilliGetNow();
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
void otTaskletsSignalPending(otInstance *aInstance)
{
    (void)aInstance;
}
#pragma clang diagnostic pop

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(int argc, char *argv[])
{
#if 0
    uint32_t last_switch = time_now();
    uint32_t now;
#endif

    otInstance *instance;

    otSysInit(argc, argv);

    instance = otInstanceInitSingle();
    assert(instance != NULL);

    humi_timer_init();

    sh_cnt_btn_init();
    sh_cnt_display_init();
    sh_cnt_mot_init(5000);

    while (1)
    {
        sh_cnt_btn_process();
        humi_timer_process();

#if 0
        now = time_now();

        if (now > last_switch + SWITCH_DELAY)
        {
            last_switch = now;

            sh_cnt_rly_toggle(m_next_rly);
            m_next_rly = (m_next_rly + 1) % RLY_CNT;
        }
#endif

        otTaskletsProcess(instance);
        otSysProcessDrivers(instance);

        //__WFE();
    }
}
#pragma clang diagnostic pop

void sh_cnt_btn_evt(void)
{
    static int i;

#if 0
    switch (i % 4)
    {
        case 0:
            sh_cnt_display_discovery();
            break;

        case 1:
            sh_cnt_display_commissioning();
            break;

        case 2:
            sh_cnt_display_connecting();
            break;

        case 3:
            sh_cnt_display_connected();
            break;
    }
#else
    switch (i % 3)
    {
        case 0:
            sh_cnt_mot_up(0);
            sh_cnt_mot_up(1);
            break;

        case 1:
            sh_cnt_mot_down(0);
            sh_cnt_mot_down(1);
            break;

        case 2:
            sh_cnt_mot_stop(0);
            sh_cnt_mot_stop(1);
            break;

        default:
            assert(false);
            break;
    }
#endif

    i++;
}

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
