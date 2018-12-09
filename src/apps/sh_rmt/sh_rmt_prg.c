//
// Created by bearh on 01.12.18.
//

#include "sh_rmt_prg.h"

#include <stddef.h>
#include <stdint.h>

#include "sh_rmt_anim.h"
#include "sh_rmt_btn.h"
#include "sh_rmt_conn.h"
#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 7
#define ALL_ZONES (0xFFFFFFFFUL >> (32 - NUM_ZONES))
#define NO_ZONES  0UL

#define INACTIVITY_TIME 5000

static const sh_rmt_btn_idx_t zone_btn_map[6] = {
        3, 8, 7, 5, 1, 0
};

#define KITCHEN_ZONES_MAP 0x44UL
static uint32_t kitchen_zones[4] = {
        0x00,
        0x44,
        0x04,
        0x40,
};

static uint32_t active_zones;
typedef enum {
    DIRECTION_NONE,
    DIRECTION_STOP,
    DIRECTION_UP,
    DIRECTION_DOWN
} direction_t;
static direction_t direction;

typedef enum {
    INACTIVITY_NONE,
    INACTIVITY_FADE,
    INACTIVITY_SLEEP,
} inactivity_t;
static inactivity_t inactivity_step;

static humi_timer_t inactivity_timer;

static void set_direction(direction_t dir)
{
    direction = dir;

    switch (direction)
    {
        case DIRECTION_NONE:
            sh_rmt_anim_dir_none();
            break;

        case DIRECTION_STOP:
            shr_rmt_conn_stop(active_zones);
            sh_rmt_anim_stop();
            break;

        case DIRECTION_UP:
            shr_rmt_conn_up(active_zones);
            sh_rmt_anim_up();
            break;

        case DIRECTION_DOWN:
            shr_rmt_conn_down(active_zones);
            sh_rmt_anim_down();
            break;
    }
}

static void inactivity_timeout(void *context)
{
    switch ((inactivity_t)context)
    {
        case INACTIVITY_FADE:
            sh_rmt_anim_fade_out_zones();

            inactivity_timer.target_time = humi_timer_get_target_from_delay(INACTIVITY_TIME);
            inactivity_timer.callback    = inactivity_timeout;
            inactivity_timer.context     = (void *)INACTIVITY_SLEEP;
            humi_timer_gen_add(&inactivity_timer);
            break;

        case INACTIVITY_SLEEP:
            direction = DIRECTION_STOP;
            sh_rmt_anim_dir_none();

            active_zones = NO_ZONES;
            sh_rmt_anim_display_zones(active_zones);
            break;

    }
}

void sh_rmt_btn_evt(sh_rmt_btn_idx_t idx)
{
    switch (idx) {
        case 6: // Right (up)
            if (active_zones)
            {
                set_direction(DIRECTION_UP);
            }
            break;

        case 2: // Left (down)
            if (active_zones)
            {
                set_direction(DIRECTION_DOWN);
            }
            break;

        case 4: // Central button
            if (active_zones)
            {
                set_direction(DIRECTION_STOP);
            }
            else
            {
                active_zones = ALL_ZONES;
            }
            break;

        case 7: // Kitchen window
            {
                uint32_t active_kitchen_zones = active_zones & KITCHEN_ZONES_MAP;
                uint32_t next_kitchen_zones;

                for (int i = 0; i < sizeof(kitchen_zones) / sizeof(kitchen_zones[0]); i++)
                {
                    if (active_kitchen_zones == kitchen_zones[i])
                    {
                        next_kitchen_zones = kitchen_zones[(i + 1) % (sizeof(kitchen_zones) / sizeof(kitchen_zones[0]))];
                        active_zones &= (~(KITCHEN_ZONES_MAP));
                        active_zones |= next_kitchen_zones;
                    }
                }
            }
            break;

        default:
            for (int i = 0; i < sizeof(zone_btn_map) / sizeof(zone_btn_map[0]); i++) {
                if (zone_btn_map[i] == 7)
                {
                    // Kitchen window - special case
                    continue;
                }
                if (zone_btn_map[i] == idx) {
                    active_zones ^= (1UL << i);
                    set_direction(DIRECTION_NONE);
                    break;
                }
            }
    }

    sh_rmt_anim_display_zones(active_zones);

    inactivity_timer.target_time = humi_timer_get_target_from_delay(INACTIVITY_TIME);
    inactivity_timer.callback    = inactivity_timeout;
    inactivity_timer.context     = (void *)INACTIVITY_FADE;
    humi_timer_gen_add(&inactivity_timer);
}
