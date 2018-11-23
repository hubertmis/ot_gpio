#include "sh_rmt_led.h"

#include <assert.h>
#include <stdint.h>

#include <nrf_gpio.h>
#include <nrfx_rtc.h>


#define PIN(PORT, NO) (PORT * 32 + NO)

#define LED1  PIN(1, 11)
#define LED2  PIN(1, 10)
#define LED3  PIN(1, 13)
#define LED4  PIN(1, 15)
#define LED5  PIN(0, 3)
#define LED6  PIN(0, 2)
#define LED7  PIN(0, 28)
#define LED8  PIN(0, 29)
#define LED9  PIN(0, 30)
#define LED10 PIN(0, 31)
#define LED11 PIN(0, 4)
#define LED12 PIN(0, 5)

static const uint32_t led_pins[] = {LED1, LED2, LED3, LED4, LED5, LED6, LED7, LED8, LED9, LED10, LED11, LED12};
static uint32_t led_bitmap;


#define RTC_FREQ 2000UL
static const nrfx_rtc_t nrfx_rtc_instance = NRFX_RTC_INSTANCE(0);

static void rtc_handler(nrfx_rtc_int_type_t int_type)
{
    static uint32_t m_last_led;

    assert(int_type == NRFX_RTC_INT_COMPARE0);

    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + 3) % nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, 0, next_ctr, led_bitmap ? true : false);

    nrf_gpio_pin_clear(led_pins[m_last_led]);

    m_last_led = (m_last_led + 1) % (sizeof(led_pins) / sizeof(led_pins[0]));

    if (led_bitmap & (1UL << m_last_led))
    {
        nrf_gpio_pin_set(led_pins[m_last_led]);
    }

    if (!led_bitmap)
    {
        nrfx_rtc_cc_disable(&nrfx_rtc_instance, 0);
        m_last_led = 0;
    }
}

static void rtc_init(void)
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

void sh_rmt_led_init(void)
{
    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); i++)
    {
        uint32_t pin = led_pins[i];

        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_clear(pin);
    }

    rtc_init();
}

void sh_rmt_led_toggle(sh_rmt_led_idx_t idx)
{
    led_bitmap ^= (1UL << idx);

    uint32_t next_ctr = (nrfx_rtc_counter_get(&nrfx_rtc_instance) + 4) % nrfx_rtc_max_ticks_get(&nrfx_rtc_instance);
    nrfx_rtc_cc_set(&nrfx_rtc_instance, 0, next_ctr, true);
}

