#include "sh_rmt_btn.h"

#include <assert.h>
#include <stdbool.h>

#include <nrfx_gpiote.h>

#define PIN(PORT, NO) (PORT * 32 + NO)

#define BTN1 PIN(0, 10)
#define BTN2 PIN(0, 9)
#define BTN3 PIN(1, 6)
#define BTN4 PIN(1, 4)
#define BTN5 PIN(1, 2)
#define BTN6 PIN(1, 1)
#define BTN7 PIN(1, 3)
#define BTN8 PIN(1, 0)
#define BTN9 PIN(0, 22)

#define NUM_BTNS 9

static const nrfx_gpiote_pin_t btn_pins[NUM_BTNS] = {BTN1, BTN2, BTN3, BTN4, BTN5, BTN6, BTN7, BTN8, BTN9};

static bool btn_evts[NUM_BTNS];

static void btn_evt_enqueue(sh_rmt_btn_idx_t i)
{
    btn_evts[i] = true;
}

static void btn_evt_dequeue(sh_rmt_btn_idx_t i)
{
    btn_evts[i] = false;
}

static void btn_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)action;

    for (int i = 0; i < sizeof(btn_pins) / sizeof(btn_pins[0]); i++)
    {
        if (btn_pins[i] == pin)
        {
            btn_evt_enqueue(i);
            break;
        }
    }
}

void sh_rmt_btn_init(void)
{
    nrfx_err_t err;

    err = nrfx_gpiote_init();
    assert(err == NRFX_SUCCESS);

    const nrfx_gpiote_in_config_t btn_config =
            {
                .is_watcher = false,
                .hi_accuracy = false,
                .pull = NRF_GPIO_PIN_PULLUP,
                .sense = NRF_GPIOTE_POLARITY_HITOLO,
            };
    
    for (int i = 0; i < sizeof(btn_pins) / sizeof(btn_pins[0]); i++)
    {
        err = nrfx_gpiote_in_init(btn_pins[i], &btn_config, btn_evt_handler);
        assert(err == NRFX_SUCCESS);

        nrfx_gpiote_in_event_enable(btn_pins[i], true);
    }
}

void sh_rmt_btn_process(void)
{
    for (int i = 0; i < sizeof(btn_evts) / sizeof(btn_evts[0]); i++)
    {
        if (btn_evts[i])
        {
            sh_rmt_btn_evt(i);

            btn_evt_dequeue(i);
        }
    }
}
