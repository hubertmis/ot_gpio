#include "sh_cnt_rly.h"

#include <stdint.h>

#include "../../lib/rly/humi_rly.h"

#define PIN(PORT, NO) (PORT * 32 + NO)

#define M1_DIR PIN(0, 10)
#define M1_SW  PIN(0, 9)
#define M2_SW  PIN(1, 6)
#define M2_DIR PIN(1, 4)

static const uint32_t rly_pins[] = {M1_DIR, M1_SW, M2_SW, M2_DIR};

void sh_cnt_rly_init(void)
{
    humi_rly_init(rly_pins, sizeof(rly_pins) / sizeof(rly_pins[0]));
}

void sh_cnt_rly_on(sh_cnt_rly_idx_t idx)
{
    humi_rly_on(idx);
}

void sh_cnt_rly_off(sh_cnt_rly_idx_t idx)
{
    humi_rly_off(idx);
}

void sh_cnt_rly_toggle(sh_cnt_rly_idx_t idx)
{
    humi_rly_toggle(idx);
}

