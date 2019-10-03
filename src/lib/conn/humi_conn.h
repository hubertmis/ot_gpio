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

bool humi_conn_is_commissioned(void);
void humi_conn_start(void);
void humi_conn_stop(void);
void humi_conn_join(void);
void humi_conn_decommission(void);

otInstance *humi_conn_get_instance(void);
bool humi_conn_is_addr_local(const otIp6Address *addr);

void humi_conn_fast_poll_period(void);
void humi_conn_normal_poll_period(void);

extern void humi_conn_join_result(otError error);

#endif //OT_GPIO_HUMI_CONN_H
