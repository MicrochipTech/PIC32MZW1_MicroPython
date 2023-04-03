/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/stream.h"
#include "py/mperrno.h"
#include "modmachine.h"
#include "peripheral/uart/plib_uart_common.h"
#include "peripheral/uart/plib_uart2.h"
#include "definitions.h"

#define UART_NUM_MAX 3
#define UART_NUM_1 1
#define UART_NUM_2 2
#define UART_NUM_3 3

#define UART_PIN_NOT_ASSIGN -1
#define UART_PIN_NO_CHANGE -1
// need to correct
#define UART_INV_TX 1
#define UART_INV_RX 1
#define UART_INV_RTS 1
#define UART_INV_CTS 1
#define UART_HW_FLOWCTRL_RTS 1
#define UART_HW_FLOWCTRL_CTS 1


#define UART_INV_MASK (UART_INV_TX | UART_INV_RX | UART_INV_RTS | UART_INV_CTS)

typedef struct _machine_uart_obj_t {
    mp_obj_base_t base;
    int uart_num;
    uint8_t bits;
    uint8_t parity;
    uint8_t stop;
    int8_t tx;
    int8_t rx;
    int8_t rts;
    int8_t cts;
    uint16_t txbuf;
    uint16_t rxbuf;
    uint16_t timeout;       // timeout waiting for first char (in ms)
    uint16_t timeout_char;  // timeout waiting between chars (in ms)
    uint32_t invert;        // lines to invert
    bool flowcontrol;
} machine_uart_obj_t;

STATIC const char *_parity_name[] = {"None", "1", "0"};
static uint32_t baudrate = 115200;

/******************************************************************************/
// MicroPython bindings for UART


STATIC void machine_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_printf(print, "UART(%u, baudrate=%u, bits=%u, parity=%s, stop=%u, tx=%d, rx=%d, rts=%d, cts=%d, txbuf=%u, rxbuf=%u, timeout=%u, timeout_char=%u",
        self->uart_num, baudrate, self->bits, _parity_name[self->parity],
        self->stop, self->tx, self->rx, self->rts, self->cts, self->txbuf, self->rxbuf, self->timeout, self->timeout_char);
}

