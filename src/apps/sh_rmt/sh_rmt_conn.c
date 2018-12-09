//
// Created by bearh on 02.12.18.
//

#include "sh_rmt_conn.h"

#include <stddef.h>

#include "../../lib/timer/humi_timer.h"

#define NUM_ZONES 7

static const char * const zone_names[NUM_ZONES] = {
        "dr1",
        "dr2",
        "dr3",
        "k2",
        "br",
        "o",
        "k1",
};

#define SD_RERUN_DELAY 120000

static humi_timer_t sd_timer;

static void sd_start_timer(void);

static void sd_rerun(void *context)
{
    (void)context;

    // TODO: transmit a multicast SD message
    sd_start_timer();
}

static void sd_start_timer(void)
{
    sd_timer.target_time = humi_timer_get_target_from_delay(SD_RERUN_DELAY);
    sd_timer.callback    = sd_rerun;
    sd_timer.context     = NULL;

    humi_timer_gen_add(&sd_timer);
}

static void sd_init(void)
{
    // TODO: transmit a multicast SD message and perhaps increase polling rate until results are received
    sd_start_timer();
}

void shr_rmt_conn_up(uint32_t zone_mask) {
    // TODO: Send command to appropriate zones to move shades up
}

void shr_rmt_conn_down(uint32_t zone_mask) {
    // TODO: Send command to appropriate zones to move shades down
}

void shr_rmt_conn_stop(uint32_t zone_mask) {
    // TODO: Send command to appropriate zones to stop moving shades
}
