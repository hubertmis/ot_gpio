
#include "sh_cnt_temp.h"

#include <assert.h>
#include <stddef.h>
#include <nrfx_temp.h>

#include "../../lib/timer/humi_timer.h"

#define TEMP_PERIOD 10000

static humi_timer_t timer;
static int32_t temperature;

static void timer_add(void)
{
    timer.target_time = humi_timer_get_target_from_delay(TEMP_PERIOD);
    humi_timer_gen_add(&timer);
}

static void handler(int32_t raw_temperature)
{
    temperature = raw_temperature;
}

static void timer_callback(void *context)
{
    (void)context;

    nrfx_err_t err;

    err = nrfx_temp_measure();
    assert(err == NRFX_SUCCESS);

    timer_add();
}

void sh_cnt_temp_init(void)
{
    nrfx_err_t         err;
    nrfx_temp_config_t config = {
        .interrupt_priority = 7,
    };

    timer.callback = timer_callback;
    timer.context = NULL;

    err = nrfx_temp_init(&config, handler);
    assert(err == NRFX_SUCCESS);

    err = nrfx_temp_measure();
    assert(err == NRFX_SUCCESS);

    timer_add();
}

/** Get last temperature measurement in cC (0.01 of C) **/
int32_t sh_cnt_temp_get(void)
{
    return nrfx_temp_calculate(temperature);
}
