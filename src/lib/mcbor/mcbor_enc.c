//
// Created by bearh on 11.12.18.
//

#include "mcbor_enc.h"

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#include "mcbor_const.h"

static bool is_buff_enough(const mcbor_enc_t *mcbor_enc, size_t size)
{
    size_t buf_left = mcbor_enc->buf_size - mcbor_enc->buf_pos;
    return size <= buf_left;
}

static char *get_buf_ptr(const mcbor_enc_t *mcbor_enc)
{
    return &mcbor_enc->buf[mcbor_enc->buf_pos];
}

static void incr_ptr(mcbor_enc_t *mcbor_enc, size_t size)
{
    assert(is_buff_enough(mcbor_enc, size));
    mcbor_enc->buf_pos += size;
}

void mcbor_enc_init(void * buffer, size_t buffer_size, mcbor_enc_t *mcbor_enc)
{
    mcbor_enc->buf      = buffer;
    mcbor_enc->buf_pos  = 0;
    mcbor_enc->buf_size = buffer_size;
}

size_t mcbor_get_size(const mcbor_enc_t *mcbor_enc)
{
    return mcbor_enc->buf_pos;
}

mcbor_err_t mcbor_enc_uint(mcbor_enc_t *mcbor_enc, unsigned long value)
{
    char  *buf = get_buf_ptr(mcbor_enc);
    size_t data_size;

    if (value < ADD_INFO_1_BYTE)
    {
        data_size = INIT_BYTE_SIZE;
        if (!is_buff_enough(mcbor_enc, data_size)) return MCBOR_ERR_NO_MEM;

        buf[0] = (char)value;
        incr_ptr(mcbor_enc, data_size);
    }
    else if (value <= UINT8_MAX)
    {
        data_size = INIT_BYTE_SIZE + sizeof(uint8_t);
        if (!is_buff_enough(mcbor_enc, data_size)) return MCBOR_ERR_NO_MEM;

        buf[0] = ADD_INFO_1_BYTE;
        buf[1] = (char)value;
        incr_ptr(mcbor_enc, data_size);
    }
    else if (value <= UINT16_MAX)
    {
        data_size = INIT_BYTE_SIZE + sizeof(uint16_t);
        if (!is_buff_enough(mcbor_enc, data_size)) return MCBOR_ERR_NO_MEM;

        buf[0] = ADD_INFO_2_BYTE;
        buf[1] = (char)(value >> 8);
        buf[2] = (char)(value >> 0);
        incr_ptr(mcbor_enc, data_size);
    }
    else if (value <= UINT32_MAX)
    {
        data_size = INIT_BYTE_SIZE + sizeof(uint32_t);
        if (!is_buff_enough(mcbor_enc, data_size)) return MCBOR_ERR_NO_MEM;

        buf[0] = ADD_INFO_4_BYTE;
        buf[1] = (char)(value >> 24);
        buf[2] = (char)(value >> 16);
        buf[3] = (char)(value >> 8);
        buf[4] = (char)(value >> 0);
        incr_ptr(mcbor_enc, data_size);
    }
    else
    {
        assert(false); // uint64_t is not implemented yet
    }

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_enc_nint(mcbor_enc_t *mcbor_enc, unsigned long negated_value)
{
    mcbor_err_t err;

    if (negated_value == 0) return MCBOR_ERR_INVALID_ARG;

    char *init_ptr = get_buf_ptr(mcbor_enc);

    err = mcbor_enc_uint(mcbor_enc, negated_value-1);
    if (err != MCBOR_ERR_SUCCESS) return err;

    *init_ptr |= MAJ_TYPE_NEGINT;
}

mcbor_err_t mcbor_enc_text(mcbor_enc_t *mcbor_enc, const char *text)
{
    mcbor_err_t err;

    size_t text_len = strlen(text);
    char  *init_ptr = get_buf_ptr(mcbor_enc);

    err = mcbor_enc_uint(mcbor_enc, text_len);
    if (err != MCBOR_ERR_SUCCESS) return err;

    *init_ptr |= MAJ_TYPE_TEXT_STRING;

    if (!is_buff_enough(mcbor_enc, text_len)) return MCBOR_ERR_NO_MEM;
    memcpy(get_buf_ptr(mcbor_enc), text, text_len);
    incr_ptr(mcbor_enc, text_len);

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_enc_arr(mcbor_enc_t *mcbor_enc, int num_items)
{
    mcbor_err_t err;

    char *init_ptr = get_buf_ptr(mcbor_enc);

    err = mcbor_enc_uint(mcbor_enc, num_items);
    if (err != MCBOR_ERR_SUCCESS) return err;

    *init_ptr |= MAJ_TYPE_ARRAY;

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_enc_map(mcbor_enc_t *mcbor_enc, int num_pairs)
{
    mcbor_err_t err;

    char *init_ptr = get_buf_ptr(mcbor_enc);

    err = mcbor_enc_uint(mcbor_enc, num_pairs);
    if (err != MCBOR_ERR_SUCCESS) return err;

    *init_ptr |= MAJ_TYPE_MAP;

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_enc_tag(mcbor_enc_t *mcbor_enc, mcbor_tag_t tag)
{
    mcbor_err_t err;

    char *init_ptr = get_buf_ptr(mcbor_enc);

    err = mcbor_enc_uint(mcbor_enc, tag);
    if (err != MCBOR_ERR_SUCCESS) return err;

    *init_ptr |= MAJ_TYPE_OPT;

    return MCBOR_ERR_SUCCESS;
}

