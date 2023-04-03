/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Damien P. George
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
#include "board.h"
#include "extmod/virtpin.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "definitions.h"
#include "port/pic32mzw1/pic32mzw1_gpio.h"
#include "shared/runtime/mpirq.h"

#define GPIO_NUM_8 8
#define GPIO_NUM_9 9
#define GPIO_NUM_10 10
#define GPIO_NUM_11 11
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_15 15
#define GPIO_NUM_16 16
#define GPIO_NUM_17 17
#define GPIO_NUM_18 18
#define GPIO_NUM_34 34
#define GPIO_NUM_35 35
#define GPIO_NUM_41 41
#define GPIO_NUM_42 42
#define GPIO_NUM_44 44
#define GPIO_NUM_45 45
#define GPIO_NUM_46 46
#define GPIO_NUM_47 47
#define GPIO_NUM_48 48
#define GPIO_NUM_49 49




#define GPIO_MODE_IN (0)
#define GPIO_MODE_OUT (1)
#define GPIO_MODE_OPEN_DRAIN (2)

#define GPIO_PULL_UP (0)
#define GPIO_PULL_DOWN (1)

#define GPIO_PIN_INTR_NEGEDGE (0)
#define GPIO_PIN_INTR_POSEDGE (1)
#define GPIO_PIN_INTR_LOLEVEL (2)
#define GPIO_PIN_INTR_HILEVEL (3)

typedef enum
{
    PIC32MZW1_GPIO_PORT_A = 0,
    PIC32MZW1_GPIO_PORT_B = 1,
    PIC32MZW1_GPIO_PORT_C = 2,
    PIC32MZW1_GPIO_PORT_K = 3,
} PIC32MZW1_GPIO_PORT;



typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    PIC32MZW1_GPIO_PORT   port;
    int pin;
    PIC32MZW1_INT_CHAN  irq_chan;
    int irq_id;
    int id;
} machine_pin_obj_t;

typedef struct _machine_pin_irq_obj_t {
    mp_irq_obj_t base;
    uint32_t flags;
    uint32_t trigger;
} machine_pin_irq_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[] = {
    
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_C, 11, PIC32MZW1_INT_CHANN_2, 9, GPIO_NUM_9},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_C, 10, PIC32MZW1_INT_CHANN_3, 9, GPIO_NUM_10},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_C, 12, PIC32MZW1_INT_CHANN_1, 10, GPIO_NUM_12},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_C, 15, PIC32MZW1_INT_CHANN_2, 10, GPIO_NUM_14},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_K, 12, PIC32MZW1_INT_CHANN_4, 14, GPIO_NUM_15},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_K, 13, PIC32MZW1_INT_CHANN_3, 14, GPIO_NUM_16},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_K, 14, PIC32MZW1_INT_CHANN_2, 14, GPIO_NUM_17},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_C, 9, PIC32MZW1_INT_CHANN_4, 9,GPIO_NUM_18},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_K, 1, PIC32MZW1_INT_CHANN_3, 11, GPIO_NUM_34},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_K, 3, PIC32MZW1_INT_CHANN_1, 11, GPIO_NUM_35},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_A, 10, PIC32MZW1_INT_CHANN_2, 1, GPIO_NUM_41},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_A, 13, PIC32MZW1_INT_CHANN_1, 0, GPIO_NUM_42},
    {{NULL}, -1, -1, -1, -1, -1},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_A, 14, PIC32MZW1_INT_CHANN_2, 2, GPIO_NUM_44},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_B, 12, PIC32MZW1_INT_CHANN_4, 6, GPIO_NUM_45},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_B, 6, PIC32MZW1_INT_CHANN_2, 4, GPIO_NUM_46},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_B, 8, PIC32MZW1_INT_CHANN_4, 5, GPIO_NUM_47},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_B, 9, PIC32MZW1_INT_CHANN_3, 5, GPIO_NUM_48},
    {{&machine_pin_type}, PIC32MZW1_GPIO_PORT_B, 7, PIC32MZW1_INT_CHANN_1, 4, GPIO_NUM_49},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    {{NULL}, -1, -1, -1, -1, -1},
    
};

