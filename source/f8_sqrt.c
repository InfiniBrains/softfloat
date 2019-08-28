
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

extern const uint8_t softfloat_sqrt_8[];

float8_t f8_sqrt( float8_t a )
{
    union ui8_f8 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA, uiZ;
    struct exp8_sig8 normExpSig;
    int index;
    int_fast8_t expZ;
    uint_fast8_t sigZ;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8UI( uiA );
    expA  = expF8UI( uiA );
    sigA  = fracF8UI( uiA );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0x1F ) {
        if ( sigA ) { /* nan */
            uiZ = softfloat_propagateNaNF8UI( uiA, 0 );
            goto uiZ;
        }
        if ( ! signA ) return a; /* inf */
        goto invalid;   /* neginf */
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( signA ) { /* negative numbers */
        if ( ! (expA | sigA) ) return a; /* -0 == +0 */
        goto invalid;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expA ) { /* subnormals */
        if ( ! sigA ) return a; /* +0 */
        normExpSig = softfloat_normSubnormalF8Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    | sqrt(a) = sqrt(sigA*2^expA) = sqrt(sigA)*2^(expA/2). But if expA is odd,
    | we leave a factor of 2 inside the sqrt so expA/2 is an integer. Thus
    | sqrt(a) = sqrt(2*sigA)*2^((expA-1)/2) if expA odd. Implemened in LUT.
    *------------------------------------------------------------------------*/
    expZ = ((expA - 0xF)>>1) + 0xE; /* divide exponent by 2 (floor), rebias */
    expA &= 1;  /* parity of exponent */
    index = (sigA<<1 & 0x6) + expA; /* index is mantissa and parity */
    sigZ = softfloat_sqrt_8[index];
    return softfloat_roundPackToF8( 0, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF8UI;
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

