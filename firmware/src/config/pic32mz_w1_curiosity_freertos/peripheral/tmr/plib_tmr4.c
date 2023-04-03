/*******************************************************************************
  TMR Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_tmr4.c

  Summary
    TMR4 peripheral library source file.

  Description
    This file implements the interface to the TMR peripheral library.  This
    library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "device.h"
#include "plib_tmr4.h"


static TMR_TIMER_OBJECT tmr4Obj;


void TMR4_Initialize(void)
{
    /* Disable Timer */
    T4CONCLR = _T4CON_ON_MASK;

    /*
    SIDL = 0
    TCKPS =0
    T32   = 1
    TCS = 0
    */
    T4CONSET = 0x8;

    /* Clear counter */
    TMR4 = 0x0;

    /*Set period */
    PR4 = 49999998U;

    /* Enable TMR Interrupt of odd numbered timer in 32-bit mode */
    IEC0SET = _IEC0_T5IE_MASK;

}


void TMR4_Start(void)
{
    T4CONSET = _T4CON_ON_MASK;
}


void TMR4_Stop (void)
{
    T4CONCLR = _T4CON_ON_MASK;
}

void TMR4_PeriodSet(uint32_t period)
{
    PR4  = period;
}

uint32_t TMR4_PeriodGet(void)
{
    return PR4;
}

uint32_t TMR4_CounterGet(void)
{
    return (TMR4);
}


uint32_t TMR4_FrequencyGet(void)
{
    return (100000000);
}


void TIMER_5_InterruptHandler (void)
{
    uint32_t status  = 0U;
    status = IFS0bits.T5IF;
    IFS0CLR = _IFS0_T5IF_MASK;

    if((tmr4Obj.callback_fn != NULL))
    {
        tmr4Obj.callback_fn(status, tmr4Obj.context);
    }
}


void TMR4_InterruptEnable(void)
{
    IEC0SET = _IEC0_T5IE_MASK;
}


void TMR4_InterruptDisable(void)
{
    IEC0CLR = _IEC0_T5IE_MASK;
}


void TMR4_CallbackRegister( TMR_CALLBACK callback_fn, uintptr_t context )
{
    /* Save callback_fn and context in local memory */
    tmr4Obj.callback_fn = callback_fn;
    tmr4Obj.context = context;
}
