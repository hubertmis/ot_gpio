//
// Created by bearh on 20.06.19.
//

#ifndef OT_GPIO_FT8XX_DL_H
#define OT_GPIO_FT8XX_DL_H

#include <stdint.h>

#define CLEAR(c, s, t) (0x26000000 | ((c) ? 0x04 : 0) | ((s) ? 0x02 : 0) | ((t) ? 0x01 : 0))
#define CLEAR_COLOR_RGB(red, green, blue) (0x02000000 | (((uint32_t)(red) & 0xff) << 16) | (((uint32_t)(green) & 0xff) << 8) | ((uint32_t)(blue) & 0xff))
#define DISPLAY() 0
#define TAG(s) (0x03000000 | (uint8_t)(s))

#endif //OT_GPIO_FT8XX_DL_H
