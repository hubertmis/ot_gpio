//
// Created by bearh on 01.12.18.
//

#include "sh_rmt_anim.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sh_rmt_led.h"
#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 7

#define ANIM_DIR_TIME 200

static uint32_t zones;

typedef enum {
    ANIM_DIR_NONE,
    ANIM_DIR_UP,
    ANIM_DIR_DOWN,
    ANIM_DIR_STOP,
} anim_dir_t;

static anim_dir_t   anim_dir;
static uint8_t      anim_dir_next_step;
static uint8_t      anim_dir_iter;
static humi_timer_t anim_dir_timer;

#define FADE_OUT_TIME   50
#define FADE_OUT_BLINKS 8
static bool         fading_out;
static uint32_t     fade_out_step;
static humi_timer_t fade_out_timer;

#define CONN_LED      1
#define CONN_TIME_ON  100
#define CONN_TIME_OFF 2900
static bool         connected;
static uint32_t     conn_step;
static humi_timer_t conn_timer;

const sh_rmt_led_idx_t zone_led_map[NUM_ZONES] = {
        7, 11, 9, 6, 2, 0, 10
};
const sh_rmt_led_idx_t up_leds[] = {
        3, 8
};
const sh_rmt_led_idx_t down_leds[] = {
        4, 5
};

static void anim_dir_display_up(void)
{
    for (int i = 0; i < sizeof(down_leds) / sizeof(down_leds[0]); i++)
    {
        sh_rmt_led_off(down_leds[i]);
    }

    for (int i = 0; i < sizeof(up_leds) / sizeof(up_leds[0]); i++)
    {
        sh_rmt_led_on(up_leds[i]);
    }
}

static void anim_dir_display_down(void)
{
    for (int i = 0; i < sizeof(up_leds) / sizeof(up_leds[0]); i++)
    {
        sh_rmt_led_off(up_leds[i]);
    }

    for (int i = 0; i < sizeof(down_leds) / sizeof(down_leds[0]); i++)
    {
        sh_rmt_led_on(down_leds[i]);
    }
}

static void anim_dir_display_all(void)
{
    for (int i = 0; i < sizeof(down_leds) / sizeof(down_leds[0]); i++)
    {
        sh_rmt_led_on(down_leds[i]);
    }

    for (int i = 0; i < sizeof(up_leds) / sizeof(up_leds[0]); i++)
    {
        sh_rmt_led_on(up_leds[i]);
    }
}

static void anim_dir_display_none(void)
{
    for (int i = 0; i < sizeof(down_leds) / sizeof(down_leds[0]); i++)
    {
        sh_rmt_led_off(down_leds[i]);
    }

    for (int i = 0; i < sizeof(up_leds) / sizeof(up_leds[0]); i++)
    {
        sh_rmt_led_off(up_leds[i]);
    }
}

static void anim_dir_step(void *context)
{
    (void)context;

    uint32_t max_iters = 0;

    switch(anim_dir) {
        case ANIM_DIR_NONE:
            anim_dir_display_none();
            break;

        case ANIM_DIR_UP:
            max_iters = 5;

            switch (anim_dir_next_step) {
                case 0:
                    anim_dir_display_down();
                    break;

                case 1:
                case 2:
                    anim_dir_display_up();
                    break;

                case 3:
                    anim_dir_display_none();
                    break;
            }

            anim_dir_next_step = (anim_dir_next_step + 1) % 4;
            break;

        case ANIM_DIR_DOWN:
            max_iters = 5;

            switch (anim_dir_next_step) {
                case 0:
                    anim_dir_display_up();
                    break;

                case 1:
                case 2:
                    anim_dir_display_down();
                    break;

                case 3:
                case 4:
                    anim_dir_display_none();
                    break;
            }

            anim_dir_next_step = (anim_dir_next_step + 1) % 5;
            break;

        case ANIM_DIR_STOP:
            max_iters = 2;

            switch (anim_dir_next_step) {
                case 0:
                    anim_dir_display_all();
                    break;

                case 1:
                    anim_dir_display_none();
                    break;

            }

            anim_dir_next_step = (anim_dir_next_step + 1) % 2;
            break;
    }

    if (anim_dir_next_step == 0)
    {
        anim_dir_iter++;
    }

    if (anim_dir_iter < max_iters)
    {
        anim_dir_timer.target_time = humi_timer_get_target_from_delay(ANIM_DIR_TIME);
        anim_dir_timer.callback = anim_dir_step;
        anim_dir_timer.context = NULL;
        humi_timer_gen_add(&anim_dir_timer);
    }
}

