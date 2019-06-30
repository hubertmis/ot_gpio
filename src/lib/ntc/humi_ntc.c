//
// Created by bearh on 20.06.19.
//

#include "humi_ntc.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#include <nrfx_saadc.h>

#define AVG_BASE       4UL
#define MAX_RESULT     1023UL
#define REF_RESISTANCE 10000.0
#define NOM_RESISTANCE 10000.0
#define B_VALUE        3380.0
#define REF_TEMP       25.0
#define C_OFFSET       273.15

#define NTC_CH_LOCAL  NRF_SAADC_INPUT_AIN2
#define NTC_CH_REMOTE NRF_SAADC_INPUT_AIN3

#define NTC_CH_LOCAL_IDX 0
#define NTC_CH_REMOTE_IDX 1

#define NUM_THERM   2
#define BUFFER_SIZE NUM_THERM

static uint16_t results[NUM_THERM];
static bool     busy;
static bool     updated_temp;

static nrf_saadc_value_t buffer[BUFFER_SIZE];

static void evt_handler(nrfx_saadc_evt_t const * p_event)
{
    uint32_t avg_base = 1 << AVG_BASE;
    uint16_t new_result;

    for (int i = 0; i < NUM_THERM; i++)
    {
        new_result = (results[i] * (avg_base - 1) + p_event->data.done.p_buffer[i]) / avg_base;

        if (new_result != results[i])
        {
            results[i]   = new_result;
            updated_temp = true;
        }
    }

    busy = false;
}

static int32_t calculate_temp(uint16_t adc_result)
{
    double r = REF_RESISTANCE * (double)adc_result / (double)(MAX_RESULT - adc_result);

    double steinhart = (log(r / NOM_RESISTANCE) / B_VALUE) + (1.0 / (REF_TEMP + C_OFFSET));
    steinhart = 1.0 / steinhart;

    return (int32_t)(10.0 * (steinhart - C_OFFSET) + 0.5);
}

void humi_ntc_init(void)
{
    nrfx_err_t err;
    nrfx_saadc_config_t config                = NRFX_SAADC_DEFAULT_CONFIG;
    nrf_saadc_channel_config_t channel_config = NRFX_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NTC_CH_LOCAL);
    channel_config.reference                  = NRF_SAADC_REFERENCE_VDD4;
    channel_config.gain                       = NRF_SAADC_GAIN1_4;

    err = nrfx_saadc_init(&config, evt_handler);
    assert(err == NRFX_SUCCESS);

    err = nrfx_saadc_channel_init(NTC_CH_LOCAL_IDX, &channel_config);
    assert(err == NRFX_SUCCESS);

    channel_config.pin_p = (nrf_saadc_input_t)NTC_CH_REMOTE;
    err = nrfx_saadc_channel_init(NTC_CH_REMOTE_IDX, &channel_config);
    assert(err == NRFX_SUCCESS);
}

void humi_ntc_process(void)
{
    if (updated_temp)
    {
        int32_t temperature_local  = calculate_temp(results[NTC_CH_LOCAL_IDX]);
        int32_t temperature_remote = calculate_temp(results[NTC_CH_REMOTE_IDX]);
        updated_temp = false;
        humi_ntc_update_temperature(temperature_local, temperature_remote);
    }
}

bool humi_ntc_is_pending(void)
{
    return updated_temp;
}

void humi_ntc_measure(void)
{
    nrfx_err_t err;

    if (updated_temp) return;
    if (busy)         return;

    busy = true;

    err = nrfx_saadc_buffer_convert(buffer, sizeof(buffer) / sizeof(buffer[0]));
    assert(err == NRFX_SUCCESS);

    err = nrfx_saadc_sample();
    assert(err == NRFX_SUCCESS);
}

