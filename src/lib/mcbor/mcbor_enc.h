//
// Created by bearh on 11.12.18.
//

#ifndef OT_GPIO_MCBOR_ENC_H
#define OT_GPIO_MCBOR_ENC_H

#include <stddef.h>
#include "mcbor_types.h"

typedef struct {
    char *buf;
    size_t buf_pos;
    size_t buf_size;
} mcbor_enc_t;

void mcbor_enc_init(void * buffer, size_t buffer_size, mcbor_enc_t *mcbor_enc);

size_t mcbor_get_size(const mcbor_enc_t *mcbor_enc);

mcbor_err_t mcbor_enc_uint(mcbor_enc_t *mcbor_enc, unsigned long value);
mcbor_err_t mcbor_enc_nint(mcbor_enc_t *mcbor_enc, unsigned long negated_value);
mcbor_err_t mcbor_enc_text(mcbor_enc_t *mcbor_enc, const char *text);
mcbor_err_t mcbor_enc_arr(mcbor_enc_t *mcbor_enc, int num_items);
mcbor_err_t mcbor_enc_map(mcbor_enc_t *mcbor_enc, int num_pairs);
mcbor_err_t mcbor_enc_tag(mcbor_enc_t *mcbor_enc, mcbor_tag_t tag);

#endif //OT_GPIO_MCBOR_ENC_H
