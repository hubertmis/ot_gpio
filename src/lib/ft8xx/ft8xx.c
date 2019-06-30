//
// Created by bearh on 15.06.19.
//

#include "ft8xx.h"

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <cmsis_gcc.h>

#include "ft8xx_copro.h"
#include "ft8xx_dl.h"
#include "ft8xx_drv.h"
#include "ft8xx_host_commands.h"
#include "ft8xx_memory.h"

#define FT8XX_EXPECTED_ID 0x7C

#define DLSWAP_LINE  0x01
#define DLSWAP_FRAME 0x02

#define COPRO_SIZE 4096UL

static enum {
    // Initialization
    STATE_STANDBY,
    STATE_ACTIVE,

    STATE_READING_INT_FLAGS,
    STATE_READING_TAG,
    STATE_READING_CMD_READ,
    STATE_WRITING_COMMAND,
    STATE_WRITING_CMD_WRITE,
} state;

static bool calibrating;

static volatile bool xfer_in_progress;
static volatile bool irq_pending;// = true;

static uint16_t cmd_write = 0;
static uint16_t cmd_read = 0;

static uint16_t cmd_buffer_len = 0;
static uint16_t cmd_last_write_len = 0;

static void encode_u16(uint8_t *buffer, uint16_t data)
{
    buffer[0] = (uint8_t)data;
    buffer[1] = (uint8_t)(data >> 8);
}

static void encode_u32(uint8_t *buffer, uint32_t data)
{
    buffer[0] = (uint8_t)data;
    buffer[1] = (uint8_t)(data >> 8);
    buffer[2] = (uint8_t)(data >> 16);
    buffer[3] = (uint8_t)(data >> 24);
}

static uint16_t parse_u16(void)
{
    uint16_t result;
    uint8_t *buffer = ft8xx_drv_get_data();

    result = (uint16_t)buffer[0];
    result |= (uint16_t)buffer[1] << 8;

    return result;
}

static uint8_t parse_u8(void)
{
    uint8_t result;
    uint8_t *buffer = ft8xx_drv_get_data();

    result = buffer[0];

    return result;
}

static void wait_for_xfer(void)
{
    while (xfer_in_progress)
    {
        __WFE();
    }
}

static uint8_t * read(uint32_t address, uint16_t len)
{
    int res;

    assert(!xfer_in_progress);
    xfer_in_progress = true;

    res = ft8xx_drv_read(address, len);
    assert(res == 0);

    wait_for_xfer();
    return ft8xx_drv_get_data();
}

static void write(uint32_t address, uint8_t *data, uint16_t len)
{
    int res;

    assert(!xfer_in_progress);
    xfer_in_progress = true;

    res = ft8xx_drv_write(address, data, len);
    assert(res == 0);

    wait_for_xfer();
}

static void wr8(uint32_t address, uint8_t data)
{
    write(address, &data, sizeof(data));
}

static void wr16(uint32_t address, uint16_t data)
{
    uint8_t buffer[2];

    encode_u16(buffer, data);
    write(address, buffer, sizeof(buffer));
}

static void wr32(uint32_t address, uint32_t data)
{
    uint8_t buffer[4];

    encode_u32(buffer, data);
    write(address, buffer, sizeof(buffer));
}

static void command(uint8_t cmd)
{
    int res;

    assert(!xfer_in_progress);
    xfer_in_progress = true;

    res = ft8xx_drv_command(cmd);
    assert(res == 0);

    wait_for_xfer();
}

static bool verify_chip(void)
{
    uint8_t *id;

    id = read(REG_ID, 4);
    return *id == FT8XX_EXPECTED_ID;
}

static uint32_t copro_fullness(void)
{
    return (cmd_write - cmd_read) % COPRO_SIZE;
}

static void busy_wait(void)
{
    for (volatile int i = 0; i < 200000; i++);
}

