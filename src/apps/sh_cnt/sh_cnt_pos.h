
#ifndef OT_GPIO_SH_CNT_POS_H
#define OT_GPIO_SH_CNT_POS_H

#include <stdint.h>
#include "sh_cnt_mot.h"

#define SH_CNT_POS_STOP 0xFFFFU
#define SH_CNT_POS_UP   0xFFFEU
#define SH_CNT_POS_DOWN 0xFFFDU

void sh_cnt_pos_init(void);

int sh_cnt_pos_remote_set(sh_cnt_mot_idx_t idx, uint16_t val);
int sh_cnt_pos_local_set(sh_cnt_mot_idx_t idx, uint16_t val);

#endif // OT_GPIO_SH_CNT_POS_H
