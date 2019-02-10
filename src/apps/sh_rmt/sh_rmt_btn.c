#include "sh_rmt_btn.h"

#include <assert.h>
#include <stdbool.h>

#include "../../lib/btn/humi_btn.h"

#define PIN(PORT, NO) (PORT * 32 + NO)

#define BTN1 PIN(0, 10)
#define BTN2 PIN(0, 9)
#define BTN3 PIN(1, 6)
#define BTN4 PIN(1, 4)
#define BTN5 PIN(1, 2)
#define BTN6 PIN(1, 1)
#define BTN7 PIN(1, 3)
#define BTN8 PIN(1, 0)
#define BTN9 PIN(0, 22)

#define NUM_BTNS 9

static const humi_btn_t btn_pins[NUM_BTNS] = {BTN1, BTN2, BTN3, BTN4, BTN5, BTN6, BTN7, BTN8, BTN9};

void sh_rmt_btn_init(void)
{
    humi_btn_init(btn_pins, sizeof(btn_pins) / sizeof(btn_pins[0]));
}

void sh_rmt_btn_process(void)
{
    humi_btn_process();
}

void humi_btn_press_evt(humi_btn_idx_t idx)
{
    sh_rmt_btn_evt(idx);
}

bool sh_rmt_btn_reset_btn_pressed(void)
{
    return humi_btn_is_pressed(2) && humi_btn_is_pressed(6);
}
