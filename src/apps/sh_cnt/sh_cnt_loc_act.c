
#include "sh_cnt_loc_act.h"

#include <assert.h>
#include <stdlib.h>

#include "sh_cnt_pos.h"
#include "../../lib/timer/humi_timer.h"

#define EDGE_TIME 40000

#define CYCLE_TIME (1000UL * 60UL * 60UL)
#define NUM_CYCLES (24UL * 15UL)

typedef struct
{
    sh_cnt_loc_act_action_t act;
    unsigned int            step;
    humi_timer_t            timer;
} action_t;

action_t actions[SH_CNT_MOT_NUM];

static void up_on_init(sh_cnt_mot_idx_t idx, action_t *action)
{
    switch(action->step)
    {
        case 0: // Set local position request to up
            sh_cnt_pos_local_set(idx, SH_CNT_POS_UP);
            action->step++;

            action->timer.target_time = humi_timer_get_target_from_delay(EDGE_TIME);
            humi_timer_gen_add(&action->timer);

            break;

        case 1: // Reset local position request
            sh_cnt_pos_local_set(idx, SH_CNT_POS_STOP);
            action->step++;
            break;

        default:
            assert(false);
    }
}

static void cyclic_edges(sh_cnt_mot_idx_t idx, action_t *action)
{
    switch(action->step)
    {
        case 0: // Set local position request to up
            sh_cnt_pos_local_set(idx, SH_CNT_POS_UP);
            action->step++;

            action->timer.target_time = humi_timer_get_target_from_delay(EDGE_TIME);
            humi_timer_gen_add(&action->timer);

            break;

        case 1: // Set local position request to down
            sh_cnt_pos_local_set(idx, SH_CNT_POS_DOWN);
            action->step++;

            action->timer.target_time = humi_timer_get_target_from_delay(EDGE_TIME);
            humi_timer_gen_add(&action->timer);

            break;

        case 2: // Reset local position request
            sh_cnt_pos_local_set(idx, SH_CNT_POS_STOP);
            action->step++;

            action->timer.target_time = humi_timer_get_target_from_delay(CYCLE_TIME);
            humi_timer_gen_add(&action->timer);

            break;

        default:
            assert(action->step <= NUM_CYCLES + 2);

            action->step              = (action->step < NUM_CYCLES + 2) ? action->step + 1 : 0;
            action->timer.target_time = humi_timer_get_target_from_delay(CYCLE_TIME);
            humi_timer_gen_add(&action->timer);
            break;
    }
}

static void timer_callback(void *context)
{
    sh_cnt_mot_idx_t idx = (sh_cnt_mot_idx_t)context;
    action_t *action     = &actions[idx];

    switch(action->act)
    {
        case SH_CNT_LOC_ACT_UP_ON_INIT:
            up_on_init(idx, action);
            break;

        case SH_CNT_LOC_ACT_CYCLIC_EDGES:
            cyclic_edges(idx, action);
            break;

        default:
            assert(false);
    }
}

void sh_cnt_loc_act_init(void)
{
    for (int i = 0; i < SH_CNT_MOT_NUM; i++)
    {
        actions[i].act            = SH_CNT_LOC_ACT_NONE;
        actions[i].timer.callback = timer_callback;
        actions[i].timer.context  = (void *)i;
    }
}

void sh_cnt_loc_act_set(sh_cnt_mot_idx_t idx, sh_cnt_loc_act_action_t act)
{
    assert(idx < SH_CNT_MOT_NUM);
    action_t *action = &actions[idx];

    humi_timer_gen_remove(&action->timer);

    action->act  = act;
    action->step = 0;
    
    switch(act)
    {
        case SH_CNT_LOC_ACT_UP_ON_INIT:
        case SH_CNT_LOC_ACT_CYCLIC_EDGES:
        {
            uint32_t delay = rand() % (500);
            action->timer.target_time = humi_timer_get_target_from_delay(delay);
            humi_timer_gen_add(&action->timer);
        }
        break;

        case SH_CNT_LOC_ACT_NONE:
        default:
            // Do nothing
            break;
    }
}

