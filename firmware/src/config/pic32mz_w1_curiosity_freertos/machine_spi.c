/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020-2021 Damien P. George
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
#include "extmod/machine_spi.h"
#include "modmachine.h"
#include "peripheral/spi/spi_master/plib_spi2_master.h"
#include "definitions.h"

//#include "hardware/spi.h"
//#include "hardware/dma.h"

//need correct this
#define SPI_MSB_FIRST (0)
#define SPI_LSB_FIRST (1)

#define DEFAULT_SPI_BAUDRATE    (1000000)
#define DEFAULT_SPI_POLARITY    (0)
#define DEFAULT_SPI_PHASE       (0)
#define DEFAULT_SPI_BITS        (8)
#define DEFAULT_SPI_FIRSTBIT    (SPI_MSB_FIRST)

#ifndef MICROPY_HW_SPI0_SCK
#define MICROPY_HW_SPI0_SCK     (6)
#define MICROPY_HW_SPI0_MOSI    (7)
#define MICROPY_HW_SPI0_MISO    (4)
#endif

#ifndef MICROPY_HW_SPI1_SCK
#define MICROPY_HW_SPI1_SCK     (10)
#define MICROPY_HW_SPI1_MOSI    (11)
#define MICROPY_HW_SPI1_MISO    (8)
#endif

//#define IS_VALID_PERIPH(spi, pin)   ((((pin) & 8) >> 3) == (spi))
#define IS_VALID_SCK(spi, pin)      ( pin == 43)
#define IS_VALID_MOSI(spi, pin)     ( pin == 24)
#define IS_VALID_MISO(spi, pin)     ( pin == 27)

bool isTransferDone;

typedef struct _machine_spi_obj_t {
    mp_obj_base_t base;
    //spi_inst_t *const spi_inst;
    uint8_t spi_id;
    uint8_t polarity;
    uint8_t phase;
    uint8_t bits;
    uint8_t firstbit;
    uint8_t sck;
    uint8_t mosi;
    uint8_t miso;
    uint32_t baudrate;
} machine_spi_obj_t;

STATIC machine_spi_obj_t machine_spi_obj[] = {
    {
        {&machine_spi_type}, 0,
        DEFAULT_SPI_POLARITY, DEFAULT_SPI_PHASE, DEFAULT_SPI_BITS, DEFAULT_SPI_FIRSTBIT,
        MICROPY_HW_SPI0_SCK, MICROPY_HW_SPI0_MOSI, MICROPY_HW_SPI0_MISO,
        0,
    },
    {
        {&machine_spi_type}, 1,
        DEFAULT_SPI_POLARITY, DEFAULT_SPI_PHASE, DEFAULT_SPI_BITS, DEFAULT_SPI_FIRSTBIT,
        MICROPY_HW_SPI1_SCK, MICROPY_HW_SPI1_MOSI, MICROPY_HW_SPI1_MISO,
        0,
    },
};

/* This function will be called by SPI PLIB when transfer is completed */
void APP_SST26_SPIEventHandler(uintptr_t context )
{          
    GPIO_PortSet(GPIO_PORT_C, 1<<15);
    isTransferDone = true;
}



STATIC void machine_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_spi_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "SPI(%u, baudrate=%u, polarity=%u, phase=%u, bits=%u, sck=%u, mosi=%u, miso=%u)",
        self->spi_id, self->baudrate, self->polarity, self->phase, self->bits,
        self->sck, self->mosi, self->miso);
}

mp_obj_t machine_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit, ARG_sck, ARG_mosi, ARG_miso };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,       MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_baudrate, MP_ARG_INT, {.u_int = DEFAULT_SPI_BAUDRATE} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_SPI_POLARITY} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_SPI_PHASE} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_SPI_BITS} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_SPI_FIRSTBIT} },
        { MP_QSTR_sck,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_mosi,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
        { MP_QSTR_miso,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_rom_obj = MP_ROM_NONE} },
    };

    // Parse the arguments.
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Get the SPI bus id.
    int spi_id = mp_obj_get_int(args[ARG_id].u_obj);
    if (spi_id == 1){
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI1 is not allow to use"));
    }
    if (spi_id < 1 || spi_id > MP_ARRAY_SIZE(machine_spi_obj)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("SPI(%d) doesn't exist"), spi_id);
    }


    // Get static peripheral object.
    machine_spi_obj_t *self = (machine_spi_obj_t *)&machine_spi_obj[spi_id -1];

    // Set SCK/MOSI/MISO pins if configured.
    if (args[ARG_sck].u_obj != mp_const_none) {
        int sck = mp_hal_get_pin_obj(args[ARG_sck].u_obj);
        if (!IS_VALID_SCK(self->spi_id, sck)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad SCK pin"));
        }
        self->sck = sck;
    }
    if (args[ARG_mosi].u_obj != mp_const_none) {
        int mosi = mp_hal_get_pin_obj(args[ARG_mosi].u_obj);
        if (!IS_VALID_MOSI(self->spi_id, mosi)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad MOSI pin"));
        }
        self->mosi = mosi;
    }
    if (args[ARG_miso].u_obj != mp_const_none) {
        int miso = mp_hal_get_pin_obj(args[ARG_miso].u_obj);
        if (!IS_VALID_MISO(self->spi_id, miso)) {
            mp_raise_ValueError(MP_ERROR_TEXT("bad MISO pin"));
        }
        self->miso = miso;
    }
    
    // Initialise the SPI peripheral if any arguments given, or it was not initialised previously.
    if (n_args > 1 || n_kw > 0 || self->baudrate == 0) {
        self->baudrate = args[ARG_baudrate].u_int;
        self->polarity = args[ARG_polarity].u_int;
        self->phase = args[ARG_phase].u_int;
        self->bits = args[ARG_bits].u_int;
        self->firstbit = args[ARG_firstbit].u_int;
        if (self->firstbit == SPI_LSB_FIRST) {
            mp_raise_NotImplementedError(MP_ERROR_TEXT("LSB"));
        }
    }
    
    SPI_TRANSFER_SETUP spi_setup;
    bool res;
    
    spi_setup.clockFrequency = self->baudrate;
    
    if (self->phase == 0)
        spi_setup.clockPhase = SPI_CLOCK_PHASE_TRAILING_EDGE;
    else
        spi_setup.clockPhase = SPI_CLOCK_PHASE_LEADING_EDGE;
    
    if (self->polarity == 0)
        spi_setup.clockPolarity = SPI_CLOCK_POLARITY_IDLE_LOW;
    else
        spi_setup.clockPolarity = SPI_CLOCK_POLARITY_IDLE_HIGH;
    
    if (self->bits == 8)
        spi_setup.dataBits = SPI_DATA_BITS_8;
    else if (args[ARG_bits].u_int == 16)
        spi_setup.dataBits = SPI_DATA_BITS_16;
    else if (args[ARG_bits].u_int == 32)
        spi_setup.dataBits = SPI_DATA_BITS_32;
    else
        spi_setup.dataBits = SPI_DATA_BITS_8; // default is 8 bits
    
    res = true;
    if (!res)
        mp_raise_ValueError(MP_ERROR_TEXT("fail to setup SPI"));

    SPI2_CallbackRegister(APP_SST26_SPIEventHandler, NULL);
    
    return MP_OBJ_FROM_PTR(self);

}

