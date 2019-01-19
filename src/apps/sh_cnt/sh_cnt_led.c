//
// Created by bearh on 08.12.18.
//

#include "sh_cnt_led.h"

#include "../../lib/led/humi_led.h"

static const humi_led_t led_pin = 22;

void sh_cnt_led_init(void)
{
    humi_led_init(&led_pin, 1);
}

void sh_cnt_led_on(void)
{
    humi_led_on(0);
}

void sh_cnt_led_off(void)
{
    humi_led_off(0);
}

void sh_cnt_led_toggle(void)
{
    humi_led_toggle(0);
}

