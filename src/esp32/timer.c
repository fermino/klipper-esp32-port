#include "timer.h"
#include "autoconf.h"
#include "sdkconfig.h"
#include "command.h"
#include "generic/misc.h"
#include "esp_attr.h"
#include "esp_cpu.h"
#include "esp_intr_alloc.h"
#include "board/timer_irq.h"
#include "xtensa/core-macros.h"

#if !CONFIG_IDF_TARGET_ARCH_XTENSA
#   error This Klipper timer implementation is for xtensa cores only.
#endif

#if !CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ_240 || CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ != 240 || CONFIG_CLOCK_FREQ != 240000000
#   error Klipper needs a clock frequency of 240MHz. Check CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ and CONFIG_CLOCK_FREQ.
#endif
DECL_CONSTANT("CLOCK_FREQ", CONFIG_CLOCK_FREQ);

/**
 * Klipper's ESP32 timer implementation (FOR XTENSA CORES ONLY).
 *
 * This uses the clock cycle counter register (CCOUNT, that automatically
 * increments on each clock cycle) and the per-core timer interrupts
 * (CCOMPAREn) to dispatch Klipper timers at the time they're scheduled.
 *
 * One tick is equal to 1s / clock_freq.
 *
 * The usual flow with ESP-IDF is to use ETS_INTERNAL_*_INTR_SOURCE when
 * calling esp_intr_alloc(), that in turn calls get_available_int() to get the
 * proper interrupt number. Given that this is a local (core-bound) interrupt
 * (and as such does not go through the interrupt matrix), we can skip most of
 * it.
 *
 * As a side note, when debugging the interrupt assignment process in ESP-IDF
 * it might be useful to set DEBUG_INT_ALLOC_DECISIONS in intr_alloc.c.
 *
 * The interrupt number comes from the section "CPU Interrupt" in the technical
 * reference manual (8.3.2 for the ESP32/ESP32S2 and 9.3.2 for the ESP32S3).
 */

#define TIMER_CCOMPARE_NO   1       // Needs to be outside parentheses
#define TIMER_CCOMP_INTR_NO (15)    // ETS_INTERNAL_TIMER1_INTR_NO

static void timer_set_ccompare(uint32_t next);

/**
 * Timer's ISR: dispatchs Klipper timers.
 *
 * We need to "fence" the interrupt by disabling the interrupt source, as
 * timer_dispatch_many() assumes global control of interrupts and might
 * lower the interrupt mask even when we're still inside the ISR context
 * (which would result in an interrupt storm).
 */
static void IRAM_ATTR timer_isr()
{
    esp_intr_disable_source(TIMER_CCOMP_INTR_NO);

    uint32_t next = timer_dispatch_many();
    timer_set_ccompare(next);

    esp_intr_enable_source(TIMER_CCOMP_INTR_NO);
}

/**
 * Timer init.
 *
 * Initializes the timer and makes sure it is in a safe state before enabling
 * its corresponding interrupt.
 */
void timer_init()
{
    // Disable interrupt in case it's enabled
    esp_intr_disable_source(TIMER_CCOMP_INTR_NO);

    esp_cpu_intr_set_handler(TIMER_CCOMP_INTR_NO, timer_isr, NULL);

    // Sometimes the registers end up in an unsafe state so we'll reset them
    XTHAL_SET_CCOUNT(0);
    timer_set_ccompare(0);

    // Kick the timer and enable the interrupt
    timer_kick();
    esp_intr_enable_source(TIMER_CCOMP_INTR_NO);
}
DECL_INIT(timer_init);

/**
 * Get the current timestamp as a 32-bit field (it wraps around when overflown)
 */
uint32_t timer_read_time()
{
    return XTHAL_GET_CCOUNT();
}

/**
 * Set the CCOMPAREn register which triggers timer_isr when due.
 *
 * @param next next timestamp (in absolute clock cycles, wraps around)
 * @static internal only.
 */
static void timer_set_ccompare(uint32_t next)
{
    XTHAL_SET_CCOMPARE(TIMER_CCOMPARE_NO, next);
}

/**
 * Kick the timer.
 * Schedules the next interrupt 50us from now.
 */
void timer_kick()
{
    timer_set_ccompare(timer_read_time() + timer_from_us(50));
}
