//
// Created by bearh on 13.12.18.
//

#include "mcbor_dec.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "mcbor_const.h"

static bool is_ptr_in_buf(const mcbor_dec_t *mcbor_dec, const void *ptr)
{
    if (ptr < mcbor_dec->buf)
    {
        return false;
    }

    if (ptr >= (const void *)((const uint8_t *)(mcbor_dec->buf) + mcbor_dec->buf_size))
    {
        return false;
    }

    return true;
}

static bool is_ptr_at_buf_end(const mcbor_dec_t *mcbor_dec, const void *ptr)
{
    return (ptr == (const void *)((const uint8_t *)(mcbor_dec->buf) + mcbor_dec->buf_size));
}

static bool is_ptr_in_buf_including_end(const mcbor_dec_t *mcbor_dec, const void *ptr)
{
    return is_ptr_in_buf(mcbor_dec, ptr) || is_ptr_at_buf_end(mcbor_dec, ptr);
}

static mcbor_err_t get_int(const mcbor_dec_t *mcbor_dec, const uint8_t *item, uint32_t *value, uint8_t *size)
{
    assert(is_ptr_in_buf(mcbor_dec, item));

    uint32_t result = 0;
    uint8_t  add_info = (*item) & (~MAJ_TYPE_MASK);

    if (add_info < ADD_INFO_1_BYTE)
    {
        *value = add_info;
        *size  = INIT_BYTE_SIZE;
        return MCBOR_ERR_SUCCESS;
    }

    switch (add_info)
    {
        case ADD_INFO_1_BYTE:
            if (!is_ptr_in_buf_including_end(mcbor_dec, item + 1))
            {
                return MCBOR_ERR_PARSE;
            }

            result = item[1];
            *size  = INIT_BYTE_SIZE + 1;
            break;

        case ADD_INFO_2_BYTE:
            if (!is_ptr_in_buf_including_end(mcbor_dec, item + 2))
            {
                return MCBOR_ERR_PARSE;
            }

            result  = (uint32_t)item[1] << 8;
            result |= (uint32_t)item[2] << 0;
            *size  = INIT_BYTE_SIZE + 2;
            break;

        case ADD_INFO_4_BYTE:
            if (!is_ptr_in_buf_including_end(mcbor_dec, item + 4))
            {
                return MCBOR_ERR_PARSE;
            }

            result  = (uint32_t)item[1] << 24;
            result |= (uint32_t)item[2] << 16;
            result |= (uint32_t)item[3] << 8;
            result |= (uint32_t)item[4] << 0;
            *size  = INIT_BYTE_SIZE + 4;
            break;

        default:
            return MCBOR_ERR_PARSE;
    }

    *value = result;
    return MCBOR_ERR_SUCCESS;
}

static mcbor_err_t get_len(const mcbor_dec_t *mcbor_dec, const uint8_t *item, uint32_t *len, uint8_t *size)
{
    return get_int(mcbor_dec, item, len, size);
}

void mcbor_dec_init(const void *buffer, size_t buffer_size, mcbor_dec_t *mcbor_dec)
{
    mcbor_dec->buf      = buffer;
    mcbor_dec->buf_size = buffer_size;
}

void mcbor_dec_iter_init(const mcbor_dec_t *mcbor_dec, mcbor_iter_t *mcbor_iter)
{
    (void)mcbor_dec;
    mcbor_iter->last_ptr = NULL;
}

