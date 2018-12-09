//
// Created by bearh on 08.12.18.
//

#ifndef OT_GPIO_SH_CNT_DISPLAY_H
#define OT_GPIO_SH_CNT_DISPLAY_H

void sh_cnt_display_init(void);

void sh_cnt_display_discovery(void);
void sh_cnt_display_commissioning(void);
void sh_cnt_display_connecting(void);
void sh_cnt_display_connected(void);

void sh_cnt_display_block(void);
void sh_cnt_display_unblock(void);

#endif //OT_GPIO_SH_CNT_DISPLAY_H
