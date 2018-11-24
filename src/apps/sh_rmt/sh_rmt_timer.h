//
// Created by bearh on 24.11.18.
//

#ifndef OT_GPIO_SH_RMT_TIMER_H
#define OT_GPIO_SH_RMT_TIMER_H

void sh_rmt_timer_init(void);

void sh_rmt_timer_led_start(void);
void sh_rmt_timer_led_stop(void);
extern void sh_rmt_timer_led_fired(void);

void sh_rmt_timer_btn_start(void);
void sh_rmt_timer_btn_fired(void);

#endif //OT_GPIO_SH_RMT_TIMER_H