static void calibrate_test(void)
{
    uint32_t buf_len = 60;
    uint8_t buffer[60];
    uint32_t len;
    uint32_t offset = 0;
    uint8_t *read_offset;

    // dlstart
    len = ft8xx_cmd_dlstart(buffer, buf_len);
    write(RAM_CMD + offset, buffer, len);
    offset += len;
    wr16(REG_CMD_WRITE, offset);

    do {
        read_offset = read(REG_CMD_READ, 2);
    } while(read_offset[0] != offset);

    // FT8XX_COPRO_ARGS(cmd, CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
    len = ft8xx_cmd(buffer, buf_len, CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
    write(RAM_CMD + offset, buffer, len);
    offset += len;
    wr16(REG_CMD_WRITE, offset);

    do {
        read_offset = read(REG_CMD_READ, 2);
    } while(read_offset[0] != offset);

    // FT8XX_COPRO_ARGS(cmd, CLEAR(1, 1, 1));
    len = ft8xx_cmd(buffer, buf_len, CLEAR(1, 1, 1));
    write(RAM_CMD + offset, buffer, len);
    offset += len;
    wr16(REG_CMD_WRITE, offset);

    do {
        read_offset = read(REG_CMD_READ, 2);
    } while(read_offset[0] != offset);

    // cmd_calibrate
    len = ft8xx_cmd_calibrate(buffer, buf_len);
    write(RAM_CMD + offset, buffer, len);
    offset += len;
    wr16(REG_CMD_WRITE, offset);

    do {
        read_offset = read(REG_CMD_READ, 2);
    } while(read_offset[0] != offset);
}

int ft8xx_init(const ft8xx_config_t *config)
{
    bool chip_verified;

    ft8xx_drv_init(&(config->drv_config));

    state = STATE_STANDBY;

    command(CORERST);
    command(ACTIVE);
    busy_wait();
    command(CLKEXT);
    command(CLK48M);
    busy_wait();

    command(CORERST);
    command(ACTIVE);
    busy_wait();
    command(CLKEXT);
    command(CLK48M);
    busy_wait();

    state = STATE_ACTIVE;

    while (!verify_chip());

    // Diable LCD
    wr8(REG_GPIO, 0);
    wr8(REG_PCLK, 0);

    // Configure LCD
    wr16(REG_HSIZE, 480);
    wr16(REG_HCYCLE, 548);
    wr16(REG_HOFFSET, 43);
    wr16(REG_HSYNC0, 0);
    wr16(REG_HSYNC1, 41);
    wr16(REG_VSIZE, 272);
    wr16(REG_VCYCLE, 292);
    wr16(REG_VOFFSET, 12);
    wr16(REG_VSYNC0, 0);
    wr16(REG_VSYNC1, 10);
    wr8(REG_SWIZZLE, 0);
    wr8(REG_PCLK_POL, 1);
    wr8(REG_CSPREAD, 1);

    // TODO: Configure touch
    // TODO: Display initial screen
    wr32(RAM_DL + 0, CLEAR_COLOR_RGB(0, 0x80, 0));//Set the initial color
    wr32(RAM_DL + 4, CLEAR(1, 1, 1)); //Clear to the initial color
    wr32(RAM_DL + 8, DISPLAY()); //End the display list
    wr8(REG_DLSWAP, DLSWAP_FRAME);

    // Enable LCD
    wr8(REG_GPIO_DIR, 0x80);
    wr8(REG_GPIO, 0x80);
    wr16(REG_PWM_HZ, 0x00FA);
    wr8(REG_PWM_DUTY, 0x10);
    wr8(REG_PCLK, 0x05);

    // Enable interrupts
    wr8(REG_INT_MASK, 0x04);
    wr8(REG_INT_EN, 0x01);

    //calibrate_test();

    xfer_in_progress = true;
    state = STATE_READING_CMD_READ;
    ft8xx_drv_read(REG_CMD_READ, 2);
}

void ft8xx_process(void)
{
    switch (state)
    {
        case STATE_ACTIVE:
            // Check interrupt reason
            if (!xfer_in_progress && irq_pending)
            {
                xfer_in_progress = true;
                state = STATE_READING_INT_FLAGS;
                ft8xx_drv_read(REG_INT_FLAGS, 1);
            }
            // Read copro read pointer until copro buffer is empty.
            else if (!xfer_in_progress && copro_fullness())
            {
                xfer_in_progress = true;
                state = STATE_READING_CMD_READ;
                ft8xx_drv_read(REG_CMD_READ, 2);
            }
            break;

        case STATE_READING_INT_FLAGS:
            if (!xfer_in_progress)
            {
                uint8_t irq_flags = parse_u8();

                if (irq_flags & 0x04) // Only TAG interrupt is requested
                {
                    xfer_in_progress = true;
                    state = STATE_READING_TAG;
                    ft8xx_drv_read(REG_TOUCH_TAG, 1);
                }
                else
                {
                    irq_pending = false;
                    state = STATE_ACTIVE;
                }
            }
            break;

        case STATE_READING_TAG:
            if (!xfer_in_progress)
            {
                uint8_t tag = parse_u8();
                state = STATE_ACTIVE;
                ft8xx_tag(tag);
            }
            break;

        case STATE_READING_CMD_READ:
            if (!xfer_in_progress)
            {
                static bool following = false;
                cmd_read = parse_u16();
                state = STATE_ACTIVE;

                if (!following)
                {
                    following = true;
                    ft8xx_calibrate();
                }

                if (!copro_fullness())
                {
                    if (calibrating)
                    {
                        calibrating = false;
                        ft8xx_display();
                    }
                }
            }
            break;

        case STATE_WRITING_COMMAND:
            if (!xfer_in_progress)
            {
                if (cmd_buffer_len)
                {
                    // Continue previously started transfer.
                    if (ft8xx_copro_get_freespace())
                    {
                        uint8_t *buffer = ft8xx_drv_get_tx_buffer(NULL);
                        memcpy(buffer, buffer + cmd_last_write_len, cmd_buffer_len);
                        ft8xx_copro_run(cmd_buffer_len);
                    }
                    // Wait for space in the buffer.
                    else
                    {
                        xfer_in_progress = true;
                        state = STATE_READING_CMD_READ;
                        ft8xx_drv_read(REG_CMD_READ, 2);
                    }
                }
                // Finish transfer by writing CMD_WRITE register.
                else
                {
                    uint8_t buffer[2];

                    encode_u16(buffer, cmd_write);

                    xfer_in_progress = true;
                    state = STATE_WRITING_CMD_WRITE;
                    ft8xx_drv_write(REG_CMD_WRITE, buffer, sizeof(buffer));
                }
            }
            break;

        case STATE_WRITING_CMD_WRITE:
            if (!xfer_in_progress)
            {
                state = STATE_ACTIVE;
            }
            break;

        default:
            break;
    }
}

bool ft8xx_is_pending(void)
{
    if (xfer_in_progress) return false;
    if (copro_fullness()) return true;

    return false;
}

bool ft8xx_calibrate(void)
{
    FT8XX_COPRO_BLOCK
    {
        FT8XX_COPRO(cmd_dlstart);

        FT8XX_COPRO_ARGS(cmd, CLEAR_COLOR_RGB(0x00, 0x00, 0x00));
        FT8XX_COPRO_ARGS(cmd, CLEAR(1, 1, 1));

        //FT8XX_COPRO_ARGS(cmd, CLEAR_COLOR_RGB(0xff, 0xff, 0xff));

        //FT8XX_COPRO_ARGS(cmd_text, 80, 30, 27, 0, "Please tap on the dot"); // TODO: OPT_CENTER

        FT8XX_COPRO(cmd_calibrate);
    }

    calibrating = true;
}

uint32_t ft8xx_copro_get_freespace(void)
{
    return COPRO_SIZE - 4 - copro_fullness();
}

bool ft8xx_copro_is_idle(void)
{
    return state == STATE_ACTIVE && !xfer_in_progress && !calibrating;
}

void ft8xx_copro_run(uint32_t command_len)
{
    uint16_t curr_len;
    uint16_t freespace = ft8xx_copro_get_freespace();
    uint16_t overflow_distance = COPRO_SIZE - cmd_write;

    curr_len = command_len;
    if (freespace < curr_len) curr_len = freespace;
    if (overflow_distance < curr_len) curr_len = overflow_distance;

    cmd_last_write_len = curr_len;
    cmd_buffer_len     = command_len - curr_len;

    assert(!xfer_in_progress);
    state = STATE_WRITING_COMMAND;
    xfer_in_progress = true;

    ft8xx_drv_write_no_copy(RAM_CMD + cmd_write, curr_len);
    cmd_write += curr_len;

    if (cmd_write >= COPRO_SIZE)
    {
        cmd_write -= COPRO_SIZE;
    }
}

void ft8xx_drv_ready(void)
{
    xfer_in_progress = false;
}

void ft8xx_drv_irq(void)
{
    irq_pending = true;
}

