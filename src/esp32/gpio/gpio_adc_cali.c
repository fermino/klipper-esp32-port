#include "gpio/gpio_adc_cali.h"
#include "gpio/gpio_adc.h"
#include "command.h"
#include "esp_adc/adc_cali_scheme.h"
#include "adc_cali_interface.h"

/**
 * Calibration handles. Curve fitting MAY have per-channel calibration data,
 * all other methods are unit only.
 */
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
#   error ADC calibration through curve fitting not yet implemented.
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    static adc_cali_handle_t adc_cali_handles[SOC_ADC_PERIPH_NUM];
#else
#   error ADC calibration is not supported.
#endif

void adc_cali_init()
{
    for (size_t unit = 0; unit < SOC_ADC_PERIPH_NUM; unit++) {
        adc_cali_line_fitting_config_t config = {
            .unit_id = unit,
            .atten = ADC_ATTEN,
            .bitwidth = ADC_BITWIDTH,
        };
        if (adc_cali_create_scheme_line_fitting(&config, &adc_cali_handles[unit]) != ESP_OK) {
            shutdown("ADC calibration failed.");
        }
    }
}

uint16_t adc_cali_map(adc_unit_t unit, adc_channel_t channel, uint16_t value)
{
    int mapped;
    if (adc_cali_handles[unit]->raw_to_voltage(adc_cali_handles[unit]->ctx, (int)value, &mapped) != ESP_OK) {
        shutdown("ADC calibrated mapping failed.");
    }
    return mapped;
}
