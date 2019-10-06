
#ifndef OT_GPIO_SH_CNT_LOC_ACT_H
#define OT_GPIO_SH_CNT_LOC_ACT_H

#include "sh_cnt_mot.h"

typedef enum
{
    SH_CNT_LOC_ACT_NONE,
    SH_CNT_LOC_ACT_UP_ON_INIT,
    SH_CNT_LOC_ACT_CYCLIC_EDGES,
} sh_cnt_loc_act_action_t;

void sh_cnt_loc_act_init(void);
void sh_cnt_loc_act_set(sh_cnt_mot_idx_t idx, sh_cnt_loc_act_action_t action);

#endif //OT_GPIO_SH_CNT_LOC_ACT_H
