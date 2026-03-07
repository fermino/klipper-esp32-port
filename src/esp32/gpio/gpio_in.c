#include "gpio/gpio_in.h"
#include "command.h"
#include "driver/gpio.h"
#include "hal/gpio_ll.h"

struct gpio_in gpio_in_setup(gpio_num_t pin, int_fast8_t pull_up)
{
    if (pin >= GPIO_NUM_MAX) {
        shutdown("Input pin outside of range");
    }

    struct gpio_in gpio = { .pin = pin };
    gpio_in_reset(gpio, pull_up);
    return gpio;
}

void gpio_in_reset(struct gpio_in gpio, int_fast8_t pull_up)
{
    gpio_ll_input_enable(GPIO_LL_GET_HW(GPIO_PORT_0), gpio.pin);

    // Todo: check return values
    if (pull_up == 1) {
        gpio_pullup_en(gpio.pin);
    } else {
        gpio_pullup_dis(gpio.pin);
    }

    // Todo: check return values
    if (pull_up == -1) {
        gpio_pulldown_en(gpio.pin);
    } else {
        gpio_pulldown_dis(gpio.pin);
    }

    gpio_ll_output_disable(GPIO_LL_GET_HW(GPIO_PORT_0), gpio.pin);
}
