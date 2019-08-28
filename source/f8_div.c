
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

extern const uint8_t softfloat_recip_8[];
extern const uint8_t softfloat_div_8[];

float8_t f8_div( float8_t a, float8_t b )
{
    union ui8_f8 uA;
    uint_fast8_t uiA;
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    union ui8_f8 uB;
    uint_fast8_t uiB;
    bool signB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    bool signZ;
    struct exp8_sig8 normExpSig;
    int_fast8_t expZ;
#ifdef SOFTFLOAT_FAST_DIV16TO8
    uint_fast16_t sig16A;
#else
    int index;
    uint16_t r0;
#endif
    uint_fast8_t sigZ;
    uint_fast8_t uiZ;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    signA = signF8UI( uiA );
    expA  = expF8UI( uiA );
    sigA  = fracF8UI( uiA );
    uB.f = b;
    uiB = uB.ui;
    signB = signF8UI( uiB );
    expB  = expF8UI( uiB );
    sigB  = fracF8UI( uiB );
    signZ = signA ^ signB;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( expA == 0x1F ) {
        if ( sigA ) goto propagateNaN;
        if ( expB == 0x1F ) {
            if ( sigB ) goto propagateNaN;
            goto invalid;
        }
        goto infinity;
    }
    if ( expB == 0x1F ) {
        if ( sigB ) goto propagateNaN;
        goto zero;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    if ( ! expB ) {
        if ( ! sigB ) {
            if ( ! (expA | sigA) ) goto invalid;
            softfloat_raiseFlags( softfloat_flag_infinite );
            goto infinity;
        }
        normExpSig = softfloat_normSubnormalF8Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    if ( ! expA ) {
        if ( ! sigA ) goto zero;
        normExpSig = softfloat_normSubnormalF8Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    expZ = expA - expB + 0xE;   /* Rebias with +14 (we're always off by 1) */
    sigA |= 0x04;   /* Put in hidden bit */
    sigB |= 0x04;   /* Put in hidden bit*/
#ifdef SOFTFLOAT_FAST_DIV16TO8 /* potentially longer than LUT */
    if ( sigA < sigB ) {
        --expZ;
        sig16A = (uint_fast16_t) sigA<<7;  /* 2 mant +4 round +1 as div<1 */
    } else {
        sig16A = (uint_fast16_t) sigA<<6;  /* need 2 mant bits +4 round bits */
    }
    sigZ = sig16A / sigB;
    if ( ! (sigZ & 7) ) sigZ |= ((uint_fast16_t) sigB * sigZ != sig16A); /* if
        there was a remainder we set the sticky bit to indicate inexactness */
#elif defined SOFTFLOAT_LUT_DIV8 /* Implemented completely in LUTs */
    if ( sigA < sigB ) --expZ; /* Result will be < 1 */
    index = ((sigA & 0x3)<<2 | (sigB & 0x3)); /* index with both mantissae */
    sigZ = softfloat_div_8[index];
#else /* this reciprocal is in a LUT as with larger versions */
    if ( sigA < sigB ) {
        --expZ;
        sigA <<= 5; /* 4 round bits and 1 as div<1 */
    } else {
        sigA <<= 4; /* 4 round bits */
    }
    index = sigB & 0x3; /* only 4 entries in LUT needed */
    r0 = softfloat_recip_8[index];
    sigZ = ((uint_fast16_t) sigA * r0)>>16;
    ++sigZ;
#endif
    return softfloat_roundPackToF8( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN:
    uiZ = softfloat_propagateNaNF8UI( uiA, uiB );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 invalid:
    softfloat_raiseFlags( softfloat_flag_invalid );
    uiZ = defaultNaNF8UI;
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infinity:
    uiZ = packToF8UI( signZ, 0x1F, 0 );
    goto uiZ;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 zero:
    uiZ = packToF8UI( signZ, 0, 0 );
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

