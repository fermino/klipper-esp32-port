export IDF_TARGET=$(patsubst "%",%,$(CONFIG_MCU))
export ESPPORT=$(FLASH_DEVICE)

ifndef ESP_IDF_VERSION
	DEVICE_FLAGS=$(if $(ESPPORT), --device "$(ESPPORT):$(ESPPORT)",)
	# Run as current user to avoid permission issues with the directories
	DOCKER_CMD := docker run \
		--interactive \
		--tty \
		--rm \
		--user "$(shell id -u):$(shell id -g)" \
		--volume "$(PWD)/$(OUT):/$(OUT)" \
		--volume "$(PWD)/klippy:/klippy" \
		--volume "$(PWD)/scripts:/scripts" \
		--volume "$(PWD)/src:/src" \
		--env ESPPORT \
		--env IDF_TARGET \
		--env ESPIDF_SOURCES \
		--env ESPIDF_INCLUDE_DIRS \
		--env ESPIDF_COMPONENTS \
		$(DEVICE_FLAGS) \
		espressif/idf:release-v5.5
endif

# Core
src-y += esp32/main.c
src-y += esp32/timer.c
src-y += esp32/watchdog.c
src-y += generic/alloc.c
src-y += generic/crc16_ccitt.c
src-y += generic/timer_irq.c
# Features
src-$(CONFIG_SERIAL) += esp32/serial.c
src-$(CONFIG_SERIAL) += generic/serial_irq.c
src-$(CONFIG_HAVE_GPIO) += esp32/gpio/gpio.c
src-$(CONFIG_HAVE_GPIO) += esp32/gpio/gpio_in.c
src-$(CONFIG_HAVE_GPIO) += esp32/gpio/gpio_out.c
src-$(CONFIG_HAVE_GPIO) += esp32/gpio/shift_register.c # Todo: move to a separated feature
src-$(CONFIG_WANT_ADC) += esp32/gpio/gpio_adc.c
src-$(CONFIG_WANT_ADC) += esp32/gpio/gpio_adc_cali.c
src-$(CONFIG_WANT_HARD_PWM) += esp32/gpio/gpio_pwm.c

export ESPIDF_SOURCES=$(src-y)

ESPIDF_INCLUDE_DIRS += out
ESPIDF_INCLUDE_DIRS += src
ESPIDF_INCLUDE_DIRS += src/esp32

export ESPIDF_INCLUDE_DIRS

ESPIDF_COMPONENTS += esp_adc
ESPIDF_COMPONENTS += esp_driver_gpio
ESPIDF_COMPONENTS += esp_driver_ledc
ESPIDF_COMPONENTS += esp_driver_spi
ESPIDF_COMPONENTS += esp_driver_uart
export ESPIDF_COMPONENTS

# Absolute links break inside docker
fix_symlinks: create-board-link
	$(Q)ln -snf ../src/esp32 $(OUT)board
	rm -fv $(OUT)board-generic/board

$(OUT)CMakeCache.txt: $(OUT)autoconf.h
	$(Q)$(DOCKER_CMD) cmake -B $(OUT) -S src/esp32 -G Ninja

espmenuconfig: $(OUT)CMakeCache.txt fix_symlinks $(OUT)autoconf.h src/esp32/sdkconfig.defaults
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) menuconfig

SKIP_KLIPPERELF_RECIPE := true
$(OUT)klipper.elf: $(OUT)CMakeCache.txt fix_symlinks $(OUT)autoconf.h $(shell find src/esp32/ -type f)
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) klipper.elf

flash: $(OUT)klipper.elf
	$(Q)if [ -z $(FLASH_DEVICE) ]; then echo "Please specify FLASH_DEVICE"; exit 1; fi
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) flash
