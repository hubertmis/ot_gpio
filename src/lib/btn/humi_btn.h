//
// Created by bearh on 08.12.18.
//

#ifndef OT_GPIO_HUMI_BTN_H
#define OT_GPIO_HUMI_BTN_H

#include <stdint.h>

typedef uint32_t humi_btn_t;
typedef uint32_t humi_btn_idx_t;

void humi_btn_init(const humi_btn_t *btns, int num_of_btns);
void humi_btn_process(void);

extern void humi_btn_evt(humi_btn_idx_t idx);

#endif //OT_GPIO_HUMI_BTN_H