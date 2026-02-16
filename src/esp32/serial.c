#include "board/irq.h"
#include "autoconf.h"
#include "command.h"
#include "esp_err.h"
#include "serial_irq.h"
#include "driver/uart.h"
#include "hal/uart_ll.h"
#include "soc/interrupts.h"
#include "soc/uart_struct.h"

#define UART_NUM            UART_NUM_2
#define UART_PERIPH_INTR    ETS_UART2_INTR_SOURCE
#define UART_CPU_INTR       2 // Is not related to the above, see esp_cpu_intr.c

#define UART_DEV            (UART_LL_GET_HW(UART_NUM))

/**
 * Write data from klipper's buffer into UART's TXFIFO.
 *
 * This function *DOES NOT* check that there is enough space in the TXFIFO.
 */
static inline __attribute__((always_inline)) void uart_write_tx()
{
    uint8_t buffer;
    while (!serial_get_tx_byte(&buffer)) {
        uart_ll_write_txfifo(UART_DEV, &buffer, sizeof(buffer));
    }
}

/**
 * Interrupt handler for UART.
 *
 * The uart_ll_set_rxfifo_full_thr() is set to 1 so that an interrupt is
 * triggered on each received byte. Process it and pass it along to
 * klipper.
 *
 * There is a possibility of a buffer overflow (when more bytes are
 * received than klipper can handle), but this is already handled by
 * serial_rx_byte().
 *
 * Also, if TXFIFO has become empty, try to send more bytes if available.
 */
static void IRAM_ATTR uart_isr(void *arg)
{
    uint32_t current_mask = uart_ll_get_intsts_mask(UART_DEV);

    if (current_mask & UART_INTR_RXFIFO_FULL) {
        uint32_t length = uart_ll_get_rxfifo_len(UART_DEV);
        uint8_t buffer[length];
        uart_ll_read_rxfifo(UART_DEV, buffer, length);

        for (uint32_t i = 0; i < length; i++) {
            serial_rx_byte(buffer[i]);
        }

        uart_ll_clr_intsts_mask(UART_DEV, UART_INTR_RXFIFO_FULL);
    }

    if (current_mask & UART_INTR_TX_DONE) {
        uart_write_tx();
        uart_ll_clr_intsts_mask(UART_DEV, UART_INTR_TX_DONE);
    }
}

/**
 * Initialize serial port (UART).
 *
 * Set up UART interface without installing ESP-IDF drivers, that way we can
 * directly access the internal FIFO buffers and manage the interrupts without
 * IDF getting in between.
 *
 * @todo check that klipper's queue is smaller than ESP's TXFIFO
 */
void serial_init(void)
{
    #if CONFIG_SERIAL_BAUD == 0
    #   error Invalid baudrate, check CONFIG_SERIAL_BAUD.
    #endif

    ESP_ERROR_CHECK(uart_param_config(
        UART_NUM,
        &(uart_config_t) {
            .baud_rate = CONFIG_SERIAL_BAUD,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_APB,
        }
    ));
    ESP_ERROR_CHECK(uart_set_pin(
        UART_NUM,
        CONFIG_UART_TX_PIN,
        CONFIG_UART_RX_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));

    // Clear FIFOs
    uart_ll_rxfifo_rst(UART_DEV);
    uart_ll_txfifo_rst(UART_DEV);

    // Disable interrupt and clear pending status
    uart_ll_disable_intr_mask(UART_DEV, UART_LL_INTR_MASK);
    uart_ll_clr_intsts_mask(UART_DEV, UART_LL_INTR_MASK);

    // Set TX idle time between transfers
    uart_ll_set_tx_idle_num(UART_DEV, 0);

    // Set RXFIFO_FULL interrupt threshold to 1 => an interrupt is fired on every RX byte.
    uart_ll_set_rxfifo_full_thr(UART_DEV, 1);

    // Configure interrupt
    esp_cpu_intr_set_handler(UART_CPU_INTR, uart_isr, NULL);
    esp_rom_route_intr_matrix(esp_cpu_get_core_id(), UART_PERIPH_INTR, UART_CPU_INTR);

    // Enable RXFIFO_FULL interrupt
    uart_ll_ena_intr_mask(UART_DEV, UART_INTR_RXFIFO_FULL | UART_INTR_TX_DONE);
    esp_intr_enable_source(UART_CPU_INTR);
}
DECL_INIT(serial_init);

/**
 * Send data out.
 *
 * If the transmitter is idle (meaning the buffer is empty) data will be
 * written to it. If it's not, uart_isr will take care of writing data out
 * when there's enough space (that's why the IRQ barrier is needed).
 *
 * As a side note, the naming stems from the AVR architecture.
 */
void serial_enable_tx_irq()
{
    irqstatus_t flag = irq_save();
    if (uart_ll_is_tx_idle(UART_DEV)) {
        uart_write_tx();
    }
    irq_restore(flag);
}
