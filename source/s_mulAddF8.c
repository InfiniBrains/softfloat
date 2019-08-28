
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

float8_t
 softfloat_mulAddF8(
     uint_fast8_t uiA, uint_fast8_t uiB, uint_fast8_t uiC, uint_fast8_t op )
{
    bool signA;
    int_fast8_t expA;
    uint_fast8_t sigA;
    bool signB;
    int_fast8_t expB;
    uint_fast8_t sigB;
    bool signC;
    int_fast8_t expC;
    uint_fast8_t sigC;
    bool signProd;
    uint_fast8_t magBits, uiZ;
    struct exp8_sig8 normExpSig;
    int_fast8_t expProd;
    uint_fast16_t sigProd;
    bool signZ;
    int_fast8_t expZ;
    uint_fast8_t sigZ;
    int_fast8_t expDiff;
    uint_fast16_t sig16Z, sig16C;
    int_fast8_t shiftDist;
    union ui8_f8 uZ;

    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
    signA = signF8UI( uiA );
    expA  = expF8UI( uiA );
    sigA  = fracF8UI( uiA );
    signB = signF8UI( uiB );
    expB  = expF8UI( uiB );
    sigB  = fracF8UI( uiB );
    signC = signF8UI( uiC ) ^ (op == softfloat_mulAdd_subC); /* A*B-C */
    expC  = expF8UI( uiC );
    sigC  = fracF8UI( uiC );
    signProd = signA ^ signB ^ (op == softfloat_mulAdd_subProd); /* -(A*B)+C */
    /*------------------------------------------------------------------------
    | Inf/NaN handling on input operands
    *------------------------------------------------------------------------*/
    if ( expA == 0x1F ) { /* inf/nan a */
        if ( sigA || ((expB == 0x1F) && sigB) ) goto propagateNaN_ABC;
        magBits = expB | sigB; /* is b non-zero? */
        goto infProdArg;
    }
    if ( expB == 0x1F ) { /* inf/nan b */
        if ( sigB ) goto propagateNaN_ABC; /* nan b */
        magBits = expA | sigA; /* is a non-zero? */
        goto infProdArg;
    }
    if ( expC == 0x1F ) { /* inf/nan c */
        if ( sigC ) {
            uiZ = 0;
            goto propagateNaN_ZC; /* adding a nan */
        }
        uiZ = uiC; /* adding inf */
        goto uiZ;
    }
    /*------------------------------------------------------------------------
    | Denormals for multiplication
    *------------------------------------------------------------------------*/
    if ( ! expA ) {
        if ( ! sigA ) goto zeroProd; /* a is 0 */
        normExpSig = softfloat_normSubnormalF8Sig( sigA );
        expA = normExpSig.exp;
        sigA = normExpSig.sig;
    }
    if ( ! expB ) {
        if ( ! sigB ) goto zeroProd; /* b is 0 */
        normExpSig = softfloat_normSubnormalF8Sig( sigB );
        expB = normExpSig.exp;
        sigB = normExpSig.sig;
    }
    /*------------------------------------------------------------------------
    | Multiplication of A and B, normalization of C
    *------------------------------------------------------------------------*/
    expProd = expA + expB - 0xE; /* add exponents and rebias + 1 renorm */
    sigA = (sigA | 0x04)<<4; /* add hidden bit, add 4 round bits */
    sigB = (sigB | 0x04)<<4; /* add hidden bit, add 4 round bits */
    sigProd = (uint_fast16_t) sigA * sigB; /* wow such mult */
    if ( sigProd < 0x2000 ) { /* if there was no overflow, renormalize */
        --expProd;
        sigProd <<= 1;
    } /* sigProd is expl normalized with 3 rnd bits, shifted left by 1 byte */
    signZ = signProd; /* set the result sign to prod sign for now */
    if ( ! expC ) { /* denormal c */
        if ( ! sigC ) { /* c==0 */
            expZ = expProd - 1; /* adjust exponent as we pack with int bit */
            sigZ = sigProd>>7 | ((sigProd & 0x7F) != 0); /* sticky bit */
            goto roundPack; /* we're done */
        }
        normExpSig = softfloat_normSubnormalF8Sig( sigC );
        expC = normExpSig.exp;
        sigC = normExpSig.sig;
    }
    sigC = (sigC | 0x04)<<3; /* add hidden bit and 3 rnd bits */
    /*------------------------------------------------------------------------
    | Addition stage
    *------------------------------------------------------------------------*/
    expDiff = expProd - expC;
    if ( signProd == signC ) {
        /*--------------------------------------------------------------------
        | Effective addition of C
        *--------------------------------------------------------------------*/
        if ( expDiff <= 0 ) { /* Prod <= C */
            expZ = expC;
            /* shift the (smaller) product right to align, with sticky */
            sigZ = sigC + softfloat_shiftRightJam32( sigProd, 8 - expDiff );
        } else { /* Prod > C */
            expZ = expProd;
            sig16Z = /* sigz = prod + (C>>diff with sticky) */
                sigProd
                    + (uint_fast16_t) softfloat_shiftRightJam32(
                          (uint_fast16_t) sigC<<8, expDiff ); 
            sigZ = sig16Z>>8 | ((sig16Z & 0xFF) != 0 ); /* shift and sticky */
        }
        if ( sigZ < 0x40 ) { /* normalize denormal sig */
            --expZ;
            sigZ <<= 1;
        }
    } else {
        /*--------------------------------------------------------------------
        | Effective subtraction of C, prod and C have different signs
        *--------------------------------------------------------------------*/
        sig16C = (uint_fast16_t) sigC<<8; /* put C into upper byte */
        if ( expDiff < 0 ) { /* Prod < C */
            signZ = signC; /* C's sign will prevail */
            expZ = expC;
            /* subtract inside upper byte, sticky, result has layout like C */
            sig16Z = sig16C - softfloat_shiftRightJam32( sigProd, -expDiff );
        } else if ( ! expDiff ) { /* exponents equal */
            expZ = expProd;
            sig16Z = sigProd - sig16C; /* just subtract */
            if ( ! sig16Z ) goto completeCancellation; /* +- zero result */
            if ( sig16Z & 0x8000 ) { /* negative result -> prod < C */
                signZ = ! signZ; /* flip the sign */
                sig16Z = -sig16Z; /* get abs value back */
            }
        } else { /* Prod > C */
            expZ = expProd;
            /* subtract inside upper byte, result has layout like sigprod */
            sig16Z = sigProd - softfloat_shiftRightJam32( sig16C, expDiff );
        }
        shiftDist = softfloat_countLeadingZeros16( sig16Z ) - 1;
        expZ -= shiftDist;
        shiftDist -= 8; /* we're shifted by a byte and need to go back */
        if ( shiftDist < 0 ) { /* need to care for shifted out sticky */
            sigZ =
                sig16Z>>(-shiftDist) /* shift by 8 - leading zeros */
                        /* check for sticky in the shifted out bits */
                    | ((uint16_t) (sig16Z<<(shiftDist & 15)) != 0);
        } else {
            sigZ = (uint_fast8_t) sig16Z<<shiftDist; /* just shift it left */
        }
    }
 roundPack:
    return softfloat_roundPackToF8( signZ, expZ, sigZ );
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 propagateNaN_ABC:
    uiZ = softfloat_propagateNaNF8UI( uiA, uiB );
    goto propagateNaN_ZC;
    /*------------------------------------------------------------------------
    *------------------------------------------------------------------------*/
 infProdArg:
    if ( magBits ) { /* non-zero product to inf */
        uiZ = packToF8UI( signProd, 0x1F, 0 );
        if ( expC != 0x1F ) goto uiZ; /* c is finite */
        if ( sigC ) goto propagateNaN_ZC; /* c is a nan */
        if ( signProd == signC ) goto uiZ; /* c is same infinity as z */
    }
    softfloat_raiseFlags( softfloat_flag_invalid ); /* otherwise -inf + inf */
    uiZ = defaultNaNF8UI;
 propagateNaN_ZC:
    uiZ = softfloat_propagateNaNF8UI( uiZ, uiC );
    goto uiZ;
    /*------------------------------------------------------------------------
    | A*B was 0, so put out +-C if its finite
    *------------------------------------------------------------------------*/
 zeroProd:
    uiZ = uiC;
    if ( ! (expC | sigC) && (signProd != signC) ) { /* +- zero result */
 completeCancellation:
        uiZ =
            packToF8UI(
                (softfloat_roundingMode == softfloat_round_min), 0, 0 );
    }
 uiZ:
    uZ.ui = uiZ;
    return uZ.f;

}

