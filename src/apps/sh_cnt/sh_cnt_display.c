//
// Created by bearh on 08.12.18.
//

#include "sh_cnt_display.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sh_cnt_led.h"
#include "../../lib/lite_log/log_data.h"
#include "../../lib/lite_log/lite_log.h"
#include "../../lib/timer/humi_timer.h"

#define DISCOVERY_PHASE_TIME 500
#define COMMISSIONING_PHASE_TIME 50
#define CONNECTING_PHASE_TIME 200

#define CONNECTED_ON_TIME 50
#define CONNECTED_OFF_TIME 4950

static bool blocked;

static enum anim_state_t {
    ANIM_DISCOVERY,
    ANIM_COMMISSIONING,
    ANIM_CONNECTING,
    ANIM_CONNECTED,
} anim_state;

static uint8_t anim_step;

static humi_timer_t timer;

static void anim(void *context)
{
    llog(EV_ENTER, FN_ANIM);
    uint32_t delay;
    (void)context;

    if (blocked)
    {
        return;
    }

    if (anim_step)
    {
        sh_cnt_led_off();
    }
    else
    {
        sh_cnt_led_on();
    }

    switch (anim_state)
    {
        case ANIM_DISCOVERY:
            delay = DISCOVERY_PHASE_TIME;
            break;

        case ANIM_COMMISSIONING:
            delay = COMMISSIONING_PHASE_TIME;
            break;

        case ANIM_CONNECTING:
            delay = CONNECTING_PHASE_TIME;
            timer.target_time = humi_timer_get_target_from_delay(CONNECTING_PHASE_TIME);
            break;

        case ANIM_CONNECTED:
            delay = anim_step ? CONNECTED_OFF_TIME : CONNECTED_ON_TIME;
            break;

        default:
            assert(false);
    }

    timer.callback    = anim;
    timer.target_time = humi_timer_get_target_from_delay(delay);
    timer.context     = NULL;
    anim_step = (anim_step + 1) % 2;

    humi_timer_gen_add(&timer);
    llog(EV_EXIT, FN_ANIM);
}

static void set_anim_state(enum anim_state_t new_state)
{
    humi_timer_gen_remove(&timer);
    anim_state = new_state;
    anim_step  = 0;
    anim(NULL);
}

void sh_cnt_display_init(void)
{
    sh_cnt_led_init();
}

void sh_cnt_display_discovery(void)
{
    set_anim_state(ANIM_DISCOVERY);
}

void sh_cnt_display_commissioning(void)
{
    set_anim_state(ANIM_COMMISSIONING);
}

void sh_cnt_display_connecting(void)
{
    set_anim_state(ANIM_CONNECTING);
}

void sh_cnt_display_connected(void)
{
    set_anim_state(ANIM_CONNECTED);
}

void sh_cnt_display_block(void)
{
    blocked = true;
    sh_cnt_led_off();
}

void sh_cnt_display_unblock(void)
{
    blocked = false;
    anim_step = 0;
    anim(NULL);
}

