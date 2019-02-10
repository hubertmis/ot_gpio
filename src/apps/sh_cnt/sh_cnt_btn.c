//
// Created by bearh on 08.12.18.
//

#include "sh_cnt_btn.h"

#include "../../lib/btn/humi_btn.h"

static const humi_btn_t btn_pin = 1 * 32 + 0;

void sh_cnt_btn_init(void)
{
    humi_btn_init(&btn_pin, 1);
}

void sh_cnt_btn_process(void)
{
    humi_btn_process();
}

void humi_btn_press_evt(humi_btn_idx_t idx)
{
    sh_cnt_btn_evt();
}

void humi_btn_release_evt(humi_btn_idx_t idx)
{
    sh_cnt_btn_release_evt();
}
