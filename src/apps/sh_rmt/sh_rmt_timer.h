//
// Created by bearh on 24.11.18.
//

#ifndef OT_GPIO_SH_RMT_TIMER_H
#define OT_GPIO_SH_RMT_TIMER_H

#include <stdbool.h>
#include <stdint.h>

void sh_rmt_timer_init(void);
void sh_rmt_timer_process(void);
bool sh_rmt_timer_is_pending(void);

void sh_rmt_timer_led_start(void);
void sh_rmt_timer_led_stop(void);
extern void sh_rmt_timer_led_fired(void);

void sh_rmt_timer_btn_start(void);
extern void sh_rmt_timer_btn_fired(void);

/** @section General timer */

typedef void (*sh_rmt_timer_callback_t)(void *context);
typedef struct sh_rmt_timer_t {
    uint32_t target_time; // In RTC ticks
    sh_rmt_timer_callback_t callback;
    void *context;

    struct sh_rmt_timer_t *next;
} sh_rmt_timer_t;

/** @brief Get current time in RTC ticks.
 *
 * @return  Current time [RTC ticks].
 */
uint32_t sh_rmt_timer_get_time(void);

/** @brief Get value to set in timer's target_time field.
 *
 * @param t0  Base time [RTC ticks]
 * @param dt  Delta time [ms]
 * @return    Target time [RTC ticks]
 */
uint32_t sh_rmt_timer_get_target(uint32_t t0, uint32_t dt);

/** @brief Get value to set in timer's target_time field.
 *
 * @param delay  Delay since now [ms]
 * @return       Target time [RTC ticks]
 */
uint32_t sh_rmt_timer_get_target_from_delay(uint32_t delay);

/** @brief Start timer that is going to be triggered at given target time.
 *
 * @param timer  Pointer to a structure containing timer definition.
 */
void sh_rmt_timer_gen_add(sh_rmt_timer_t *timer);

/** @brief stop given timer.
 *
 * @param timer  Pointer to a structure containing timer to stop.
 */
void sh_rmt_timer_gen_remove(sh_rmt_timer_t *timer);

#endif //OT_GPIO_SH_RMT_TIMER_H
