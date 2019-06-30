//
// Created by bearh on 29.06.19.
//

#include "ctrl.h"

#include <stdint.h>

#include "temp_tscrn_data.h"

#define HYSTERESIS 10
#define OFF_VALUE 0
#define ON_VALUE UINT16_MAX

temp_tscrn_data_subscribe_t temp_meas_subscribe;
temp_tscrn_data_subscribe_t temp_set_subscribe;

static void updated_data(void)
{
    const temp_tscrn_data_publish_t *measurement;
    const temp_tscrn_data_publish_t *setting;
    const temp_tscrn_data_publish_t *output;

    temp_tscrn_data_get(DATA_TEMP_MEASUREMENT, &measurement);
    temp_tscrn_data_get(DATA_TEMP_SETTING, &setting);
    temp_tscrn_data_get(DATA_OUTPUT, &output);

    struct {
        int32_t meas;
        int32_t set;
        uint16_t output;
    } data[] = {
            {
                .meas   = measurement->temp_measurement.local,
                .set    = setting->temp_setting.local,
                .output = output->output.local,
                        },
            {
                .meas   = measurement->temp_measurement.remote,
                .set    = setting->temp_setting.remote,
                .output = output->output.remote,
                        },
    };

    // Typical on-off controller with hysteresis
    for (int i = 0; i < sizeof(data) / sizeof(data[0]); i++)
    {
        uint32_t result;

        if (data[i].meas <= data[i].set - HYSTERESIS) data[i].output = ON_VALUE;
        if (data[i].meas >= data[i].set + HYSTERESIS) data[i].output = OFF_VALUE;
    }

    temp_tscrn_data_publish_t output_pub = {
            .type = DATA_OUTPUT,
            .output = {
                    .local  = data[0].output,
                    .remote = data[1].output,
            }
    };

    temp_tscrn_data_publish(&output_pub);
}

static void updated_measurement(const temp_tscrn_data_publish_t *meas)
{
    (void)meas;

    updated_data();
}

static void updated_setting(const temp_tscrn_data_publish_t *set)
{
    (void)set;

    updated_data();
}

void temp_tscrn_ctrl_init(void)
{
    temp_meas_subscribe = (temp_tscrn_data_subscribe_t){
            .callback = updated_measurement,
    };
    temp_set_subscribe = (temp_tscrn_data_subscribe_t){
            .callback = updated_setting,
    };

    temp_tscrn_data_subscribe(DATA_TEMP_MEASUREMENT, &temp_meas_subscribe);
    temp_tscrn_data_subscribe(DATA_TEMP_SETTING, &temp_set_subscribe);
}