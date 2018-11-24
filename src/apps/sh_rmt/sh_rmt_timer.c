//
// Created by bearh on 24.11.18.
//

#include "sh_rmt_timer.h"

#include <nrfx_rtc.h>

#define RTC_FREQ 2000UL
#define BTN_DEBOUNCING_TIME 50000UL

#define RTC_CH_LED 0
#define RTC_CH_BTN 1

#define NRFX_RTC_INT_COMPARE(id) NRFX_CONCAT_2(NRFX_RTC_INT_COMPARE, id)

static const nrfx_rtc_t nrfx_rtc_instance = NRFX_RTC_INSTANCE(0);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
    switch (int_type)
    {
        case NRFX_RTC_INT_COMPARE(RTC_CH_LED):
        {
            sh_rmt_timer_led_fired();
            break;
        }

        case NRFX_RTC_INT_COMPARE(RTC_CH_BTN):
        {
            sh_rmt_timer_btn_fired();
            break;
        }

        default:
            assert(false);
    }

}

void sh_rmt_timer_init(void)
{
    nrfx_err_t err;

    nrfx_rtc_config_t config =
            {
                    .prescaler          = RTC_FREQ_TO_PRESCALER(RTC_FREQ),
                    .interrupt_priority = 6,
                    .reliable           = false,
                    .tick_latency       = NRFX_RTC_US_TO_TICKS(5, RTC_FREQ),
            };

    err = nrfx_rtc_init(&nrfx_rtc_instance, &config, rtc_handler);
    assert(err == NRFX_SUCCESS);

    nrfx_rtc_enable(&nrfx_rtc_instance);
}

void sh_rmt_timer_led_start(void)

{
    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + 3) % nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, RTC_CH_LED, next_ctr, true);
}

void sh_rmt_timer_led_stop(void)
{
    nrfx_rtc_cc_disable(&nrfx_rtc_instance, RTC_CH_LED);
}

void sh_rmt_timer_btn_start(void)
{
    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + (BTN_DEBOUNCING_TIME / RTC_FREQ)) %
            nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, RTC_CH_BTN, next_ctr, true);
}

