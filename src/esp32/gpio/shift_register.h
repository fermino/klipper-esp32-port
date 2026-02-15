#pragma once

#include "gpio/gpio.h"
#include "autoconf.h"
#include "command.h"
#include <stdbool.h>
#include <stdint.h>
#include "xtensa/core-macros.h"
#include "esp_rom_sys.h"
#include "hal/spi_ll.h"

#if CONFIG_HAVE_GPIO_SR

#define SR_SPI_HOST             (SPI2_HOST)
#define SR_BIT_NO               (CONFIG_SR_BYTE_NO * 8)
#define SR_MAX_WAIT_FOR_IDLE_US (5)

// @todo Critical section? noirq?
static inline void __attribute__((always_inline)) gpio_sr_shift_out()
{
    extern volatile uint8_t sr_data[CONFIG_SR_BYTE_NO];

    uint8_t local_buffer[CONFIG_SR_BYTE_NO];

    for (uint8_t i = 0; i < CONFIG_SR_BYTE_NO; i++) {
        local_buffer[i] = sr_data[CONFIG_SR_BYTE_NO - 1 - i];
    }

    uint32_t start = XTHAL_GET_CCOUNT();
    while (HAL_FORCE_READ_U32_REG_FIELD(SPI_LL_GET_HW(SR_SPI_HOST)->cmd, usr)) {
        if (XTHAL_GET_CCOUNT() - start >= SR_MAX_WAIT_FOR_IDLE_US * esp_rom_get_cpu_ticks_per_us()) {
            try_shutdown("SPI transaction took too long.");
            return;
        }
    }

    spi_ll_write_buffer(SPI_LL_GET_HW(SR_SPI_HOST), local_buffer, SR_BIT_NO);
    spi_ll_user_start(SPI_LL_GET_HW(SR_SPI_HOST));
}

/**
 * This function should be optimized out if the feature is
 * disabled (CONFIG_HAVE_GPIO_SR == false)
 */
static inline bool __attribute__((always_inline)) gpio_is_sr(struct gpio_out gpio)
{
    return CONFIG_HAVE_GPIO_SR && (gpio.pin & 0b10000000) && (gpio.pin & 0b01111111) < SR_BIT_NO;
}

static inline uint8_t __attribute__((always_inline)) gpio_sr_bit_index(struct gpio_out gpio)
{
    return gpio.pin & 0b111;
}

static inline uint8_t __attribute__((always_inline)) gpio_sr_byte_index(struct gpio_out gpio)
{
    return (gpio.pin & 0b01111000) >> 3;
}

static inline void __attribute__((always_inline)) gpio_sr_write(struct gpio_out gpio, bool val)
{
    extern volatile uint8_t sr_data[CONFIG_SR_BYTE_NO];

    if (val) {
        sr_data[gpio_sr_byte_index(gpio)] |= 1 << gpio_sr_bit_index(gpio);
    } else {
        sr_data[gpio_sr_byte_index(gpio)] &= ~(1 << gpio_sr_bit_index(gpio));
    }

    gpio_sr_shift_out();
}

static inline bool __attribute__((always_inline)) gpio_sr_read(struct gpio_out gpio)
{
    extern volatile uint8_t sr_data[CONFIG_SR_BYTE_NO];

    return (bool)(sr_data[gpio_sr_byte_index(gpio)] & (1 << gpio_sr_bit_index(gpio)));
}

#endif
