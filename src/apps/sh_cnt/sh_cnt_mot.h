//
// Created by bearh on 09.12.18.
//

#ifndef OT_GPIO_SH_CNT_MOT_H
#define OT_GPIO_SH_CNT_MOT_H

#include <stdbool.h>
#include <stdint.h>

#define SH_CNT_MOT_NUM 2

typedef int sh_cnt_mot_idx_t;

typedef struct {
    int16_t target_val;
    int16_t current_val;
} sh_cnt_mot_details_t;

/** @brief Initialize the motor module of the shades controller application.
 *
 * @param[in] tot_mov_time  Total movement time in one direction [ms].
 */
void sh_cnt_mot_init(void);

void sh_cnt_mot_up(sh_cnt_mot_idx_t idx);
void sh_cnt_mot_down(sh_cnt_mot_idx_t idx);
void sh_cnt_mot_stop(sh_cnt_mot_idx_t idx);
int sh_cnt_mot_val(sh_cnt_mot_idx_t idx, uint16_t val);
bool sh_cnt_mot_val_is_correct(sh_cnt_mot_idx_t idx, uint16_t val);

void sh_cnt_mot_get_details(sh_cnt_mot_idx_t idx, sh_cnt_mot_details_t *details);

#endif //OT_GPIO_SH_CNT_MOT_H
