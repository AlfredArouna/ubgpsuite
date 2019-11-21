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

#ifndef UBGP_U128_H_
#define UBGP_U128_H_

#include "bitops.h"
#include "branch.h"
#include "funcattribs.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * SECTION: u128
 * @title:   128-bit Integer type
 * @include: u128.h
 *
 * 128-bits precision unsigned integer types and functions.
 *
 * `u128.h` is guaranteed to include `stdint.h`, `stdbool.h` it may include
 * additional standard or ubgp-specific headers in the interest of
 * providing inline implementations of its functions.
 */

/**
 * u128:
 *
 * Opaque structure representing an unsigned integer with 128-bits precision.
 */
typedef struct {
    /*< private >*/

#if defined(__GNUC__) && !defined(UBGP_C_U128)
    // use compiler supplied type
    __uint128_t u128;
#else
    // emulate in plain C
    uint64_t upper, lower;
#endif
} u128;

static_assert(sizeof(u128) == 16, "Unsupported platform");

/**
 * udiv128_s
 * @quot: quotient
 * @rem: remainder
 *
 * Result returned by u128divqr().
 */
typedef struct udiv128 {
    u128 quot, rem;
} udiv128_s;

#if defined(__GNUC__) && !defined(UBGP_C_U128)

static const u128 U128_ZERO = { .u128 = 0  };
static const u128 U128_ONE  = { .u128 = 1  };
static const u128 U128_TEN  = { .u128 = 10 };
static const u128 U128_MAX  = { .u128 = ~((__uint128_t) 0) };

#define U128_C(x) { .u128 = (x) }

#else

static const u128 U128_ZERO = { .upper = 0, .lower = 0  };
static const u128 U128_ONE  = { .upper = 0, .lower = 1  };
static const u128 U128_TEN  = { .upper = 0, .lower = 10 };
static const u128 U128_MAX  = {
    .upper = UINT64_MAX,
    .lower = UINT64_MAX
};

#define U128_C(x) { .upper = 0, .lower = (x) }

#endif

static inline u128 u128from(uint64_t up, uint64_t lw)
{
    u128 r;

#if defined(__GNUC__) && !defined(UBGP_C_U128)
    r.u128 =   up;
    r.u128 <<= 64;
    r.u128 |=  lw;
#else
    r.upper = up;
    r.lower = lw;
#endif
    return r;
}

static inline u128 tou128(uint64_t u)
{
    u128 r;

#if defined(__GNUC__) && !defined(UBGP_C_U128)
    r.u128 = u;
#else
    r.upper = 0;
    r.lower = u;
#endif
    return r;
}

static inline uint64_t u128upper(u128 u)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    return u.u128 >> 64;
#else
    return u.upper;
#endif
}

static inline uint64_t u128lower(u128 u)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    return u.u128 & 0xffffffffffffffffull;
#else
    return u.lower;
#endif
}

static inline u128 u128add(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)

    a.u128 += b.u128;

#else

    uint64_t t = a.lower + b.lower;

    a.upper += (b.upper + (t < a.lower));
    a.lower = t;

#endif
    return a;
}

static inline u128 u128addu(u128 a, uint64_t b)
{
    return u128add(a, tou128(b));
}

static inline u128 u128sub(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)

    a.u128 -= b.u128;

#else

    uint64_t t = a.lower - b.lower;
    a.upper -= (b.upper - (t > a.lower));
    a.lower = t;

#endif
    return a;
}

static inline u128 u128subu(u128 a, uint64_t b)
{
    return u128sub(a, tou128(b));
}

static inline u128 u128neg(u128 u)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    u.u128 = -u.u128;
#else
    // don't use UINT128_ZERO, otherwise it wouldn't be constfunc!
    const u128 zero = { .upper = 0, .lower = 0 };
    u = u128sub(zero, u);

#endif
    return u;
}

#if defined(__GNUC__) && !defined(UBGP_C_U128)

static inline u128 u128mul(u128 a, u128 b)
{
    a.u128 *= b.u128;
    return a;
}

#else

UBGP_API CONSTFUNC u128 u128mul(u128 a, u128 b);

#endif

static inline u128 u128mulu(u128 a, uint64_t b)
{
    return u128mul(a, tou128(b));
}

/**
 * u128muladd:
 *
 * Convenience for multiply-add operation.
 *
 * Shorthand for:
 * ```c
 *    u128add(u128mul(a, b), c);
 * ```
 */
static inline u128 u128muladd(u128 a, u128 b, u128 c)
{
    return u128add(u128mul(a, b), c);
}

/**
 * u128muladdu:
 *
 * Multiply-add between #u128 and plain unsigned integers.
 *
 * Shorthand for:
 * ```c
 *    u128muladd(a, tou128(b), tou128(c));
 * ```
 */
static inline u128 u128muladdu(u128 a, uint64_t b, uint64_t c)
{
    return u128muladd(a, tou128(b), tou128(c));
}

#if defined(__GNUC__) && !defined(UBGP_C_U128)

