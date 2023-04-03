/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "py/obj.h"
#include "extmod/machine_i2c.h"
#include "modmachine.h"
#include "peripheral/i2c/master/plib_i2c1_master.h"
#include "definitions.h"

#ifndef MICROPY_HW_I2C0_SCL
#define MICROPY_HW_I2C0_SCL (GPIO_NUM_18)
#define MICROPY_HW_I2C0_SDA (GPIO_NUM_19)
#endif

#ifndef MICROPY_HW_I2C1_SCL
#define MICROPY_HW_I2C1_SCL (GPIO_NUM_9)
#define MICROPY_HW_I2C1_SDA (GPIO_NUM_8)
#endif

#define I2C_DEFAULT_TIMEOUT_US (10000) // 10ms

#define I2C_NUM_MAX 1
#define I2C_NUM_1 1

#define APP_TRANSFER_STATUS_IN_PROGRESS (0)
#define APP_TRANSFER_STATUS_SUCCESS (1)
#define APP_TRANSFER_STATUS_ERROR (2)

typedef struct _machine_i2c_obj_t {
    mp_obj_base_t base;
    int8_t port;
    int8_t scl;
    int8_t sda;
} machine_i2c_obj_t;

STATIC machine_i2c_obj_t machine_hw_i2c_obj[I2C_NUM_MAX];

uint8_t transferStatus;

void APP_I2CCallback(uintptr_t context )
{
    
    if(I2C1_ErrorGet() == I2C_ERROR_NONE)
    {
        transferStatus = APP_TRANSFER_STATUS_SUCCESS;        
    }
    else
    {
        transferStatus = APP_TRANSFER_STATUS_ERROR;
        
    }
}

STATIC void machine_hw_i2c_init(machine_i2c_obj_t *self, uint32_t freq, uint32_t timeout_us) {
    I2C_TRANSFER_SETUP i2c_setup;
    
    i2c_setup.clkSpeed = freq;
    I2C1_TransferSetup(&i2c_setup, 100000000UL);
}

int machine_hw_i2c_transfer(mp_obj_base_t *self_in, uint16_t addr, size_t n, mp_machine_i2c_buf_t *bufs, unsigned int flags) {
    bool res;
    //machine_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);
    
    int data_len = 0;
    for (; n--; ++bufs) {
        if (flags & MP_MACHINE_I2C_FLAG_READ) {
            
            transferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;
            
            while (I2C1_IsBusy());
            res = I2C1_Read(addr, bufs->buf, bufs->len);
            if (!res)
                SYS_CONSOLE_PRINT("i2c read, fail\r\n");
            
            while (transferStatus == APP_TRANSFER_STATUS_IN_PROGRESS)
            {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            
            if (transferStatus != APP_TRANSFER_STATUS_SUCCESS)
                mp_raise_ValueError(MP_ERROR_TEXT("fail to read I2C data"));
#ifdef PIC32MZW1_DEBUG           
            SYS_CONSOLE_PRINT("i2c read, data = 0x%x\r\n", bufs->buf[0]);
#endif
        }
        else{
#ifdef PIC32MZW1_DEBUG            
            SYS_CONSOLE_PRINT("i2c write, len = %d data=0x%x\r\n", bufs->len, bufs->buf[0]);
#endif
            transferStatus = APP_TRANSFER_STATUS_IN_PROGRESS;

            while (I2C1_IsBusy());
            res = I2C1_Write(addr, bufs->buf, bufs->len);
            if (!res)
                SYS_CONSOLE_PRINT("i2c write, fail\r\n");
            
            while (transferStatus == APP_TRANSFER_STATUS_IN_PROGRESS)
            {
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }
            if (transferStatus != APP_TRANSFER_STATUS_SUCCESS)
                mp_raise_ValueError(MP_ERROR_TEXT("fail to write I2C data"));

        }
                   
        data_len += bufs->len;
    }
    
    if (flags & MP_MACHINE_I2C_FLAG_STOP) {
        //i2c_master_stop(cmd);
    }
    
    return data_len;
}

/******************************************************************************/
// MicroPython bindings for machine API

STATIC void machine_hw_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    //machine_i2c_obj_t *self = MP_OBJ_TO_PTR(self_in);

}

mp_obj_t machine_hw_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    //MP_MACHINE_I2C_CHECK_FOR_LEGACY_SOFTI2C_CONSTRUCTION(n_args, n_kw, all_args);

    // Parse args
    enum { ARG_id, ARG_scl, ARG_sda, ARG_freq, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_scl, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_sda, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_freq, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 50000} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = I2C_DEFAULT_TIMEOUT_US} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Get I2C bus
    mp_int_t i2c_id = mp_obj_get_int(args[ARG_id].u_obj);
    if (i2c_id > I2C_NUM_MAX) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("I2C(%d) doesn't exist"), i2c_id);
    }

    // Get static peripheral object
    machine_i2c_obj_t *self = (machine_i2c_obj_t *)&machine_hw_i2c_obj[i2c_id-1];

    if (self->base.type == NULL) {
        // Created for the first time, set default pins
        self->base.type = &machine_i2c_type;
        self->port = i2c_id;
        if (self->port == I2C_NUM_1) {
            self->scl = 33;
            self->sda = 32;
        } else {
            self->scl = -1;
            self->sda = -1;
        }
    }

    
    
    // Initialise the I2C peripheral
    machine_hw_i2c_init(self, args[ARG_freq].u_int, args[ARG_timeout].u_int);
#ifdef PIC32MZW1_DEBUG    
    SYS_CONSOLE_PRINT("Register I2C1_CallbackRegister\r\n");
#endif
    I2C1_CallbackRegister( APP_I2CCallback, NULL );

    return MP_OBJ_FROM_PTR(self);

}

STATIC const mp_machine_i2c_p_t machine_hw_i2c_p = {
    .transfer = machine_hw_i2c_transfer,
};

const mp_obj_type_t machine_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .print = machine_hw_i2c_print,
    .make_new = machine_hw_i2c_make_new,
    .protocol = &machine_hw_i2c_p,
    .locals_dict = (mp_obj_dict_t *)&mp_machine_i2c_locals_dict,
};