STATIC void machine_uart_init_helper(machine_uart_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_baudrate, ARG_bits, ARG_parity, ARG_stop, ARG_tx, ARG_rx, ARG_rts, ARG_cts, ARG_txbuf, ARG_rxbuf, ARG_timeout, ARG_timeout_char, ARG_invert, ARG_flow };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_parity, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
        { MP_QSTR_stop, MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_tx, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_PIN_NO_CHANGE} },
        { MP_QSTR_rx, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_PIN_NO_CHANGE} },
        { MP_QSTR_rts, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_PIN_NO_CHANGE} },
        { MP_QSTR_cts, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = UART_PIN_NO_CHANGE} },
        { MP_QSTR_txbuf, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_rxbuf, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_timeout_char, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_invert, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_flow, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // wait for all data to be transmitted before changing settings
    //uart_wait_tx_done(self->uart_num, pdMS_TO_TICKS(1000));

    if (args[ARG_txbuf].u_int >= 0 || args[ARG_rxbuf].u_int >= 0) {
        // must reinitialise driver to change the tx/rx buffer size
        if (self->uart_num == UART_NUM_2) {
            mp_raise_ValueError(MP_ERROR_TEXT("No need to set buffer size for UART2"));
        }

        if (args[ARG_txbuf].u_int >= 0) {
            self->txbuf = args[ARG_txbuf].u_int;
        }
        if (args[ARG_rxbuf].u_int >= 0) {
            self->rxbuf = args[ARG_rxbuf].u_int;
        }
    }

    UART_SERIAL_SETUP uart2_setup;
    
    uart2_setup.baudRate = args[ARG_baudrate].u_int;
    baudrate = uart2_setup.baudRate;

    // set data bits
    switch (args[ARG_bits].u_int) {
        case 0:
            break;
        case 8:
            
            self->bits = 8;
            break;
        case 9:
            
            self->bits = 9;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid data bits"));
            break;
    }
    if (self->bits == 8)
        uart2_setup.dataWidth = UART_DATA_8_BIT;
    else
        uart2_setup.dataWidth = UART_DATA_9_BIT;
    
    // set parity
    if (args[ARG_parity].u_obj != MP_OBJ_NULL) {
        if (args[ARG_parity].u_obj == mp_const_none) {
            
            self->parity = 0;
        } else {
            mp_int_t parity = mp_obj_get_int(args[ARG_parity].u_obj);
            if (parity & 1) {
                uart2_setup.parity = UART_PARITY_ODD;
                self->parity = 1;
            } else {
                uart2_setup.parity = UART_PARITY_EVEN;
                self->parity = 2;
            }
        }
    }
    if (self->parity == 0)
        uart2_setup.parity = UART_PARITY_NONE;
    else if (self->parity == 1)
        uart2_setup.parity = UART_PARITY_ODD;
    else
        uart2_setup.parity = UART_PARITY_EVEN;
    
    // set stop bits
    switch (args[ARG_stop].u_int) {
        case 0:
            break;
        case 1:          
            self->stop = 1;
            break;
        case 2:           
            self->stop = 2;
            break;
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid stop bits"));
            break;
    }
    if (self->stop == 1)
        uart2_setup.stopBits = UART_STOP_1_BIT;
    else
        uart2_setup.stopBits = UART_STOP_2_BIT;
  
    
    UART2_SerialSetup(&uart2_setup, 0);
            
            

    if ((args[ARG_tx].u_int != UART_PIN_NO_CHANGE) && (self->tx != args[ARG_tx].u_int)) {

        int8_t org_tx = self->tx;
        
        switch (args[ARG_tx].u_int)
        {
            case 49:
                RPB7R = 2; 
                self->tx = args[ARG_tx].u_int;
                break;
            case 35:
                RPK3R = 2;
                self->tx = args[ARG_tx].u_int;
                break;
            case 28:
                RPK7R = 2;
                self->tx = args[ARG_tx].u_int;
                break;
            case 12:
                RPC12R = 2;
                self->tx = args[ARG_tx].u_int;
                break;
            default:
                org_tx = -1;
                mp_raise_ValueError(MP_ERROR_TEXT("invalid tx pin"));
                break;
        }
        
        switch (org_tx)
        {
            case 49:
                RPB7R = 0; 
                break;
            case 35:
                RPK3R = 0;
                break;
            case 28:
                RPK7R = 0;
                break;
            case 12:
                RPC12R = 0;
                break;
            default:
                break;
        }
  
    }

     if ((args[ARG_rx].u_int != UART_PIN_NO_CHANGE) && (self->rx != args[ARG_rx].u_int)) {
        SYS_CONSOLE_PRINT("set rx, log1\r\n");
        switch (args[ARG_rx].u_int)
        {
            case 42:
                U2RXR = 2;
                self->rx = args[ARG_rx].u_int;
                break; 
            case 48:
                U2RXR = 5;
                self->rx = args[ARG_rx].u_int;
                break;
            case 10:
                U2RXR = 9;
                self->rx = args[ARG_rx].u_int;
                break;
            case 34:
                U2RXR = 11;
                self->rx = args[ARG_rx].u_int;
                break;
            case 16:
                U2RXR = 14;
                self->rx = args[ARG_rx].u_int;
                break;
            default:
                mp_raise_ValueError(MP_ERROR_TEXT("invalid rx pin"));
                break;
        }
 
    }

    if (args[ARG_rts].u_int != UART_PIN_NO_CHANGE) {
        mp_printf(&mp_plat_print, "option 'rts' is not supported\r\n");
    }

    if (args[ARG_cts].u_int != UART_PIN_NO_CHANGE) {
        mp_printf(&mp_plat_print, "option 'cts' is not supported\r\n");
    }

    
    // set timeout
    if (args[ARG_timeout].u_int != -1) {
        self->timeout = args[ARG_timeout].u_int;
    }

    // set timeout_char
    // make sure it is at least as long as a whole character (13 bits to be safe)
    if (args[ARG_timeout_char].u_int != -1) {
        self->timeout_char = args[ARG_timeout_char].u_int;
    }

    // set line inversion
    if (args[ARG_invert].u_int != -1) {
        mp_printf(&mp_plat_print, "option 'invert' is not supported\r\n");
    }

    // set hardware flow control
    if (args[ARG_flow].u_int != -1) {
        mp_printf(&mp_plat_print, "option 'flowcontrol' is not supported\r\n");
    }

}

STATIC mp_obj_t machine_uart_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get uart id
    mp_int_t uart_num = mp_obj_get_int(args[0]);
    if (uart_num < 1 || uart_num >= UART_NUM_MAX) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("UART(%d) does not exist"), uart_num);
    }
    else if (uart_num == 1 || uart_num == 3)
    {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("UART(%d) is used for device console"), uart_num);
    }
    
    // create instance
    machine_uart_obj_t *self = m_new_obj(machine_uart_obj_t);
    self->base.type = &machine_uart_type;
    self->uart_num = uart_num;
    self->bits = 8;
    self->parity = 0;
    self->stop = 1;
    self->rts = UART_PIN_NOT_ASSIGN;
    self->cts = UART_PIN_NOT_ASSIGN;
    self->txbuf = 256;
    self->rxbuf = 256; // IDF minimum
    self->timeout = 0;
    self->timeout_char = 0;
    self->invert = 0;
    self->flowcontrol = false;

    switch (uart_num) {
        case UART_NUM_2:
            self->rx = 48;
            self->tx = 28;
            break;
        default:
            
            break;
    }

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
    machine_uart_init_helper(self, n_args - 1, args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);

}

