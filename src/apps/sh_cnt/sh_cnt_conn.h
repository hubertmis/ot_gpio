//
// Created by bearh on 09.12.18.
//

#ifndef OT_GPIO_SH_CNT_CONN_H
#define OT_GPIO_SH_CNT_CONN_H

#include <stdbool.h>

void sh_cnt_conn_init(void);
void sh_cnt_conn_process(void);
bool sh_cnt_conn_is_pending(void);
void sh_cnt_conn_reset(void);

#endif //OT_GPIO_SH_CNT_CONN_H
