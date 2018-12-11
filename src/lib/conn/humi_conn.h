//
// Created by bearh on 09.12.18.
//

#ifndef OT_GPIO_HUMI_CONN_H
#define OT_GPIO_HUMI_CONN_H

#include <stdbool.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>

void humi_conn_init(bool sed);
void humi_conn_process(void);
bool humi_conn_is_pending(void);

void humi_conn_start(void);

otInstance *humi_conn_get_instance(void);
bool humi_conn_is_addr_local(const otIp6Address *addr);

#endif //OT_GPIO_HUMI_CONN_H
