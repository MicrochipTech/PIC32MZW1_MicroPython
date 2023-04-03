
#include "peripheral/gpio/plib_gpio.h"
#include "peripheral/evic/plib_evic.h"

typedef enum
{
    PIC32MZW1_GPIO_IN = 0,
    PIC32MZW1_GPIO_OUT = 1,
} PIC32MZW1_GPIO_DIR;

typedef enum
{
    PIC32MZW1_IRQ_FALLING_TRIGGER = 0,
    PIC32MZW1_IRQ_RISING_TRIGGER = 1,    
} PIC32MZW1_IRQ_TRIGGER;

typedef enum
{
    PIC32MZW1_INT_CHANN_0 = 0,
    PIC32MZW1_INT_CHANN_1 = 1,
    PIC32MZW1_INT_CHANN_2 = 2,
    PIC32MZW1_INT_CHANN_3 = 3,
    PIC32MZW1_INT_CHANN_4 = 4,
} PIC32MZW1_INT_CHAN;

void gpio_set_dir(GPIO_PORT port, int pin, PIC32MZW1_GPIO_DIR dir);