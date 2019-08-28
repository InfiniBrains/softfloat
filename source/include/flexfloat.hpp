#pragma once

#include "softfloat.hpp"
#include "flexfloat.h"

#include <iostream>
#include <bitset>

#ifndef INLINE
#define INLINE 
//__attribute__ ((noinline)) 
#endif 

/* STREAM MANIPULATORS */

// Get the first unused global index value in the  private storage of std::ios_base
static INLINE int get_manipulator_id() { 
    static int id = std::ios_base::xalloc();
    return id;
}

// Set output mode to double format
static INLINE std::ostream& flexfloat_as_double(std::ostream& os) {
    os.iword(get_manipulator_id()) = 0;
    return os;
}

// Set output mode to bits format
static INLINE std::ostream& flexfloat_as_bits(std::ostream& os) {
    os.iword(get_manipulator_id()) = 1;
    return os;
} 

/* CLASS DEFINITION */

template <uint_fast8_t exp_bits, uint_fast8_t frac_bits> class flexfloat {
protected:
    flexfloat_t v;

public:
    #ifdef FLEXFLOAT_ON_DOUBLE
    INLINE double getValue() const {
    #else
    INLINE float64_t getValue() const {
    #endif
        return v.value;
    }

    // Empty constructor --> initialize to positive zero.
    INLINE flexfloat ()
    {
    #ifdef FLEXFLOAT_ON_DOUBLE
        v.value = 0.0;
    #else
        v.value = i64_to_f64(0UL);
    #endif
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;
    }

    INLINE flexfloat (const flexfloat &o) {
        v.value = o.getValue();
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;
    }

   INLINE flexfloat (flexfloat &&o) noexcept : v(o.v) {  } ;


   INLINE flexfloat& operator = (const flexfloat &o) {
        v.value = o.getValue();
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;
        return *this;
    }

    INLINE flexfloat& operator = (const flexfloat &&o) noexcept {
        v = o.v;
        return *this;
    }

    // Constructor from flexfloat types
    template <uint_fast8_t e, uint_fast8_t f> INLINE flexfloat (const flexfloat<e, f> &w) {
        v.value = w.getValue();
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;
        flexfloat_sanitize(&v);
    }

    // Constructor from softfloat types
    template <typename U> INLINE flexfloat (const softfloat<U> &w) {
        #ifdef FLEXFLOAT_ON_DOUBLE
        v.value = double(U(w));
        #else
        v.value = softfloat_cast<U,float64_t>(U(w));
        #endif
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;
        flexfloat_sanitize(&v);
    }


    // Constructor from castable type
    template <typename U> INLINE flexfloat (const U &w)
    {
        #ifdef FLEXFLOAT_ON_DOUBLE
        v.value = double(w);
        #else
        float64_t result = softfloat_cast<U,float64_t>(w);
        CAST_TO_INT64(v.value) = result.v;
        #endif
        v.desc.exp_bits = exp_bits;
        v.desc.frac_bits = frac_bits;

        flexfloat_sanitize(&v);
    }


    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: CASTS
    *------------------------------------------------------------------------*/

    INLINE explicit operator flexfloat_t() const {
        return v;
    }

    INLINE explicit operator float() const {
        return float(*(reinterpret_cast<const double *>(&(v.value))));
    }

    INLINE explicit operator double() const {
        return *(reinterpret_cast<const double *>(&(v.value)));
    }

    INLINE explicit operator long double() const {
        return (long double)(*(reinterpret_cast<const double *>(&(v.value))));
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Arithmetics
    *------------------------------------------------------------------------*/

    /* UNARY MINUS (-) */
    INLINE flexfloat operator-() const
    {

    #ifdef FLEXFLOAT_ON_DOUBLE
        return flexfloat(- v.value);
    #else
        return flexfloat(softfloat_sub(ui64_to_f64(0L), v.value));
    #endif
    }

    /* UNARY PLUS (+) */
    INLINE flexfloat operator+() const
    {
        return flexfloat(v.value);
    }

    /* ADD (+) */
    friend INLINE flexfloat operator+(const flexfloat &a, const flexfloat &b)
    {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return flexfloat(a.v.value + b.v.value);
    #else
        return flexfloat(softfloat_add(a.v.value, b.v.value));
    #endif
    }

     /* SUB (-) */
    friend INLINE flexfloat operator-(const flexfloat &a, const flexfloat &b)
    {

    #ifdef FLEXFLOAT_ON_DOUBLE
        return flexfloat(a.v.value - b.v.value);
    #else
        return flexfloat(softfloat_sub(a.v.value, b.v.value));
    #endif
    }

     /* MUL (-) */
    friend INLINE flexfloat operator*(const flexfloat &a, const flexfloat &b)
    {

    #ifdef FLEXFLOAT_ON_DOUBLE
        return flexfloat(a.v.value * b.v.value);
    #else
        return flexfloat(softfloat_mul(a.v.value, b.v.value));
    #endif
    }

     /* DIV (/) */
    friend INLINE flexfloat operator/(const flexfloat &a, const flexfloat &b)
    {

    #ifdef FLEXFLOAT_ON_DOUBLE
        return flexfloat(a.v.value / b.v.value);
    #else
        return flexfloat(softfloat_div(a.v.value, b.v.value));
    #endif
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Relational operators
    *------------------------------------------------------------------------*/

    /* EQUALITY (==) */
    INLINE bool operator==(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value == b.v.value);
    #else
        return softfloat_eq(v.value,b.v.value);
    #endif
    }

    /* INEQUALITY (!=) */
    INLINE bool operator!=(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value != b.v.value);
    #else
        return !(softfloat_eq(v.value,b.v.value));
    #endif
    }

    /* GREATER-THAN (>) */
    INLINE bool operator>(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value > b.v.value);
    #else
        return !(softfloat_le(v.value,b.v.value));
    #endif
    }

    /* LESS-THAN (<) */
    INLINE bool operator<(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value < b.v.value);
    #else
        return softfloat_lt(v.value,b.v.value);
    #endif
    }

    /* GREATER-THAN-OR-EQUAL-TO (>=) */
    INLINE bool operator>=(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value >= b.v.value);
    #else
        return !(softfloat_lt(v.value,b.v.value));
    #endif
    }

    /* LESS-THAN-OR-EQUAL-TO (<=) */
    INLINE bool operator<=(const flexfloat &b) const {
    #ifdef FLEXFLOAT_ON_DOUBLE
        return (v.value <= b.v.value);
    #else
        return softfloat_le(v.value,b.v.value);
    #endif
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Compound assignment operators (no bitwise ops)
    *------------------------------------------------------------------------*/
    INLINE flexfloat &operator+=(const flexfloat &b) {
        return *this = *this + b;
    }

    INLINE flexfloat &operator-=(const flexfloat &b) {
        return *this = *this - b;
    }

    INLINE flexfloat &operator*=(const flexfloat &b) {
        return *this = *this * b;
    }

    INLINE flexfloat &operator/=(const flexfloat &b) {
        return *this = *this / b;
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: IO streams operators
    *------------------------------------------------------------------------*/
    friend std::ostream& operator<<(std::ostream& os, const flexfloat& obj)
    {
        if(os.iword(get_manipulator_id()) == 0)
        {
            os << double(obj);
        }
        else
        {
            os << flexfloat_sign(&(obj.v)) << "-";
            os << std::bitset<exp_bits>(flexfloat_exp(&(obj.v))) << "-";
            os << std::bitset<frac_bits>(flexfloat_frac(&(obj.v)));
        }
        return os;
    }

};

template <> class flexfloat<8, 23> {
protected:
    float v;

public:

    INLINE float getValue() const {
        return v;
    }

    // Empty constructor --> initialize to positive zero.
    INLINE flexfloat () { v = 0 ; }

    INLINE flexfloat (const flexfloat &) = default;
    INLINE flexfloat (flexfloat &&) = default;
    INLINE flexfloat& operator = (const flexfloat &) = default;

    // Constructor from flexfloat types
    template <uint_fast8_t e, uint_fast8_t f> INLINE flexfloat (const flexfloat<e, f> &w) {
        v = float(w.getValue());
    }

    // Constructor from softfloat types
    template <typename U> INLINE flexfloat (const softfloat<U> &w) {
        v= float(U(w));
    }

    // Constructor from castable type
    template <typename U> INLINE flexfloat (const U &w)
    {
        v = float(w);
    }



    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: CASTS
    *------------------------------------------------------------------------*/

    INLINE explicit operator flexfloat_t() const {
        return flexfloat_t();
    }

    INLINE explicit operator float() const {
        return v;
    }

    INLINE explicit operator double() const {
        return double(v);
    }

    INLINE explicit operator long double() const {
        return (long double)(v);
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Arithmetics
    *------------------------------------------------------------------------*/

    /* ADD (+) */
    friend INLINE flexfloat operator+(const flexfloat &a, const flexfloat &b)
    {
        return flexfloat(a.v + b.v);
    }

     /* SUB (-) */
    friend INLINE flexfloat operator-(const flexfloat &a, const flexfloat &b)
    {
        return flexfloat(a.v - b.v);
    }

     /* MUL (-) */
    friend INLINE flexfloat operator*(const flexfloat &a, const flexfloat &b)
    {
        return flexfloat(a.v * b.v);
    }

     /* DIV (/) */
    friend INLINE flexfloat operator/(const flexfloat &a, const flexfloat &b)
    {
        return flexfloat(a.v / b.v);
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Relational operators
    *------------------------------------------------------------------------*/

    /* EQUALITY (==) */
    INLINE bool operator==(const flexfloat &b) const {
        return v == b.v;
    }

    /* INEQUALITY (!=) */
    INLINE bool operator!=(const flexfloat &b) const {
        return v != b.v;
    }

    /* GREATER-THAN (>) */
    INLINE bool operator>(const flexfloat &b) const {
        return v > b.v;
    }

    /* LESS-THAN (<) */
    INLINE bool operator<(const flexfloat &b) const {
        return v < b.v;
    }

    /* GREATER-THAN-OR-EQUAL-TO (>=) */
    INLINE bool operator>=(const flexfloat &b) const {
        return v >= b.v;
    }

    /* LESS-THAN-OR-EQUAL-TO (<=) */
    INLINE bool operator<=(const flexfloat &b) const {
        return v <= b.v;
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: Compound assignment operators (no bitwise ops)
    *------------------------------------------------------------------------*/
    INLINE flexfloat &operator+=(const flexfloat &b) {
        return *this = *this + b;
    }

    INLINE flexfloat &operator-=(const flexfloat &b) {
        return *this = *this - b;
    }

    INLINE flexfloat &operator*=(const flexfloat &b) {
        return *this = *this * b;
    }

    INLINE flexfloat &operator/=(const flexfloat &b) {
        return *this = *this / b;
    }

    /*------------------------------------------------------------------------
    | OPERATOR OVERLOADS: IO streams operators
    *------------------------------------------------------------------------*/
    friend std::ostream& operator<<(std::ostream& os, const flexfloat& obj)
    {
      os << float(obj);
    }

};
