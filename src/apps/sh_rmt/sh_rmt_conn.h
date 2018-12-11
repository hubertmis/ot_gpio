//
// Created by bearh on 02.12.18.
//

#ifndef OT_GPIO_SH_RMT_CONN_H
#define OT_GPIO_SH_RMT_CONN_H

#include <stdbool.h>
#include <stdint.h>

void sh_rmt_conn_init(void);
void sh_rmt_conn_process(void);
bool sh_rmt_conn_is_pending(void);

void sh_rmt_conn_up(uint32_t zone_mask);
void sh_rmt_conn_down(uint32_t zone_mask);
void sh_rmt_conn_stop(uint32_t zone_mask);

#endif //OT_GPIO_SH_RMT_CONN_H