static void start_anim_dir(anim_dir_t dir)
{
    anim_dir           = dir;
    anim_dir_next_step = 0;
    anim_dir_iter      = 0;

    anim_dir_step(NULL);
}

static void display_zones(void)
{
    for (int i = 0; i < NUM_ZONES; i++)
    {
        if ((1UL << i) & zones)
        {
            sh_rmt_led_on(zone_led_map[i]);
        }
        else
        {
            sh_rmt_led_off(zone_led_map[i]);
        }
    }
}

static void disable_zones(void)
{
    for (int i = 0; i < NUM_ZONES; i++)
    {
        sh_rmt_led_off(zone_led_map[i]);
    }
}

static void fade_out_anim(void *context)
{
    (void)context;

    if (!fading_out)
    {
        display_zones();
        return;
    }

    int blinks = (fade_out_step / FADE_OUT_BLINKS / 2);

    if (fade_out_step % 2)
    {
        display_zones();
    }
    else if (fade_out_step / 2 % FADE_OUT_BLINKS <= blinks)
    {
        disable_zones();
    }
    else
    {
        display_zones();
    }

    fade_out_step++;

    fade_out_timer.target_time = humi_timer_get_target_from_delay(FADE_OUT_TIME);
    fade_out_timer.callback    = fade_out_anim;
    fade_out_timer.context     = NULL;
    humi_timer_gen_add(&fade_out_timer);
}

static void conn_anim(void *context)
{
    (void)context;

    uint32_t delay;

    if (connected)
    {
        sh_rmt_led_off(CONN_LED);
        return;
    }

    if (conn_step % 2)
    {
        sh_rmt_led_on(CONN_LED);
        delay = CONN_TIME_ON;
    }
    else
    {
        sh_rmt_led_off(CONN_LED);
        delay = CONN_TIME_OFF;
    }

    conn_step ^= 1;

    conn_timer.target_time = humi_timer_get_target_from_delay(delay);
    humi_timer_gen_add(&conn_timer);
}


void sh_rmt_anim_display_zones(uint32_t zone_mask)
{
    fading_out = false;
    zones      = zone_mask;

    display_zones();
}

void sh_rmt_anim_fade_out_zones(void)
{
    fading_out = true;
    fade_out_step = 0;

    fade_out_timer.target_time = humi_timer_get_target_from_delay(FADE_OUT_TIME);
    fade_out_timer.callback    = fade_out_anim;
    fade_out_timer.context     = NULL;
    humi_timer_gen_add(&fade_out_timer);
}

void sh_rmt_anim_dir_none(void) {
    start_anim_dir(ANIM_DIR_NONE);
}

void sh_rmt_anim_stop(void) {
    start_anim_dir(ANIM_DIR_STOP);
}

void sh_rmt_anim_up(void) {
    start_anim_dir(ANIM_DIR_UP);
}

void sh_rmt_anim_down(void) {
    start_anim_dir(ANIM_DIR_DOWN);
}

void sh_rmt_anim_connecting(void)
{
    connected = false;
    conn_step = 0;

    sh_rmt_led_on(CONN_LED);

    conn_timer.target_time = humi_timer_get_target_from_delay(CONN_TIME_ON);
    conn_timer.callback    = conn_anim;
    conn_timer.context     = NULL;

    humi_timer_gen_add(&conn_timer);
}

void sh_rmt_anim_connected(void)
{
    connected = true;

    sh_rmt_led_off(CONN_LED);
}

