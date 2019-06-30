//
// Created by bearh on 22.06.19.
//

#include "display.h"

#include <assert.h>

#include "temp_scrn.h"
#include "temp_tscrn_data.h"

static temp_tscrn_data_subscribe_t temp_subscribe;
static temp_tscrn_data_subscribe_t setting_subscribe;

static void temp_update(const temp_tscrn_data_publish_t *data)
{
    const temp_tscrn_data_publish_t *temp_setting;

    temp_tscrn_data_get(DATA_TEMP_SETTING, &temp_setting);

    assert(data->type == DATA_TEMP_MEASUREMENT);
    assert(temp_setting->type == DATA_TEMP_SETTING);

    temp_scrn_display(data->temp_measurement.local, data->temp_measurement.remote,
                      temp_setting->temp_setting.local, temp_setting->temp_setting.remote);
}

static void setting_update(const temp_tscrn_data_publish_t *data)
{
    const temp_tscrn_data_publish_t *temp_meas;

    temp_tscrn_data_get(DATA_TEMP_MEASUREMENT, &temp_meas);

    assert(data->type == DATA_TEMP_SETTING);
    assert(temp_meas->type == DATA_TEMP_MEASUREMENT);

    temp_scrn_display(temp_meas->temp_measurement.local, temp_meas->temp_measurement.remote,
                      data->temp_setting.local, data->temp_setting.remote);
}

static void update_setting(int32_t value)
{
    const temp_tscrn_data_publish_t *prev_data;
    temp_tscrn_data_publish_t       new_data;

    temp_tscrn_data_get(DATA_TEMP_SETTING, &prev_data);
    new_data = *prev_data;
    new_data.temp_setting.local += value ;

    temp_tscrn_data_publish(&new_data);
}

void ft8xx_tag(uint8_t tag)
{
    switch(tag)
    {
        case 1:
            update_setting(1);
            break;

        case 2:
            update_setting(-1);
            break;

        default:
            break;
    }
}

void temp_tscrn_display_init(void)
{
    temp_subscribe = (temp_tscrn_data_subscribe_t){
            .callback = temp_update,
    };
    setting_subscribe = (temp_tscrn_data_subscribe_t){
            .callback = setting_update,
    };

    temp_tscrn_data_subscribe(DATA_TEMP_MEASUREMENT, &temp_subscribe);
    temp_tscrn_data_subscribe(DATA_TEMP_SETTING, &setting_subscribe);
}

void temp_tscrn_dispaly_refresh(void)
{
    const temp_tscrn_data_publish_t *data;

    temp_tscrn_data_get(DATA_TEMP_MEASUREMENT, &data);

    temp_update(data);
}

