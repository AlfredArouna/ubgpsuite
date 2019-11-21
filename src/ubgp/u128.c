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

#include "u128.h"

#include <ctype.h>
#include <errno.h>

static int digval(int ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'Z')
        return ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'z')
        return ch - 'a' + 10;
    else
        return -1;
}

#if !defined(__GNUC__) || defined(UBGP_C_U128)

UBGP_API u128 u128mul(u128 a, u128 b)
{
    // split values into 4 32-bit parts
    uint64_t top[4] = {
        a.upper >> 32,
        a.upper & 0xffffffff,
        a.lower >> 32,
        a.lower & 0xffffffff
    };
    uint64_t bottom[4] = {
        b.upper >> 32,
        b.upper & 0xffffffff,
        b.lower >> 32,
        b.lower & 0xffffffff
    };

    uint64_t products[4][4];

    for (int y = 3; y > -1; y--) {
        for (int x = 3; x > -1; x--)
            products[3 - x][y] = top[x] * bottom[y];
    }

    // initial row
    uint64_t fourth32 =  products[0][3] & 0xffffffff;
    uint64_t third32  = (products[0][2] & 0xffffffff) + (products[0][3] >> 32);
    uint64_t second32 = (products[0][1] & 0xffffffff) + (products[0][2] >> 32);
    uint64_t first32  = (products[0][0] & 0xffffffff) + (products[0][1] >> 32);

    // second row
    third32  +=  products[1][3] & 0xffffffff;
    second32 += (products[1][2] & 0xffffffff) + (products[1][3] >> 32);
    first32  += (products[1][1] & 0xffffffff) + (products[1][2] >> 32);

    // third row
    second32 +=  products[2][3] & 0xffffffff;
    first32  += (products[2][2] & 0xffffffff) + (products[2][3] >> 32);

    // fourth row
    first32 += products[3][3] & 0xffffffff;

    // combines the values, taking care of carry over
    u128 x = u128from(first32 << 32, 0);
    u128 y = u128from(third32 >> 32, third32 << 32);
    u128 z = u128from(second32, 0);
    u128 w = tou128(fourth32);

    u128 r = u128add(x, y);
    r      = u128add(r, z);
    r      = u128add(r, w);
    return r;
}

UBGP_API udiv128_s u128divqr(u128 a, u128 b)
{
    udiv128_s qr = {
        .quot = 0,
        .rem  = b
    };

    // keep dreaming about trivial cases...
    if (unlikely(u128eq(b, U128_ONE))) {
        qr.quot = a;
        qr.rem = 0;
        return qr;
    }
    if (unlikely(u128eq(a, b))) {
        qr.quot = 1;
        qr.rem = 0;
        return qr;
    }
    if (u128eq(a, U128_ZERO) || u128cmp(a, b) < 0) {
        qr.quot = 0;
        qr.rem = b;
        return qr;
    }

    // sorry state of affairs...
    int abits = u128bits(a);
    int bbits = u128bits(b);
    u128 copyd = u128shl(b, abits - bbits);
    u128 adder = u128shl(U128_ONE, abits - bbits);
    if (u128cmp(copyd, qr.rem) > 0) {
        u128shru(copyd, 1);
        u128shru(adder, 1);
    }
    while (u128cmp(qr.rem, b) >= 0) {
        if (u128cmp(qr.rem, copyd) >= 0) {
            qr.rem  = u128sub(qr.rem, copyd);
            qr.quot = u128or(qr.quot, adder);
        }
        u128shr(copyd, 1);
        u128shr(adder, 1);
    }
    return qr;
}

#endif


UBGP_API u128 stou128(const char *s, char **eptr, int base)
{
    while (isspace((uchar) *s)) s++;

    bool minus = false;
    if (*s == '-' || *s == '+')
        minus = (*s++ == '-');

    if (base == 0) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
            s += 2;
            base = 16;
        } else if (s[0] == '0') {
            s++;
            base = 8;
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
            s += 2;
    }

    int dig;
    u128 u = U128_ZERO;
    while ((dig = digval(*s)) >= 0 && dig < base) {
        u128 v = u128muladdu(u, base, dig);
        if (unlikely(u128cmp(u, v) > 0)) {
            // overflow (keep going to consume all digits in string)
            errno = ERANGE;
            v = U128_MAX;
        }

        u = v;
        s++;
    }

    if (eptr)
        *eptr = (char *) s;

    if (minus)
        u = u128neg(u);

    return u;
}

UBGP_API char *u128tos(u128 u, int base)
{
    static _Thread_local char buf[2 + digsof(u) + 1];

    if (base < 2 || base > 36)
        base = 0;

    if (base == 0)
        base = 10;

    udiv128_s qr;
    qr.quot = u;
    qr.rem  = U128_ZERO;

    const char *digs = "0123456789abcdefghijklmnopqrstuvwxyz";
    char *ptr        = buf + sizeof(buf) - 1;

    *ptr = '\0';
    do {
        qr = u128divqru(qr.quot, base);

        *--ptr = digs[u128lower(qr.rem)];
    } while (!u128eq(qr.quot, U128_ZERO));

    if (base == 16) {
        *--ptr = 'x';
        *--ptr = '0';
    }
    if (base == 8 && !u128eq(u, U128_ZERO))
        *--ptr = '0';

    return ptr;
}

