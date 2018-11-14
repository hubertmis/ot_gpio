#include "sh_cnt_rly.h"

#include <stdint.h>

#include <nrf_gpio.h>


#define PIN(PORT, NO) (PORT * 32 + NO)

#define M1_DIR PIN(0, 3)
#define M1_SW  PIN(0, 2)
#define M2_SW  PIN(0, 28)
#define M2_DIR PIN(0, 29)

static const uint32_t rly_pins[] = {M1_DIR, M1_SW, M2_SW, M2_DIR};

void sh_cnt_rly_init(void)
{
    for (size_t i = 0; i < sizeof(rly_pins) / sizeof(rly_pins[0]); i++)
    {
        uint32_t pin = rly_pins[i];

        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_set(pin);
    }
}

void sh_cnt_rly_toggle(sh_cnt_rly_idx_t idx)
{
    uint32_t pin = rly_pins[idx];

    nrf_gpio_pin_toggle(pin);
}

