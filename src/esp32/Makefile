export IDF_TARGET=$(subst ",,$(CONFIG_MCU))
export ESPPORT=$(FLASH_DEVICE)

ifndef ESP_IDF_VERSION
	DEVICE_FLAGS=$(if $(ESPPORT), --device "$(ESPPORT):$(ESPPORT)",)
	DOCKER_CMD := docker run \
		--interactive \
		--tty \
		--rm \
		--volume "$(PWD)/$(OUT):/$(OUT)" \
		--volume "$(PWD)/klippy:/klippy" \
		--volume "$(PWD)/scripts:/scripts" \
		--volume "$(PWD)/src:/src" \
		--env ESPPORT \
		--env IDF_TARGET \
		$(DEVICE_FLAGS) \
		espressif/idf:release-v5.5
endif

# Absolute links break inside docker
fix_symlinks: create-board-link
	$(Q)ln -snf ../src/esp32		$(OUT)board
	$(Q)ln -snf ../../src/generic	$(OUT)board-generic/board

$(OUT)CMakeCache.txt: $(OUT)autoconf.h
	$(Q)$(DOCKER_CMD) cmake -B $(OUT) -S src/esp32 -G Ninja

espmenuconfig: $(OUT)CMakeCache.txt fix_symlinks $(OUT)autoconf.h src/esp32/sdkconfig.defaults
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) menuconfig

$(OUT)klipper.elf: $(OUT)CMakeCache.txt fix_symlinks $(OUT)autoconf.h $(shell find src/esp32/ -type f)
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) klipper.elf

flash: $(OUT)klipper.elf
	$(Q)if [ -z $(FLASH_DEVICE) ]; then echo "Please specify FLASH_DEVICE"; exit 1; fi
	$(Q)$(DOCKER_CMD) ninja -C $(OUT) flash
