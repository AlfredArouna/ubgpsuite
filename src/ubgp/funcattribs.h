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

#ifndef UBGP_FUNCATTRIBS_H_
#define UBGP_FUNCATTRIBS_H_

/**
 * SECTION: funcattribs
 * @title:   Function attributes
 * @include: funcattribs.h
 *
 * Funcattribs Compiler-specific optimization and diagnostic hints
 *
 * Function diagnostic and optimizing attributes.
 *
 * These additional attributes are intended to decorate function declarations
 * with additional optimization and diagnostic hints, enabling the compiler to
 * operate more aggressive optimizations and checks on code.
 *
 * **Misusing compiler hints in a function declaration may introduce
 *   surprising bugs and compromize code compilation, use these hints only if
 *   you know what you are doing!**
 *
 * See [the GCC manual](https://gcc.gnu.org/onlinedocs/gcc/Common-Function-Attributes.html#Common-Function-Attributes)
 */

/**
 * SENTINEL:
 * @arg: Sentinel argument index, 1-based.
 *
 * Check for the presence of a %NULL sentinel in a variadic function.
 *
 * This function attribute ensures that a parameter in a function call is an explicit %NULL.
 * The attribute is only valid on variadic functions.
 * When @arg is 0, the sentinel must be present in the last parameter of the function call.
 * Any other integer position signifies that the sentinel must be located at position @arg
 * counting backwards from the end of the argument list.
 *
 * A valid \a NULL in this context is defined as zero with any pointer type.
 * If the \a NULL macro is defined with an integer type then you need to add an explicit cast.
 * GCC replaces \a stddef.h with a copy that redefines %NULL appropriately.
 * In GCC, the warnings for missing or incorrect sentinels are enabled with \a -Wformat.
 */

/**
 * MALLOCFUNC:
 *
 * @brief Mark function as malloc()-like.
 *
 * This tells the compiler that a function is malloc()-like,
 * i.e., that the pointer returned by the function cannot alias any other pointer
 * valid when the function returns, and moreover pointers to valid objects occur
 * in any storage addressed by it.
 *
 * Using this attribute can improve optimization.
 * Functions like malloc() and calloc() have this property
 * because they return a pointer to uninitialized or zeroed-out storage.
 * However, functions like realloc() do not have this property, as they can return a pointer to storage containing pointers.
 */

/**
 * PUREFUNC:
 *
 * Mark function as pure.
 *
 * Many functions have no effects except the return value and their return value depends
 * only on the parameters and/or global variables.
 * Calls to such functions can be subject to common subexpression elimination and loop optimization just as an arithmetic operator would be.
 * These functions should be declared with the attribute pure.
 * For example:
 *
 * ```c
 *     PUREFUNC int square(int x);
 * ```
 *
 * says that the hypothetical function square is safe to call fewer times than the program says.
 * Some common examples of pure functions are strlen() or memcmp().
 * Interesting non-pure functions are functions with infinite loops or those depending on volatile
 * memory or other system resource, that may change between two consecutive calls (such as feof in a multithreading environment).
 *
 * The pure attribute imposes similar but looser restrictions on a function’s defintion than the const attribute:
 * it allows the function to read global variables. Decorating the same function with both the pure and the
 * const attribute is diagnosed.
 * Because a pure function cannot have any side effects it does not make sense for such a function to return void.
 * Declaring such a function is diagnosed.
 */

/**
 * CONSTFUNC:
 *
 * Mark function as const.
 *
 * Many functions do not examine any values except their arguments,
 * and have no effects except to return a value.
 * Calls to such functions lend themselves to optimization such as common
 * subexpression elimination. The const attribute imposes greater restrictions
 * on a function’s definition than the similar pure attribute because it
 * prohibits the function from reading global variables. Consequently,
 * the presence of the attribute on a function declaration allows the compiler
 * to emit more efficient code for some calls to the function.
 * Decorating the same function with both the const and the pure attribute
 * is diagnosed.
 *
 * A function that has pointer arguments and examines the data pointed to must
 * not be declared const. Likewise, a function that calls a non-const function
 * usually must not be const. Because a const function cannot have any side
 * effects it does not make sense for such a function to return void.
 * Declaring such a function is diagnosed.
 */

#ifdef __GNUC__
#define UBGP_API                __attribute__((__visibility__("default")))
#define CHECK_PRINTF(fmt, args) __attribute__((__format__(printf, fmt, args)))
#define SENTINEL(arg)           __attribute__((__sentinel__(arg)))
#define CHECK_NONNULL(...)      __attribute__((__nonnull__(__VA_ARGS__)))
#define MALLOCFUNC              __attribute__((__malloc__))
#define CHECK_SIZE(arg)         __attribute__((__alloc_size__(arg)))
#define CHECK_SIZE2(arg1, arg2) __attribute__((__alloc_size__(arg1, arg2)))
#define RETURNS_NONNULL         __attribute__((__returns_nonnull__))
#define PUREFUNC                __attribute__((__pure__))
#define CONSTFUNC               __attribute__((__const__))
#define WARN_UNUSED             __attribute__((__warn_unused_result__))

#endif

#ifndef UBGP_API
#define UBGP_API
#endif
#ifndef CHECK_PRINTF
#define CHECK_PRINTF(fmt, args)
#endif
#ifndef SENTINEL
#define SENTINEL(idx)
#endif
#ifndef CHECK_NONNULL
#define CHECK_NONNULL(...)
#endif
#ifndef MALLOCFUNC
#define MALLOCFUNC
#endif
#ifndef CHECK_SIZE
#define CHECK_SIZE(arg)
#endif
#ifndef CHECK_SIZE2
#define CHECK_SIZE2(arg1, arg2)
#endif
#ifndef RETURNS_NONNULL
#define RETURNS_NONNULL
#endif
#ifndef PUREFUNC
#define PUREFUNC
#endif
#ifndef CONSTFUNC
#define CONSTFUNC
#endif
#ifndef WARN_UNUSED
#define WARN_UNUSED
#endif

#endif

