//
// Created by bearh on 20.06.19.
//

#ifndef OT_GPIO_HUMI_NTC_H
#define OT_GPIO_HUMI_NTC_H

#include <stdbool.h>
#include <stdint.h>

void humi_ntc_init(void);
void humi_ntc_process(void);
bool humi_ntc_is_pending(void);

void humi_ntc_measure(void);

extern void humi_ntc_update_temperature(int32_t temp_local, int32_t temp_remote);

#endif //OT_GPIO_HUMI_NTC_H
