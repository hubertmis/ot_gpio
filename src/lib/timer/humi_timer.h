//
// Created by bearh on 24.11.18.
//

#ifndef OT_GPIO_HUMI_TIMER_H
#define OT_GPIO_HUMI_TIMER_H

#include <stdbool.h>
#include <stdint.h>

void humi_timer_init(void);
void humi_timer_process(void);
bool humi_timer_is_pending(void);

void humi_timer_led_start(void);
void humi_timer_led_stop(void);
extern void humi_timer_led_fired(void);

void humi_timer_btn_start(void);
extern void humi_timer_btn_fired(void);

/** @section General timer */

typedef void (*humi_timer_callback_t)(void *context);
typedef struct humi_timer_t {
    uint32_t target_time; // In RTC ticks
    humi_timer_callback_t callback;
    void *context;

    struct humi_timer_t *next;
} humi_timer_t;

/** @brief Get current time in RTC ticks.
 *
 * @return  Current time [RTC ticks].
 */
uint32_t humi_timer_get_time(void);

/** @brief Get value to set in timer's target_time field.
 *
 * @param t0  Base time [RTC ticks]
 * @param dt  Delta time [ms]
 * @return    Target time [RTC ticks]
 */
uint32_t humi_timer_get_target(uint32_t t0, uint32_t dt);

/** @brief Get value to set in timer's target_time field.
 *
 * @param delay  Delay since now [ms]
 * @return       Target time [RTC ticks]
 */
uint32_t humi_timer_get_target_from_delay(uint32_t delay);

/** @brief Start timer that is going to be triggered at given target time.
 *
 * @param timer  Pointer to a structure containing timer definition.
 */
void humi_timer_gen_add(humi_timer_t *timer);

/** @brief stop given timer.
 *
 * @param timer  Pointer to a structure containing timer to stop.
 */
void humi_timer_gen_remove(humi_timer_t *timer);

#endif //OT_GPIO_HUMI_TIMER_H
