
#include "peripheral/adchs/plib_adchs.h"

void adc_set_pin(int pin_id);
uint16_t adc_read(ADCHS_CHANNEL_NUM chann_num);
uint32_t adc_read_voltage(ADCHS_CHANNEL_NUM chann_num);
