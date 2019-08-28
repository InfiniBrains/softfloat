
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3c, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016 The Regents of the University of
California.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice,
    this list of conditions, and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions, and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 3. Neither the name of the University nor the names of its contributors may
    be used to endorse or promote products derived from this software without
    specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS "AS IS", AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, ARE
DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

----

Extended for float8 by Stefan Mach, Integrated Systems Institute, ETH Zurich

=============================================================================*/

#include <stdbool.h>
#include <stdint.h>
#include "platform.h"
#include "internals.h"
#include "softfloat.h"

float8_t i32_to_f8( int32_t a )
{
    bool sign;
    uint_fast32_t absA;
    int_fast8_t shiftDist;
    union ui8_f8 u;
    uint_fast8_t sig;

    sign = (a < 0);
    absA = sign ? -(uint_fast32_t) a : (uint_fast32_t) a;
    shiftDist = softfloat_countLeadingZeros32( absA ) - 29; /* int + 2frac */
    if ( 0 <= shiftDist ) { /* integer guaranteed without loss in precision */
        u.ui =
            a ? packToF8UI( /* exponent of 17 is first pure int values. shift
                               so that we have 1 int bit + 2 frac bits */
                    sign, 0x10 - shiftDist, (uint_fast8_t) absA<<shiftDist )
                : 0; /* zero */
        return u.f;
    } else {
        shiftDist += 4; /* add 4 round bits */
        sig =
            (shiftDist < 0) /* not all bits fitting into round bits */
                ? absA>>(-shiftDist)
                      | ((uint32_t) (absA<<(shiftDist & 31)) != 0) /*sticky*/
                : (uint_fast8_t) absA<<shiftDist; /* fits within rnd bits */
        return softfloat_roundPackToF8( sign, 0x14 - shiftDist, sig );
    }

}

