//
// Created by bearh on 08.12.18.
//

#include "humi_led.h"

#include <stdint.h>

#include <nrf_gpio.h>
#include "../timer/humi_timer.h"

static const humi_led_t *led_pins;
static int               num_leds;

static uint32_t    led_bitmap;

void humi_timer_led_fired(void)
{
    static uint32_t m_last_led;

    if (led_bitmap)
    {
        humi_timer_led_start();
    }
    else
    {
        humi_timer_led_stop();
    }

    nrf_gpio_pin_clear(led_pins[m_last_led]);

    m_last_led = (m_last_led + 1) % num_leds;

    if (led_bitmap & (1UL << m_last_led))
    {
        nrf_gpio_pin_set(led_pins[m_last_led]);
    }
}

void humi_led_init(const humi_led_t *leds, int num_of_leds)
{
    led_pins = leds;
    num_leds = num_of_leds;

    for (int i = 0; i < num_leds; i++)
    {
        uint32_t pin = led_pins[i];

        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_clear(pin);
    }
}

void humi_led_on(humi_led_idx_t idx)
{
    led_bitmap |= (1UL << idx);

    humi_timer_led_start();
}

void humi_led_off(humi_led_idx_t idx)
{
    led_bitmap &= (~(1UL << idx));

    humi_timer_led_start();
}

void humi_led_toggle(humi_led_idx_t idx)
{
    led_bitmap ^= (1UL << idx);

    humi_timer_led_start();
}


