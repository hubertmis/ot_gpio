
#ifndef OT_GPIO_SH_CNT_TEMP_H
#define OT_GPIO_SH_CNT_TEMP_H

#include <stdint.h>

void sh_cnt_temp_init(void);

/** Get last temperature measurement in cC (0.01 of C) **/
int32_t sh_cnt_temp_get(void);

#endif //OT_GPIO_SH_CNT_TEMP_H

