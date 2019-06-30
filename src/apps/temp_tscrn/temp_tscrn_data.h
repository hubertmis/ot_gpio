//
// Created by bearh on 22.06.19.
//

#ifndef OT_GPIO_TEMP_TSCRN_DATA_H
#define OT_GPIO_TEMP_TSCRN_DATA_H

#include <stdint.h>

typedef enum {
    DATA_TEMP_MEASUREMENT,
    DATA_TEMP_SETTING,
    DATA_OUTPUT,
    DATA_CONTROLLER,

    DATA_NUM
} data_t;

typedef struct {
    data_t type;
    union {
        struct {
            int16_t local;
            int16_t remote;
        } temp_measurement;
        struct {
            int16_t local;
            int16_t remote;
        } temp_setting;
        struct {
            uint16_t local;
            uint16_t remote;
        } output;
        struct {
        } controller;
    };
} temp_tscrn_data_publish_t;

typedef void (*temp_tscrn_data_callback_t)(const temp_tscrn_data_publish_t *data);

typedef struct temp_tscrn_data_subscribe {
    temp_tscrn_data_callback_t        callback;

    struct temp_tscrn_data_subscribe *next;
} temp_tscrn_data_subscribe_t;

void temp_tscrn_data_init(void);

void temp_tscrn_data_subscribe(data_t type, temp_tscrn_data_subscribe_t *subscribe);
void temp_tscrn_data_publish(temp_tscrn_data_publish_t *data);

void temp_tscrn_data_get(data_t type, const temp_tscrn_data_publish_t **data);

#endif //OT_GPIO_TEMP_TSCRN_DATA_H
