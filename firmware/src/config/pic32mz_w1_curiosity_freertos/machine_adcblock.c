/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
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
#include "machine_adcblock.h"

#define DEFAULT_VREF 1100

madcblock_obj_t madcblock_obj[] = {

};

STATIC void madcblock_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    madcblock_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "ADCBlock(%u, bits=%u)", self->unit_id, self->bits);
}

void madcblock_bits_helper(madcblock_obj_t *self, mp_int_t bits) {
    switch (bits) {
        #if 0
        case 9:
            self->width = ADC_WIDTH_BIT_9;
            break;
        #endif      
        default:
            mp_raise_ValueError(MP_ERROR_TEXT("invalid bits"));
    }
    self->bits = bits;

}

STATIC void madcblock_init_helper(madcblock_obj_t *self, size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_bits,
    };

    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_bits, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_pos_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t bits = args[ARG_bits].u_int;
    if (bits != -1) {
        madcblock_bits_helper(self, bits);
    } else if (self->width == -1) {
        madcblock_bits_helper(self, self->bits);
    }
}

STATIC mp_obj_t madcblock_make_new(const mp_obj_type_t *type, size_t n_pos_args, size_t n_kw_args, const mp_obj_t *args) {
    mp_arg_check_num(n_pos_args, n_kw_args, 1, MP_OBJ_FUN_ARGS_MAX, true);
    int unit = mp_obj_get_int(args[0]);
    madcblock_obj_t *self = NULL;
    for (int i = 0; i < MP_ARRAY_SIZE(madcblock_obj); i++) {
        if (unit == madcblock_obj[i].unit_id) {
            self = &madcblock_obj[i];
            break;
        }
    }
    if (!self) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid block id"));
    }

    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw_args, args + n_pos_args);
    madcblock_init_helper(self, n_pos_args - 1, args + 1, &kw_args);

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t madcblock_init(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    madcblock_obj_t *self = pos_args[0];
    madcblock_init_helper(self, n_pos_args - 1, pos_args + 1, kw_args);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(madcblock_init_obj, 1, madcblock_init);

STATIC mp_obj_t madcblock_connect(size_t n_pos_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    //madcblock_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(madcblock_connect_obj, 2, madcblock_connect);


STATIC const mp_rom_map_elem_t madcblock_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&madcblock_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&madcblock_connect_obj) },
};
STATIC MP_DEFINE_CONST_DICT(madcblock_locals_dict, madcblock_locals_dict_table);

const mp_obj_type_t machine_adcblock_type = {
    { &mp_type_type },
    .name = MP_QSTR_ADCBlock,
    .print = madcblock_print,
    .make_new = madcblock_make_new,
    .locals_dict = (mp_obj_t)&madcblock_locals_dict,
};
