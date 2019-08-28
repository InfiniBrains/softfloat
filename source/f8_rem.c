
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
#include "specialize.h"
#include "softfloat.h"

float8_t f8_rem( float8_t a, float8_t b ) /* a–(round(a/b)·b) */
{

 //    union ui8_f8 uA;
 //    uint_fast8_t uiA;
 //    bool signA;
 //    int_fast8_t expA;
 //    uint_fast8_t sigA;
 //    union ui8_f8 uB;
 //    uint_fast8_t uiB;
 //    int_fast8_t expB;
 //    uint_fast8_t sigB;
 //    struct exp8_sig8 normExpSig;
 //    uint8_t rem;
 //    int_fast8_t expDiff;
 //    uint_fast8_t q;
 //    uint16_t recip16, q16;
 //    uint8_t altRem, meanRem;
 //    bool signRem;
 //    uint_fast8_t uiZ;
     union ui8_f8 uZ;

    /* for now, returns Zero */
    uZ.ui = packToF8UI(0,0,0);
    return uZ.f;

 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 //    uA.f = a;
 //    uiA = uA.ui;
 //    signA = signF8UI( uiA );
 //    expA  = expF8UI( uiA );
 //    sigA  = fracF8UI( uiA );
 //    uB.f = b;
 //    uiB = uB.ui;
 //    expB = expF8UI( uiB );
 //    sigB = fracF8UI( uiB );
 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 //    if ( expA == 0x1F ) {
 //        if ( sigA || ((expB == 0x1F) && sigB) ) goto propagateNaN;
 //        goto invalid;
 //    }
 //    if ( expB == 0x1F ) {
 //        if ( sigB ) goto propagateNaN;
 //        return a;
 //    }
 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 //    if ( ! expB ) {
 //        if ( ! sigB ) goto invalid;
 //        normExpSig = softfloat_normSubnormalF8Sig( sigB );
 //        expB = normExpSig.exp;
 //        sigB = normExpSig.sig;
 //    }
 //    if ( ! expA ) {
 //        if ( ! sigA ) return a;
 //        normExpSig = softfloat_normSubnormalF8Sig( sigA );
 //        expA = normExpSig.exp;
 //        sigA = normExpSig.sig;
 //    }
 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 //    rem = sigA | 0x04;
 //    sigB |= 0x04;
 //    expDiff = expA - expB;
 //    if ( expDiff < 1 ) {    /* a has same or smaller exp than b -> a/b < 2 */
 //        if ( expDiff < -1 ) return a; /* a/b < 0.5 -> round(a/b) is 0  */
 //        sigB <<= 3; /* add GRS bits */
 //        if ( expDiff ) { /* expdiff = -1 -> a/b in (0.25,1) */
 //            rem <<= 2;
 //            q = 0;
 //        } else {
 //            rem <<= 3;
 //            q = (sigB <= rem);
 //            if ( q ) rem -= sigB;
 //        }
 //    } else {
 //        recip16 = softfloat_approxRecip32_1( (uint_fast16_t) sigB<<21 );
 //        /*--------------------------------------------------------------------
 //        | Changing the shift of `rem' here requires also changing the initial
 //        | subtraction from `expDiff'.
 //        *--------------------------------------------------------------------*/
 //        rem <<= 4;
 //        expDiff -= 31;
 //        /*--------------------------------------------------------------------
 //        | The scale of `sigB' affects how many bits are obtained during each
 //        | cycle of the loop.  Currently this is 29 bits per loop iteration,
 //        | which is believed to be the maximum possible.
 //        *--------------------------------------------------------------------*/
 //        sigB <<= 3;
 //        for (;;) {
 //            q16 = (rem * (uint_fast64_t) recip16)>>16;
 //            if ( expDiff < 0 ) break;
 //            rem = -((uint_fast8_t) q16 * sigB);
 //            expDiff -= 29;
 //        }
 //        /*--------------------------------------------------------------------
 //        | (`expDiff' cannot be less than -30 here.)
 //        *--------------------------------------------------------------------*/
 //        q16 >>= ~expDiff & 31;
 //        q = q16;
 //        rem = (rem<<(expDiff + 30)) - q * sigB;
 //    }
 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 //    do {
 //        altRem = rem;
 //        ++q;
 //        rem -= sigB;
 //    } while ( ! (rem & 0x8000) );
 //    meanRem = rem + altRem;
 //    if ( (meanRem & 0x8000) || (! meanRem && (q & 1)) ) rem = altRem;
 //    signRem = signA;
 //    if ( 0x8000 <= rem ) {
 //        signRem = ! signRem;
 //        rem = -rem;
 //    }
 //    return softfloat_normRoundPackToF8( signRem, expB, rem );
 //    /*------------------------------------------------------------------------
 //    *------------------------------------------------------------------------*/
 // propagateNaN:
 //    uiZ = softfloat_propagateNaNF8UI( uiA, uiB );
 //    goto uiZ;
 // invalid:
 //    softfloat_raiseFlags( softfloat_flag_invalid );
 //    uiZ = defaultNaNF8UI;
 // uiZ:
 //    uZ.ui = uiZ;
 //    return uZ.f;

}

