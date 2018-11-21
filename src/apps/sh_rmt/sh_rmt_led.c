#include "sh_rmt_led.h"

#include <stdint.h>

#include <nrf_gpio.h>


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

void sh_rmt_led_init(void)
{
    for (size_t i = 0; i < sizeof(led_pins) / sizeof(led_pins[0]); i++)
    {
        uint32_t pin = led_pins[i];

        nrf_gpio_cfg_output(pin);
        nrf_gpio_pin_clear(pin);
    }
}

void sh_rmt_led_toggle(sh_rmt_led_idx_t idx)
{
    uint32_t pin = led_pins[idx];

    nrf_gpio_pin_toggle(pin);
}

