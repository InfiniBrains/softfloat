# SoftFloat

This repository hosts the Berkeley SoftFloat port for float8 support for PULP.

This documentation needs to be expanded. Check `README.html` for the original SoftFloat documentation. There are only few additions to what is there.

## Building SoftFloat

For now, running `make` in the root directory is all you need to do to build SoftFloat on a Linux x86 system. No optimizations (x86-64, SSE2 etc.) have been ported yet.

### Modifying SoftFloat behavior

Two defines have been created that impact how the float8 operations perform. Define one of the following:

- `-DSOFTFLOAT_FAST_DIV16TO8` if your architecture supports reasonably fast uint16 by uint8 divisions.
- `-DSOFTFLOAT_LUT_DIV8` to pull float8 division results directly from a LUT. This is most likely the fastest option since there's only 16 entries.
- None of the above to pull the reciprocal value of the second operand from a LUT which is then multiplied with the first one.

## Usage

### C

Additionally to all types and functions in vanilla SoftFloat, there are following additions:

- The type `float8_t` for float8 numbers. Its actual size in memory is 8 bits.
- All functions in SoftFloat can also be performed on float8. Most functions have the prefix `f8_`.

Check `softfloat.h` for a complete listing of available functions.

In order to use SoftFloat, include `softfloat.h` from the `source/include` directory in your source file. When linking, provide `softfloat.a` from the `build/<target>` directory (after having built it there once).

### C++

A C++ wrapper is provided in order to make use of operator overloading to swap out types without modifying functional source code. It provides:

- A templated `softfloat` class wrapping the C types. It can be constructed
	- from the SoftFloat C types,
	- from C-native floating-point types (`float`, `double`, `long double`),
	- from another `softfloat` object (by explicitly or implicitly casting),
	- through explicit casting from a castable integer type.
- Overloaded casts from the `softfloat` class to
    - C-native floating-point types (`float`, `double`, `long double`),
    - SoftFloat C types. Thus, any C function in SoftFloat can be run on a `smallfloat` object as well.
- The following types that are specializations of the `softfloat` class for convenience:
	- `float8`
	- `float16`
	- `float32`
	- `float64`
	- `extFloat80`
	- `float128`
- Overloaded functions for:
	- Arithmetic operators (`+`,`-`,`*`,`/`)
	- Relational operators (`==`,`!=`,`>`,`<`,`>=`,`<=`)
	- Compound assignment operators (`+=`,`-=`,`*=`,`/=`)

In order to use SoftFloat in C++, include `softfloat.hpp` from the `source/include`directory in your source file. When linking, provide `softfloat.a` from the `build/<target>` directory (after having built it there once).

## Flexfloat
A specific type is provided to enable the analisys of custom floating-point types. Flexfloat values have this format: sign(1 bit)+exponent(E bits)+mantissa(M bits). E and M characterize the variable precision. This format is compliant with IEEE formats, i.e., the encoding of exponents includes a bias and the mantissa representation assumes an implicit 1 bit.
- C interface (flexfloat.h): "flexfloat_t" type and related functions
- C++ interface (flexfloat.hpp): "flexfloat" template class and its methods

## Known Issues

- The float8 remainder function, `f8_rem`, is currently not implemented and always returns positive zero.
- The overloaded relational C++ operators `>` and `>=` will return `true` on `NaN` inputs instead of `false`.
