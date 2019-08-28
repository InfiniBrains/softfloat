
/*============================================================================

This C source file is part of the SoftFloat IEEE Floating-Point Arithmetic
Package, Release 3c, by John R. Hauser.

Copyright 2011, 2012, 2013, 2014, 2015, 2016, 2017 The Regents of the
University of California.  All rights reserved.

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

float8_t softfloat_subMagsF8( uint_fast8_t uiA, uint_fast8_t uiB )
{
    int_fast8_t expA;
    uint_fast8_t sigA;
    int_fast8_t expB;
    uint_fast8_t sigB;
    int_fast8_t expDiff;
    uint_fast8_t uiZ;
    int_fast8_t sigDiff;
    bool signZ;
    int_fast8_t shiftDist, expZ;
    uint_fast8_t sigZ, sigX, sigY;
    uint_fast32_t sig16Z;
    int_fast8_t roundingMode;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expA = expF8UI( uiA );
    sigA = fracF8UI( uiA );
    expB = expF8UI( uiB );
    sigB = fracF8UI( uiB );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expDiff = expA - expB;
    if ( ! expDiff ) {
        /*--------------------------------------------------------------------
        | Same exponents
        *--------------------------------------------------------------------*/
        if ( expA == 0x1F ) { /* inf/nan A */
            if ( sigA | sigB ) goto propagateNaN;
            softfloat_raiseFlags( softfloat_flag_invalid ); /* inf - inf */
            uiZ = defaultNaNF8UI; 
            goto uiZ;
        }
        sigDiff = sigA - sigB;
        if ( ! sigDiff ) { /* cancelled to zero */
            uiZ =
                packToF8UI( /* round to neginf causes -0 as result here */
                    (softfloat_roundingMode == softfloat_round_min), 0, 0 );
            goto uiZ;
        }
        if ( expA ) --expA; /* non-zero exponent */
        signZ = signF8UI( uiA );
        if ( sigDiff < 0 ) { /* A < B  -> "negative" reult */
            signZ = ! signZ; /* sign flopped */
            sigDiff = -sigDiff; /* abs value of the result */
        }
        shiftDist = softfloat_countLeadingZeros16( sigDiff ) - 5; /* norm */
        expZ = expA - shiftDist; /* adjust to normalizing */
        if ( expZ < 0 ) { /* went to tininess -> denormalized */ 
            shiftDist = expA; /* adjust for denormal */
            expZ = 0;
        }
        sigZ = sigDiff<<shiftDist;
        goto pack; /* packing will add the int bit to the exponent */
    } else {
        /*--------------------------------------------------------------------
        | Different exponents
        *--------------------------------------------------------------------*/
        signZ = signF8UI( uiA ); /* keep sign of A */
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            | A < B 
            *----------------------------------------------------------------*/
            signZ = ! signZ; /* flip the sign */
            if ( expB == 0x1F ) { /* inf/nan B */
                if ( sigB ) goto propagateNaN;
                uiZ = packToF8UI( signZ, 0x1F, 0 );
                goto uiZ;
            }
            if ( expDiff <= -5 ) { /* diff exceeds mantissa + 2 RS bits */
                uiZ = packToF8UI( signZ, expB, sigB ); /* A no impact on B */
                if ( expA | sigA ) goto subEpsilon; /* for inexact sub */
                goto uiZ;
            }
            expZ = expA + 11; /* bias to shift into upper byte (-1 for int1) */
            sigX = sigB | 0x04; /* add hidden bit */
            sigY = sigA + (expA ? 0x04 : sigA); /* normalize second op */
            expDiff = -expDiff; /* abs of diff */
        } else {
            /*----------------------------------------------------------------
            | A > B
            *----------------------------------------------------------------*/
            uiZ = uiA; /* set z to a */
            if ( expA == 0x1F ) { /* inf/nan A */
                if ( sigA ) goto propagateNaN;
                goto uiZ;
            }
            if ( 5 <= expDiff ) { /* diff exceeds mantissa + 2 RS bits */
                if ( expB | sigB ) goto subEpsilon; /* for inexact result */
                goto uiZ;
            }
            expZ = expB + 11; /* bias to shift into upper byte (-1 for int1) */
            sigX = sigA | 0x04; /* add hidden bit */
            sigY = sigB + (expB ? 0x04 : sigB); /* normalize second op */
        }
        sig16Z = ((uint_fast16_t) sigX<<expDiff) - sigY; /* do the subtr */
        shiftDist = softfloat_countLeadingZeros16( sig16Z ) - 1;
        sig16Z <<= shiftDist; /* norm-shift into upper byte */
        expZ -= shiftDist; /* take the exp back down */
        sigZ = sig16Z>>8; /* put back into the lower byte with 4 round bits*/
        if ( sig16Z & 0xFF ) { /* if we truncated (lost) bits */
            sigZ |= 1; /* set sticky */
        } else { /* only round bits to care about */
            /* if we don't have round bits and no overflow */
            if ( ! (sigZ & 0xF) && ((unsigned int) expZ < 0x1E) ) {
                sigZ >>= 4; /* cut round bits */
                goto pack; /* direclty pack, int bit adds 1 to exp */
            }
        }
        return softfloat_roundPackToF8( signZ, expZ, sigZ ); /* rnd */
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF8UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 subEpsilon: /* sub something to the round bits to signify inexact op */
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if ( /* all the modes that round Z towards zero */
            (roundingMode == softfloat_round_minMag)
                || (roundingMode
                        == (signF8UI( uiZ ) ? softfloat_round_max
                                : softfloat_round_min))
        ) {
            --uiZ; /* decrease mantissa by 1 */
        }
#ifdef SOFTFLOAT_ROUND_ODD
        else if ( roundingMode == softfloat_round_odd ) {
            uiZ = (uiZ - 1) | 1; /* set odd bit for a Z-1 result */
        }
#endif
    }
    softfloat_exceptionFlags |= softfloat_flag_inexact;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 pack:
    uiZ = packToF8UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