void (*interrupt_callback_handler[5]) (EXTERNAL_INT_PIN pin, uintptr_t context);

void interrupt0_callback_handler(EXTERNAL_INT_PIN irq_pin, uintptr_t context)
{ 
    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[0]);
    mp_irq_handler(&irq_obj->base);
}
void interrupt1_callback_handler(EXTERNAL_INT_PIN irq_pin, uintptr_t context)
{ 
    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[1]);
    mp_irq_handler(&irq_obj->base);
}
void interrupt2_callback_handler(EXTERNAL_INT_PIN irq_pin, uintptr_t context)
{
    //PIC32MZW1_INT_CHAN * irq_chan = (PIC32MZW1_INT_CHAN *) context;
    
    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[2]);
    mp_irq_handler(&irq_obj->base);

}
void interrupt3_callback_handler(EXTERNAL_INT_PIN irq_pin, uintptr_t context)
{ 
    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[3]);
    mp_irq_handler(&irq_obj->base);
}
void interrupt4_callback_handler(EXTERNAL_INT_PIN irq_pin, uintptr_t context)
{ 
    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[4]);
    mp_irq_handler(&irq_obj->base);
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)", self->id);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    
    if (n_args == 0) {
        // get pin
        return MP_OBJ_NEW_SMALL_INT(gpio_get_level(self->port, self->pin));
    } else {
        // set pin
        gpio_put(self->port, self->pin, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

STATIC mp_uint_t pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;
	
    switch (request) {
        case MP_PIN_READ: {
            return gpio_get_level(self->port, self->pin);
        }
        case MP_PIN_WRITE: {
            gpio_put(self->port, self->pin, mp_obj_is_true(arg));
            return 0;
        }
    }
	
    return -1;
}

STATIC const mp_pin_p_t pin_pin_p = {
    .ioctl = pin_ioctl,
};

// pin.init(mode=None, pull=-1, *, value, drive, hold)
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = MP_OBJ_NEW_SMALL_INT(-1)}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // configure mode
    if (args[ARG_mode].u_obj != mp_const_none) {
        mp_int_t mode = mp_obj_get_int(args[ARG_mode].u_obj);
        if (mode == GPIO_MODE_IN) {
            gpio_set_dir(self->port, self->pin, GPIO_MODE_IN);
        } else if (mode == GPIO_MODE_OUT) {
            gpio_set_dir(self->port, self->pin, GPIO_MODE_OUT);
        } else if (mode == GPIO_MODE_OPEN_DRAIN) {
            //To do
        } else {
            // Alternate function.
        }
    }

    // set initial value (do this before configuring mode/pull)
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        gpio_put(self->port, self->pin, mp_obj_is_true(args[ARG_value].u_obj));
    }

    return mp_const_none;
}


// constructor(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    const machine_pin_obj_t *self = NULL;
    if (0 < wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)) {
        self = (machine_pin_obj_t *)&machine_pin_obj[wanted_pin-1];
    }
    if (self == NULL || self->base.type == NULL) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid pin"));
    }

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }
    
    interrupt_callback_handler[0] = interrupt0_callback_handler;
    interrupt_callback_handler[1] = interrupt1_callback_handler;
    interrupt_callback_handler[2] = interrupt2_callback_handler;
    interrupt_callback_handler[3] = interrupt3_callback_handler;
    interrupt_callback_handler[4] = interrupt4_callback_handler;

    return MP_OBJ_FROM_PTR(self);
}


// pin.off()
STATIC mp_obj_t machine_pin_off(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
#ifdef PIC32MZW1_DEBUG   
    SYS_CONSOLE_PRINT("machine_pin_off Pin(%u)\r\n", self->id);
#endif    
    gpio_put(self->port, self->pin, 0);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_off_obj, machine_pin_off);

