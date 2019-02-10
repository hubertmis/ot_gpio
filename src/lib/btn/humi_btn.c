//
// Created by bearh on 08.12.18.
//

#include "humi_btn.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <nrfx_gpiote.h>

#include "../timer/humi_timer.h"

typedef enum {
    EVT_TYPE_PRS,
    EVT_TYPE_RLS,
} btn_evt_type_t;

static const humi_btn_t *btn_pins;
static int               num_btn_pins;

static uint32_t          btn_prs_evts;
static uint32_t          btn_rls_evts;
static volatile bool     ignore_evts;

static uint32_t * get_evt_map(btn_evt_type_t type)
{
    uint32_t *evts;

    switch (type)
    {
        case EVT_TYPE_PRS:
            evts = &btn_prs_evts;
            break;

        case EVT_TYPE_RLS:
            evts = &btn_rls_evts;
            break;

        default:
            evts = NULL;
            assert(false);
    }

    return evts;
}

static void btn_evt_enqueue(btn_evt_type_t type, humi_btn_idx_t i)
{
    uint32_t *evts = get_evt_map(type);

    *evts |= (1 << i);
}

static void btn_evt_dequeue(btn_evt_type_t type, humi_btn_idx_t i)
{
    uint32_t *evts = get_evt_map(type);

    *evts &= ~(1 << i);
}

static void process_evt(btn_evt_type_t type, humi_btn_idx_t i)
{
    switch (type)
    {
        case EVT_TYPE_PRS:
            btn_evt_enqueue(EVT_TYPE_PRS, i);
            break;

        case EVT_TYPE_RLS:
            btn_evt_dequeue(EVT_TYPE_PRS, i);
            btn_evt_enqueue(EVT_TYPE_RLS, i);
            break;
    }
}

static void btn_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)action;

    if (ignore_evts)
    {
        return;
    }

    for (int i = 0; i < num_btn_pins; i++)
    {
        if (btn_pins[i] == pin)
        {
            process_evt(nrfx_gpiote_in_is_set(pin) ? EVT_TYPE_RLS : EVT_TYPE_PRS, i);
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
        if (btn_prs_evts & (1 << i))
        {
            humi_btn_press_evt(i);

            btn_evt_dequeue(EVT_TYPE_PRS, i);
        }

        if (btn_rls_evts & (1 << i))
        {
            humi_btn_release_evt(i);

            btn_evt_dequeue(EVT_TYPE_RLS, i);
        }
    }
}

bool humi_btn_is_pressed(humi_btn_idx_t idx)
{
    return !nrfx_gpiote_in_is_set(btn_pins[idx]);
}

void humi_timer_btn_fired(void)
{
    ignore_evts = false;
}

void __attribute__((weak)) humi_btn_press_evt(humi_btn_idx_t idx)
{
    // Intentionally empty
}

void __attribute__((weak)) humi_btn_release_evt(humi_btn_idx_t idx)
{
    // Intentionally empty
}