static inline udiv128_s u128divqr(u128 a, u128 b)
{
    udiv128_s qr;
    qr.quot.u128 = a.u128 / b.u128;
    qr.rem.u128  = a.u128 % b.u128;
    return qr;
}

#else

UBGP_API CONSTFUNC udiv128_s u128divqr(u128 a, u128 b);

#endif

static inline udiv128_s u128divqru(u128 a, uint64_t b)
{
    return u128divqr(a, tou128(b));
}

static inline u128 u128div(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    a.u128 /= b.u128;
    return a;
#else
    return u128divqr(a, b).quot;
#endif
}

static inline u128 u128divu(u128 a, uint64_t b)
{
    return u128div(a, tou128(b));
}

static inline u128 u128mod(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    a.u128 %= b.u128;
    return a;
#else
    return u128divqr(a, b).rem;
#endif
}

static inline u128 u128modu(u128 a, uint64_t b)
{
    return u128mod(a, tou128(b));
}

static inline u128 u128and(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    a.u128 &= b.u128;
#else
    a.upper &= b.upper;
    a.lower &= b.lower;
#endif
    return a;
}

static inline u128 u128andu(u128 a, uint64_t b)
{
    return u128and(a, tou128(b));
}

static inline u128 u128or(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    a.u128 |= b.u128;
#else
    a.upper |= b.upper;
    a.lower |= b.lower;
#endif
    return a;
}

static inline u128 u128oru(u128 a, uint64_t b)
{
    return u128or(a, tou128(b));
}

static inline u128 u128xor(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    a.u128 ^= b.u128;
#else
    a.upper ^= b.upper;
    a.lower ^= b.upper;
#endif
    return a;
}

static inline u128 u128xoru(u128 a, uint64_t b)
{
    return u128xor(a, tou128(b));
}

static inline u128 u128cpl(u128 u)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    u.u128 = ~u.u128;
#else
    u.upper = ~u.upper;
    u.lower = ~u.lower;
#endif
    return u;
}

static inline u128 u128shl(u128 u, uint bits)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)

    u.u128 <<= bits;
    return u;

#else

    // don't access UINT128_ZERO, better for inlining
    if (unlikely(bits < 0 || bits >= 128))
        return u128{ .upper = 0, .lower = 0};

    if (bits < 64) {
        u.upper = (u.upper << bits) | (u.lower >> (64 - bits));
        u.lower <<= bits;
    } else {
        u.upper = (u.lower << (bits - 64));
        u.lower = 0;
    }
    return u;

#endif
}

static inline u128 u128shr(u128 u, uint bits)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)

    u.u128 >>= bits;
    return u;

#else

    // don't access UINT128_ZERO, better for inlining
    if (unlikely(bits < 0 || bits >= 128))
        return u128{ .upper = 0, .lower = 0 };

    if (bits < 64) {
        u.lower = (u.upper << (64 - bits)) | (u.lower >> bits);
        u.upper = (u.upper >> bits);
    } else {
        u.lower = u.upper >> (bits - 64);
        u.upper = 0;
    }

    return u;

#endif
}

/**
 * u128bits:
 *
 * Calculate the number of bits necessary to represent a value.
 */
static inline int u128bits(u128 u)
{
    uint cup = bsr64(u128upper(u));
    return cup ? cup + 64 : bsr64(u128lower(u));
}


static inline int u128cmp(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)

    return (a.u128 > b.u128) - (a.u128 < b.u128);

#else

    if (a.upper != b.upper)
        return (a.upper > b.upper) - (a.upper < b.upper);

    return (a.lower > b.lower) - (a.lower < b.lower);

#endif
}

static inline bool u128eq(u128 a, u128 b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    return a.u128 == b.u128;
#else
    return a.lower == b.lower && a.upper == b.upper;
#endif
}

static inline int u128cmpu(u128 a, uint64_t b)
{
    return u128cmp(a, tou128(b));
}

static inline bool u128equ(u128 a, uint64_t b)
{
#if defined(__GNUC__) && !defined(UBGP_C_U128)
    return a.u128 == b;
#else
    return a.lower == b && a.upper == 0;
#endif
}

/**
 * u128tos:
 * @u: an #u128
 * @base: conversion base
 *
 * Unsigned 128-bit int to string.
 *
 * Convert a @u to its string representation.
 *
 * Returns: string representation of @u in the requested base,
 * the returned pointer references a possibly statically
 * allocated storage managed by the library, it must not be free()d.
 */
UBGP_API RETURNS_NONNULL char *u128tos(u128 u, int base);

/**
 * stou128:
 * @s: string representing the integer
 * @eptr: (nullable): destination for the end pointer, if not %NULL, it is set
 * to the char in @s immediately following the last processed digit
 * @base: integer base
 *
 * Convert string to #u128.
 *
 * Returns: integer value in string as a #u128, sets #errno on error.
 */
UBGP_API CHECK_NONNULL(1) u128 stou128(const char *s, char **eptr, int base);

#endif