// pin.on()
STATIC mp_obj_t machine_pin_on(mp_obj_t self_in) {
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(self_in);
#ifdef PIC32MZW1_DEBUG 
    SYS_CONSOLE_PRINT("machine_pin_on Pin(%u)\r\n", self->id);
#endif
    gpio_put(self->port, self->pin, 1);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pin_on_obj, machine_pin_on);

STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);

STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {    
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);


STATIC mp_obj_t machine_pin_irq(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_handler, ARG_trigger };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_handler, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_trigger, MP_ARG_INT, {.u_int = GPIO_PIN_INTR_POSEDGE | GPIO_PIN_INTR_NEGEDGE} },
    };
    
    machine_pin_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    machine_pin_irq_obj_t *irq_obj = MP_STATE_PORT(machine_pin_irq_objects[self->irq_chan]);
    
    if (n_args > 1 || kw_args->used != 0) {
        // configure irq
        mp_obj_t handler = args[ARG_handler].u_obj;
        uint32_t trigger = args[ARG_trigger].u_int;
        
        if (irq_obj != NULL) {
                machine_pin_obj_t *irq_pin = irq_obj->base.parent;
                if (irq_pin->id != self->id)
                {
                    SYS_CONSOLE_PRINT("IRQ pin is already set, skip this setting\r\n");
                    return mp_const_none;
                }
        }
        
        // Configure IRQ.
        // Disable all IRQs while data is updated.
        gpio_irq_disable(self->irq_chan);
        gpio_set_irq(self->pin, self->irq_chan, self->irq_id, trigger, interrupt_callback_handler[self->irq_chan]);
        
        if (irq_obj == NULL) {
            irq_obj = m_new_obj(machine_pin_irq_obj_t);
            irq_obj->base.base.type = &mp_irq_type;
            irq_obj->base.methods = NULL;
            irq_obj->base.parent = MP_OBJ_FROM_PTR(self);
            irq_obj->base.handler = mp_const_none;
            irq_obj->base.ishard = false;
            MP_STATE_PORT(machine_pin_irq_objects[self->irq_chan]) = irq_obj;
        }
        // Update IRQ data.
        irq_obj->base.handler = args[ARG_handler].u_obj;
        irq_obj->flags = 1;
        irq_obj->trigger = args[ARG_trigger].u_int;
        // Enable IRQ if a handler is given.
        if (args[ARG_handler].u_obj != mp_const_none) {
            gpio_irq_enable(self->irq_chan);
        }
    }
    

    // return the irq object
    return MP_OBJ_FROM_PTR(irq_obj);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_irq_obj, 1, machine_pin_irq);


STATIC const mp_rom_map_elem_t machine_pin_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },
    { MP_ROM_QSTR(MP_QSTR_off), MP_ROM_PTR(&machine_pin_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_on), MP_ROM_PTR(&machine_pin_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_irq), MP_ROM_PTR(&machine_pin_irq_obj) },
    
    // class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPIO_MODE_IN) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPIO_MODE_OUT) },
    { MP_ROM_QSTR(MP_QSTR_OPEN_DRAIN), MP_ROM_INT(GPIO_MODE_OPEN_DRAIN) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPIO_PULL_UP) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPIO_PULL_DOWN) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_RISING), MP_ROM_INT(GPIO_PIN_INTR_POSEDGE) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_FALLING), MP_ROM_INT(GPIO_PIN_INTR_NEGEDGE) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_LOW_LEVEL), MP_ROM_INT(GPIO_PIN_INTR_LOLEVEL) },
    { MP_ROM_QSTR(MP_QSTR_IRQ_HIGH_LEVEL), MP_ROM_INT(GPIO_PIN_INTR_HILEVEL) },
};

STATIC MP_DEFINE_CONST_DICT(machine_pin_locals_dict, machine_pin_locals_dict_table);

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
    .call = machine_pin_call,
    .protocol = &pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals_dict,
};
