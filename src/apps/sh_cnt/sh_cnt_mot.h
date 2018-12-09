//
// Created by bearh on 09.12.18.
//

#ifndef OT_GPIO_SH_CNT_MOT_H
#define OT_GPIO_SH_CNT_MOT_H

typedef int sh_cnt_mot_idx_t;

/** @brief Initialize the motor module of the shades controller application.
 *
 * @param[in] tot_mov_time  Total movement time in one direction [ms].
 */
void sh_cnt_mot_init(int tot_mov_time);

void sh_cnt_mot_up(sh_cnt_mot_idx_t idx);
void sh_cnt_mot_down(sh_cnt_mot_idx_t idx);
void sh_cnt_mot_stop(sh_cnt_mot_idx_t idx);

#endif //OT_GPIO_SH_CNT_MOT_H
