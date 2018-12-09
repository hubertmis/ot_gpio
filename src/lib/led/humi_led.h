//
// Created by bearh on 08.12.18.
//

#ifndef OT_GPIO_HUMI_LED_H
#define OT_GPIO_HUMI_LED_H

#include <stdint.h>

typedef uint32_t humi_led_t;
typedef uint32_t humi_led_idx_t;

void humi_led_init(const humi_led_t *leds, int num_of_leds);
void humi_led_on(humi_led_idx_t idx);
void humi_led_off(humi_led_idx_t idx);
void humi_led_toggle(humi_led_idx_t idx);

#endif //OT_GPIO_HUMI_LED_H
