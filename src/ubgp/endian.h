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

#ifndef UBGP_ENDIAN_H_
#define UBGP_ENDIAN_H_

#include "funcattribs.h"

#include <stdint.h>

/**
 * SECTION: Endian Conversion
 * @include: endian.h
 * @title:  Byte swapping and endian conversion
 *
 * This section contains byte swapping primitives and unions useful to
 * perform endian conversion.
 * A number of useful macros are also provided to detect host endianness,
 * which can also be used for conditional compilation.
 */

/**
 * doubleint_u:
 * @val:  `double` value corresponding to the current bit pattern.
 * @bits; quadword corresponding to the current `double` value.
 *
 * An `union` value to convert bit patterns to their `double` representation and
 * vice-versa. Mostly useful to swap `double` values and to examine the
 * bit-level representation of `double`s.
 */
typedef union doubleint {
    double   val;
    uint64_t bits;
} doubleint_u;

/**
 * floatint_u:
 * @val:  `float` value corresponding to the current bit pattern.
 * @bits: longword corresponding to the current `float` value.
 *
 * Raw bytes to `float` union, same concept as #doubleint_u applied
 * to `float` types.
 */
typedef union floatint {
    float    val;
    uint32_t bits;
} floatint_u;

/**
 * byteswap16:
 * @w: 16-bits word to be swapped.
 *
 * Swap bytes inside a 16-bits word.
 *
 * Returns: the resulting byte-swapped word.
 */
static inline uint16_t byteswap16(uint16_t w)
{
#ifdef __GNUC__
    return __builtin_bswap16(w);
#else
    return ((w & 0xff00) >> 8) | ((w & 0x00ff) << 8);
#endif
}

/**
 * byteswap32:
 * @w: 32-bits quadword to be swapped.
 *
 * Swap bytes inside a 32-bits quadword.
 *
 * Returns: the byte-swapped quadword.
 */
static inline uint32_t byteswap32(uint32_t w)
{
#ifdef __GNUC__
    return __builtin_bswap32(w);
#else
    return ((w & 0xff000000) >> 24) | ((w & 0x00ff0000) >> 8) |
           ((w & 0x0000ff00) << 8)  | ((w & 0x000000ff) << 24);
#endif
}

/**
 * byteswap64:
 * @w: 64-bits quadword to be swapped.
 *
 * Swap bytes inside a 64-bits quadword.
 *
 * Returns: the byte-swapped quadword.
 */
static inline uint64_t byteswap64(uint64_t w)
{
#ifdef __GNUC__
    return __builtin_bswap64(w);
#else
    return ((w & 0xff00000000000000ull) >> 56) | ((w & 0x00ff000000000000ull) >> 40) |
           ((w & 0x0000ff0000000000ull) >> 24) | ((w & 0x000000ff00000000ull) >> 8)  |
           ((w & 0x00000000ff000000ull) << 8)  | ((w & 0x0000000000ff0000ull) << 24) |
           ((w & 0x000000000000ff00ull) << 40) | ((w & 0x00000000000000ffull) << 56);
#endif
}

/**
 * ENDIAN_BIG:
 *
 * A macro constant identifying the big endian byte ordering.
 */

/**
 * ENDIAN_LITTLE:
 *
 * A macro constant identifying the little endian byte ordering.
 */

/**
 * ENDIAN_NATIVE:
 *
 * Macro identifying host machine endianness.
 *
 * This macro expands to either %ENDIAN_BIG or %ENDIAN_LITTLE at
 * preprocessing time.
 *
 * This macro differs from its enumeration counterpart because it allows
 * accessing the host byte ordering information at preprocessor level,
 * which is seldom useful, whenever possible such test should be made
 * inside a regular if at the C level, which is going to be optimized
 * away by the compiler anyway. Though, when absolutely necessary, the
 * macro variant offers the possibility of implementing preprocessor logic
 * depending on such information.
 */

#ifdef __TINYC__

#define ENDIAN_BIG    0
#define ENDIAN_LITTLE 1
#define ENDIAN_NATIVE ENDIAN_LITTLE

#else

#define ENDIAN_BIG    __ORDER_BIG_ENDIAN__
#define ENDIAN_LITTLE __ORDER_LITTLE_ENDIAN__
#define ENDIAN_NATIVE __BYTE_ORDER__

#endif

/**
 * BIG16_C:
 *
 * @brief Portable initialization of a 16-bits word in big-endian format.
 *
 * Macro used to initialize a 16-bits word to a constant encoded in big-endian.
 *
 * @param [in] C A compile-time constant to be encoded in big-endian.
 *
 * @return The constant in big-endian format, suitable to be used in an assignment
 *         to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tobig16().
 *
 * @def BIG32_C(C)
 *
 * @brief Portable initialization of a 32-bits longword in big-endian format.
 *
 * Macro used to initialize a 32-bits longword to a constant encoded in big-endian.
 *
 * @param [in] C A compile-time constant to be encoded in big-endian.
 *
 * @return The constant in big-endian format, suitable to be used in an
 *         assignment to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tobig32().
 *
 * @def BIG64_C(C)
 *
 * @brief Portable initialization of a 64-bits quadword in big-endian format.
 *
 * Macro used to initialize a 64-bits quadword to a constant encoded in big-endian.
 *
 * @param [in] C A compile-time constant to be encoded in big-endian.
 *
 * @return The constant in big-endian format, suitable to be used in an
 *         assignment to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tobig64().
 *
 * @def LITTLE16_C(C)
 *
 * @brief Portable initialization of a 16-bits word in little-endian format.
 *
 * Macro used to initialize a 16-bits word to a constant encoded in little-endian.
 *
 * @param [in] C A compile-time constant to be encoded in little-endian.
 *
 * @return The constant in little-endian format, suitable to be used in an assignment
 *         to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tolittle16().
 *
 * @def LITTLE32_C(C)
 *
 * @brief Portable initialization of a 32-bits longword in little-endian format.
 *
 * Macro used to initialize a 32-bits longword to a constant encoded in little-endian.
 *
 * @param [in] C A compile-time constant to be encoded in little-endian.
 *
 * @return The constant in little-endian format, suitable to be used in an
 *         assignment to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tolittle32().
 *
 * @def LITTLE64_C(C)
 *
 * @brief Portable initialization of a 64-bits quadword in little-endian format.
 *
 * Macro used to initialize a 64-bits quadword to a constant encoded in little-endian.
 *
 * @param [in] C A compile-time constant to be encoded in little-endian.
 *
 * @return The constant in little-endian format, suitable to be used in an
 *         assignment to initialize a variable.
 *
 * @warning This macro is unsafe and may evaluate \a C more than once, only
 *          use this macro with compile-time constant arguments, otherwise
 *          always use tolittle64().
 */
