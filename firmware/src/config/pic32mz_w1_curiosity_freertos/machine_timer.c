/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013-2015 Damien P. George
 * Copyright (c) 2016 Paul Sokolovsky
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

#include <stdint.h>
#include <stdio.h>

#include "py/obj.h"
#include "py/runtime.h"
#include "modmachine.h"
#include "peripheral/tmr1/plib_tmr1.h"
#include "peripheral/tmr/plib_tmr2.h"
#include "definitions.h"


#define TIMER_INTR_SEL TIMER_INTR_LEVEL
#define TIMER_DIVIDER  8

// TIMER_BASE_CLK is normally 80MHz. TIMER_DIVIDER ought to divide this exactly
//#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)
#define TIMER_SCALE    1
#define TIMER_FLAGS    0

//typedef void (*TMR_CALLBACK)(uint32_t status, uintptr_t context);
typedef void (*TMR_Start) (void);
typedef void (*TMR_Stop) (void);
typedef void (*TMR_PeriodSet)(uint32_t period);
typedef void (*TMR_CallbackRegister)( TMR1_CALLBACK callback_fn, uintptr_t context );

typedef struct _machine_timer_obj_t {
    mp_obj_base_t base;
    mp_uint_t group;
    mp_uint_t index;

    mp_uint_t repeat;
    uint64_t period;

    mp_obj_t callback;
    
    TMR_Start start;
    TMR_Stop stop;
    TMR_PeriodSet period_set;
    TMR_CallbackRegister callback_register;

    //intr_handle_t handle;

    struct _machine_timer_obj_t *next;
} machine_timer_obj_t;

const mp_obj_type_t machine_timer_type;


void machine_timer_deinit_all(void) {
    // Disable, deallocate and remove all timers from list
    machine_timer_obj_t **t = &MP_STATE_PORT(machine_timer_obj_head);
    while (*t != NULL) {
        (*t)->stop();
        machine_timer_obj_t *next = (*t)->next;
        m_del_obj(machine_timer_obj_t, *t);
        *t = next;
    }
}

STATIC void machine_timer_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_timer_obj_t *self = self_in;
    
    mp_printf(print, "Timer(%p) ", self);
    mp_printf(print, "Index(%d) ", self->index);
    mp_printf(print, "Period(%d) ", self->period);
    mp_printf(print, "repeat(%d) ", self->repeat);

}

STATIC mp_obj_t machine_timer_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_uint_t group = (mp_obj_get_int(args[0]) >> 1) & 1;
    mp_uint_t index = mp_obj_get_int(args[0]) & 1;
    
    if (index > 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid index"));
    }

    // Check whether the timer is already initialized, if so return it
    for (machine_timer_obj_t *t = MP_STATE_PORT(machine_timer_obj_head); t; t = t->next) {
        if (t->group == group && t->index == index) {
            return t;
        }
    }

    machine_timer_obj_t *self = m_new_obj(machine_timer_obj_t);
    self->base.type = &machine_timer_type;
    self->group = group;
    self->index = index;

    if (index == 0)
    {
        self->start = TMR2_Start;
        self->stop = TMR2_Stop;
        self->period_set = TMR2_PeriodSet;
        self->callback_register = TMR2_CallbackRegister;
    }
    else if (index == 1)
    {
        self->start = TMR4_Start;
        self->stop = TMR4_Stop;
        self->period_set = TMR4_PeriodSet;
        self->callback_register = TMR4_CallbackRegister;
    }
    else
    {
        self->start = TMR6_Start;
        self->stop = TMR6_Stop;
        self->period_set = TMR6_PeriodSet;
        self->callback_register = TMR6_CallbackRegister;
    }
    
    // Add the timer to the linked-list of timers
    self->next = MP_STATE_PORT(machine_timer_obj_head);
    MP_STATE_PORT(machine_timer_obj_head) = self;

    return self;
}


STATIC void machine_timer_isr(void *self_in) {
    //machine_timer_obj_t *self = self_in;

}

tmr_callback(uint32_t status, uintptr_t context)
{
    machine_timer_obj_t *self = (machine_timer_obj_t*) context;
    mp_sched_schedule(self->callback, self);
    
    if (!self->repeat)
        self->stop();
}

STATIC void machine_timer_enable(machine_timer_obj_t *self) {
    
    self->period_set(self->period * 99998);
    self->callback_register(tmr_callback, self);
    self->start();

}

STATIC mp_obj_t machine_timer_init_helper(machine_timer_obj_t *self, mp_uint_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum {
        ARG_mode,
        ARG_callback,
        ARG_period,
        ARG_tick_hz,
        ARG_freq,
    };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_callback,     MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_period,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0xffffffff} },
        { MP_QSTR_tick_hz,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000} },
        #if MICROPY_PY_BUILTINS_FLOAT
        { MP_QSTR_freq,         MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        #else
        { MP_QSTR_freq,         MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0xffffffff} },
        #endif
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    self->period = (uint64_t)args[ARG_period].u_int;

    self->repeat = args[ARG_mode].u_int;
    self->callback = args[ARG_callback].u_obj;

    machine_timer_enable(self);

    return mp_const_none;
}

STATIC mp_obj_t machine_timer_deinit(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;
    
    self->stop();

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_deinit_obj, machine_timer_deinit);

STATIC mp_obj_t machine_timer_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_timer_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_timer_init_obj, 1, machine_timer_init);

STATIC mp_obj_t machine_timer_value(mp_obj_t self_in) {
    machine_timer_obj_t *self = self_in;
    double result = 0;

    //timer_get_counter_time_sec(self->group, self->index, &result);

    return MP_OBJ_NEW_SMALL_INT((mp_uint_t)(result * 1000));  // value in ms
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_timer_value_obj, machine_timer_value);

STATIC const mp_rom_map_elem_t machine_timer_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_timer_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_timer_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_timer_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_ONE_SHOT), MP_ROM_INT(false) },
    { MP_ROM_QSTR(MP_QSTR_PERIODIC), MP_ROM_INT(true) },
};
STATIC MP_DEFINE_CONST_DICT(machine_timer_locals_dict, machine_timer_locals_dict_table);

const mp_obj_type_t machine_timer_type = {
    { &mp_type_type },
    .name = MP_QSTR_Timer,
    .print = machine_timer_print,
    .make_new = machine_timer_make_new,
    .locals_dict = (mp_obj_t)&machine_timer_locals_dict,
};
