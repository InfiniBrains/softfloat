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
 * Author: Giuseppe Tagliavini
 * Email:  giuseppe.tagliavini@unibo.it
 * Date:   2017-07-28 12:09:11
 * Last Modified by:   Giuseppe Tagliavini
 * Last Modified time: 2017-07-28 14:36:42
 */

/*
 * Find types and Operations in source/include/flexfloat.hpp
 *
 * To compile:
 * gcc example_flexfloat.c -I ../source/include/ ../build/Linux-386-GCC/softfloat.a -o example_flexfloat
 *
 */
#include "flexfloat.h"
#include <stdio.h>

int main(){

	double dbl;

        // Double-precision variables
        flexfloat_t ff_a, ff_b, ff_c;
        ff_init_double(&ff_a, 10.4, (flexfloat_desc_t) {11, 52});
        ff_init_double(&ff_b, 11.5, (flexfloat_desc_t) {11, 52});
        ff_init(&ff_c, (flexfloat_desc_t) {11, 52});
        // Arithmetic operators
        ff_add(&ff_c, &ff_a, &ff_b); // c=a+b
        // C-style printf (it requires an explicit cast)
        printf("[printf] ff_c = %f\n", ff_get_double(&ff_c));
        // checksum with double result
        if(ff_get_double(&ff_c) == (10.4+11.5)) printf("checksum ok\n");
        else printf("checksum WRONG!!!");

        // IEEE float16: 5 bits (exponent) + 10 bits (explicit mantissa)
        flexfloat_t n1, n2, n3;
        ff_init_double(&n1, 1.11, (flexfloat_desc_t) {5, 10});
        ff_init_double(&n2, 3.754, (flexfloat_desc_t) {5, 10});
        ff_init(&n3, (flexfloat_desc_t) {5, 10});

        ff_add(&n3, &n1, &n2);
        printf("n1+n2 = %f\n", ff_get_double(&n3));
        // Automatic cast from different precisions
        ff_init_ff(&n3, &ff_c, (flexfloat_desc_t) {5, 10}); //  n3 = ff_c;
        printf("n3 = %f\n", ff_get_double(&n3));

        // float16 with extended dynamic: : 8 bits (exponent) + 7 bits (explicit mantissa)
        flexfloat_t n4;
        ff_init_ff(&n4, &n3, (flexfloat_desc_t) {8, 7}); // n4 = n3
        printf("n4 = %f\n", ff_get_double(&n4));
//        std::cout << "n4 = " << flexfloat_as_double << n4 << flexfloat_as_bits << " (" << n4 << ")" << std::endl;
        


	return 0;
}