STATIC void machine_spi_init(mp_obj_base_t *self_in, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_phase,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_firstbit, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };

    // Parse the arguments.
    machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // Reconfigure the format if requested.
    bool set_format = false;
    
    if (args[ARG_baudrate].u_int != -1) {
        self->baudrate = args[ARG_baudrate].u_int;
        set_format = true;
    }

    if (args[ARG_polarity].u_int != -1) {
        self->polarity = args[ARG_polarity].u_int;
        set_format = true;
    }
    if (args[ARG_phase].u_int != -1) {
        self->phase = args[ARG_phase].u_int;
        set_format = true;
    }
    if (args[ARG_bits].u_int != -1) {
        self->bits = args[ARG_bits].u_int;
        set_format = true;
    }
    if (args[ARG_firstbit].u_int != -1) {
        self->firstbit = args[ARG_firstbit].u_int;
        if (self->firstbit == SPI_LSB_FIRST) {
            mp_raise_NotImplementedError(MP_ERROR_TEXT("LSB"));
        }
    }
    if (set_format) {
        SPI_TRANSFER_SETUP spi_setup;
        bool res;

        spi_setup.clockFrequency = self->baudrate;

        if (self->phase == 0)
            spi_setup.clockPhase = SPI_CLOCK_PHASE_TRAILING_EDGE;
        else
            spi_setup.clockPhase = SPI_CLOCK_PHASE_LEADING_EDGE;

        if (self->polarity == 0)
            spi_setup.clockPolarity = SPI_CLOCK_POLARITY_IDLE_LOW;
        else
            spi_setup.clockPolarity = SPI_CLOCK_POLARITY_IDLE_HIGH;

        if (self->bits == 8)
            spi_setup.dataBits = SPI_DATA_BITS_8;
        else if (args[ARG_bits].u_int == 16)
            spi_setup.dataBits = SPI_DATA_BITS_16;
        else if (args[ARG_bits].u_int == 32)
            spi_setup.dataBits = SPI_DATA_BITS_32;
        else
            spi_setup.dataBits = SPI_DATA_BITS_8; // default is 8 bits

        //res = SPI2_TransferSetup (&spi_setup, 100000000);

        //if (!res)
        //    mp_raise_ValueError(MP_ERROR_TEXT("fail to setup SPI"));
    }

}

STATIC void machine_spi_transfer(mp_obj_base_t *self_in, size_t len, const uint8_t *src, uint8_t *dest) {
    //machine_spi_obj_t *self = (machine_spi_obj_t *)self_in;   
    bool res;
    int i;
    
    isTransferDone = false;
    if (dest == NULL)
    {
        GPIO_PortClear(GPIO_PORT_C, 1<<15);
        res = SPI2_Write(src, len);
    }
    else
    {
        GPIO_PortClear(GPIO_PORT_C, 1<<15);
        res = SPI2_WriteRead(src, len, dest, len);
    }

    while (isTransferDone == false)
    {
        for (i = 0; i < 100; i++)
        {
            asm("NOP");
        }      
    }
    
    if (!res)
        mp_raise_ValueError(MP_ERROR_TEXT("fail to transfer SPI data"));
}

STATIC const mp_machine_spi_p_t machine_spi_p = {
    .init = machine_spi_init,
    .transfer = machine_spi_transfer,
};

const mp_obj_type_t machine_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .print = machine_spi_print,
    .make_new = machine_spi_make_new,
    .protocol = &machine_spi_p,
    .locals_dict = (mp_obj_dict_t *)&mp_machine_spi_locals_dict,
};
