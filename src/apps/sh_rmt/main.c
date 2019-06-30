
#include <assert.h>
#include <stdint.h>

#include <cmsis_gcc.h>

#include "sh_rmt_btn.h"
#include "sh_rmt_conn.h"
#include "sh_rmt_led.h"
#include "../../lib/timer/humi_timer.h"

#define SWITCH_DELAY 500
#define LED_CNT 12
static int m_next_led;

int main(int argc, char *argv[])
{
    humi_timer_init();
    sh_rmt_btn_init();
    sh_rmt_led_init();

    sh_rmt_conn_init();

    while (1)
    {
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
