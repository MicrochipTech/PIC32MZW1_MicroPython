#ifndef MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
#define MICROPY_INCLUDED_MACHINE_ADCBLOCK_H

typedef struct _madcblock_obj_t {
    mp_obj_base_t base;
    int unit_id;
    mp_int_t bits;
    int width;
} madcblock_obj_t;

extern madcblock_obj_t madcblock_obj[];

extern void madcblock_bits_helper(madcblock_obj_t *self, mp_int_t bits);

#endif // MICROPY_INCLUDED_MACHINE_ADCBLOCK_H
