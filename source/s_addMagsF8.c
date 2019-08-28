
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

float8_t softfloat_addMagsF8( uint_fast8_t uiA, uint_fast8_t uiB )
{
    int_fast8_t expA;
    uint_fast8_t sigA;
    int_fast8_t expB;
    uint_fast8_t sigB;
    int_fast8_t expDiff;
    uint_fast8_t uiZ;
    bool signZ;
    int_fast8_t expZ;
    uint_fast8_t sigZ;
    uint_fast8_t sigX, sigY;
    int_fast8_t shiftDist;
    uint_fast16_t sig16Z;
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
        if ( ! expA ) { /* exp is 0 -> denormals */
            uiZ = uiA + sigB; /* can just add, carry sets exp == 1 */
            goto uiZ;
        }
        if ( expA == 0x1F ) { /* inf/nan */
            if ( sigA | sigB ) goto propagateNaN;
            uiZ = uiA; /* propagate inf */
            goto uiZ;
        }
        signZ = signF8UI( uiA ); /* keep sign */
        expZ = expA; /* keep exp */
        sigZ = 0x08 + sigA + sigB; /* add sigs, add int bit with bias */
        if ( ! (sigZ & 1) && (expZ < 0x1E) ) { /* no prec loss, not inf */
            sigZ >>= 1; /* shift back into place */
            goto pack; /* directly pack, int bit increases the exponent by 1 */
        }
        sigZ <<= 3; /* otherwise add 3 more round bits so we have 4 */
    } else { 
        /*--------------------------------------------------------------------
        | different exponents
        *--------------------------------------------------------------------*/
        signZ = signF8UI( uiA ); /* keep sign */
        if ( expDiff < 0 ) {
            /*----------------------------------------------------------------
            | a < b
            *----------------------------------------------------------------*/
            if ( expB == 0x1F ) { /* inf/nan */
                if ( sigB ) goto propagateNaN;
                uiZ = packToF8UI( signZ, 0x1F, 0 );
                goto uiZ;
            }
            if ( expDiff <= -5 ) { /* diff exceeds mantissa + 2 RS bits */
                uiZ = packToF8UI( signZ, expB, sigB ); /* floor(z) is b */
                if ( expA | sigA ) goto addEpsilon; /* for inexact add */
                goto uiZ;
            }
            expZ = expB; /* result exp is in the order of b */
            sigX = sigB | 0x04; /* add hidden bit */
            sigY = sigA + (expA ? 0x04 : sigA); /* normalize second op */
            shiftDist = 11 + expDiff; /* shift so that denormal in high byte */
        } else {
            /*----------------------------------------------------------------
            | a > b
            *----------------------------------------------------------------*/
            uiZ = uiA; /* set z to a */
            if ( expA == 0x1F ) { /* inf/nan */
                if ( sigA ) goto propagateNaN;
                goto uiZ;
            }
            if ( 5 <= expDiff ) { /* diff exceeds mantissa + 2 RS bits */
                if ( expB | sigB ) goto addEpsilon; /* for inexact add */
                goto uiZ;
            }
            expZ = expA; /* result exp is in the order of a */
            sigX = sigA | 0x04; /* add hidden bit */
            sigY = sigB + (expB ? 0x04 : sigB); /* normalize second op */
            shiftDist = 11 - expDiff; /* shift so that denormal in high byte */
        }
        sig16Z = 
            ((uint_fast16_t) sigX<<11) + ((uint_fast16_t) sigY<<shiftDist);
        if ( sig16Z < 0x4000 ) { /* no carry -> still denormal */
            --expZ;
            sig16Z <<= 1; /* normalize again */
        }
        sigZ = sig16Z>>8; /* shift back into place so that 4 round bits rem */
        if ( sig16Z & 0xFF ) { /* detect need for sticky bit */
            sigZ |= 1;
        } else { /* only round bits to worry about */
            if ( ! (sigZ & 0xF) && (expZ < 0x1E) ) { /* exact result, < inf */
                sigZ >>= 4; /* cut round bits */
                goto pack; /* directly pack, int bit adds 1 to exp */
            }
        }
    }
    return softfloat_roundPackToF8( signZ, expZ, sigZ ); /* round the result */
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF8UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 addEpsilon: /* add something to the round bits to signify inexact op */
    roundingMode = softfloat_roundingMode;
    if ( roundingMode != softfloat_round_near_even ) {
        if (
            roundingMode
                == (signF8UI( uiZ ) ? softfloat_round_min
                        : softfloat_round_max) /* towards +- inf */
        ) {
            ++uiZ; /* increase mantissa by 1 */
            if ( (uint8_t) (uiZ<<1) == 0xF8 ) { /* exp == max (overflow) */
                softfloat_raiseFlags( /* raise the flag (non-silent) */
                    softfloat_flag_overflow | softfloat_flag_inexact );
            }
        }
#ifdef SOFTFLOAT_ROUND_ODD
        else if ( roundingMode == softfloat_round_odd ) {
            uiZ |= 1;
        }
#endif
    } /* otherwise just return the larger operand but set the nx flag */
    softfloat_exceptionFlags |= softfloat_flag_inexact; /* set inexact flag */
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 pack:
    uiZ = packToF8UI( signZ, expZ, sigZ );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

