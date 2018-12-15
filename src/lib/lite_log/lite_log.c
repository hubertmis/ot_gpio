//
// Created by bearh on 15.12.18.
//

#include "lite_log.h"

#define NUM_LOGS 1024

static volatile uint16_t logs[NUM_LOGS];
static volatile uint32_t log_ptr;

void llog(uint8_t event, uint8_t arg)
{
    uint32_t ptr = log_ptr;

    logs[ptr] = ((uint16_t)event << 8) | arg;

    log_ptr = (ptr + 1)%NUM_LOGS;
}

