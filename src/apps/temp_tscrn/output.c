//
// Created by bearh on 29.06.19.
//

#include "output.h"

#include <assert.h>
#include <stdint.h>

#include "temp_tscrn_data.h"

#include "../../lib/rly/humi_rly.h"

static const uint32_t rly_pins[] = {7};

temp_tscrn_data_subscribe_t output_subscribe;

static void set_output(const temp_tscrn_data_publish_t *output)
{
    assert(output->type == DATA_OUTPUT);

    if (output->output.local)
    {
        humi_rly_on(0);
    }
    else
    {
        humi_rly_off(0);
    }
}

void temp_tscrn_output_init(void)
{
    output_subscribe = (temp_tscrn_data_subscribe_t){
            .callback = set_output,
    };

    humi_rly_init(rly_pins, sizeof(rly_pins) / sizeof(rly_pins[0]));
    temp_tscrn_data_subscribe(DATA_OUTPUT, &output_subscribe);
}