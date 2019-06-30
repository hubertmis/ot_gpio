//
// Created by bearh on 15.06.19.
//

#ifndef OT_GPIO_FT8XX_H
#define OT_GPIO_FT8XX_H

#include <stdbool.h>
#include "ft8xx_drv.h"

typedef struct {
    ft8xx_drv_config_t drv_config;
} ft8xx_config_t;

int ft8xx_init(const ft8xx_config_t *config);
void ft8xx_process(void);
bool ft8xx_is_pending(void);

bool ft8xx_calibrate(void);

uint32_t ft8xx_copro_get_freespace(void);
bool ft8xx_copro_is_idle(void);
void ft8xx_copro_run(uint32_t len);

extern void ft8xx_display(void);
extern void ft8xx_tag(uint8_t tag);

#define FT8XX_COPRO_BLOCK                                          \
uint32_t buffer_len;                                               \
uint8_t *buffer = ft8xx_drv_get_tx_buffer(&buffer_len);            \
uint16_t copro_len = ft8xx_copro_get_freespace();                  \
buffer_len = copro_len < buffer_len ? copro_len : buffer_len;      \
for (uint32_t commands_len = 0,                                    \
     command_len = 0;                                              \
     ft8xx_copro_is_idle() && buffer_len;                          \
     buffer_len = 0, ft8xx_copro_run(commands_len))

#define FT8XX_COPRO(cmd)                                                                                               \
command_len = ft8xx_##cmd(buffer + commands_len, buffer_len - commands_len);                                           \
if (!command_len) return -1;                                                                                           \
commands_len += command_len;

#define FT8XX_COPRO_ARGS(cmd, ...)                                                                                     \
command_len = ft8xx_##cmd(buffer + commands_len, buffer_len - commands_len, __VA_ARGS__);                              \
if (!command_len) return -1;                                                                                           \
commands_len += command_len;


#endif //OT_GPIO_FT8XX_H
