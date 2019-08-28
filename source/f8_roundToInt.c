
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3c, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2017 The Regents of the University of
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

float8_t f8_roundToInt( float8_t a, uint_fast8_t roundingMode, bool exact )
{
    union ui8_f8 uA;
    uint_fast8_t uiA;
    int_fast8_t exp;
    uint_fast8_t uiZ, lastBitMask, roundBitsMask;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    exp = expF8UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( exp <= 0xE ) { /* a < 1 */
        if ( ! (uint8_t) (uiA<<1) ) return a; /* a = +-0 */
        if ( exact ) softfloat_exceptionFlags |= softfloat_flag_inexact;
        uiZ = uiA & packToF8UI( 1, 0, 0 ); /* set 0 with sign of a*/ 
        switch ( roundingMode ) {
         case softfloat_round_near_even:
            if ( ! fracF8UI( uiA ) ) break; /* 0 is even nearest pow of 2 */
         case softfloat_round_near_maxMag:
            if ( exp == 0xE ) uiZ |= packToF8UI( 0, 0xF, 0 ); /* [0.5,1) ->1 */
            break;
         case softfloat_round_min:
            if ( uiZ ) uiZ = packToF8UI( 1, 0xF, 0 ); /* -1 for neg. numbers */
            break;
         case softfloat_round_max:
            if ( ! uiZ ) uiZ = packToF8UI( 0, 0xF, 0 ); /* +1 for pos numbers*/
            break;
        }
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( 0x11 <= exp ) { /* exp greater or equal 17 -> all numbers ints */
        if ( (exp == 0x1F) && fracF8UI( uiA ) ) { /* nan */
            uiZ = softfloat_propagateNaNF8UI( uiA, 0 );
            goto uiZ;
        }
        return a;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uiZ = uiA; /* here we need to actually round */
    lastBitMask = (uint_fast8_t) 1<<(0x11 - exp); /* last bit that is int */
    roundBitsMask = lastBitMask - 1;    /* all these bits are fract to int*/
    if ( roundingMode == softfloat_round_near_maxMag ) {
        uiZ += lastBitMask>>1; /* bias towards max mag */
    } else if ( roundingMode == softfloat_round_near_even ) {
        uiZ += lastBitMask>>1; /* bias towards max mag */
        if ( ! (uiZ & roundBitsMask) ) uiZ &= ~lastBitMask; /* rnd even */
    } else if (
        roundingMode
            == (signF8UI( uiZ ) ? softfloat_round_min : softfloat_round_max)
    ) {
        uiZ += roundBitsMask;
    }
    uiZ &= ~roundBitsMask;
    if ( exact && (uiZ != uiA) ) {
        softfloat_exceptionFlags |= softfloat_flag_inexact;
    }
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

