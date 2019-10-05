//
// Created by bearh on 13.12.18.
//

#ifndef OT_GPIO_MCBOR_TYPES_H
#define OT_GPIO_MCBOR_TYPES_H

#define MCBOR_ERR_SUCCESS   0

#define MCBOR_ERR_NO_MEM      -0x01
#define MCBOR_ERR_INVALID_ARG -0x02

#define MCBOR_ERR_DATA_END    -0x80
#define MCBOR_ERR_PARSE       -0x81
#define MCBOR_ERR_NOT_FOUND   -0x82

typedef int mcbor_err_t;

#define MCBOR_TAG_STANDARD_DATE_TIME 0
#define MCBOR_TAG_EPOCH_DATE_TIME    1
#define MCBOR_TAG_POSITIVE_BIGNUM    2
#define MCBOR_TAG_NEGATIVE_BIGNUM    3
#define MCBOR_TAG_DECIMAL_FRACTION   4

typedef unsigned int mcbor_tag_t;

#endif //OT_GPIO_MCBOR_TYPES_H
