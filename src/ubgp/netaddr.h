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

#ifndef UBGP_NETADDR_H_
#define UBGP_NETADDR_H_

#include "branch.h"
#include "funcattribs.h"
#include "ubgpdef.h"

#include <assert.h>
#include <arpa/inet.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

/**
 * SECTION: netaddr
 * @title:   Network Prefixes and Addresses
 * @include: netaddr.h
 *
 * Network address family types, utilities and constants.
 *
 * This file is guaranteed to include `arpa/inet.h` and `stdint.h`,
 * it may include additional headers in the interest of providing inline
 * versions of its functions.
 */

/**
 * afi_t:
 *
 * Address Family Identifier values.
 */
typedef uint16_t afi_t;

/// @brief Notable values for \a afi_t.
enum {
    AFI_BAD = -1,

    AFI_IPV4 = 1,
    AFI_IPV6 = 2,
    AFI_IPX = 11,
    AFI_APPLETALK = 12
};

/**
 * @NADDR_CIDR:  prints with /bitlen
 * @NADDR_PLAIN: prints without /bitlen
 *
 * #netaddr_t printing modes, see naddrtos().
 */
enum {
    NADDR_CIDR,
    NADDR_PLAIN
};

/**
 * safi_t:
 *
 * Subsequent Address Family Identifier values
 *
 * See: [SAFI namespace](https://www.iana.org/assignments/safi-namespace/safi-namespace.xhtml)
 */
typedef uint8_t safi_t;

/**
 * @SAFI_BAD: bad value marker.
 * @SAFI_UNICAST: see [RFC4760](http://www.iana.org/go/rfc4760)
 * @SAFI_MULTICAST: see [RFC4760](http://www.iana.org/go/rfc4760)
 *
 * Notable values for #safi_t.
 */
enum {
    SAFI_BAD       = -1,
    SAFI_UNICAST   =  1,
    SAFI_MULTICAST =  2
};

/**
 * netaddr_t:
 * @family: Unix address family: %AF_UNSPEC, %AF_INET or %AF_INET6
 * @bitlen: address length in bits
 * @sin:    actual IPV4 address, valid when @family equals %AF_INET
 * @sin6:   actual IPV6 address. valid when @family equals %AF_INET6
 * @u16:    address payload as 16-bit words
 * @u32:    address payload as 32-bit words
 * @bytes:  address payload as raw bytes
 *
 * A network prefix.
 */
typedef struct {
    short family;
    ushort bitlen;
    union {
        struct in_addr  sin;
        struct in6_addr sin6;
        uint16_t        u16[sizeof(struct in6_addr) / sizeof(uint16_t)];
        uint32_t        u32[sizeof(struct in6_addr) / sizeof(uint32_t)];
        byte            bytes[sizeof(struct in6_addr)];
    };
} netaddr_t;

#define IPV4_SIZE sizeof(struct in_addr)
#define IPV4_BIT  (IPV4_SIZE * CHAR_BIT)
#define IPV6_SIZE sizeof(struct in6_addr)
#define IPV6_BIT  (IPV6_SIZE * CHAR_BIT)

typedef struct {
    netaddr_t pfx;
    uint32_t pathid;
} netaddrap_t;

/**
 * stonaddr:
 *
 * String to network address.
 */
UBGP_API CHECK_NONNULL(1, 2) int stonaddr(netaddr_t *ip, const char *s);

/**
 * naddrsize:
 * @bitlen: address size in bits
 *
 * Convert a bit size to byte size.
 *
 * Returns: network address size in bytes
 */
static inline uint naddrsize(uint bitlen)
{
    return (bitlen >> 3) + ((bitlen & 7) != 0);
}

/**
 * saddrfamily:
 * @s: (nullable): a string.
 *
 * Deduce address family from string representation.
 *
 * This function uses a simple and quick heuristic to detect the
 * family of an IP address in its string representation.
 * The fact that an address **might** be of a certain family does not
 * imply that it is correct, that may only be verified by
 * explicitly trying to convert it using stonaddr().
 * It is however true that if the address **is** actually valid, then
 * its family is certainly the one returned by this function.
 *
 * Returns: address type encoded inside @s:
 * * %AF_INET if address appears to be an IPv4 address,
 * * %AF_INET6 if address appears to be an IPv6 address,
 * * %AF_UNSPEC if string is definitely a bad address.
 */
