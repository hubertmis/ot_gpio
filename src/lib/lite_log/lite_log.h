//
// Created by bearh on 15.12.18.
//

#ifndef OT_GPIO_LITE_LOG_H
#define OT_GPIO_LITE_LOG_H

#include <stdint.h>

#define DEBUG_LOG 1

#if DEBUG_LOG
void llog(uint8_t event, uint8_t arg);
#else // DEBUG_LOG
#define llog(...)
#endif // DEBUG_LOG

#endif //OT_GPIO_LITE_LOG_H
