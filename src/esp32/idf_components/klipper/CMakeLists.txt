#[[
    Register klipper as an IDF component (everything except the CTR).

    This is the bread and butter of the project and when adding features
    you will be likely working here.
]]

idf_component_register(
    SRCS
        # Core
        ${KLIPPER_ROOT}/src/esp32/main.c
        ${KLIPPER_ROOT}/src/esp32/timer.c
        ${KLIPPER_ROOT}/src/esp32/watchdog.c
        ${KLIPPER_ROOT}/src/esp32/serial.c

        # Others
        ${KLIPPER_ROOT}/src/adccmds.c
        ${KLIPPER_ROOT}/src/basecmd.c
        ${KLIPPER_ROOT}/src/buttons.c
        ${KLIPPER_ROOT}/src/command.c
        ${KLIPPER_ROOT}/src/debugcmds.c
        ${KLIPPER_ROOT}/src/endstop.c
        ${KLIPPER_ROOT}/src/esp32/gpio/gpio.c
        ${KLIPPER_ROOT}/src/esp32/gpio/gpio_adc.c
        ${KLIPPER_ROOT}/src/esp32/gpio/gpio_in.c
        ${KLIPPER_ROOT}/src/esp32/gpio/gpio_out.c
        ${KLIPPER_ROOT}/src/esp32/gpio/gpio_pwm.c
        ${KLIPPER_ROOT}/src/esp32/gpio/shift_register.c
        ${KLIPPER_ROOT}/src/generic/alloc.c
        ${KLIPPER_ROOT}/src/generic/crc16_ccitt.c
        ${KLIPPER_ROOT}/src/generic/serial_irq.c
        ${KLIPPER_ROOT}/src/generic/timer_irq.c
        ${KLIPPER_ROOT}/src/gpiocmds.c
        ${KLIPPER_ROOT}/src/initial_pins.c
        ${KLIPPER_ROOT}/src/pwmcmds.c
        ${KLIPPER_ROOT}/src/sched.c
        ${KLIPPER_ROOT}/src/stepper.c
        ${KLIPPER_ROOT}/src/trsync.c

        #    ${KLIPPER_ROOT}/src/i2ccmds.c
        #    ${KLIPPER_ROOT}/src/i2c_software.c
        #    ${KLIPPER_ROOT}/src/lcd_hd44780.c
        #    ${KLIPPER_ROOT}/src/lcd_st7920.c
        #    ${KLIPPER_ROOT}/src/neopixel.c
        #    ${KLIPPER_ROOT}/src/pulse_counter.c
        #    ${KLIPPER_ROOT}/src/sensor_adxl345.c
        #    ${KLIPPER_ROOT}/src/sensor_angle.c
        #    ${KLIPPER_ROOT}/src/sensor_bulk.c
        #    ${KLIPPER_ROOT}/src/sensor_ldc1612.c
        #    ${KLIPPER_ROOT}/src/sensor_lis2dw.c
        #    ${KLIPPER_ROOT}/src/sensor_mpu9250.c
        #    ${KLIPPER_ROOT}/src/spicmds.c
        #    ${KLIPPER_ROOT}/src/spi_software.c
        #    ${KLIPPER_ROOT}/src/thermocouple.c
        #    ${KLIPPER_ROOT}/src/tmcuart.c
    INCLUDE_DIRS
        ${KLIPPER_ROOT}/src
        ${KLIPPER_ROOT}/src/esp32
        ${KLIPPER_ROOT}/src/generic
        ${KLIPPER_ROOT}/out

    # Include the required ESP components here. To reduce build times no
    # component will be automatically added besides sub-dependencies.
    PRIV_REQUIRES
        esp_adc
        esp_driver_gpio
        esp_driver_ledc
        esp_driver_spi
        esp_driver_uart

    # Map the whole app to IRAM on startup to avoid possible flash cache misses
    # that would introduce jitter. You can verify this by by running
    # `idf.py size` and making sure IRAM and Flash Code `.text` sections match
    # with the smallest difference possible (usually around 1KiB).
    LDFRAGMENTS "noflash.lf"
)
