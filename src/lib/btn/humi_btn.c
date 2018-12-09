//
// Created by bearh on 08.12.18.
//

#include "humi_btn.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <nrfx_gpiote.h>

#include "../timer/humi_timer.h"

static const humi_btn_t *btn_pins;
static int               num_btn_pins;

static uint32_t          btn_evts;
static volatile bool     ignore_evts;

static void btn_evt_enqueue(humi_btn_idx_t i)
{
    btn_evts |= (1 << i);
}

static void btn_evt_dequeue(humi_btn_idx_t i)
{
    btn_evts &= ~(1 << i);
}

static void btn_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)action;

    if (ignore_evts)
    {
        return;
    }

    if (nrfx_gpiote_in_is_set(pin))
    {
        ignore_evts = true;
        humi_timer_btn_start();
        return;
    }

    for (int i = 0; i < num_btn_pins; i++)
    {
        if (btn_pins[i] == pin)
        {
            btn_evt_enqueue(i);
            ignore_evts = true;
            humi_timer_btn_start();
            break;
        }
    }
}


void humi_btn_init(const humi_btn_t *btns, int num_btns)
{
    btn_pins     = btns;
    num_btn_pins = num_btns;

    nrfx_err_t err;

    err = nrfx_gpiote_init();
    assert(err == NRFX_SUCCESS);

    const nrfx_gpiote_in_config_t btn_config =
            {
                    .is_watcher = false,
                    .hi_accuracy = false,
                    .pull = NRF_GPIO_PIN_PULLUP,
                    .sense = NRF_GPIOTE_POLARITY_TOGGLE,
            };

    for (int i = 0; i < num_btn_pins; i++)
    {
        err = nrfx_gpiote_in_init(btn_pins[i], &btn_config, btn_evt_handler);
        assert(err == NRFX_SUCCESS);

        nrfx_gpiote_in_event_enable(btn_pins[i], true);
    }

    ignore_evts = false;
}

void humi_btn_process(void)
{
    for (int i = 0; i < num_btn_pins; i++)
    {
        if (btn_evts & (1 << i))
        {
            humi_btn_evt(i);

            btn_evt_dequeue(i);
        }
    }
}

void humi_timer_btn_fired(void)
{
    ignore_evts = false;
}

