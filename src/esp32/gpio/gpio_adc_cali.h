#pragma once

#include <stdint.h>
#include <hal/adc_types.h>

void adc_cali_init();
uint16_t adc_cali_map(adc_unit_t unit, adc_channel_t channel, uint16_t value);
