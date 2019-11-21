/* Copyright (C) 2019 Alpha Cogs S.R.L.
 *
 * The ubgp library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The ubgp library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the ubgp library.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This work is based upon work authored by the Institute of Informatics
 * and Telematics of the Italian National Research Council (IIT-CNR) licensed
 * under the BSD 3-Clause license. See AKNOWLEDGEMENT and AUTHORS for more
 * details.
 */

#ifndef UBGP_DEF_H_
#define UBGP_DEF_H_

#include <limits.h>

/**
 * Section:  ubgpdef
 * @include: ubgpdef.h
 * @title:   General purpose macros and typedefs
 *
 * General purpose `typedef`s and macros used throughout the code.
 */

/**
 * byte:
 *
 * Convenience alias for `unsigned char`.
 * Should be used when raw byte data is expected (as opposed to characters).
 */
typedef unsigned char byte;

/**
 * schar:
 *
 * Convencience alias for `signed char`.
 * Should be used when explicitly signed `char` data is expected.
 * Note that standard C makes no guarantee over `char` signedness.
 */
typedef signed char schar;

/**
 * uchar:
 *
 * Convencience alias for `unsigned char`.
 * Should be used when explicitly unsigned `char` data is expected.
 * Note that standard C makes no guarantee over `char` signedness.
 */
typedef unsigned char uchar;

/**
 * ushort:
 *
 * Convenience alias for `unsigned short`.
 */
typedef unsigned short ushort;

/**
 * uint:
 *
 * Convenience alias for `unsigned int`.
 */
typedef unsigned int uint;

/**
 * ulong:
 *
 * Convenience alias for `unsigned long`.
 */
typedef unsigned long ulong;

/**
 * llong:
 *
 * Convenience alias for `long long`.
 */
typedef long long llong;

/**
 * ullong:
 *
 * Convenience alias for `unsigned long long`.
 */
typedef unsigned long long ullong;

#undef USED
#undef STR
#undef XSTR
#undef MIN
#undef MAX
#undef CLAMP
#undef digsof
#undef countof

/**
 * USED:
 * x: a variable.
 *
 * Marks a variable as used, suppressing any compiler warning.
 */
#define USED(x) ((void) (x))

/**
 * STR:
 * @x: a symbol to stringify.
 *
 * Convert macro argument to string constant.
 *
 * Invokes the preprocessor to convert the provided argument to a string
 * constant, it may also be used with expressions such as:
 *
 * ```c
 *    STR(x == 0);  // produces: "x == 0"
 * ```
 *
 * Returns: string constant.
 */
#define STR(x) #x

/**
 * XSTR:
 * @x: a symbol or macro to stringify.
 *
 * Convert macro argument to string constant, expending macro arguments
 * to their underlying values.
 *
 * Variant of XSTR() that also expands macro arguments to their
 * underlying values before stringification.
 *
 * Returns: string constant.
 */
#define XSTR(x) STR(x)

/**
 * MIN:
 * @a: first variable or expression.
 * @b: second variable or expression.
 *
 * Compute minimum value between two arguments, by comparing them using `<`.
 *
 * **This macro is unsafe and may evaluate its arguments more than once.
 *   Only provide arguments with no side-effects**.
 *
 * Returns: minimum value between @a and @b.
 */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
 * MAX:
 * @a: first variable or expression.
 * @b: second variable or expression.
 *
 * Compute maximum value between two arguments, by comparing them using `>`.
 *
 * **This macro is unsafe and may evaluate its arguments more than once.
 *   Only provide arguments with no side-effects**.
 *
 * Returns: maximum value between @a and @b.
 */
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

/**
 * CLAMP
 * @x: value or expression to be clamped.
 * @a: minumum clamping bound, inclusive.
 * @b: maximum clamping bound, inclusive.
 *
 * Clamp value to a range (inclusive).
 *
 * Takes a value @x and clamps it to the range defined by ```[a, b]```.
 * Behavior is undefined if @a is less than @b.
 *
 * **This macro is unsafe and may evaluate its arguments more than once.
 *   Only provide arguments with no side-effects**.
 *
 * Returns: Clamped value `y`, such that:
 *         * `y = a iff. x <= a`
 *         * `y = b iff. x >= b`
 *         * `y = x otherwise`
 */
#define CLAMP(x, a, b) MIN(MAX(x, a), b)

/**
 * countof:
 * @array: an array.
 *
 * Number of elements in array.
 *
 * Compute the number of elements in an array as a `size_t` compile time
 * constant.
 *
 * This macro only computes the number of elements for `arrays`, this
 * example code is not going to produce the expected result,
 * as it operates on a pointer:
 *
 * ```c
 *     #include <stdio.h>
 *
 *     void wrong(const int arr[])
 *     {
 *         printf("%zu\n", nelems(arr));  // WRONG! arr decays to a pointer
 *     }
 * ```
 *
 * **This macro is unsafe and may evaluate its arguments more than once.
 *   Only provide arguments with no side-effects**.
 */
#define countof(array) (sizeof(array) / sizeof((array)[0]))

/**
 * digsof:
 * typ: A type or expression.
 *
 * Digits required to represent a type or an expression's result.
 *
 * Returns: a compile time constant whose value is the number of digits
 *          required to represent the largest value of a type, resulting value
 *          is an upperbound, suitable to allocate buffers of char to store the
 *          string representation in digits of that type.
 *          When allocating a buffer to store conversion results, additional
 *          `char`s should be allocated to store the terminating `NUL` `char`
 *          and sign, if necessary.
 */
#define digsof(typ) (sizeof(typ) * CHAR_BIT)

#endif

