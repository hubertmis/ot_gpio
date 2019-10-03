//
// Created by bearh on 09.12.18.
//

#include "sh_cnt_mot.h"

#include <assert.h>
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

#define STATE_MIN  0
#define STATE_MAX  256
#define STATE_UP   STATE_MIN
#define STATE_DOWN STATE_MAX
#define STATE_STOP -1

typedef int16_t req_state_t;

typedef enum {
    MOV_DIR_STOP,
    MOV_DIR_UP,
    MOV_DIR_DOWN,
} mov_dir_t;

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

    req_state_t  last_state;
    uint32_t     last_state_timestamp;
    mov_dir_t    mov_dir;

    bool rly_on;
    bool rly_down;
} mot_states[NUM_MOTS];

static void stop(void *context);
static void move(void *context);

static req_state_t get_curr_state(struct mot_state *mot_state)
{
    int32_t curr_state;

    uint32_t now       = humi_timer_get_time();
    uint32_t time_diff = humi_timer_get_time_diff(now, mot_state->last_state_timestamp);

    int32_t state_diff = (STATE_MAX * time_diff + (mov_time / 2)) / mov_time;

    switch (mot_state->mov_dir)
    {
        case MOV_DIR_UP:
            state_diff = -state_diff;
            break;

        case MOV_DIR_DOWN:
            // Intentionally empty: state_diff is already correct.
            break;

        case MOV_DIR_STOP:
            state_diff = 0;
            break;

        default:
            assert(false);
            state_diff = 0;
    }

    curr_state = mot_state->last_state + state_diff;

    if (curr_state < STATE_MIN) curr_state = STATE_MIN;
    if (curr_state > STATE_MAX) curr_state = STATE_MAX;

    return (req_state_t) curr_state;
}

static bool rlys_ready_for_mov_dir(struct mot_state *mot_state)
{
    switch (mot_state->mov_dir)
    {
        case MOV_DIR_STOP:
            return mot_state->rly_on == false && mot_state->rly_down == false;

        case MOV_DIR_DOWN:
            return mot_state->rly_on == true && mot_state->rly_down == true;

        case MOV_DIR_UP:
            return mot_state->rly_on == true && mot_state->rly_down == false;

        default:
            assert(false);
            return false;
    }
}

static void set_timer_to_stop(struct mot_state *mot_state, uint32_t time_diff)
{
    uint32_t delay = time_diff;

    if (mot_state->req_state == STATE_MAX || mot_state->req_state == STATE_MIN)
    {
        delay = TOT_TIME;
    }

    humi_timer_gen_remove(&mot_state->timer);
    mot_state->timer.target_time = humi_timer_get_target_from_delay(delay);
    mot_state->timer.callback    = stop;
    humi_timer_gen_add(&mot_state->timer);
}

static void set_timer_to_move(struct mot_state *mot_state)
{
    mot_state->timer.target_time = humi_timer_get_target_from_delay(RLY_DELAY);
    mot_state->timer.callback    = move;
    humi_timer_gen_add(&mot_state->timer);
}

static bool is_timer_running_to_move(struct mot_state *mot_state)
{
    return humi_timer_gen_is_running(&mot_state->timer) && mot_state->timer.callback == move;
}

static void set_req_state(sh_cnt_mot_idx_t idx, req_state_t req_state)
{
    if (get_curr_state(&mot_states[idx]) != req_state)
    {
        mot_states[idx].req_state = req_state;
        move((void *) idx);
    }
}

static void change_mov_dir(struct mot_state *mot_state, mov_dir_t mov_dir)
{
    mot_state->last_state           = get_curr_state(mot_state);
    mot_state->last_state_timestamp = humi_timer_get_time();
    mot_state->mov_dir              = mov_dir;
}

