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

#ifndef UBGP_BITOPS_H_
#define UBGP_BITOPS_H_

#include "ubgpdef.h"

#include <limits.h>
#include <stdint.h>

/**
 * SECTION: bitops
 * @title:   Bit Twiddling
 * @include: bitops.h
 *
 * Optimized implementation for common low level bit manipulation facilities.
 *
 * See also: [Bit Twiddling Hacks by Sean Eron Anderson](https://graphics.stanford.edu/~seander/bithacks.html)
 */

/**
 * bsf:
 * @w: an #ullong
 *
 * Bit scan forward (BSF).
 * Scans for the first bit set to 1 inside @w from LSB to MSB.
 *
 * Returns: index of the first bit set, 1 is the rightmost bit (LSB).
 * 0 if no bit is set in @w.
 */
static inline uint bsf(ullong w)
{
#ifdef __GNUC__

    return __builtin_ffsll(w);

#else

    for (uint i = 1; w; w >>= 1, i++) {
        if (w & 1)
            return i;
    }

    return 0;

#endif
}

/**
 * bsf32:
 *
 * bsf() variant for 32-bit values.
 */
static inline uint bsf32(uint32_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(uint))
        return __builtin_ffs(w);
    if (sizeof(w) == sizeof(ulong))
        return __builtin_ffsl(w);
#endif

    return bsf(w);
}

/**
 * bsf64:
 *
 * bsf() variant for 64-bit values.
 */
static inline uint bsf64(uint64_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(ulong))
        return __builtin_ffsl(w);
#endif

    return bsf(w);
}

/**
 * bsr32:
 *
 * bsr64() variant for 64-bit values.
 */
static inline uint bsr32(uint32_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(uint))
        return w ? 32 - __builtin_clz(w) : 0;
    if (sizeof(w) == sizeof(ulong))
        return w ? 32 - __builtin_clzl(w) : 0;
#endif

    uint i;
    uint32_t mask;

    for (i = 32, mask = 1 << 31; mask; mask >>= 1, i--) {
        if (mask & w)
            break;
    }

    return i;
}

/**
 * bsr64:
 * @w: a 64 bit integer
 *
 * Bit scan reverse (BSR), 64-bit variant.
 * Scans for the first bit set to 1 inside @w, from MSB to LSB.
 *
 * Returns: index of the first bit set, 1 is the rightmost bit (LSB).
 * 0 if no bit is set in @w.
 */
static inline uint bsr64(uint64_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(ulong))
        return w ? 64 - __builtin_clzl(w) : 0;
    if (sizeof(w) == sizeof(ullong))
        return w ? 64 - __builtin_clzll(w) : 0;
#endif

    uint i;
    uint64_t mask;

    for (i = 64, mask = 1ull << 63; mask; mask >>= 1, i--) {
        if (mask & w)
            break;
    }

    return i;
}

/**
 * popcnt:
 * @w: an #ullong.
 *
 * Population Count, count the number of bits set to 1.
 *
 * Returns: number of bits set in @w, 0 if no bit is set.
 */
static inline uint popcnt(ullong w)
{
#ifdef __GNUC__

    return __builtin_popcountll(w);

#else

    uint c;

    for (c = 0; w; c++)
        bits &= w - 1;

    return c;

#endif
}

/**
 * popcnt32:
 *
 * popcnt() variant for 32-bit values.
 */
static inline uint popcnt32(uint32_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(uint))
        return __builtin_popcount(w);
    if (sizeof(w) == sizeof(ulong))
        return __builtin_popcountl(w);
#endif

    return popcnt(w);
}

/**
 * popcnt64:
 *
 * popcnt() variant for 64-bit values.
 */
static inline uint popcnt64(uint64_t w)
{
#ifdef __GNUC__
    if (sizeof(w) == sizeof(ulong))
        return __builtin_popcountl(w);
#endif

    return popcnt(w);
}

/**
 * rol8:
 *
 * rol64() variant for 8-bit values.
 */
static inline uint8_t rol8(uint8_t w, uint shift)
{
    return (w << shift) | (w >> (8 - shift));
}

/**
 * ror8:
 *
 * ror64() variant for 8-bit values.
 */
static inline uint8_t ror8(uint8_t w, uint shift)
{
    return (w >> shift) | (w << (8 - shift));
}

/**
 * rol16:
 *
 * rol64() variant for 16-bit values.
 */
static inline uint16_t rol16(uint16_t w, uint shift)
{
    return (w << shift) | (w >> (16 - shift));
}

/**
 * ror16:
 *
 * ror64() variant for 16-bit values.
 */
static inline uint16_t ror16(uint16_t w, uint shift)
{
    return (w >> shift) | (w << (16 - shift));
}

/**
 * rol32:
 *
 * rol64() variant for 32-bit values.
 */
static inline uint32_t rol32(uint32_t w, uint shift)
{
    return (w << shift) | (w >> (32 - shift));
}

/**
 * ror32:
 *
 * ror64() variant for 32-bit values.
 */
static inline uint32_t ror32(uint32_t w, uint shift)
{
    return (w >> shift) | (w << (32 - shift));
}

/**
 * rol64:
 * @w: an #uint64_t
 * @shift: shift quantity
 *
 * Rotate left (ROL) bits inside @w about @shift bits, 64-bit variant.
 * Bits pushed out to the left are inserted back to the right.
 *
 * Returns: rotation result.
 */
static inline uint64_t rol64(uint64_t w, uint shift)
{
    return (w << shift) | (w >> (64 - shift));
}

/**
 * ror64:
 * @w: an #uint64_t
 * @shift: shift quantity
 *
 * Rotate right (ROR) bits inside @w about @shift bits, 64-bit variant.
 * Bits pushed out to the right are inserted back to the left.
 *
 * Returns: rotation result.
 */
static inline uint64_t ror64(uint64_t w, uint shift)
{
    return (w >> shift) | (w << (64 - shift));
}

#endif

