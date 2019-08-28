
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3c, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015 The Regents of the University of
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
#include "specialize.h"
#include "softfloat.h"

float8_t extF80_to_f8( extFloat80_t a )
{
    union { struct extFloat80M s; extFloat80_t f; } uA;
    uint_fast16_t uiA64;
    uint_fast64_t uiA0;
    bool sign;
    int_fast32_t exp;
    uint_fast64_t sig;
    struct commonNaN commonNaN;
    uint_fast8_t uiZ, sig8;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA64 = uA.s.signExp;
    uiA0  = uA.s.signif;
    sign = signExtF80UI64( uiA64 );
    exp  = expExtF80UI64( uiA64 );
    sig  = uiA0;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp == 0x7FFF ) { /* inf/nan */
        if ( sig & UINT64_C( 0x7FFFFFFFFFFFFFFF ) ) {
            softfloat_extF80UIToCommonNaN( uiA64, uiA0, &commonNaN );
            uiZ = softfloat_commonNaNToF8UI( &commonNaN );
        } else { /* inf */
            uiZ = packToF8UI( sign, 0x1F, 0 );
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    | extf80 has explicit int bit and 63 fract bits. shift to keep 4 rnd bits.
    *------------------------------------------------------------------------*/
    sig8 = softfloat_shortShiftRightJam64( sig, 57 ); /* shift and sticky */
    if ( ! (exp | sig8) ) { /* zero */
        uiZ = packToF8UI( sign, 0, 0 );
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    exp -= 0x3FF1; /* bias of extf80 is 0x3FFF, rebias to bias of 0xF - 1 */
    if ( sizeof (int_fast8_t) < sizeof (int_fast32_t) ) {
        if ( exp < -0x40 ) exp = -0x40; /* cap bottom of exp if int8/16 used */
    }
    return softfloat_roundPackToF8( sign, exp, sig8 );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

