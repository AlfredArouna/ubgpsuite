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

#include "endian.h"
#include "netaddr.h"
#include "strutil.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

UBGP_API int stonaddr(netaddr_t *ip, const char *s)
{
    char *last = strrchr(s, '/');

    int af = saddrfamily(s);

    void *dst = &ip->sin;
    unsigned maxbitlen = 32;

    if (af == AF_INET6) {
        dst = &ip->sin6;
        maxbitlen = 128;
    }
    
    ip->bitlen = 0;
    ip->family = AF_UNSPEC;
    memset(&ip->sin6, 0, sizeof(ip->sin6));

    size_t n;
    long bitlen;
    if (last) {
        n = last - s;
        last++;

        char* endptr;

        errno = 0;
        bitlen = strtol(last, &endptr, 10);

        if (bitlen < 0 || bitlen > maxbitlen)
            errno = ERANGE;
        if (last == endptr || *endptr != '\0')
            errno = EINVAL;
        if (errno != 0)
            return -1;
    } else {
        n =  strlen(s);
        bitlen = maxbitlen;
    }

    char buf[n + 1];
    memcpy(buf, s, n);
    buf[n] = '\0';

    if (inet_pton(af, buf, dst) != 1)
        return -1;

    ip->bitlen = bitlen;
    ip->family = af;

    return 0;
}

UBGP_API char* naddrtos(const netaddr_t *ip, int mode)
{
    static _Thread_local char buf[INET6_ADDRSTRLEN + 1 + digsof(ip->bitlen) + 1];

    char *ptr;
    int i, best, max;

    switch (ip->family) {
    case AF_INET:
        utoa(buf, &ptr, ip->bytes[0]);
        *ptr++ = '.';
        utoa(ptr, &ptr, ip->bytes[1]);
        *ptr++ = '.';
        utoa(ptr, &ptr, ip->bytes[2]);
        *ptr++ = '.';
        utoa(ptr, &ptr, ip->bytes[3]);
        break;
    case AF_INET6:
        if (ip->u32[0] == 0 && ip->u32[1] == 0 && ip->u16[4] == 0 && ip->u16[5] == 0xff) {
            // v4 mapped to v6 (starts with 0:0:0:0:0:0:ffff)
            memcpy(buf, "::ffff:", 7);
            ptr = buf + 7;

            utoa(ptr, &ptr, ip->bytes[0]);
            *ptr++ = '.';
            utoa(ptr, &ptr, ip->bytes[1]);
            *ptr++ = '.';
            utoa(ptr, &ptr, ip->bytes[2]);
            *ptr++ = '.';
            utoa(ptr, &ptr, ip->bytes[3]);
            break;
        }

        // typical v6
        xtoa(buf, &ptr, beswap16(ip->u16[0]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[1]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[2]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[3]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[4]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[5]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[6]));
        *ptr++ = ':';
        xtoa(ptr, &ptr, beswap16(ip->u16[7]));

        // replace longest /(^0|:)[:0]{2,}/ with "::"
        for (i = best = 0, max = 2; buf[i] != '\0'; i++) {
            if (i > 0 && buf[i] != ':')
                continue;

            // count the number of consecutive 0 in this substring
            int n = 0;
            for (int j = i; buf[j] != '\0'; j++) {
                if (buf[j] != ':' && buf[j] != '0')
                    break;

                n++;
            }

            // store the position if this is the best we've seen so far
            if (n > max) {
                best = i;
                max = n;
            }
        }

        ptr = buf + i;
        if (max > 3) {
            // we can compress the string
            buf[best] = ':';
            buf[best + 1] = ':';

            int len = i - best - max;
            memmove(buf + best + 2, buf + best + max, len + 1);
            ptr = buf + best + 2 + len;
        }

        break;
    default:
        return "invalid";
    }


    if (mode == NADDR_CIDR) {
        *ptr++ = '/';
        utoa(ptr, NULL, ip->bitlen);
    }

    return buf;
}

UBGP_API bool isnaddrreserved(const netaddr_t *ip)
{
    if (ip->family == AF_INET6) {
        if (ip->bitlen == 0)
            return true;

        uint16_t a = ip->u16[0];
        a = beswap16(a);

        uint16_t b = ip->u16[1];
        b = beswap16(b);

        // 2000::/3 is the only one routable
        if (a < 8192 || a > 16383)
            return true;

        // 2001:0000::/23
        if (a == 8193)
            if (b <= 511)
                return true;

        // 2001:db8::/32
        if (a == 8193 && b == 3512)
            return true;

        // 2001:10::/28
        if (a == 8193)
            if (b >= 16 && b <= 31)
                return true;

        // 2002::/16
        if (a == 8194)
            return true;
    } else {
    
        if (ip->bitlen == 0)
            return true;

        byte a = ip->bytes[0];
        byte b = ip->bytes[1];
        byte c = ip->bytes[2];

        if (a == 10 || a == 127 || a >= 224)
            return true;

        if (a == 100)
            if (b >= 64 && b <= 127)
                return true;

        if (a == 169 && b == 254)
            return true;

        if (a == 172)
            if (b >= 16 && b <= 31)
                return true;

        if (a == 192 && b == 0 && c == 2)
            return true;

        if (a == 192 && b == 88 && c == 99)
            return true;

        if (a == 192 && b == 0 && c == 0)
            return true;

        if (a == 198)
            if (b == 18 || b == 19)
                return true;

        if (a == 198 && b == 51 && c == 100)
            return true;

        if (a == 203 && b == 0 && c == 113)
            return true;
    }
    return false;
}

