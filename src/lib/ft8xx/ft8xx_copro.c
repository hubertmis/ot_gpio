//
// Created by bearh on 16.06.19.
//

#include "ft8xx_copro.h"

#include <stdint.h>
#include <string.h>

#define CMD_SIZE 4

static void encode(uint8_t *buffer, uint32_t word)
{
    buffer[0] = (uint8_t)word;
    buffer[1] = (uint8_t)(word >> 8);
    buffer[2] = (uint8_t)(word >> 16);
    buffer[3] = (uint8_t)(word >> 24);
}

static void encode_i16(uint8_t *buffer, int16_t data)
{
    buffer[0] = (uint8_t)data;
    buffer[1] = (uint8_t)(data >> 8);
}

static void encode_u16(uint8_t *buffer, uint16_t data)
{
    buffer[0] = (uint8_t)data;
    buffer[1] = (uint8_t)(data >> 8);
}

static void encode_i32(uint8_t *buffer, int32_t data)
{
    buffer[0] = (uint8_t)data;
    buffer[1] = (uint8_t)(data >> 8);
    buffer[2] = (uint8_t)(data >> 16);
    buffer[3] = (uint8_t)(data >> 24);
}

static uint32_t cmd(uint8_t *buffer, uint32_t len, uint32_t command)
{
    if (len < CMD_SIZE) return 0;

    encode(buffer, command);
    return CMD_SIZE;
}

uint32_t ft8xx_cmd(uint8_t *buffer, uint32_t len, uint32_t command)
{
    return cmd(buffer, len, command);
}

uint32_t ft8xx_cmd_dlstart(uint8_t *buffer, uint32_t len)
{
    return cmd(buffer, len, 0xffffff00);
}

uint32_t ft8xx_cmd_swap(uint8_t *buffer, uint32_t len)
{
    return cmd(buffer, len, 0xffffff01);
}

uint32_t ft8xx_cmd_text(uint8_t *buffer, uint32_t len, int16_t x, int16_t y, int16_t font, uint16_t options, const char *s)
{
    uint32_t result = 0;
    uint32_t s_len = strlen(s);
    uint32_t cmd_len = 12 + s_len;
    uint32_t padding_bytes = 4 - (cmd_len % 4); // At least single padding byte as string terminator
    cmd_len += padding_bytes;

    if (len < cmd_len) return 0;

    result += cmd(buffer + result, len - result, 0xffffff0c);

    encode_i16(buffer + result, x);
    result += sizeof(x);

    encode_i16(buffer + result, y);
    result += sizeof(y);

    encode_i16(buffer + result, font);
    result += sizeof(font);

    encode_u16(buffer + result, options);
    result += sizeof(options);

    strncpy(buffer + result, s, len - result);
    result += s_len;

    result += padding_bytes;

    return result;
}

uint32_t ft8xx_cmd_number(uint8_t *buffer, uint32_t len, int16_t x, int16_t y, int16_t font, uint16_t options, int32_t n)
{
    uint32_t result = 0;
    uint32_t cmd_len = CMD_SIZE + sizeof(x) + sizeof(y) + sizeof(font) + sizeof(options) + sizeof(n);

    if (len < cmd_len) return 0;

    result += cmd(buffer + result, len - result, 0xffffff2e);

    encode_i16(buffer + result, x);
    result += sizeof(x);

    encode_i16(buffer + result, y);
    result += sizeof(y);

    encode_i16(buffer + result, font);
    result += sizeof(font);

    encode_u16(buffer + result, options);
    result += sizeof(options);

    encode_i32(buffer + result, n);
    result += sizeof(n);

    return result;
}

uint32_t ft8xx_cmd_calibrate(uint8_t *buffer, uint32_t len)
{
    uint32_t result = 0;
    uint32_t cmd_len = CMD_SIZE + sizeof(int32_t);

    if (len < cmd_len) return 0;

    result += cmd(buffer + result, len - result, 0xffffff15);

    encode_i32(buffer + result, 0);
    result += sizeof(int32_t);

    return result;
}


