#include "platform.h"
#include "flexfloat.h"
#include "math.h"

//#define NDEBUG
#include "assert.h"

int_fast16_t flexfloat_exp(const flexfloat_t *a)
{
    int_fast16_t a_exp   = expF64UI(CAST_TO_INT64(a->value));
    int_fast16_t bias    = flexfloat_bias(a->desc);
    int_fast16_t inf_exp = flexfloat_inf_exp(a->desc);

    if(a_exp == 0)
        return 0;
    else if(a_exp == 0x7FF)
        return 0x7FF;
    else
        return (a_exp - 1023) + bias;
}

uint64_t flexfloat_frac(const flexfloat_t *a)
{
    return (CAST_TO_INT64(a->value) & UINT64_C(0x000FFFFFFFFFFFFF)) >> (52 - a->desc.frac_bits);
}



uint64_t flexfloat_pack(flexfloat_desc_t desc, bool sign, int_fast16_t exp, uint64_t frac)
{
    int_fast16_t bias    = flexfloat_bias(desc);
    int_fast16_t inf_exp = flexfloat_inf_exp(desc);

    if(exp == 0)
        exp = 0;
    else if(exp == inf_exp)
        exp = 0x7FF;
    else 
        exp = (exp - bias) + 1023;

    return packToF64UI(sign, exp, frac << (52 - desc.frac_bits));
}

void flexfloat_sanitize(flexfloat_t *a)
{
    // frac
    uint64_t frac = flexfloat_frac(a);

    // exp
    int_fast16_t exp = flexfloat_exp(a);
    int_fast16_t inf_exp = flexfloat_inf_exp(a->desc);

    // sign
    bool sign = flexfloat_sign(a);

    // Checks
    if(exp < 0) // Zero
    {
        exp = 0;
        frac = 0UL;
    }
    else if(exp == 0x7FF)
    {
        exp = inf_exp;
        if(frac != 0) // NaN
            sign = 0;
    }
    else if(exp >= inf_exp) // Infinity
    {
        exp = inf_exp;
        frac = 0UL;
    }

    CAST_TO_INT64(a->value) = flexfloat_pack(a->desc, sign, exp, frac);

}

// Constructors

INLINE void ff_init(flexfloat_t *obj, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE
    obj->value = 0.0;
    #else
    obj->value = i64_to_f64(0UL);
    #endif
    obj->desc = desc;
}

INLINE void ff_init_float(flexfloat_t *obj, float value, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE  
    obj->value = (double)value;
    #else
    double temp = (double)value;
    const uint64_t *valptr = (const uint64_t *)(&temp);
    obj->value.v = *valptr;
    #endif
    obj->desc = desc;
    flexfloat_sanitize(obj);
}

INLINE void ff_init_double(flexfloat_t *obj, double value, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE
    obj->value = value;
    #else
    const uint64_t *valptr = (const uint64_t *)(&value);
    obj->value.v = *valptr;
    #endif
    obj->desc = desc;
    flexfloat_sanitize(obj);
}


INLINE void ff_init_longdouble(flexfloat_t *obj, long double value, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE
    obj->value = (double)value;
    #else
    double temp = (double)value;
    const uint64_t *valptr = (const uint64_t *)(&temp);
    obj->value.v = *valptr;
    #endif
    obj->desc = desc;
    flexfloat_sanitize(obj);
}

INLINE void ff_init_int(flexfloat_t *obj, int value, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE  
    obj->value = (double)value;
    #else
    double temp = (double)value;
    const uint64_t *valptr = (const uint64_t *)(&temp);
    obj->value.v = *valptr;
    #endif
    obj->desc = desc;
    flexfloat_sanitize(obj);
}


INLINE void ff_init_long(flexfloat_t *obj, long value, flexfloat_desc_t desc) {
    #ifdef FLEXFLOAT_ON_DOUBLE  
    obj->value = (double)value;
    #else
    double temp = (double)value;
    const uint64_t *valptr = (const uint64_t *)(&temp);
    obj->value.v = *valptr;
    #endif
    obj->desc = desc;
    flexfloat_sanitize(obj);
}



