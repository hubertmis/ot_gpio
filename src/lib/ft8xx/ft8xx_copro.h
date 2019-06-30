//
// Created by bearh on 16.06.19.
//

#ifndef OT_GPIO_FT8XX_COPRO_H
#define OT_GPIO_FT8XX_COPRO_H

#include <stdint.h>

uint32_t ft8xx_cmd(uint8_t *buffer, uint32_t len, uint32_t command);
uint32_t ft8xx_cmd_dlstart(uint8_t *buffer, uint32_t len);
uint32_t ft8xx_cmd_swap(uint8_t *buffer, uint32_t len);

uint32_t ft8xx_cmd_text(uint8_t *buffer, uint32_t len, int16_t x, int16_t y, int16_t font, uint16_t options, const char *s);
uint32_t ft8xx_cmd_number(uint8_t *buffer, uint32_t len, int16_t x, int16_t y, int16_t font, uint16_t options, int32_t n);

uint32_t ft8xx_cmd_calibrate(uint8_t *buffer, uint32_t len);

#endif //OT_GPIO_FT8XX_COPRO_H
