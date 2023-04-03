/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Nick Moore
 * Copyright (c) 2021 Jonathan Hogg
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


#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "machine_adc.h"
#include "port/pic32mzw1/pic32mzw1_adc.h"
#include "peripheral/adchs/plib_adchs.h"
#include "definitions.h"


#define GPIO_NUM_35 35
#define GPIO_NUM_41 41
#define GPIO_NUM_42 42
#define GPIO_NUM_44 44
#define GPIO_NUM_46 46
#define GPIO_NUM_47 47
#define GPIO_NUM_48 48
#define GPIO_NUM_49 49


STATIC const madc_obj_t madc_obj[] = {

    {{&machine_adc_type},  ADCHS_CH18, GPIO_NUM_35},
    {{&machine_adc_type},  ADCHS_CH17, GPIO_NUM_41},
    {{&machine_adc_type},  ADCHS_CH15, GPIO_NUM_42},
    {{&machine_adc_type},  ADCHS_CH14, GPIO_NUM_44},
    {{&machine_adc_type},  ADCHS_CH6, GPIO_NUM_46},
    {{&machine_adc_type},  ADCHS_CH8, GPIO_NUM_47},
    {{&machine_adc_type},  ADCHS_CH9, GPIO_NUM_48},
    {{&machine_adc_type},  ADCHS_CH7, GPIO_NUM_49},
    
};

STATIC void madc_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    const madc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADC(Pin(%u))", self->pin_id);
}

STATIC mp_obj_t madc_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *args) {
    static timer_start = 0;
    int i;    
    bool is_found = false;
    madc_obj_t *self;
    
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);
    
    int pin_arg = mp_obj_get_int(args[0]);

    for (i=0; i< (sizeof(madc_obj)/sizeof(madc_obj[0])); i++)
    {
        if (madc_obj[i].pin_id == pin_arg)
        {
            is_found = true;
            self = &madc_obj[i];
            break;
        }
    }
    
    if (!is_found)
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin"));

    adc_set_pin(pin_arg);

    if (timer_start == 0)
    {
        TMR1_Start();
        timer_start = 1;
    }

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t madc_init(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    const madc_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(madc_init_obj, 1, madc_init);

STATIC mp_obj_t madc_block(mp_obj_t self_in) {
    const madc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    //return MP_OBJ_FROM_PTR(self->block
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(madc_block_obj, madc_block);


STATIC mp_obj_t madc_read_u16(mp_obj_t self_in) {
    uint16_t adc_count;
    const madc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    
    adc_count = adc_read(self->chan_num);

    return MP_OBJ_NEW_SMALL_INT(adc_count);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(madc_read_u16_obj, madc_read_u16);

STATIC mp_obj_t madc_read_uv(mp_obj_t self_in) {
    uint32_t voltage;
    const madc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    
    voltage = adc_read_voltage(self->chan_num);
    
    return MP_OBJ_NEW_SMALL_INT(voltage);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(madc_read_uv_obj, madc_read_uv);



STATIC mp_obj_t madc_width(mp_obj_t self_in, mp_obj_t bits_in) {
    const madc_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t bits = mp_obj_get_int(bits_in);
    //madcblock_bits_helper(self->block, bits);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(madc_width_obj, madc_width);



STATIC const mp_rom_map_elem_t madc_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&madc_init_obj) },
    //{ MP_ROM_QSTR(MP_QSTR_block), MP_ROM_PTR(&madc_block_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_u16), MP_ROM_PTR(&madc_read_u16_obj) },
    { MP_ROM_QSTR(MP_QSTR_read_uv), MP_ROM_PTR(&madc_read_uv_obj) },

    // Legacy API methods:
    { MP_ROM_QSTR(MP_QSTR_width), MP_ROM_PTR(&madc_width_obj) },

};
STATIC MP_DEFINE_CONST_DICT(madc_locals_dict, madc_locals_dict_table);

const mp_obj_type_t machine_adc_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADC,
    .print = madc_print,
    .make_new = madc_make_new,
    .locals_dict = (mp_obj_t)&madc_locals_dict,
};
