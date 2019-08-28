#include "softfloat.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/*
 * Find types and Operations in source/include/softfloat.h
 *
 * To compile:
 * gcc example.c -I ../source/include/ ../build/Linux-386-GCC/softfloat.a -o example
 *
 */

int main() {

	double dbl;
	float64_t f64;
	float flt1, flt2, fltsum;
	float32_t f32, f32sum;
	float16_t f16;
	float8_t f8;

	/* double value of one third */
	dbl = (double)1/3;

	/* SoftFloat formats are bit-true in memory */
	f64 = *(float64_t*) &dbl;

	/* Casts */
	flt1 = (float) dbl; /* native cast */
	f32 = f64_to_f32(f64); /* SoftFloat cast */
	flt2 = *(float*) &f32;

	f16 = f64_to_f16(f64);
	f8 = f64_to_f8(f64);

	/* Operations */
	fltsum = flt1 + flt1; /* native add */
	f32sum = f32_add(f32, f32); /* note that there is no operator overloading
	                            in C --> check out C++ */

	printf("Double value \t%f\n", dbl);

	printf("Double is \t%#lx\n", *(uint64_t*) &dbl);
	printf("float64_t is \t%#lx\n\n", f64); /* f64 is a struct containing a unsigned long int */

	printf("Float is \t%#x\n", *(uint32_t*) &flt1);
	printf("float32_t is \t%#x\n", f32);
	printf("Float cast is \t%#x\n\n", *(uint32_t*) &flt2);

	printf("float16_t is \t%#x\n",  f16);
	printf("float8_t is \t%#x\n\n", f8);

	printf("Float sum is \t%#x\n", *(uint32_t*) &fltsum);
	printf("SoftFloat sum \t%#x\n", f32sum);

	return 0;
}
