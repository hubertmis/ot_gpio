//
// Created by bearh on 13.12.18.
//

#ifndef OT_GPIO_MCBOR_TYPES_H
#define OT_GPIO_MCBOR_TYPES_H

#define MCBOR_ERR_SUCCESS   0

#define MCBOR_ERR_NO_MEM    -0x01

#define MCBOR_ERR_DATA_END  -0x80
#define MCBOR_ERR_PARSE     -0x81
#define MCBOR_ERR_NOT_FOUND -0x82

typedef int mcbor_err_t;

#endif //OT_GPIO_MCBOR_TYPES_H
