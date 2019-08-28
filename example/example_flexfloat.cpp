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
 * g++ -std=c++11 example_flexfloat.cpp -I ../source/include/ ../build/Linux-386-GCC/softfloat.a -o example_flexfloat_cpp
 *
 */
#include "flexfloat.hpp"
#include <cstdio>
#include <iostream>

int main(){

	double dbl;

        // Double-precision variables
        flexfloat<11, 52> ff_a, ff_b, ff_c;
        // Assigment with cast (from double literal)
        ff_a = 10.4;
        ff_b = 11.5;
        // Overloaded operators
        ff_b += 2;
        ff_c = ff_a + ff_b;
        // C++ output stream
        std::cout << "[cout] ff_c = " << ff_c << " (" << flexfloat_as_bits << ff_c << ")" << std::endl;
        // C-style printf (it requires an explicit cast)
        printf("[printf] ff_c = %f\n", (double) ff_c);
        // checksum with double result
        if(ff_c == (10.4+11.5+2)) printf("checksum ok\n");
        else printf("checksum WRONG!!!");

        // IEEE float16: 5 bits (exponent) + 11 bits (expliit mantissa)
        flexfloat<5, 10> n1 = 1.11, n2 = 3.754, n3;
        std::cout << "n1+n2 = " << flexfloat_as_double << (n1 + n2) << std::endl;
        // Automatic cast from different precisions
        n3 = ff_c;
        std::cout << "n3 = " << flexfloat_as_double << n3 << flexfloat_as_bits << " (" << n3 << ")" << std::endl;
        // float16 with extended dynamic: : 8 bits (exponent) + 7 bits (explicit mantissa)
        flexfloat<8, 7> n4 = n3;
        std::cout << "n4 = " << flexfloat_as_double << n4 << flexfloat_as_bits << " (" << n4 << ")" << std::endl;
        


	return 0;
}
