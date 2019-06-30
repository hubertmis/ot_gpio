//
// Created by bearh on 22.06.19.
//

#include "temp_tscrn_data.h"

#include <assert.h>
#include <memory.h>
#include <stddef.h>

#define DEFAULT_TEMP 200

static temp_tscrn_data_publish_t   data_store[DATA_NUM];
static temp_tscrn_data_subscribe_t *subscribers[DATA_NUM];

static void set_default_data(void)
{
    for (int i = 0; i < DATA_NUM; i++)
    {
        data_store[i].type = i;
    }

    data_store[DATA_TEMP_SETTING].temp_setting.local  = DEFAULT_TEMP;
    data_store[DATA_TEMP_SETTING].temp_setting.remote = DEFAULT_TEMP;
}

void temp_tscrn_data_init(void)
{
    for (int i = 0; i < DATA_NUM; i++)
    {
        subscribers[i] = NULL;
    }

    memset(data_store, 0, sizeof(data_store));

    set_default_data();
}

void temp_tscrn_data_subscribe(data_t type, temp_tscrn_data_subscribe_t *subscribe)
{
    assert(type < DATA_NUM);
    assert(subscribe != NULL);

    subscribe->next   = subscribers[type];
    subscribers[type] = subscribe;
}

void temp_tscrn_data_unsubscribe(data_t type, temp_tscrn_data_subscribe_t *subscribe)
{
    assert(type < DATA_NUM);
    assert(subscribe != NULL);

    for (temp_tscrn_data_subscribe_t *item = subscribers[type]; item != NULL; item = item->next)
    {
        if (item->next == subscribe)
        {
            item->next = subscribe->next;
        }
    }

    if (subscribers[type] == subscribe)
    {
        subscribers[type] = subscribe->next;
    }
}

void temp_tscrn_data_publish(temp_tscrn_data_publish_t *data)
{
    assert(data != NULL);

    data_t type = data->type;

    data_store[type] = *data;

    for (temp_tscrn_data_subscribe_t *item = subscribers[type]; item != NULL; item = item->next)
    {
        item->callback(data);
    }
}

void temp_tscrn_data_get(data_t type, const temp_tscrn_data_publish_t **data)
{
    assert(type < DATA_NUM);

    *data = data_store + type;
}

