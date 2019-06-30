//
// Created by bearh on 15.06.19.
//

#include "ft8xx_drv.h"

#include <assert.h>
#include <stdint.h>
#include <memory.h>

#include <nrfx_gpiote.h>
#include <nrfx_spim.h>

#define SPI_IRQ_PRIORITY 6

#define ADDR_SIZE 3
#define DUMMY_READ_SIZE 1
#define COMMAND_SIZE 3
#define MAX_READ_LEN (UINT16_MAX - ADDR_SIZE - DUMMY_READ_SIZE)
#define MAX_WRITE_LEN (UINT16_MAX - ADDR_SIZE)

#define READ_OP 0x00
#define WRITE_OP 0x80
#define COMMAND_OP 0x40

static nrfx_spim_t spim = NRFX_SPIM_INSTANCE(3);

static bool busy;
static bool ready;

static uint8_t command_buffer[ADDR_SIZE];
static uint8_t data_buffer[UINT16_MAX];

static void spim_evt_handler(nrfx_spim_evt_t const * p_event, void * p_context)
{
    (void)p_context;

    assert(busy);
    assert(!ready);

    ready = true;
    busy  = false;

    ft8xx_drv_ready();
    ready = false;
}

static void gpiote_evt_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    (void)pin;
    assert(action == NRF_GPIOTE_POLARITY_HITOLO);

    ft8xx_drv_irq();
}

static void insert_addr(uint32_t addr, uint8_t *buff)
{
    buff[0] = (addr >> 16) & 0x3f;
    buff[1] = (addr >> 8) & 0xff;
    buff[2] = (addr) & 0xff;
}

void ft8xx_drv_init(const ft8xx_drv_config_t *config)
{
    nrfx_err_t err;

    nrfx_spim_config_t spim_config = NRFX_SPIM_DEFAULT_CONFIG;
    spim_config.sck_pin            = config->sclk;
    spim_config.miso_pin           = config->miso;
    spim_config.mosi_pin           = config->mosi;
    spim_config.ss_pin             = config->cs;
    spim_config.ss_active_high     = false;
    spim_config.use_hw_ss          = true;
    spim_config.irq_priority       = SPI_IRQ_PRIORITY;
//    spim_config.frequency          = NRF_SPIM_FREQ_16M;
    spim_config.frequency          = NRF_SPIM_FREQ_8M;
    spim_config.mode               = NRF_SPIM_MODE_0;
    spim_config.bit_order          = NRF_SPIM_BIT_ORDER_MSB_FIRST;

    err = nrfx_spim_init(&spim, &spim_config, spim_evt_handler, NULL);
    assert(err == NRFX_SUCCESS);

    nrfx_gpiote_in_config_t gpiote_config = NRFX_GPIOTE_CONFIG_IN_SENSE_HITOLO(true);

    err = nrfx_gpiote_init();
    assert(err == NRFX_SUCCESS);

    err = nrfx_gpiote_in_init(config->irq, &gpiote_config, gpiote_evt_handler);
    assert(err == NRFX_SUCCESS);

    nrfx_gpiote_in_event_enable(config->irq, true);
}

int ft8xx_drv_read(uint32_t addr, uint16_t len)
{
    if (busy) return -1;
    if (len > MAX_READ_LEN) return -2;

    assert(!ready);
    busy  = true;

    insert_addr(addr, command_buffer);
    command_buffer[0] |= READ_OP;

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TRX(command_buffer, ADDR_SIZE, data_buffer, ADDR_SIZE + DUMMY_READ_SIZE + len);
    nrfx_err_t            err;

    err = nrfx_spim_xfer(&spim, &xfer_desc, 0);
    assert(err == NRFX_SUCCESS);

    return 0;
}

int ft8xx_drv_write(uint32_t addr, uint8_t *data, uint16_t len)
{
    if (busy) return -1;
    if (len > MAX_WRITE_LEN) return -2;

    assert(!ready);
    busy  = true;

    insert_addr(addr, data_buffer);
    data_buffer[0] |= WRITE_OP;
    memcpy(data_buffer + ADDR_SIZE, data, len);

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(data_buffer, len + ADDR_SIZE);
    nrfx_err_t            err;

    err = nrfx_spim_xfer(&spim, &xfer_desc, 0);
    assert(err == NRFX_SUCCESS);

    return 0;
}

int ft8xx_drv_write_no_copy(uint32_t addr, uint16_t len)
{
    if (busy) return -1;
    if (len > MAX_WRITE_LEN) return -2;

    assert(!ready);
    busy  = true;

    insert_addr(addr, data_buffer);
    data_buffer[0] |= WRITE_OP;

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(data_buffer, len + ADDR_SIZE);
    nrfx_err_t            err;

    err = nrfx_spim_xfer(&spim, &xfer_desc, 0);
    assert(err == NRFX_SUCCESS);

    return 0;
}

int ft8xx_drv_command(uint8_t command)
{
    if (busy) return -1;

    assert(!ready);
    busy  = true;

    command_buffer[0] = command;
    command_buffer[1] = 0;
    command_buffer[2] = 0;

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_XFER_TX(command_buffer, COMMAND_SIZE);

    nrfx_err_t            err;

    err = nrfx_spim_xfer(&spim, &xfer_desc, 0);
    assert(err == NRFX_SUCCESS);

    return 0;
}

uint8_t * ft8xx_drv_get_data(void)
{
    return data_buffer + ADDR_SIZE + DUMMY_READ_SIZE;
}

uint8_t * ft8xx_drv_get_tx_buffer(uint32_t *len)
{
    if (len != NULL)
    {
        *len = sizeof(data_buffer) - ADDR_SIZE;
    }

    return data_buffer + ADDR_SIZE;
}