static void set_rlys(sh_cnt_mot_idx_t idx, uint32_t time_diff)
{
    const struct rlys *mot_rlys = &rlys[idx];
    struct mot_state *mot_state = &mot_states[idx];

    switch (mot_state->mov_dir)
    {
        case MOV_DIR_STOP:
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
            }

            break;

        case MOV_DIR_DOWN:
            if (mot_state->rly_down)
            {
                if (!mot_state->rly_on)
                {
                    sh_cnt_rly_on(mot_rlys->sw);
                    mot_state->rly_on = true;
                }

                set_timer_to_stop(mot_state, time_diff);
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

        case MOV_DIR_UP:
            if (!mot_state->rly_down)
            {
                if (!mot_state->rly_on)
                {
                    sh_cnt_rly_on(mot_rlys->sw);
                    mot_state->rly_on = true;
                }

                set_timer_to_stop(mot_state, time_diff);
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
}

static void stop(void *context)
{
    llog(EV_ENTER, FN_CB_STOP);
    llog(EV_DATA,  (uint8_t)(uint32_t)context);

    sh_cnt_mot_idx_t idx = (sh_cnt_mot_idx_t)context;

    set_req_state(idx, STATE_STOP);

    llog(EV_EXIT, FN_CB_STOP);
}

static void move(void *context) {
    llog(EV_ENTER, FN_CB_MOVE);
    llog(EV_DATA, (uint8_t) (uint32_t) context);

    sh_cnt_mot_idx_t idx = (sh_cnt_mot_idx_t) context;

    struct mot_state *mot_state = &mot_states[idx];

    if (mot_state->req_state == STATE_STOP)
    {
        // Special state: stop at the current state
        change_mov_dir(mot_state, MOV_DIR_STOP);
        set_rlys(idx, 0);
    }
    else
    {
        assert(mot_state->req_state >= STATE_MIN);
        assert(mot_state->req_state <= STATE_MAX);

        // Requested to move: verify required movement direction.
        mov_dir_t req_mov_dir;
        req_state_t curr_state = get_curr_state(mot_state);
        req_state_t state_diff;

        if (curr_state < mot_state->req_state)
        {
            req_mov_dir = MOV_DIR_DOWN;
            state_diff = mot_state->req_state - curr_state;
        }
        else if (curr_state > mot_state->req_state)
        {
            req_mov_dir = MOV_DIR_UP;
            state_diff = curr_state - mot_state->req_state;
        }
        else
        {
            req_mov_dir = MOV_DIR_STOP;
            state_diff = 0;
        }

        uint32_t time_diff = (state_diff * mov_time + (STATE_MAX / 2)) / STATE_MAX;

        // Check if current movement direction is as required.
        if (mot_state->mov_dir == req_mov_dir)
        {
            if (rlys_ready_for_mov_dir(mot_state))
            {
                set_timer_to_stop(mot_state, time_diff);
            }
            else
            {
                if (is_timer_running_to_move(mot_state))
                {
                    // Intentionally empty: next timer iteration would update the timer
                }
                else
                {
                    set_rlys(idx, time_diff);

                    // Check if rlys are in the final position. If so update the timestamp of the movement start
                    if (rlys_ready_for_mov_dir(mot_state))
                    {
                        mot_state->last_state_timestamp = humi_timer_get_time();
                    }
                }
            }
        }
        else
        {
            humi_timer_gen_remove(&mot_state->timer);

            change_mov_dir(mot_state, req_mov_dir);

            set_rlys(idx, time_diff);
        }
    }

    llog(EV_EXIT, FN_CB_MOVE);
}

void sh_cnt_mot_init(int tot_mov_time)
{
    sh_cnt_rly_init();

    mov_time = tot_mov_time;

    for (int i = 0; i < NUM_MOTS; i++)
    {
        mot_states[i].last_state    = STATE_DOWN;
        mot_states[i].timer.context = (void *)i;
        set_req_state(i, STATE_UP);
    }
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

void sh_cnt_mot_val(sh_cnt_mot_idx_t idx, uint16_t val)
{
    if (val > STATE_MAX) return;

    set_req_state(idx, (req_state_t)val);
}

void sh_cnt_mot_get_details(sh_cnt_mot_idx_t idx, sh_cnt_mot_details_t *details)
{
    int16_t current_val = get_curr_state(&mot_states[idx]);
    int16_t target_val = mot_states[idx].req_state;

    if (target_val == STATE_STOP) target_val = current_val;

    details->target_val = target_val;
    details->current_val = current_val;
}
