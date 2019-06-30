//
// Created by bearh on 15.06.19.
//

#ifndef OT_GPIO_FT8XX_SERIAL_DRV_H
#define OT_GPIO_FT8XX_SERIAL_DRV_H

#include <stdint.h>

typedef uint32_t ft8xx_pin_idx_t;

typedef struct {
    struct {
        ft8xx_pin_idx_t sclk;
        ft8xx_pin_idx_t mosi;
        ft8xx_pin_idx_t miso;
        ft8xx_pin_idx_t cs;

        ft8xx_pin_idx_t irq;
    };
} ft8xx_drv_config_t;

void ft8xx_drv_init(const ft8xx_drv_config_t *config);

int ft8xx_drv_read(uint32_t addr, uint16_t len);
int ft8xx_drv_write(uint32_t addr, uint8_t *data, uint16_t len);
int ft8xx_drv_write_no_copy(uint32_t addr, uint16_t len);
int ft8xx_drv_command(uint8_t command);
extern void ft8xx_drv_ready(void);
extern void ft8xx_drv_irq(void);

uint8_t * ft8xx_drv_get_data(void);
uint8_t * ft8xx_drv_get_tx_buffer(uint32_t *len);

#endif //OT_GPIO_FT8XX_SERIAL_DRV_H
