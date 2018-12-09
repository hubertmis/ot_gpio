//
// Created by bearh on 24.11.18.
//

#include "humi_timer.h"

#include <stdbool.h>
#include <stddef.h>
#include <nrfx_rtc.h>

#define RTC_FREQ 2048UL
#define BTN_DEBOUNCING_TIME 50000UL
#define RTC_IMM_TICKS 3UL

#define RTC_CH_LED 0
#define RTC_CH_BTN 1
#define RTC_CH_GEN 2

#define NRFX_RTC_INT_COMPARE(id) NRFX_CONCAT_2(NRFX_RTC_INT_COMPARE, id)

static const nrfx_rtc_t nrfx_rtc_instance = NRFX_RTC_INSTANCE(0);

static humi_timer_t *head;
static volatile bool  gen_timer_pending;

static void gen_fired(void);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
    switch (int_type)
    {
        case NRFX_RTC_INT_COMPARE(RTC_CH_LED):
        {
            humi_timer_led_fired();
            break;
        }

        case NRFX_RTC_INT_COMPARE(RTC_CH_BTN):
        {
            humi_timer_btn_fired();
            break;
        }

        case NRFX_RTC_INT_COMPARE(RTC_CH_GEN):
        {
            gen_timer_pending = true;
            break;
        }

        default:
            assert(false);
    }

}

void humi_timer_init(void)
{
    nrfx_err_t err;

    head = NULL;
    gen_timer_pending = false;

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

void humi_timer_process(void)
{
    if (gen_timer_pending)
    {
        gen_timer_pending = false;
        gen_fired();
    }
}

bool humi_timer_is_pending(void)
{
    return gen_timer_pending;
}

void humi_timer_led_start(void)

{
    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + RTC_IMM_TICKS) %
            nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, RTC_CH_LED, next_ctr, true);
}

void humi_timer_led_stop(void)
{
    nrfx_rtc_cc_disable(&nrfx_rtc_instance, RTC_CH_LED);
}

void humi_timer_btn_start(void)
{
    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + (BTN_DEBOUNCING_TIME / RTC_FREQ)) %
            nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, RTC_CH_BTN, next_ctr, true);
}

static bool is_earlier_than(uint32_t this, uint32_t that)
{
    uint32_t max_ticks = nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    assert(this < max_ticks);
    assert(that < max_ticks);

    uint32_t diff = that - this;
    return diff < (max_ticks / 2);
}

static void set_gen_timer(uint32_t target_ticks)
{
    uint32_t max_ticks        = nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    uint32_t curr_ticks       = nrfx_rtc_counter_get(&nrfx_rtc_instance);
    uint32_t min_target_ticks = (curr_ticks + RTC_IMM_TICKS) % max_ticks;

    bool immediately = false;

    if (min_target_ticks < max_ticks / 2)
    {
        uint32_t latest_future = min_target_ticks + max_ticks / 2;

        if ((target_ticks >= latest_future) || (target_ticks <= min_target_ticks))
        {
            immediately = true;
        }
    }
    else
    {
        uint32_t earliest_past = min_target_ticks - max_ticks / 2;

        if ((target_ticks >= earliest_past) && (target_ticks <= min_target_ticks))
        {
            immediately = true;
        }
    }

    if (immediately)
    {
        nrfx_rtc_cc_disable(&nrfx_rtc_instance, RTC_CH_GEN);
        gen_timer_pending = true;
    }
    else
    {
        nrfx_rtc_cc_set(&nrfx_rtc_instance, RTC_CH_GEN, target_ticks, true);
    }
}

static void reset_gen_timer(void)
{
    nrfx_rtc_cc_disable(&nrfx_rtc_instance, RTC_CH_GEN);
}

void gen_fired(void) {
    assert(head != NULL);

    humi_timer_t *timer = head;
    head = timer->next;

    if (head != NULL)
    {
        set_gen_timer(head->target_time);
    }
    else
    {
        reset_gen_timer();
    }

    timer->callback(timer->context);
}

uint32_t humi_timer_get_time(void) {
    uint32_t result = nrfx_rtc_counter_get(&nrfx_rtc_instance);
    return result;
}

uint32_t humi_timer_get_target(uint32_t t0, uint32_t dt)
{
    assert(dt <= (UINT32_MAX / RTC_FREQ));

    uint32_t dt_ticks = dt * RTC_FREQ / 1000;
    assert(dt_ticks < (nrfx_rtc_max_ticks_get(&nrfx_rtc_instance) / 2));

    return (t0 + dt_ticks) % nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
}

uint32_t humi_timer_get_target_from_delay(uint32_t delay)
{
    return humi_timer_get_target(humi_timer_get_time(), delay);
}

void humi_timer_gen_remove(humi_timer_t *timer)
{
    humi_timer_t *prev;
    humi_timer_t *cur;

    if (head == NULL)
    {
        return;
    }

    if (head == timer)
    {
        head = timer->next;

        if (head == NULL)
        {
            reset_gen_timer();
        }
        else
        {
            set_gen_timer(head->target_time);
        }
    }
    else
    {
        for (prev = head, cur = head->next; cur != NULL; prev = cur, cur = cur->next)
        {
            if (cur == timer)
            {
                prev->next = cur->next;
                break;
            }
        }
    }
}

void humi_timer_gen_add(humi_timer_t *timer)
{
    humi_timer_t *prev;
    humi_timer_t *cur;

    humi_timer_gen_remove(timer);

    if (head == NULL)
    {
        timer->next = NULL;
        head        = timer;

        set_gen_timer(timer->target_time);
    }
    else if (is_earlier_than(timer->target_time, head->target_time))
    {
        timer->next = head;
        head        = timer;

        set_gen_timer(timer->target_time);
    }
    else
    {
        for (prev = head, cur = head->next; cur != NULL; prev = cur, cur = cur->next)
        {
            if (is_earlier_than(timer->target_time, cur->target_time))
            {
                break;
            }
        }

        prev->next  = timer;
        timer->next = cur;
    }
}

void __attribute__((weak)) humi_timer_led_fired(void)
{
    // Intentionally empty
}

void __attribute__((weak)) humi_timer_btn_fired(void)
{
    // Intentionally empty
}
