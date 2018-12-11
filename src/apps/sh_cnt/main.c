
#include <assert.h>
#include <stdint.h>

#include <cmsis_gcc.h>
#include <openthread/platform/alarm-milli.h>

#include "sh_cnt_btn.h"
#include "sh_cnt_conn.h"
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
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(int argc, char *argv[])
{
#if 0
    uint32_t last_switch = time_now();
    uint32_t now;
#endif

    humi_timer_init();

    sh_cnt_btn_init();
    sh_cnt_display_init();
    sh_cnt_mot_init(5000);

    sh_cnt_conn_init();

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

        sh_cnt_conn_process();

        if (!humi_timer_is_pending() && !sh_cnt_conn_is_pending())
        {
            __WFE();
        }
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

