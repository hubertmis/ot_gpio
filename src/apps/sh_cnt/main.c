
#include <assert.h>
#include <stdint.h>

#include <cmsis_gcc.h>
#include <openthread/platform/alarm-milli.h>

#include "sh_cnt_btn.h"
#include "sh_cnt_conn.h"
#include "sh_cnt_display.h"
#include "sh_cnt_mot.h"
#include "../../lib/timer/humi_timer.h"

#include "sh_cnt_rly.h"
#include "sh_cnt_led.h"

#define RESET_DELAY 3000
static humi_timer_t reset_timer;

#include <nrf_gpio.h>
static void __INLINE nrf_delay_us(uint32_t volatile number_of_us) __attribute__((always_inline));
static void __INLINE nrf_delay_us(uint32_t volatile number_of_us)
{
    register uint32_t delay __ASM ("r0") = number_of_us;
    __ASM volatile (
    "1:\n"
    " SUBS %0, %0, #1\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " NOP\n"
    " BNE 1b\n"
    : "+r" (delay));
}

static void delay_ms(volatile uint32_t ms)
{
    while (--ms != 0)
    {
        nrf_delay_us(999);
    }
}

static void sh_cnt_factory_reset(void *context)
{
    (void)context;

    sh_cnt_conn_reset();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main(int argc, char *argv[])
{
    humi_timer_init();

    sh_cnt_btn_init();
    sh_cnt_display_init();
    sh_cnt_mot_init();

    nrf_gpio_pin_set(22);
    delay_ms(100);
    nrf_gpio_pin_clear(22);
    delay_ms(50);
    nrf_gpio_pin_set(22);
    delay_ms(50);
    nrf_gpio_pin_clear(22);
    delay_ms(50);
    nrf_gpio_pin_set(22);
    delay_ms(25);
    nrf_gpio_pin_clear(22);


    sh_cnt_conn_init();

    while (1)
    {
        sh_cnt_btn_process();
        humi_timer_process();

        sh_cnt_conn_process();

        if (!humi_timer_is_pending() && !sh_cnt_conn_is_pending())
        {
            __WFE();
        }
    }
}
#pragma clang diagnostic pop

void sh_cnt_btn_evt(void)
{
    reset_timer.target_time = humi_timer_get_target_from_delay(RESET_DELAY);
    reset_timer.callback    = sh_cnt_factory_reset;
    reset_timer.context     = NULL;
    humi_timer_gen_add(&reset_timer);
}

void sh_cnt_btn_release_evt(void)
{
    humi_timer_gen_remove(&reset_timer);
}

