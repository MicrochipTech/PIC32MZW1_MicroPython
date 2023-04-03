
//#include "pic32mzw1_gpio.h
//#include "pic32mzw1_adc.h"
#include "definitions.h"
#include "peripheral/adchs/plib_adchs.h"

#define ADC_VREF                (3.3f)
#define ADC_MAX_COUNT           (4095)

void adc_set_pin(int pin_id)
{
    
    //ADCCON1bits.ON = 0;  
	switch (pin_id)
    {
        case 44:    /*AN14*/
            ADCCSS1 |= 0x4000;
            break;
        case 42:    /*AN15*/
            ADCCSS1 |= 0x8000;
            break;
        case 41:    /*AN17*/
            ADCCSS1 |= 0x20000;
            break;
        case 35:    /*AN18*/
            ADCCSS1 |= 0x40000;
            break;
        case 46:    /*AN6*/
            ADCTRG2 |= 0x50000;
            ADCCSS1 |= 0x40;
            break;
        case 49:    /*AN7*/
            ADCTRG2 |= 0x5000000;
            ADCCSS1 |= 0x80;
            break;
        case 47:    /*AN8*/
            ADCCSS1 |= 0x100;
            break;
        case 48:    /*AN9*/
            ADCCSS1 |= 0x200;
            break;
        
        default:
            break;
    }  
    

#if 0
     
    /* Turn ON ADC */
    ADCCON1bits.ON = 1;
    while(!ADCCON2bits.BGVRRDY); // Wait until the reference voltage is ready
    while(ADCCON2bits.REFFLT); // Wait if there is a fault with the reference voltage

    /* ADC 7 */
    ADCANCONbits.ANEN7 = 1;      // Enable the clock to analog bias
    while(!ADCANCONbits.WKRDY7); // Wait until ADC is ready
    ADCCON3bits.DIGEN7 = 1;      // Enable ADC
#endif
}

uint16_t adc_read(ADCHS_CHANNEL_NUM chann_num)
{
    uint16_t adc_count;
    
    while(!ADCHS_ChannelResultIsReady(chann_num)){};
    
    adc_count = ADCHS_ChannelResultGet(chann_num);
    
    return adc_count;
}

uint32_t adc_read_voltage(ADCHS_CHANNEL_NUM chann_num)
{
    uint16_t adc_count;
    float input_voltage;
    
    while(!ADCHS_ChannelResultIsReady(chann_num)){};
    
    adc_count = ADCHS_ChannelResultGet(chann_num);
    
    input_voltage = adc_count * ADC_VREF / ADC_MAX_COUNT * 1000000;
    
    return input_voltage;
}
