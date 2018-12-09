#include "sh_cnt_rly.h"

#include <stdint.h>

#include <nrf_gpio.h>


#define PIN(PORT, NO) (PORT * 32 + NO)

#define M1_DIR PIN(0, 10)
#define M1_SW  PIN(0, 9)
#define M2_SW  PIN(1, 6)
#define M2_DIR PIN(1, 4)

static const uint32_t rly_pins[] = {M1_DIR, M1_SW, M2_SW, M2_DIR};

void sh_cnt_rly_init(void)
{
    for (size_t i = 0; i < sizeof(rly_pins) / sizeof(rly_pins[0]); i++)
    {
        uint32_t pin = rly_pins[i];

        nrf_gpio_cfg_output(pin);
    }
}

void sh_cnt_rly_on(sh_cnt_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_set(pin);
}

void sh_cnt_rly_off(sh_cnt_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_clear(pin);
}

void sh_cnt_rly_toggle(sh_cnt_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_toggle(pin);
}

