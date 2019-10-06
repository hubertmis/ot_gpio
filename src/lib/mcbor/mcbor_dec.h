//
// Created by bearh on 13.12.18.
//

#ifndef OT_GPIO_MCBOR_DEC_H
#define OT_GPIO_MCBOR_DEC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "mcbor_types.h"

typedef struct {
    const void *buf;
    size_t      buf_size;
} mcbor_dec_t;

typedef struct {
    const void *last_ptr;
} mcbor_iter_t;

void mcbor_dec_init(const void *buffer, size_t buffer_size, mcbor_dec_t *mcbor_dec);
void mcbor_dec_iter_init(const mcbor_dec_t *mcbor_dec, mcbor_iter_t *mcbor_iter);

mcbor_err_t mcbor_get_first_item(const mcbor_dec_t *mcbor_dec, const void **item);
mcbor_err_t mcbor_dec_skip_item(const mcbor_dec_t *mcbor_dec, const void **item);

mcbor_err_t mcbor_dec_get_unsigned(const mcbor_dec_t *mcbor_dec, const void *item, uint64_t *number);
mcbor_err_t mcbor_dec_get_text(const mcbor_dec_t *mcbor_dec, const void *item, const char **text, size_t *len);
mcbor_err_t mcbor_dec_get_map(const mcbor_dec_t *mcbor_dec, const void *item, const void **first_key, int *pairs);

mcbor_err_t mcbor_dec_iter_map(const mcbor_dec_t *mcbor_dec, mcbor_iter_t *mcbor_iter, const void **map_first_key, int *pairs);

bool mcbor_dec_is_text_equal_to_str(const char *cbor_text, size_t cbor_text_len, const char *str);

#endif //OT_GPIO_MCBOR_DEC_H