STATIC mp_obj_t machine_uart_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    machine_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_uart_init_obj, 1, machine_uart_init);

STATIC mp_obj_t machine_uart_deinit(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_deinit_obj, machine_uart_deinit);

STATIC mp_obj_t machine_uart_any(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t rxbufsize;
    rxbufsize = UART2_ReadCountGet();
    return MP_OBJ_NEW_SMALL_INT(rxbufsize);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_any_obj, machine_uart_any);

STATIC mp_obj_t machine_uart_sendbreak(mp_obj_t self_in) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    
    mp_printf(&mp_plat_print, "UART.sendbreak is not support\r\n");

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_uart_sendbreak_obj, machine_uart_sendbreak);

STATIC const mp_rom_map_elem_t machine_uart_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_uart_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_uart_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_any), MP_ROM_PTR(&machine_uart_any_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&mp_stream_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readline), MP_ROM_PTR(&mp_stream_unbuffered_readline_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&mp_stream_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&mp_stream_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_sendbreak), MP_ROM_PTR(&machine_uart_sendbreak_obj) },

    { MP_ROM_QSTR(MP_QSTR_INV_TX), MP_ROM_INT(UART_INV_TX) },
    { MP_ROM_QSTR(MP_QSTR_INV_RX), MP_ROM_INT(UART_INV_RX) },
    { MP_ROM_QSTR(MP_QSTR_INV_RTS), MP_ROM_INT(UART_INV_RTS) },
    { MP_ROM_QSTR(MP_QSTR_INV_CTS), MP_ROM_INT(UART_INV_CTS) },

    { MP_ROM_QSTR(MP_QSTR_RTS), MP_ROM_INT(UART_HW_FLOWCTRL_RTS) },
    { MP_ROM_QSTR(MP_QSTR_CTS), MP_ROM_INT(UART_HW_FLOWCTRL_CTS) },
};

STATIC MP_DEFINE_CONST_DICT(machine_uart_locals_dict, machine_uart_locals_dict_table);


STATIC mp_uint_t machine_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    int bytes = 0;
    int total_bytes = 0;
    
    // make sure we want at least 1 char
    if (size == 0) {
        return 0;
    }
    
    // todo: add the timeout feature
    TickType_t time_to_wait;
    if (self->timeout == 0) {
        time_to_wait = 0;
    } else {
        time_to_wait = pdMS_TO_TICKS(self->timeout);
    }
            
    bytes = UART2_Read(buf_in, size);
    
    total_bytes += bytes;
    
    TimeOut_t xTimeOut;
    TickType_t xTicksToWait;
    

    xTicksToWait = (TickType_t) time_to_wait;
    
    vTaskSetTimeOutState( &xTimeOut );
    
    while ((total_bytes < size) && xTaskCheckForTimeOut( &xTimeOut, &xTicksToWait ) == pdFALSE)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        bytes = UART2_Read(&buf_in[total_bytes], size - total_bytes);   
        total_bytes += bytes;
    }
    
    SYS_CONSOLE_PRINT("machine_uart_read, total_bytes = %d\r\n", total_bytes);
    return total_bytes;
}

STATIC mp_uint_t machine_uart_write(mp_obj_t self_in, const void *buf_in, mp_uint_t size, int *errcode) {
    machine_uart_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool res = false;
    
    
    switch (self->uart_num)
    {
        case 2:
            res = UART2_Write(buf_in, size);
            break;
        default:
            break;
    }
    
    // return number of bytes written
    if (res)
        return size;
    else
        return 0;
    
}

STATIC mp_uint_t machine_uart_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    //machine_uart_obj_t *self = self_in;
    //mp_uint_t ret;

    return 0;
}

STATIC const mp_stream_p_t uart_stream_p = {
    .read = machine_uart_read,
    .write = machine_uart_write,
    .ioctl = machine_uart_ioctl,
    .is_text = false,
};

const mp_obj_type_t machine_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = machine_uart_print,
    .make_new = machine_uart_make_new,
    .getiter = mp_identity_getiter,
    .iternext = mp_stream_unbuffered_iter,
    .protocol = &uart_stream_p,
    .locals_dict = (mp_obj_dict_t *)&machine_uart_locals_dict,
};
