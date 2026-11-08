// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "soc/gpio_num.h"

struct gpio { gpio_num_t pin; };
#define gpio_in gpio
#define gpio_out gpio