#if ENDIAN_NATIVE == ENDIAN_BIG

#define BIG16_C(C) (C)
#define LITTLE16_C(C) ((((C) & 0xff) << 8) | (((C) & 0xff00) >> 8))

#define BIG32_C(C) (C)
#define LITTLE32_C(C) (                                      \
    (((C) & 0xff000000) >> 24) | (((C) & 0x00ff0000) >> 8) | \
    (((C) & 0x0000ff00) << 8)  | (((C) & 0x000000ff) << 24)  \
)

#define BIG64_C(C) (C)
#define LITTLE64_C(C) (                                                             \
    (((C) & 0xff00000000000000ull) >> 56) | (((C) & 0x00ff000000000000ull) >> 40) | \
    (((C) & 0x0000ff0000000000ull) >> 24) | (((C) & 0x000000ff00000000ull) >> 8)  | \
    (((C) & 0x00000000ff000000ull) << 8)  | (((C) & 0x0000000000ff0000ull) << 24) | \
    (((C) & 0x000000000000ff00ull) << 40) | (((C) & 0x00000000000000ffull) << 56)   \
)

#else

#define BIG16_C(C) ((((C) & 0xff) << 8) | (((C) & 0xff00) >> 8))
#define LITTLE16_C(C) (C)

#define BIG32_C(C) (                                         \
    (((C) & 0xff000000) >> 24) | (((C) & 0x00ff0000) >> 8) | \
    (((C) & 0x0000ff00) << 8)  | (((C) & 0x000000ff) << 24)  \
)
#define LITTLE32_C(C) (C)

#define BIG64_C(C) (                                                                \
    (((C) & 0xff00000000000000ull) >> 56) | (((C) & 0x00ff000000000000ull) >> 40) | \
    (((C) & 0x0000ff0000000000ull) >> 24) | (((C) & 0x000000ff00000000ull) >> 8)  | \
    (((C) & 0x00000000ff000000ull) << 8)  | (((C) & 0x0000000000ff0000ull) << 24) | \
    (((C) & 0x000000000000ff00ull) << 40) | (((C) & 0x00000000000000ffull) << 56)   \
)
#define LITTLE64_C(C) (C)

#endif

/**
 * leswap16:
 * @w: a little endian 16-bits word.
 *
 * Convert a 16-bits little endian word to a host word.
 *
 * Returns: the corresponding 16-bits word in host byte order.
 */
static inline uint16_t leswap16(uint16_t w)
{
    if (ENDIAN_NATIVE != ENDIAN_LITTLE)
        w = byteswap16(w);

    return w;
}

/**
 * @brief Convert a 32-bits little endian longword to a host longword.
 *
 * @param [in] l A little endian 32-bits longword.
 *
 * @return The corresponding 32-bits longword in host byte order.
 *
 * @see tolittle32()
 */
static inline uint32_t leswap32(uint32_t l)
{
    if (ENDIAN_NATIVE != ENDIAN_LITTLE)
        l = byteswap32(l);

    return l;
}

/**
 * @brief Convert a 64-bits little endian quadword to a host quadword.
 *
 * @param [in] q A little endian 64-bits quadword.
 *
 * @return The corresponding 64-bits quadword in host byte order.
 *
 * @see tolittle64()
 */
static inline uint64_t leswap64(uint64_t q)
{
    if (ENDIAN_NATIVE != ENDIAN_LITTLE)
        q = byteswap64(q);

    return q;
}

/**
 * @brief Convert a 16-bits host word to a big endian word.
 *
 * @param [in] w A 16-bits word in host byte ordering.
 *
 * @return The corresponding 16-bits word in big endian byte order.
 *
 * @see tobig16()
 */
static inline uint16_t beswap16(uint16_t w)
{
    if (ENDIAN_NATIVE != ENDIAN_BIG)
        w = byteswap16(w);

    return w;
}

/**
 * Convert a 32-bits big endian longword to a host longword.
 *
 * @param [in] l A big endian 32-bits longword.
 *
 * @return The corresponding 32-bits longword in host byte order.
 *
 * @see tobig32()
 */
static inline uint32_t beswap32(uint32_t w)
{
    if (ENDIAN_NATIVE != ENDIAN_BIG)
        w = byteswap32(w);

    return w;
}

/**
 * @brief Convert a 64-bits big endian quadword to a host quadword.
 *
 * @param [in] q A big endian 64-bits quadword.
 *
 * @return The corresponding 64-bits quadword in host byte order.
 */
static inline uint64_t beswap64(uint64_t q)
{
    if (ENDIAN_NATIVE != ENDIAN_BIG)
        q = byteswap64(q);

    return q;
}

#endif

