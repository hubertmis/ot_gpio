//
// Created by bearh on 31.03.19.
//

#include <nrf.h>

void humi_pwr_enable_dcdc(void)
{

}

void humi_pwr_low_power_mode(void)
{
    NRF_POWER->TASKS_LOWPWR = 1;
}