INLINE void ff_init_ff(flexfloat_t *obj, const flexfloat_t *source, flexfloat_desc_t desc ) {
    obj->value = source->value;
    obj->desc  = desc;
    if(desc.exp_bits != source->desc.exp_bits || desc.frac_bits != source->desc.frac_bits)
      flexfloat_sanitize(obj);
}


// Casts

INLINE float ff_get_float(const flexfloat_t *obj) {
    return (float)(*((const double *)(&(obj->value))));
}

INLINE double ff_get_double(const flexfloat_t *obj) {
    return *((const double *)(&(obj->value)));
}

INLINE long double ff_get_longdouble(const flexfloat_t *obj) {
    return (long double)(*((const double *)(&(obj->value))));
}


// Arithmetics

INLINE void ff_inverse(flexfloat_t *dest, const flexfloat_t *a) {
    assert((dest->desc.exp_bits == a->desc.exp_bits) && (dest->desc.frac_bits == a->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    dest->value = - a->value;
    #else
    dest->value = f64_sub(ui64_to_f64(0L), a->value);
    #endif
}


INLINE void ff_add(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b) {
    assert((dest->desc.exp_bits == a->desc.exp_bits) && (dest->desc.frac_bits == a->desc.frac_bits) &&
           (a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    dest->value = a->value + b->value;
    #else
    dest->value = f64_add(a->value, b->value);
    #endif
    flexfloat_sanitize(dest);
}

INLINE void ff_sub(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b) {   
    assert((dest->desc.exp_bits == a->desc.exp_bits) && (dest->desc.frac_bits == a->desc.frac_bits) &&
           (a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    dest->value = a->value - b->value;
    #else
    dest->value = f64_sub(a->value, b->value);
    #endif
    flexfloat_sanitize(dest);
}

INLINE void ff_mul(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b) {
    assert((dest->desc.exp_bits == a->desc.exp_bits) && (dest->desc.frac_bits == a->desc.frac_bits) &&
           (a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    dest->value = a->value * b->value;
    #else
    dest->value = f64_mul(a->value, b->value);
    #endif
    flexfloat_sanitize(dest);
}

INLINE void ff_div(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b) {
    assert((dest->desc.exp_bits == a->desc.exp_bits) && (dest->desc.frac_bits == a->desc.frac_bits) &&
           (a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    dest->value = a->value / b->value;
    #else
    dest->value = f64_div(a->value, b->value);
    #endif
    flexfloat_sanitize(dest);
}

// Relational operators

INLINE bool ff_eq(const flexfloat_t *a, const flexfloat_t *b) {
    assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return a->value == b->value;
    #else
    return f64_eq(a->value, b->value);
    #endif
}

INLINE bool ff_neq(const flexfloat_t *a, const flexfloat_t *b) {
        assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return a->value != b->value;
    #else
    return !(f64_eq(a->value, b->value));
    #endif
}

INLINE bool ff_le(const flexfloat_t *a, const flexfloat_t *b) {
    assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return (a->value <= b->value);
    #else
    return f64_le(a->value, b->value);
    #endif
}

INLINE bool ff_lt(const flexfloat_t *a, const flexfloat_t *b) {
    assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return (a->value < b->value);
    #else
    return f64_lt(a->value, b->value);
    #endif
}

INLINE bool ff_ge(const flexfloat_t *a, const flexfloat_t *b) {
    assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return (a->value >= b->value);
    #else
    return f64_ge(a->value, b->value);
    #endif
}

INLINE bool ff_gt(const flexfloat_t *a, const flexfloat_t *b) {
    assert((a->desc.exp_bits == b->desc.exp_bits) && (a->desc.frac_bits == b->desc.frac_bits));
    #ifdef FLEXFLOAT_ON_DOUBLE
    return (a->value > b->value);
    #else
    return f64_gt(a->value, b->value);
    #endif
}