static inline sa_family_t saddrfamily(const char *s)
{
    if (unlikely(!s))
        return AF_UNSPEC;

    for (uint i = 0; i < 4; i++) {
        char c = *s++;
        if (c == '.')
            return AF_INET;
        if (c == ':')
            return AF_INET6;
        if (unlikely(c == '\0'))
            return AF_UNSPEC;
    }
    return likely(*s == ':') ? AF_INET6 : AF_UNSPEC;
}

/**
 * makenaddr:
 *
 * Make a network address from an IP address.
 */
static inline CHECK_NONNULL(1) void makenaddr(netaddr_t   *ip,
                                              sa_family_t  family,
                                              const void  *sin,
                                              uint         bitlen)
{
    assert(bitlen <= 128);

    ip->bitlen = bitlen;
    ip->family = family;
    ip->u32[0] = ip->u32[1] = ip->u32[2] = ip->u32[3] = 0;
    memcpy(ip->bytes, sin, naddrsize(bitlen));
}

static inline CHECK_NONNULL(1, 2) bool prefixeqwithmask(const netaddr_t *addr,
                                                        const netaddr_t *dest,
                                                        uint             mask)
{
    assert(mask <= 128);

    uint m = ~0u << (8 - (mask & 7));

    switch (mask) {
    NODEFAULT;

    case 128:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u32[2] == dest->u32[2] && addr->u32[3] == dest->u32[3];

    case 127: case 126: case 125: case 124: case 123: case 122: case 121:
        if (((addr->bytes[15] & m) != (dest->bytes[15] & m)))
            return false;

        FALLTHROUGH;
    case 120:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u32[2] == dest->u32[2] && addr->u16[6] == dest->u16[6] &&
               addr->bytes[14] == dest->bytes[14];

    case 119: case 118: case 117: case 116: case 115: case 114: case 113:
        if (((addr->bytes[14] & m) != (dest->bytes[14] & m)))
            return false;

        FALLTHROUGH;
    case 112:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u32[2] == dest->u32[2] && addr->u16[6] == dest->u16[6];

    case 111: case 110: case 109: case 108: case 107: case 106: case 105:
        if (((addr->bytes[13] & m) != (dest->bytes[13] & m)))
            return false;

        FALLTHROUGH;
    case 104:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u32[2] == dest->u32[2] && addr->bytes[12] == dest->bytes[12];

    case 103: case 102: case 101: case 100: case 99: case 98: case 97:
        if (((addr->bytes[12] & m) != (dest->bytes[12] & m)))
            return false;

        FALLTHROUGH;
    case 96:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u32[2] == dest->u32[2];

    case 95: case 94: case 93: case 92: case 91: case 90: case 89:
        if (((addr->bytes[11] & m) != (dest->bytes[11] & m)))
            return 0;

        FALLTHROUGH;
    case 88:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u16[4] == dest->u16[4] && addr->bytes[10] == dest->bytes[10];

    case 87: case 86: case 85: case 84: case 83: case 82: case 81:
        if (((addr->bytes[10] & m) != (dest->bytes[10] & m)))
            return false;

        FALLTHROUGH;
    case 80:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->u16[4] == dest->u16[4];

    case 79: case 78: case 77: case 76: case 75: case 74: case 73:
        if (((addr->bytes[9] & m) != (dest->bytes[9] & m)))
            return false;

        FALLTHROUGH;
    case 72:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1] &&
               addr->bytes[8] == dest->bytes[8];

    case 71: case 70: case 69: case 68: case 67: case 66: case 65:
        if (((addr->bytes[8] & m) != (dest->bytes[8] & m)))
            return false;

        FALLTHROUGH;
    case 64:
        return addr->u32[0] == dest->u32[0] && addr->u32[1] == dest->u32[1];

    case 63: case 62: case 61: case 60: case 59: case 58: case 57:
        if (((addr->bytes[7] & m) != (dest->bytes[7] & m)))
            return false;

        FALLTHROUGH;
    case 56:
        return addr->u32[0] == dest->u32[0] && addr->u16[2] == dest->u16[2] &&
               addr->bytes[6] == addr->bytes[6];

    case 55: case 54: case 53: case 52: case 51: case 50: case 49:
        if (((addr->bytes[6] & m) != (dest->bytes[6] & m)))
            return 0;

        FALLTHROUGH;
    case 48:
        return addr->u32[0] == dest->u32[0] && addr->u16[2] == dest->u16[2];

    case 47: case 46: case 45: case 44: case 43: case 42: case 41:
        if (((addr->bytes[5] & m) != (dest->bytes[5] & m)))
            return false;

        FALLTHROUGH;
    case 40:
        return addr->u32[0] == dest->u32[0] && addr->bytes[4] == dest->bytes[4];

    case 39: case 38: case 37: case 36: case 35: case 34: case 33:
        if (((addr->bytes[4] & m) != (dest->bytes[4] & m)))
            return false;

        FALLTHROUGH;
    case 32:
        return addr->u32[0] == dest->u32[0];

    case 31: case 30: case 29: case 28: case 27: case 26: case 25:
        if (((addr->bytes[3] & m) != (dest->bytes[3] & m)))
            return false;

        FALLTHROUGH;
    case 24:
        return addr->u16[0] == dest->u16[0] && addr->bytes[2] == dest->bytes[2];

    case 23: case 22: case 21: case 20: case 19: case 18: case 17:
        if (((addr->bytes[2] & m) != (dest->bytes[2] & m)))
            return false;

        FALLTHROUGH;
    case 16:
        return addr->u16[0] == dest->u16[0];

    case 15: case 14: case 13: case 12: case 11: case 10: case 9:
        if (((addr->bytes[1] & m) != (dest->bytes[1] & m)))
            return false;

        FALLTHROUGH;
    case 8:
        return addr->bytes[0] == dest->bytes[0];

    case 7: case 6: case 5: case 4: case 3: case 2: case 1:
        return (addr->bytes[0] & m) == (dest->bytes[0] & m);

    case 0:
        return true;
    }
}

