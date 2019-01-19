//
// Created by bearh on 09.12.18.
//

#include "sh_cnt_mot.h"

#include <stdbool.h>

#include "sh_cnt_display.h"
#include "sh_cnt_rly.h"
#include "../../lib/lite_log/lite_log.h"
#include "../../lib/lite_log/log_data.h"
#include "../../lib/timer/humi_timer.h"

#define NUM_MOTS 2
#define RLY_DELAY 250

#define TOT_FACTOR     312
#define TOT_FACTOR_REL 256

#define TOT_TIME (mov_time * TOT_FACTOR / TOT_FACTOR_REL)

typedef enum {
    STATE_STOP,
    STATE_UP,
    STATE_DOWN
} req_state_t;

const struct rlys {
    sh_cnt_rly_idx_t sw;
    sh_cnt_rly_idx_t dir;
} rlys[NUM_MOTS] = {
        {1, 0},
        {2, 3},
};

static int mov_time;

static struct mot_state {
    humi_timer_t timer;
    req_state_t  req_state;

    bool rly_on;
    bool rly_down;
} mot_states[NUM_MOTS];

static void stop(void *context);
static void move(void *context);

static void try_display_unblock(void)
{
    bool unblock = true;

    for (int i = 0; i < NUM_MOTS; i++)
    {
        if ((mot_states[i].req_state != STATE_STOP) ||
            (mot_states[i].rly_on) ||
            (mot_states[i].rly_down))
        {
            unblock = false;
            break;
        }
    }

    if (unblock)
    {
    //    sh_cnt_display_unblock();
    }
}

static void set_timer_to_stop(struct mot_state *mot_state)
{
    mot_state->timer.target_time = humi_timer_get_target_from_delay(TOT_TIME);
    mot_state->timer.callback    = stop;
    humi_timer_gen_add(&mot_state->timer);
}

static void set_timer_to_move(struct mot_state *mot_state)
{
    mot_state->timer.target_time = humi_timer_get_target_from_delay(RLY_DELAY);
    mot_state->timer.callback    = move;
    humi_timer_gen_add(&mot_state->timer);
}

static void set_req_state(sh_cnt_mot_idx_t idx, req_state_t req_state)
{
//    sh_cnt_display_block();

    mot_states[idx].req_state = req_state;
    move((void*)idx);
}

static void stop(void *context)
{
    llog(EV_ENTER, FN_CB_STOP);
    llog(EV_DATA,  (uint8_t)(uint32_t)context);

    sh_cnt_mot_idx_t idx = (sh_cnt_mot_idx_t)context;

    set_req_state(idx, STATE_STOP);

    llog(EV_EXIT, FN_CB_STOP);
}

static void move(void *context)
{
    llog(EV_ENTER, FN_CB_MOVE);
    llog(EV_DATA,  (uint8_t)(uint32_t)context);

    sh_cnt_mot_idx_t idx = (sh_cnt_mot_idx_t)context;

    const struct rlys *mot_rlys  = &rlys[idx];
    struct mot_state  *mot_state = &mot_states[idx];

    switch (mot_state->req_state)
    {
        case STATE_STOP:
            if (mot_state->rly_on)
            {
                sh_cnt_rly_off(mot_rlys->sw);
                mot_state->rly_on = false;

                set_timer_to_move(mot_state);
            }
            else
            {
                sh_cnt_rly_off(mot_rlys->dir);
                mot_state->rly_down = false;

                try_display_unblock();
            }

            break;

        case STATE_DOWN:
            if (mot_state->rly_down)
            {
                if (!mot_state->rly_on)
                {
                    sh_cnt_rly_on(mot_rlys->sw);
                    mot_state->rly_on = true;
                }

                set_timer_to_stop(mot_state);
            }
            else
            {
                if (mot_state->rly_on)
                {
                    sh_cnt_rly_off(mot_rlys->sw);
                    mot_state->rly_on = false;
                }
                else
                {
                    sh_cnt_rly_on(mot_rlys->dir);
                    mot_state->rly_down = true;
                }

                set_timer_to_move(mot_state);
            }

            break;

        case STATE_UP:
            if (!mot_state->rly_down)
            {
                if (!mot_state->rly_on)
                {
                    sh_cnt_rly_on(mot_rlys->sw);
                    mot_state->rly_on = true;
                }

                set_timer_to_stop(mot_state);
            }
            else
            {
                if (mot_state->rly_on)
                {
                    sh_cnt_rly_off(mot_rlys->sw);
                    mot_state->rly_on = false;
                }
                else
                {
                    sh_cnt_rly_off(mot_rlys->dir);
                    mot_state->rly_down = false;
                }

                set_timer_to_move(mot_state);
            }

            break;
    }

    llog(EV_EXIT, FN_CB_MOVE);
}

void sh_cnt_mot_init(int tot_mov_time)
{
    mov_time = tot_mov_time;

    for (int i = 0; i < NUM_MOTS; i++)
    {
        mot_states[i].timer.context  = (void *)i;
    }

    sh_cnt_rly_init();
}

void sh_cnt_mot_up(sh_cnt_mot_idx_t idx)
{
    llog(EV_ENTER, FN_UP);
    llog(EV_DATA,  idx);

    set_req_state(idx, STATE_UP);

    llog(EV_EXIT, FN_UP);
}

void sh_cnt_mot_down(sh_cnt_mot_idx_t idx)
{
    llog(EV_ENTER, FN_DOWN);
    llog(EV_DATA,  idx);

    set_req_state(idx, STATE_DOWN);

    llog(EV_EXIT, FN_DOWN);
}

void sh_cnt_mot_stop(sh_cnt_mot_idx_t idx)
{
    llog(EV_ENTER, FN_STOP);
    llog(EV_DATA,  idx);

    set_req_state(idx, STATE_STOP);

    llog(EV_EXIT, FN_STOP);
}

