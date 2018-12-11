
#include <assert.h>
#include <stdint.h>

#include <cmsis_gcc.h>
#include <openthread/tasklet.h>
#include <openthread/platform/alarm-milli.h>
#include "openthread-system.h"

#include "sh_rmt_btn.h"
#include "sh_rmt_conn.h"
#include "sh_rmt_led.h"
#include "../../lib/timer/humi_timer.h"

#define SWITCH_DELAY 500
#define LED_CNT 12
static int m_next_led;

static uint32_t time_now(void)
{
    return otPlatAlarmMilliGetNow();
}

int main(int argc, char *argv[])
{
    uint32_t last_switch = time_now();
    uint32_t now;

    humi_timer_init();
    sh_rmt_btn_init();
    sh_rmt_led_init();

    sh_rmt_conn_init();

    while (1)
    {
#if 0
        now = time_now();

        if (now > last_switch + SWITCH_DELAY)
        {
            last_switch = now;

            sh_rmt_led_toggle((sh_rmt_led_idx_t)m_next_led/2);
            m_next_led = (m_next_led + 1) % (LED_CNT * 2);
        }
#endif

        humi_timer_process();
        sh_rmt_btn_process();

        sh_rmt_conn_process();

        if (!humi_timer_is_pending() && !sh_rmt_conn_is_pending())
        {
            __WFE();
        }
    }

    return 0;
}
