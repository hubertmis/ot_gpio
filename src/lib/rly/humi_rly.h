//
// Created by bearh on 29.06.19.
//

#ifndef OT_GPIO_HUMI_RLY_H
#define OT_GPIO_HUMI_RLY_H

#include <stdint.h>

typedef uint32_t humi_rly_t;
typedef uint32_t humi_rly_idx_t;

void humi_rly_init(const humi_rly_t *rlys, int num_of_rlys);
void humi_rly_on(humi_rly_idx_t idx);
void humi_rly_off(humi_rly_idx_t idx);
void humi_rly_toggle(humi_rly_idx_t idx);

#endif //OT_GPIO_HUMI_RLY_H
