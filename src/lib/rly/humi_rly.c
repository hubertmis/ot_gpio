//
// Created by bearh on 29.06.19.
//

#include "humi_rly.h"

#include <nrf_gpio.h>

static const humi_rly_t *rly_pins;
static int               num_rlys;

void humi_rly_init(const humi_rly_t *rlys, int num_of_rlys)
{
    rly_pins = rlys;
    num_rlys = num_of_rlys;

    for (int i = 0; i < num_rlys; i++)
    {
        uint32_t pin = rly_pins[i];

        nrf_gpio_cfg_output(pin);
    }
}

void humi_rly_on(humi_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_set(pin);
}

void humi_rly_off(humi_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_clear(pin);
}

void humi_rly_toggle(humi_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_toggle(pin);
}