static inline CHECK_NONNULL(1, 2) bool prefixeq(const netaddr_t *a,
                                                const netaddr_t *b)
{
    return a->family == b->family
        && a->bitlen == b->bitlen
        && prefixeqwithmask(a, b, a->bitlen);
}

static inline CHECK_NONNULL(1, 2) bool naddreq(const netaddr_t *a,
                                               const netaddr_t *b)
{
    if (a->family != b->family)
        return false;

    // TODO benchmark a branch-less option
    switch (a->family) {
    case AF_INET:
        return memcmp(&a->sin, &b->sin, sizeof(a->sin)) == 0;
    case AF_INET6:
        return memcmp(&a->sin6, &b->sin6, sizeof(a->sin6)) == 0;
    default:
        return true;  // both AF_UNSPEC...
    }
}

/**
 * naddrtos:
 *
 * Network address to string.
 *
 * Returns: string representation of the provided network address.
 * The returned pointer refers to a possibly statically allocated
 * zone managed by the library and must not be free()d.
 */
UBGP_API RETURNS_NONNULL CHECK_NONNULL(1)
char *naddrtos(const netaddr_t *ip, int mode);

/**
 * isnaddrreserved:
 * @ip: a #netaddr_t.
 *
 * Determines if the address is reserved according to IANA specs
 * [1](http://www.iana.org/assignments/ipv6-address-space/ipv6-address-space.xhtml)
 * [2](http://www.iana.org/assignments/ipv4-address-space/ipv4-address-space.xhtml)
 * [2015-05-11].
 *
 * Returns: %true if the address is reserved, %false otherwise.
 */
UBGP_API PUREFUNC CHECK_NONNULL(1) bool isnaddrreserved(const netaddr_t *ip);

#endif

