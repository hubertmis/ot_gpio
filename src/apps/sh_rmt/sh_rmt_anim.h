//
// Created by bearh on 01.12.18.
//

#ifndef OT_GPIO_SH_RMT_ANIM_H
#define OT_GPIO_SH_RMT_ANIM_H

#include <stdint.h>

void sh_rmt_anim_display_zones(uint32_t zone_mask);
void sh_rmt_anim_fade_out_zones(void);

void sh_rmt_anim_dir_none(void);
void sh_rmt_anim_stop(void);
void sh_rmt_anim_up(void);
void sh_rmt_anim_down(void);

void sh_rmt_anim_connecting(void);
void sh_rmt_anim_connected(void);

#endif //OT_GPIO_SH_RMT_ANIM_H
