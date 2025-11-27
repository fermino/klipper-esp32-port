// Copyright 2025, Fermin Olaiz <ferminolaiz@gmail.com>
// SPDX-License-Identifier: GPL-3.0-only

#include "watchdog.h"
#include "command.h"
#include "esp_clk_tree.h"
#include "esp_private/periph_ctrl.h"
#include "hal/wdt_hal.h"
#include "hal/timer_ll.h"

/**
 * Watchdog timer for ESP32 platforms.
 *
 * This is loosely based on IDF's int_wdt.c as the usage of MWDTs is not super
 * clearly documented.
 *
 * There is some assumptions made in this code: there's a bug in the ESP32 that
 * can lead to a live-lock [1], this is circumvented in the IDF code BUT NOT
 * HERE. Because we assume an unicore environment we can just gloss over that.
 *
 * @todo it should be possible to implement a controlled-shutdown stage before
 * the actual reset, but I have not been able to make it work yet. Maybe moving
 * to the RTC watchdog, but a high level interrupt should be used because they
 * can be masked (irq.c). You're welcome to improve it! :)
 *
 * [1] https://docs.espressif.com/projects/esp-chip-errata/en/latest/esp32/03-errata-description/esp32/watchdog-issue-caused-by-live-lock.html
 */

#if CONFIG_ESP_INT_WDT || CONFIG_ESP_TASK_WDT_EN || CONFIG_ESP_INT_WDT_CHECK_CPU1
#   error Klipper needs CONFIG_ESP_INT_WDT and CONFIG_ESP_TASK_WDT_EN disabled.
#endif
#if CONFIG_ESP32_ECO3_CACHE_LOCK_FIX && !CONFIG_FREERTOS_UNICORE
#   error Klipper assumes a unicore environment to avoid the live-lock bug. See the errata for ESP32.
#endif

#define WDT_TIMER_GROUP 0
#define WDT_PERIPH      PERIPH_TIMG0_MODULE
#define WDT_INSTANCE    WDT_MWDT0

wdt_hal_context_t wdt;

/**
 * Set up the watchdog. It resets the MCU if it hasn't been fed after 500us.
 */
void watchdog_init()
{
    // Initialize peripheral clock
    PERIPH_RCC_ACQUIRE_ATOMIC(WDT_PERIPH, references) {
        if (references == 0) {
            timer_ll_enable_bus_clock(WDT_TIMER_GROUP, true);
            timer_ll_reset_register(WDT_TIMER_GROUP);
        }
    }

    // Calculate prescaler such that 1 tick = 1 us
    uint32_t clk_freq;
    if (esp_clk_tree_src_get_freq_hz(MWDT_CLK_SRC_DEFAULT, ESP_CLK_TREE_SRC_FREQ_PRECISION_EXACT, &clk_freq) != ESP_OK) {
        shutdown("WDT: can't calculate watchdog prescaler.");
    }
    uint16_t prescaler = clk_freq / (uint32_t)1e6;

    // Initialize the watchdog with a 500us timeout
    wdt_hal_init(&wdt, WDT_INSTANCE, prescaler, false);
    wdt_hal_write_protect_disable(&wdt);
    wdt_hal_config_stage(&wdt, WDT_STAGE0, (uint32_t)500e3, WDT_STAGE_ACTION_RESET_SYSTEM);
    wdt_hal_enable(&wdt);
    wdt_hal_write_protect_enable(&wdt);
}

/**
 * Feed the watchdog periodically.
 */
void watchdog_feed()
{
    wdt_hal_write_protect_disable(&wdt);
    wdt_hal_feed(&wdt);
    wdt_hal_write_protect_enable(&wdt);
}
DECL_TASK(watchdog_feed);