mcbor_err_t mcbor_get_first_item(const mcbor_dec_t *mcbor_dec, const void **item)
{
    *item = mcbor_dec->buf;

    if (is_ptr_at_buf_end(mcbor_dec, *item))
    {
        return MCBOR_ERR_DATA_END;
    }

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_dec_skip_item(const mcbor_dec_t *mcbor_dec, const void **item_ptr)
{
    const uint8_t *item = *item_ptr;

    assert(is_ptr_in_buf(mcbor_dec, item));

    uint8_t init_byte = *item;

    switch (init_byte & MAJ_TYPE_MASK)
    {
        case MAJ_TYPE_TEXT_STRING:
        {
            const char *text;
            size_t      len;

            mcbor_err_t err = mcbor_dec_get_text(mcbor_dec, item, &text, &len);

            if (err != MCBOR_ERR_SUCCESS)
            {
                return err;
            }

            item = text + len;
            break;
        }

        case MAJ_TYPE_MAP:
        {
            const void *key;
            const void *value;
            int         pairs;

            mcbor_err_t err = mcbor_dec_get_map(mcbor_dec, item, &key, &pairs);
            if (err != MCBOR_ERR_SUCCESS)
            {
                return err;
            }

            for (int i = 0; i < pairs; i++)
            {
                err = mcbor_dec_skip_item(mcbor_dec, &key);
                if (err != MCBOR_ERR_SUCCESS)
                {
                    return err;
                }

                value = key; // Skipped key, now pointer is at value
                err = mcbor_dec_skip_item(mcbor_dec, &value);
                if (err != MCBOR_ERR_SUCCESS)
                {
                    return err;
                }

                key = value; // Skipped value, now pointer is at next key
            }

            item = value; // Skipped value, now pointer is at next item
            break;
        }

        default:
            return MCBOR_ERR_PARSE;
    }

    if (!is_ptr_in_buf(mcbor_dec, item))
    {
        if (is_ptr_at_buf_end(mcbor_dec, item))
        {
            return MCBOR_ERR_DATA_END;
        }

        return MCBOR_ERR_PARSE;
    }

    *item_ptr = item;
    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_dec_get_unsigned(const mcbor_dec_t *mcbor_dec, const void *item, uint64_t *number)
{
    mcbor_err_t err;
    uint32_t    value;
    uint8_t     size;
    
    assert(is_ptr_in_buf(mcbor_dec, item));

    if (((*(const uint8_t *)item) & MAJ_TYPE_MASK) != MAJ_TYPE_UINT)
    {
        return MCBOR_ERR_NOT_FOUND;
    }

    err = get_int(mcbor_dec, item, &value, &size);
    if (err != MCBOR_ERR_SUCCESS)
    {
        return err;
    }

    *number = value;

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_dec_get_text(const mcbor_dec_t *mcbor_dec, const void *item, const char **text, size_t *len)
{
    mcbor_err_t err;
    uint32_t    local_len;
    uint8_t     size;

    assert(is_ptr_in_buf(mcbor_dec, item));

    if (((*(const uint8_t *)item) & MAJ_TYPE_MASK) != MAJ_TYPE_TEXT_STRING)
    {
        return MCBOR_ERR_NOT_FOUND;
    }

    err = get_len(mcbor_dec, item, &local_len, &size);
    if (err != MCBOR_ERR_SUCCESS)
    {
        return err;
    }

    *text = (const char *)item + size;
    *len  = local_len;

    if (!is_ptr_in_buf_including_end(mcbor_dec, *text + local_len))
    {
        return MCBOR_ERR_PARSE;
    }

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_dec_get_map(const mcbor_dec_t *mcbor_dec, const void *item, const void **first_key, int *pairs)
{
    mcbor_err_t err;
    uint8_t     size;
    uint32_t    len;

    assert(is_ptr_in_buf(mcbor_dec, item));

    if (((*(const uint8_t *)item) & MAJ_TYPE_MASK) != MAJ_TYPE_MAP)
    {
        return MCBOR_ERR_NOT_FOUND;
    }

    err = get_len(mcbor_dec, item, &len, &size);
    if (err != MCBOR_ERR_SUCCESS)
    {
        return err;
    }

    *first_key = (const char *)item + size;
    *pairs     = len;

    if (!is_ptr_in_buf(mcbor_dec, *first_key) && len)
    {
        return MCBOR_ERR_PARSE;
    }

    return MCBOR_ERR_SUCCESS;
}

mcbor_err_t mcbor_dec_iter_map(const mcbor_dec_t *mcbor_dec, mcbor_iter_t *mcbor_iter, const void **map_first_key, int *pairs)
{
    mcbor_err_t err = MCBOR_ERR_SUCCESS;

    while (err == MCBOR_ERR_SUCCESS)
    {
        if (mcbor_iter->last_ptr == NULL)
        {
            mcbor_iter->last_ptr = mcbor_dec->buf;
        }
        else
        {
            err = mcbor_dec_skip_item(mcbor_dec, &mcbor_iter->last_ptr);
            if (err != MCBOR_ERR_SUCCESS)
            {
                return err;
            }
        }

        if (!is_ptr_in_buf(mcbor_dec, mcbor_iter->last_ptr))
        {
            if (is_ptr_at_buf_end(mcbor_dec, mcbor_iter->last_ptr))
            {
                return MCBOR_ERR_DATA_END;
            }

            return MCBOR_ERR_PARSE;
        }

        uint8_t init_byte = *(const uint8_t *)mcbor_iter->last_ptr;

        if ((init_byte & MAJ_TYPE_MASK) == MAJ_TYPE_MAP)
        {
            return mcbor_dec_get_map(mcbor_dec, mcbor_iter->last_ptr, map_first_key, pairs);
        }
    }
}

bool mcbor_dec_is_text_equal_to_str(const char *cbor_text, size_t cbor_text_len, const char *str)
{
    if (strlen(str) != cbor_text_len) return false;
    return 0 == strncmp(cbor_text, str, cbor_text_len);
}

