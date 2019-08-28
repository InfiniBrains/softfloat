/* Copyright (C) 2017 ETH Zurich, University of Bologna
 * All rights reserved.
 *
 * This code is under development and not yet released to the public.
 * Until it is released, the code is under the copyright of ETH Zurich and
 * the University of Bologna, and may contain confidential and/or unpublished
 * work. Any reuse/redistribution is strictly forbidden without written
 * permission from ETH Zurich.
 *
 * Bug fixes and contributions will eventually be released under the
 * SolderPad open hardware license in the context of the PULP platform
 * (http://www.pulp-platform.org), under the copyright of ETH Zurich and the
 * University of Bologna.
 *
 * -------
 *
 * Author: Stefan Mach
 * Email:  smach@iis.ee.ethz.ch
 * Date:   2017-05-19 10:09:31
 * Last Modified by:   Stefan Mach
 * Last Modified time: 2017-06-13 20:36:13
 */

/*
 * Find types and Operations in source/include/softfloat.hpp
 *
 * To compile:
 * g++ example.cpp -I ../source/include/ ../build/Linux-386-GCC/softfloat.a -o example_cpp
 *
 */
#include "softfloat.hpp" // Only this include needed, it includes the C stuff //
#include <cstdio>

int main(){

	double dbl;

	float64 f64;				// A SoftFloat class
	float64_t f64t;				// The SoftFloat C type

	float32 f32, f32tmp;

	float16 f16, f16sum, f16sum2, f16sum3;	// A SoftFloat Class
	float16_t f16t, f16tsum;				// The SoftFloat C type

	dbl = (double)1/3;

	f64t = *(float64_t*) &dbl;							// The C way of casting to the C type
	// NEEDS TO BE MERGED TOGETHER WITH THE BELOW MISSING FEATURE
//	f64 = float64(dbl); 	// Use the constructor to cast to f64 (C++ class)
	f64 = f64t;				// direct assignment from the C types is possible

	f32 = float32(f64);			// also works implicitly
	f32tmp = softfloat_cast<float64_t,float32_t>(f64); // You can also make use of the templated function

	// This addition needs to be merged into master (using double and float instead of SoftFloat types)
//	f16 = softfloat_cast<double,float16_t>(dbl);

	f16 = softfloat_cast<float64_t,float16_t>(f64t);
	f16t = f64_to_f16(f64t);

	// You can use the C functions with the C++ Class Objects
	f16tsum = f16_add(f16, f16);
	// You can use the templated functions
	f16sum = softfloat_add<float16_t>(f16,f16);
	// Or simply the overloaded operators
	f16sum2 = f16 + f16;
	// Or even these work
	f16sum3 = f16;
	f16sum3 += f16;


	printf("Double value \t%f\n", dbl);

	printf("Double is \t%#lx\n", *(uint64_t*) &dbl);
	printf("float64 is \t%#lx\n", f64); /* f64 is a class object */
	printf("float64_t is \t%#lx\n\n", f64t); /* f64t is a struct as in the C case */

	printf("float32 is \t%#x\n", f32);
	printf("float32 casted \t%#x\n\n", f32tmp);

	printf("float16 is \t%#x\n",  f16);
	printf("float16_t is \t%#x\n\n",  f16t);

	printf("float16_t sum is \t%#x\n", f16sum);
	printf("float16_t sum2 is \t%#x\n", f16sum2);
	printf("float16_t sum3 is \t%#x\n", f16sum3);

	return 0;
}
