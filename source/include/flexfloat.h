/* C++ */
#ifdef __cplusplus
extern "C" {
#endif

#ifndef flexfloat_h
#define flexfloat_h 1

#ifndef FLEXFLOAT_ON_SOFTFLOAT
#define FLEXFLOAT_ON_DOUBLE
#endif

#include <stdbool.h>
#include "internals.h"
#include "softfloat_types.h"
#include "softfloat.h"
#include <stdio.h>

#ifdef FLEXFLOAT_ON_DOUBLE
#define CAST_TO_INT64(d) (*((int64_t *)(&(d))))
#else
#define CAST_TO_INT64(d) ((d).v)	
#endif


#ifndef INLINE
#define INLINE inline
#endif 	

// Types
typedef struct { 
	uint8_t exp_bits;
	uint8_t frac_bits;
} flexfloat_desc_t;

typedef struct {
#ifdef FLEXFLOAT_ON_DOUBLE
	double value;
#else
	float64_t value;
#endif
	flexfloat_desc_t desc;
} flexfloat_t;


// Helper functions
static inline int_fast16_t flexfloat_inf_exp(const flexfloat_desc_t desc)
{
	return (int_fast16_t) ((1 << desc.exp_bits) - 1);
}

static inline int_fast16_t flexfloat_bias(const flexfloat_desc_t desc)
{
	return (int_fast16_t) ((1 << (desc.exp_bits - 1)) - 1);
}

static inline bool flexfloat_sign(const flexfloat_t *a)
{
	return signF64UI(CAST_TO_INT64(a->value));
}

int_fast16_t flexfloat_exp(const flexfloat_t *a);
uint64_t flexfloat_frac(const flexfloat_t *a);
uint64_t flexfloat_pack(flexfloat_desc_t desc, bool sign, int_fast16_t exp, uint64_t frac);
void flexfloat_sanitize(flexfloat_t *a);

// Constructors
void ff_init(flexfloat_t *obj, flexfloat_desc_t desc);
void ff_init_float(flexfloat_t *obj, float value, flexfloat_desc_t desc);
void ff_init_double(flexfloat_t *obj, double value, flexfloat_desc_t desc);
void ff_init_longdouble(flexfloat_t *obj, long double value, flexfloat_desc_t desc);
void ff_init_int(flexfloat_t *obj, int value, flexfloat_desc_t desc);
void ff_init_long(flexfloat_t *obj, long value, flexfloat_desc_t desc);
void ff_init_ff(flexfloat_t *obj, const flexfloat_t *source, flexfloat_desc_t desc);

// Casts
float ff_get_float(const flexfloat_t *obj);
double ff_get_double(const flexfloat_t *obj);
long double ff_get_longdouble(const flexfloat_t *obj);

// Artihmetics
void ff_inverse(flexfloat_t *dest, const flexfloat_t *a);
void ff_add(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b);
void ff_sub(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b);
void ff_mul(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b);
void ff_div(flexfloat_t *dest, const flexfloat_t *a, const flexfloat_t *b);

// Relational operators
bool ff_eq(const flexfloat_t *a, const flexfloat_t *b);
bool ff_neq(const flexfloat_t *a, const flexfloat_t *b);
bool ff_le(const flexfloat_t *a, const flexfloat_t *b);
bool ff_lt(const flexfloat_t *a, const flexfloat_t *b);
bool ff_ge(const flexfloat_t *a, const flexfloat_t *b);
bool ff_gt(const flexfloat_t *a, const flexfloat_t *b);

#endif

/* C++ */
#ifdef __cplusplus
}
#endif
