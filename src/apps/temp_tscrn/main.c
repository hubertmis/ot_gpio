//
// Created by bearh on 15.06.19.
//

#include <stdint.h>
#include <cmsis_gcc.h>

#include "ctrl.h"
#include "display.h"
#include "output.h"
#include "temp_tscrn_data.h"
#include "../../lib/ntc/humi_ntc.h"
#include "../../lib/ft8xx/ft8xx.h"
#include "../../lib/timer/humi_timer.h"

#include "hal/nrf_gpio.h"

void touchscreen_init(void)
{
    ft8xx_config_t config = {0};
    config.drv_config.sclk = 19;
    config.drv_config.mosi = 23;
    config.drv_config.miso = 21;
    config.drv_config.cs   = 12;

    config.drv_config.irq  = 32 + 9;

    ft8xx_init(&config);
}

void ft8xx_display(void)
{
    temp_tscrn_dispaly_refresh();
}

void humi_ntc_update_temperature(int32_t t1, int32_t t2)
{
    temp_tscrn_data_publish_t data = {
            .type = DATA_TEMP_MEASUREMENT,
            .temp_measurement = {
                    .local  = t1,
                    .remote = t2,
            }
    };

    temp_tscrn_data_publish(&data);
}

void
nrf_802154_clock_lfclk_start(void);

int main(int argc, char *argv[])
{
    nrf_802154_clock_lfclk_start();

    humi_ntc_init();
    humi_timer_init();

    temp_tscrn_data_init();

    touchscreen_init();
    temp_tscrn_output_init();
    temp_tscrn_ctrl_init();
    temp_tscrn_display_init();

//    temp_tscrn_conn_init();

    while (1)
    {
//        static uint32_t last_measurement;
//        uint32_t now = humi_timer_get_time();

        humi_ntc_process();
        humi_timer_process();
        ft8xx_process();

//        if (now - last_measurement > 300)
        {
            humi_ntc_measure();
//            last_measurement =  now;
        }

//        if (!nrf_gpio_pin_read(32+9))
//        {
//            ft8xx_tag(200);
//        }

        if (!humi_timer_is_pending() && !ft8xx_is_pending() && !humi_ntc_is_pending()/* && !sh_rmt_conn_is_pending()*/)
        {
            //__WFE();
        }
    }

    return 0;
}

