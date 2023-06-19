
#include "pic32mzw1_gpio.h"
#include "definitions.h"


void gpio_set_dir(GPIO_PORT port, int pin, PIC32MZW1_GPIO_DIR dir)
{
	if (dir == PIC32MZW1_GPIO_IN)
		GPIO_PortInputEnable(port, 1<<pin);
	else
		GPIO_PortOutputEnable(port, 1<<pin);
}

void gpio_put(GPIO_PORT port, int pin, int val)
{
    if (val == 0)
        GPIO_PortClear(port, 1<<pin);
    else
        GPIO_PortSet(port, 1<<pin);

}

unsigned int gpio_get_level(GPIO_PORT port, int pin)
{
    uint32_t port_val;
    
    port_val = GPIO_PortRead(port);
    
    return ((port_val >> pin) & 0x1);
            
}

unsigned int gpio_set_irq(int pin, PIC32MZW1_INT_CHAN irq_chan, int irq_id, PIC32MZW1_IRQ_TRIGGER trigger, void* handler)
{
    //PIC32MZW1_INT_CHAN * pirq_chan = NULL;
    //pirq_chan = malloc(sizeof(PIC32MZW1_INT_CHAN));
    //*pirq_chan = irq_chan;
        
    /* Unlock system for PPS configuration */
    //SYSKEY = 0x00000000;
    //SYSKEY = 0xAA996655;
    //SYSKEY = 0x556699AA;
    
    CFGCON0bits.IOLOCK = 0;
    //INT2R = 10;
    switch (irq_chan)
    {
        case PIC32MZW1_INT_CHANN_0:
            INT0R = irq_id;
            if (trigger == PIC32MZW1_IRQ_RISING_TRIGGER)
                INTCONSET |= _INTCON_INT0EP_MASK;
            EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_0, handler, (uintptr_t) NULL); // need correct to EXTERNAL_INT_0
            break;
        
        case PIC32MZW1_INT_CHANN_1:
            INT1R = irq_id;
            if (trigger == PIC32MZW1_IRQ_RISING_TRIGGER)
                INTCONSET |= _INTCON_INT1EP_MASK;
            EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_1, handler, (uintptr_t) NULL); // need correct to EXTERNAL_INT_1
            break;
        case PIC32MZW1_INT_CHANN_2:
            INT2R = irq_id;
            if (trigger == PIC32MZW1_IRQ_RISING_TRIGGER)
                INTCONSET |= _INTCON_INT2EP_MASK;
            
            //printf("[%s] log1\r\n", __func__);
            EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_2, handler, (uintptr_t) NULL);
            break;
        case PIC32MZW1_INT_CHANN_3:
            INT3R = irq_id;
            if (trigger == PIC32MZW1_IRQ_RISING_TRIGGER)
                INTCONSET |= _INTCON_INT3EP_MASK;
            
            EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_3, handler, (uintptr_t) NULL);
            break;
        case PIC32MZW1_INT_CHANN_4:
            INT4R = irq_id;
            if (trigger == PIC32MZW1_IRQ_RISING_TRIGGER)
                INTCONSET |= _INTCON_INT4EP_MASK;
            
            EVIC_ExternalInterruptCallbackRegister(EXTERNAL_INT_4, handler, (uintptr_t) NULL);
            break;
        default:
            break;
    }

    /* Lock back the system after PPS configuration */
    //CFGCON0bits.IOLOCK = 1;
    
    //SYSKEY = 0x00000000;
    
    /* Configure External Interrupt Edge Polarity */
    
  
    return 0;
            
}


void gpio_irq_enable( PIC32MZW1_INT_CHAN irq_chan )
{
    EXTERNAL_INT_PIN    irq_num;
    
    switch (irq_chan)
    {
        case PIC32MZW1_INT_CHANN_0:
            irq_num = EXTERNAL_INT_0;
            break;
        case PIC32MZW1_INT_CHANN_1:
            irq_num = EXTERNAL_INT_1;
            break;
        case PIC32MZW1_INT_CHANN_2:
            irq_num = EXTERNAL_INT_2;
            break;
        case PIC32MZW1_INT_CHANN_3:
            irq_num = EXTERNAL_INT_3;
            break;
        case PIC32MZW1_INT_CHANN_4:
            irq_num = EXTERNAL_INT_4;
            break;
        default:
            irq_num = EXTERNAL_INT_0;
            break;
    }
    
    EVIC_ExternalInterruptEnable(irq_num);
}

void gpio_irq_disable( PIC32MZW1_INT_CHAN irq_chan )
{
    EXTERNAL_INT_PIN    irq_num;
    
    switch (irq_chan)
    {
        case PIC32MZW1_INT_CHANN_0:
            irq_num = EXTERNAL_INT_0;
            break;
        case PIC32MZW1_INT_CHANN_1:
            irq_num = EXTERNAL_INT_1;
            break;
        case PIC32MZW1_INT_CHANN_2:
            irq_num = EXTERNAL_INT_2;
            break;
        case PIC32MZW1_INT_CHANN_3:
            irq_num = EXTERNAL_INT_3;
            break;
        case PIC32MZW1_INT_CHANN_4:
            irq_num = EXTERNAL_INT_4;
            break;
        default:
            irq_num = EXTERNAL_INT_0;
            break;
    }
    
    EVIC_ExternalInterruptDisable(irq_num);
}

#define MAX_PIN_NUM 52
bool gpio_inuse[MAX_PIN_NUM] = {0};
bool gpio_check_available( int pin )
{
    return gpio_inuse[pin-1] ? false: true;
}