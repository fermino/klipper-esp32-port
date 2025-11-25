#pragma once

#include "autoconf.h"
#include "sdkconfig.h"
#include "xtensa/core-macros.h"
#include "xtensa/xtruntime.h"

/**
 * ESP-IDF uses a set of CPU interrupts to perform some tasks that go from
 * FreeRTOS ticks to radio and panic handlers. Because we are running Klipper
 * before FreeRTOS even initializes, most of those things are not being used
 * (or have not generated enough jitter for me to notice it).
 *
 * We still want the panic handlers though, so we will assume that system
 * interrupts are at level 4 (CONFIG_ESP_SYSTEM_CHECK_INT_LEVEL_4=y), which
 * should mask the IRQs in timer.c and serial.c.
 *
 * For more info see components/esp_hw_support/port/${TARGET}/esp_cpu_intr.c
 * or dump the info with esp_intr_dump() (be aware that some interrupts are
 * marked as reserved even when they are not being currently used).
 *
 * It's worth noting that when an intlevel is set, interrupts *including*
 * that level are masked, therefore we need to set it to 4 (4-1).
 */
#if !CONFIG_ESP_SYSTEM_CHECK_INT_LEVEL_4
#   error Klipper assumes ESP system interrupts are at level 4 (CONFIG_ESP_SYSTEM_CHECK_INT_LEVEL_4=y)
#endif

#define IRQ_DISABLE_MASK    (4 - 1)     // Would also usually be XCHAL_EXCM_LEVEL
#define IRQ_ENABLE_MASK     (0)

#define NS_TO_TICKS(nseconds) ((uint64_t)CONFIG_CLOCK_FREQ * (uint64_t)(nseconds) / (1ull * 1000ull * 1000ull * 1000ull))

typedef unsigned irqstatus_t;

static inline void __attribute__((always_inline)) irq_disable()
{
    XTOS_SET_MIN_INTLEVEL(IRQ_DISABLE_MASK);
}

static inline void __attribute__((always_inline)) irq_enable()
{
    XTOS_SET_INTLEVEL(IRQ_ENABLE_MASK);
}

static inline irqstatus_t __attribute__((always_inline)) irq_save()
{
    return XTOS_SET_MIN_INTLEVEL(IRQ_DISABLE_MASK);
}

static inline void __attribute__((always_inline)) irq_restore(irqstatus_t flag)
{
    XTOS_RESTORE_JUST_INTLEVEL(flag);
}

/**
 * Enable interrupts for a small time then disable them. This is supposed to be
 * used only by Klipper's scheduler to "fence" where the interrupts are executed
 * and timers dispatched.
 */
static inline void __attribute__((always_inline)) irq_wait()
{
    uint32_t start = XTHAL_GET_CCOUNT();
    irq_enable();
    while (XTHAL_GET_CCOUNT() - start < (uint32_t)NS_TO_TICKS(200)) {
        asm("nop");
    }
    irq_disable();
}

/**
 * As far as I understand this is only used on architectures without hardware
 * based interrupts.
 */
static inline void __attribute__((always_inline)) irq_poll() {}
