// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "sdkconfig.h"

#if (!defined(CONFIG_FREERTOS_UNICORE) || !CONFIG_FREERTOS_UNICORE)
#    error Klipper needs CONFIG_FREERTOS_UNICORE enabled
#endif

void app_main();
