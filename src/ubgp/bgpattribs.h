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

#ifndef UBGP_BGPATTRIBS_H_
#define UBGP_BGPATTRIBS_H_

#include "branch.h"
#include "endian.h"
#include "netaddr.h"

#include <assert.h>
#include <arpa/inet.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/**
 * SECTION: bgpattribs
 * @title:  BGP UPDATE Attributes
 * @include: bgpattribs.h
 *
 * Functions for reading and encoding BGP attributes.
 */

/**
 * @ATTR_BAD_CODE: invalid attribute code, error indicator value.
 * @ORIGIN_CODE: origin attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @AS_PATH_CODE: AS Path attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @NEXT_HOP_CODE: next hop attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @MULTI_EXIT_DISC_CODE: next hop attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @LOCAL_PREF_CODE: local pref attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @ATOMIC_AGGREGATE_CODE: atomic aggregator attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @AGGREGATOR_CODE: aggregator attribute code, see [RFC 4271](https://datatracker.ietf.org/doc/rfc4271/)
 * @COMMUNITY_CODE:  RFC 1997
 * @ORIGINATOR_ID_CODE: RFC 4456
 * @CLUSTER_LIST_CODE: RFC 4456
 * @DPA_CODE:  RFC 6938
 * @ADVERTISER_CODE: RFC 1863, RFC 4223 and RFC 6938
 * @RCID_PATH_CLUSTER_ID_CODE: RFC 1863, RFC 4223 and RFC 6938
 * @MP_REACH_NLRI_CODE: RFC 4760
 * @MP_UNREACH_NLRI_CODE: RFC 4760
 * @EXTENDED_COMMUNITY_CODE: RFC 4360
 * @AS4_PATH_CODE: RFC 6793
 * @AS4_AGGREGATOR_CODE: RFC 6793
 * @SAFI_SSA_CODE: https://tools.ietf.org/html/draft-kapoor-nalawade-idr-bgp-ssa-02 and https://tools.ietf.org/html/draft-wijnands-mt-discovery-01
 * @CONNECTOR_CODE: RFC 6037
 * @AS_PATHLIMIT_CODE: https://tools.ietf.org/html/draft-ietf-idr-as-pathlimit-03
 * @PMSI_TUNNEL_CODE: RFC 6514
 * @TUNNEL_ENCAPSULATION_CODE: RFC 5512
 * @TRAFFIC_ENGINEERING_CODE: RFC 5543
 * @IPV6_ADDRESS_SPECIFIC_EXTENDED_COMMUNITY_CODE: RFC 5701
 * @AIGP_CODE: RFC 7311
 * @PE_DISTINGUISHER_LABELS_CODE: RFC 6514
 * @BGP_ENTROPY_LEVEL_CAPABILITY_CODE: RFC 6790 and RFC 7447
 * @BGP_LS_CODE: RFC 7752
 * @LARGE_COMMUNITY_CODE: RFC 8092
 * @BGPSEC_PATH_CODE: https://tools.ietf.org/html/draft-ietf-sidr-bgpsec-protocol-22
 * @BGP_COMMUNITY_CONTAINER_CODE: https://tools.ietf.org/html/draft-ietf-idr-wide-bgp-communities-04
 * @BGP_PREFIX_SID_CODE: https://tools.ietf.org/html/draft-ietf-idr-bgp-prefix-sid-06
 * @ATTR_SET_CODE: RFC 6368
 * @RESERVED_CODE: RFC 2042
 *
 *[IANA Reference](https://www.iana.org/assignments/bgp-parameters/bgp-parameters.xhtml#bgp-parameters-2)
 */
enum {
    ATTR_BAD_CODE = -1,

    ORIGIN_CODE                                   = 1,
    AS_PATH_CODE                                  = 2,
    NEXT_HOP_CODE                                 = 3,
    MULTI_EXIT_DISC_CODE                          = 4,
    LOCAL_PREF_CODE                               = 5,
    ATOMIC_AGGREGATE_CODE                         = 6,
    AGGREGATOR_CODE                               = 7,
    COMMUNITY_CODE                                = 8,
    ORIGINATOR_ID_CODE                            = 9,
    CLUSTER_LIST_CODE                             = 10,
    DPA_CODE                                      = 11,
    ADVERTISER_CODE                               = 12,
    RCID_PATH_CLUSTER_ID_CODE                     = 13,
    MP_REACH_NLRI_CODE                            = 14,
    MP_UNREACH_NLRI_CODE                          = 15,
    EXTENDED_COMMUNITY_CODE                       = 16,
    AS4_PATH_CODE                                 = 17,
    AS4_AGGREGATOR_CODE                           = 18,
    SAFI_SSA_CODE                                 = 19,
    CONNECTOR_CODE                                = 20,
    AS_PATHLIMIT_CODE                             = 21,
    PMSI_TUNNEL_CODE                              = 22,
    TUNNEL_ENCAPSULATION_CODE                     = 23,
    TRAFFIC_ENGINEERING_CODE                      = 24,
    IPV6_ADDRESS_SPECIFIC_EXTENDED_COMMUNITY_CODE = 25,
    AIGP_CODE                                     = 26,
    PE_DISTINGUISHER_LABELS_CODE                  = 27,
    BGP_ENTROPY_LEVEL_CAPABILITY_CODE             = 28,
    BGP_LS_CODE                                   = 29,
    LARGE_COMMUNITY_CODE                          = 32,
    BGPSEC_PATH_CODE                              = 33,
    BGP_COMMUNITY_CONTAINER_CODE                  = 34,
    BGP_PREFIX_SID_CODE                           = 40,
    ATTR_SET_CODE                                 = 128,
    RESERVED_CODE                                 = 255
};

/**
 * @ATTR_EXTENDED_LENGTH: attribute length has an additional byte
 * @ATTR_PARTIAL:         attribute is partial
 * @ATTR_TRANSITIVE:      attribute is transitive
 * @ATTR_OPTIONAL:        attribute is optional
 *
 * Bit constants for the attribute flags fields.
 */
enum {
    ATTR_EXTENDED_LENGTH = 1 << 4,
    ATTR_PARTIAL         = 1 << 5,
    ATTR_TRANSITIVE      = 1 << 6,
    ATTR_OPTIONAL        = 1 << 7
};

enum {
    ORIGIN_BAD        = -1,

    ORIGIN_IGP        = 0,
    ORIGIN_EGP        = 1,
    ORIGIN_INCOMPLETE = 2
};

/**
 * Constants relevant for AS Path segments.
 *
 * See: putasseg16(), putasseg32()
 */
enum {
    AS_SEGMENT_HEADER_SIZE = 2,
    AS_SEGMENT_COUNT_MAX = 0xff,

    AS_SEGMENT_BAD = -1,

    AS_SEGMENT_SET = 1,
    AS_SEGMENT_SEQ = 2
};

enum {
    ATTR_HEADER_SIZE          = 3 * sizeof(uint8_t),
    ATTR_EXTENDED_HEADER_SIZE = 2 * sizeof(uint8_t) + sizeof(uint16_t),

    ATTR_LENGTH_MAX          = 0xff,
    ATTR_EXTENDED_LENGTH_MAX = 0xffff,

    // Origin attribute offset, relative to BGP attribute header size.
    ORIGIN_LENGTH           = sizeof(uint8_t),
    ORIGINATOR_ID_LENGTH    = sizeof(uint32_t),
    ATOMIC_AGGREGATE_LENGTH = 0,
    NEXT_HOP_LENGTH         = sizeof(struct in_addr),
    MULTI_EXIT_DISC_LENGTH  = sizeof(uint32_t),
    LOCAL_PREF_LENGTH       = sizeof(uint32_t),

    AGGREGATOR_AS32_LENGTH = sizeof(uint32_t) + sizeof(struct in_addr),
    AGGREGATOR_AS16_LENGTH = sizeof(uint16_t) + sizeof(struct in_addr),

    AS4_AGGREGATOR_LENGTH  = sizeof(uint32_t) + sizeof(struct in_addr)
};

/**
 * @COMMUNITY_PLANNED_SHUT: Planned Shut well known community,
 *                          [Graceful BGP session shutdown](https://datatracker.ietf.org/doc/draft-francois-bgp-gshut/)
 * @COMMUNITY_ACCEPT_OWN: RFC 7611
 * @COMMUNITY_ROUTE_FILTER_TRANSLATED_V4: https://tools.ietf.org/html/draft-francois-bgp-gshut-01
 * @COMMUNITY_ROUTE_FILTER_V4: https://tools.ietf.org/html/draft-l3vpn-legacy-rtc-00
 * @COMMUNITY_ROUTE_FILTER_TRANSLATED_V6: https://tools.ietf.org/html/draft-l3vpn-legacy-rtc-00
 * @COMMUNITY_ROUTE_FILTER_V6: https://tools.ietf.org/html/draft-l3vpn-legacy-rtc-00
 * @COMMUNITY_LLGR_STALE: https://tools.ietf.org/html/draft-uttaro-idr-bgp-persistence-03
 * @COMMUNITY_NO_LLGR: https://tools.ietf.org/html/draft-uttaro-idr-bgp-persistence-03
 * @COMMUNITY_ACCEPT_OWN_NEXTHOP: ?
 * @COMMUNITY_BLACKHOLE: BLACKHOLE community, see [RFC 7999](https://datatracker.ietf.org/doc/rfc7999/)
 * @COMMUNITY_NO_EXPORT: see [RFC 1997](https://datatracker.ietf.org/doc/rfc1997/)
 * @COMMUNITY_NO_ADVERTISE: see [RFC 1997](https://datatracker.ietf.org/doc/rfc1997/)
 * @COMMUNITY_NO_EXPORT_SUBCONFED: see [RFC 1997](https://datatracker.ietf.org/doc/rfc1997/)
 * COMMUNITY_NO_PEER: see [RFC 3765](https://datatracker.ietf.org/doc/rfc3765/)
 *
 * Well known communities constants,
 * see [IANA Well known communities list](https://www.iana.org/assignments/bgp-well-known-communities/bgp-well-known-communities.xhtml)
 */
enum {
    COMMUNITY_PLANNED_SHUT               = (int) 0xffff0000,
    COMMUNITY_ACCEPT_OWN                 = (int) 0xffff0001,
    COMMUNITY_ROUTE_FILTER_TRANSLATED_V4 = (int) 0xffff0002,
    COMMUNITY_ROUTE_FILTER_V4            = (int) 0xffff0003,
    COMMUNITY_ROUTE_FILTER_TRANSLATED_V6 = (int) 0xffff0004,
    COMMUNITY_ROUTE_FILTER_V6            = (int) 0xffff0005,
    COMMUNITY_LLGR_STALE                 = (int) 0xffff0006,
    COMMUNITY_NO_LLGR                    = (int) 0xffff0007,
    COMMUNITY_ACCEPT_OWN_NEXTHOP         = (int) 0xffff0008,
    COMMUNITY_BLACKHOLE                  = (int) 0xffff029a,
    COMMUNITY_NO_EXPORT                  = (int) 0xffffff01,
    COMMUNITY_NO_ADVERTISE               = (int) 0xffffff02,
    COMMUNITY_NO_EXPORT_SUBCONFED        = (int) 0xffffff03,
    COMMUNITY_NO_PEER                    = (int) 0xffffff04
};

/**
 * community_t:
 *
 * BGP Community, see [BGP Communities Attribute](https://tools.ietf.org/html/rfc1997).
 */
typedef uint32_t community_t;

/**
 * ex_community_t:
 * @hitype:      high octet of the type field
 * @lotype:      low octet of the type field, may be part of the value
 * @hival:       most significant portion of value field
 * @loval:       least significant portion of value field
 * @two_subtype: two-octets subtype field
 * @two_global;  two-octets global field
 * @two_local;   two-octets local field
 * @v4_subtype;  IPv4 Address subtype field
 * @v4_higlobal; most significant portion of IPv4 Address global field
 * @v4_loglobal; least significant portion of IPv4 Address global field
 * @v4_local;    IPv4 Address local field
 * @typeval;     to manage the extended community as a whole
 *
 * Extended community structure, see [RFC4360](https://datatracker.ietf.org/doc/rfc4360/).
 */
typedef union {
    struct {
        uint8_t hitype;
        uint8_t lotype;
        uint16_t hival;
        uint32_t loval;
    };
    struct {
        uint8_t : 8;
        uint8_t two_subtype;
        uint16_t two_global;
        uint32_t two_local;
    };
    struct {
        uint8_t : 8;
        uint8_t v4_subtype;
        uint16_t v4_higlobal;
        uint16_t v4_loglobal;
        uint16_t v4_local;
    };
    struct {
        uint8_t : 8;
        uint8_t opq_subtype;
        uint16_t opq_hival;
        uint32_t opq_loval;
    };
    uint64_t typeval;
} ex_community_t;

static_assert(sizeof(ex_community_t) == 8, "Unsupported platform");

/**
 * Notable bits inside #ex_community_t `hitype` field.
 */
enum {
    IANA_AUTHORITY_BIT       = 1 << 7,
    TRANSITIVE_COMMUNITY_BIT = 1 << 6
};

/**
 * getv4addrglobal:
 * @ecomm: an #ex_community_t
 *
 * Retrieve the whole global v4 address extended community field.
 */
static inline uint32_t getv4addrglobal(ex_community_t ecomm)
{
    return ((uint32_t) ecomm.v4_higlobal << 16) | ecomm.v4_loglobal;
}

/**
 * getopaquevalue:
 * @ecomm: an #ex_community_t
 *
 * Retrieve the opaque value inside an extended community.
 */
static inline uint64_t getopaquevalue(ex_community_t ecomm)
{
    return ((uint64_t) ecomm.opq_hival << 16) | ecomm.opq_loval;
}

/**
 * ex_community_v6_t:
 * @hitype: high order byte of the type field
 * @lotype: low order byte of the type field
 * @global: packed IPv6 global administrator address
 * @local:  local administrator field
 *
 * IPv6 specific extended community structure,
 * see [RFC 5701](https://datatracker.ietf.org/doc/rfc5701/)
 */
typedef struct {
    uint8_t  hitype;
    uint8_t  lotype;
    uint8_t  global[16];
    uint16_t local;
} ex_community_v6_t;

static_assert(sizeof(ex_community_v6_t) == 20, "Unsupported platform");

static inline void getexcomm6globaladdr(struct in6_addr         *dst,
                                        const ex_community_v6_t *excomm6)
{
    memcpy(dst, excomm6->global, sizeof(*dst));
}

/**
 * large_community_t:
 *
 * Large community structure,
 * see [RFC 8092](https://datatracker.ietf.org/doc/rfc8092/).
 */
typedef struct {
    uint32_t global;
    uint32_t hilocal;
    uint32_t lolocal;
} large_community_t;

static_assert(sizeof(large_community_t) == 12, "Unsupported platform");

typedef struct {
    uint8_t flags;
    uint8_t code;
    union {
        struct {
            uint8_t len;
            uint8_t data[1];
        };
        struct {
            uint8_t exlen[2];
            uint8_t exdata[1];
        };
    };
} bgpattr_t;

static inline void *getattrlen(const bgpattr_t *attr, size_t *psize)
{
    byte   *ptr = (byte *) &attr->len;
    size_t  len = *ptr++;
    if (attr->flags & ATTR_EXTENDED_LENGTH) {
        len <<= 8;
        len |= *ptr++;
    }
    if (likely(psize))
        *psize = len;

    return ptr;
}

static inline int getorigin(const bgpattr_t *attr)
{
    assert(attr->code == ORIGIN_CODE);

    return attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)];
}

static inline bgpattr_t *setorigin(bgpattr_t *dst, int origin)
{
    assert(dst->code == ORIGIN_CODE);

    dst->data[!!(dst->flags & ATTR_EXTENDED_LENGTH)] = origin;
    return dst;
}

enum {
    DEFAULT_ORIGIN_FLAGS = ATTR_TRANSITIVE,
    EXTENDED_ORIGIN_FLAGS = DEFAULT_ORIGIN_FLAGS | ATTR_EXTENDED_LENGTH,

    DEFAULT_NEXT_HOP_FLAGS = ATTR_TRANSITIVE,
    EXTENDED_NEXT_HOP_FLAGS = DEFAULT_NEXT_HOP_FLAGS | ATTR_EXTENDED_LENGTH,

    DEFAULT_AS_PATH_FLAGS = ATTR_TRANSITIVE,
    EXTENDED_AS_PATH_FLAGS = DEFAULT_AS_PATH_FLAGS | ATTR_EXTENDED_LENGTH,

    DEFAULT_AS4_PATH_FLAGS = ATTR_TRANSITIVE | ATTR_OPTIONAL,
    EXTENDED_AS4_PATH_FLAGS = DEFAULT_AS4_PATH_FLAGS | ATTR_EXTENDED_LENGTH,

    DEFAULT_MP_REACH_NLRI_FLAGS = ATTR_OPTIONAL,
    EXTENDED_MP_REACH_NLRI_FLAGS = DEFAULT_MP_REACH_NLRI_FLAGS | ATTR_EXTENDED_LENGTH,
    MP_REACH_BASE_LEN = sizeof(uint16_t) + 3 * sizeof(uint8_t),

    DEFAULT_MP_UNREACH_NLRI_FLAGS = ATTR_OPTIONAL,
    EXTENDED_MP_UNREACH_NLRI_FLAGS = DEFAULT_MP_UNREACH_NLRI_FLAGS | ATTR_EXTENDED_LENGTH,
    MP_UNREACH_BASE_LEN = sizeof(uint16_t) + sizeof(uint8_t),

    DEFAULT_COMMUNITY_FLAGS = ATTR_TRANSITIVE | ATTR_OPTIONAL,
    EXTENDED_COMMUNITY_FLAGS = DEFAULT_COMMUNITY_FLAGS | ATTR_EXTENDED_LENGTH
};

UBGP_API bgpattr_t *setmpafisafi(bgpattr_t *dst, afi_t afi, safi_t safi);

UBGP_API bgpattr_t *putmpnexthop(bgpattr_t *dst, int family, const void *addr);

UBGP_API bgpattr_t *putmpnlri(bgpattr_t *dst, const netaddr_t *addr);

UBGP_API bgpattr_t *putmpnlriap(bgpattr_t *dst, const netaddrap_t *addr);

static inline void *getaspath(const bgpattr_t *attr, size_t *pn)
{
    assert(attr->code == AS_PATH_CODE || attr->code == AS4_PATH_CODE);

    byte *ptr = (byte *) &attr->len;

    size_t len = *ptr++;
    if (attr->flags & ATTR_EXTENDED_LENGTH) {
        len <<= 8;
        len |= *ptr++;
    }

    if (likely(pn))
        *pn = len;

    return ptr;
}

/**
 * putasseg16:
 *
 * Put 16-bits wide AS segment into AS path attribute.
 *
 * This function is identical to putasseg32(), but takes 16-bits wide ASes.
 */
UBGP_API bgpattr_t *putasseg16(bgpattr_t      *dst,
                               int             type,
                               const uint16_t *seg,
                               size_t          count);

/**
 * putasseg32:
 * @attr:            buffer containing an AS Path attribute, the segment
 *                   is appended at attribute's end. Behavior is undefined if
 *                   this argument references anything other than a legitimate
 *                   AS Path attribute.
 * @type             segment type, either %AS_SEGMENT_SEQ or %AS_SEGMENT_SET,
 *                   specifying anything else is undefined behavior.
 * @seg: (nullable): AS buffer for segment, this value may only be %NULL if
 *                   @count is also 0.
 * @count:           number of ASes stored into @seg.
 *
 * Put 32-bits wide AS segment into AS path attribute.
 *
 * Append an AS segment to an AS Path attribute, ASes are 32-bits wide.
 *
 * This function does not take any additional parameter to specify the
 * size of @buf, because in no way this function shall append more than:
 * ```c
 *     AS_SEGMENT_HEADER_SIZE + count * sizeof(*seg);
 * ```
 * bytes to the provided attribute, whose current size can be computed
 * using:
 * ```c
 *     bgpattrhdrsize(buf) + bgpattrlen(buf);
 * ```
 *
 *  The caller has the ability to ensure that this function shall not
 *  overflow the buffer.
 *  In any way, this function cannot generate oversized attributes, hence
 *  a buffer of size %ATTR_EXTENDED_SIZE_MAX is always large enough for
 *  any extended attribute, and %ATTR_SIZE_MAX is always large enough to
 *  hold non-extended attributes.
 *
 * Returns: the @buf argument, %NULL on error. Possible errors are:
 *          * The ASes count would overflow the segment header count field
 *            (i.e. @count is larger than %AS_SEGMENT_COUNT_MAX).
 *          * Appending the segment to the specified AS Path would overflow
 *            the attribute length (i.e. the overall AS Path length in bytes
 *            would be larger than %ATTR_LENGTH_MAX or
 *            %ATTR_EXTENDED_LENGTH_MAX, depending on the
 *            %ATTR_EXTENDED_LENGTH flag status of the attribute).
 *
 */
UBGP_API bgpattr_t *putasseg32(bgpattr_t      *attr,
                               int             type,
                               const uint32_t *seg,
                               size_t          count);

// TODO size_t stoaspath(bgpattr_t *attr, size_t n, int code, int flags, size_t as_size, const char *s, char **eptr);

static inline uint32_t getoriginatorid(const bgpattr_t *attr)
{
    assert(attr->code == ORIGINATOR_ID_CODE);

    uint32_t id;
    memcpy(&id, &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], sizeof(id));
    return beswap32(id);
}

static inline bgpattr_t *setoriginatorid(bgpattr_t *attr, uint32_t id)
{
    assert(attr->code == ORIGINATOR_ID_CODE);

    id = beswap32(id);
    memcpy(&attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], &id, sizeof(id));
    return attr;
}

static inline struct in_addr getnexthop(const bgpattr_t *attr)
{
    assert(attr->code == NEXT_HOP_CODE);

    struct in_addr in;
    memcpy(&in, &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], sizeof(in));
    return in;
}

static inline bgpattr_t *setnexthop(bgpattr_t *attr, struct in_addr in)
{
    assert(attr->code == NEXT_HOP_CODE);

    memcpy(&attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], &in, sizeof(in));
    return attr;
}

static inline uint32_t getmultiexitdisc(const bgpattr_t *attr)
{
    assert(attr->code == MULTI_EXIT_DISC_CODE);

    uint32_t disc;
    memcpy(&disc, &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], sizeof(disc));
    return beswap32(disc);
}

static inline bgpattr_t *setmultiexitdisc(bgpattr_t *attr, uint32_t disc)
{
    assert(attr->code == MULTI_EXIT_DISC_CODE);

    disc = beswap32(disc);
    memcpy(&attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], &disc, sizeof(disc));
    return attr;
}

static inline uint32_t getlocalpref(const bgpattr_t *attr)
{
    assert(attr->code == LOCAL_PREF_CODE);

    uint32_t pref;
    memcpy(&pref, &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], sizeof(pref));
    return beswap32(pref);
}

static inline bgpattr_t *setlocalpref(bgpattr_t *attr, uint32_t pref)
{
    assert(attr->code == LOCAL_PREF_CODE);

    pref = beswap32(pref);
    memcpy(&attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)], &pref, sizeof(pref));
    return attr;
}

static inline uint32_t getaggregatoras(const bgpattr_t *attr)
{
    assert(attr->code == AGGREGATOR_CODE || attr->code == AS4_AGGREGATOR_CODE);

    const byte *ptr = &attr->len;
    size_t len = *ptr++;
    if (attr->flags & ATTR_EXTENDED_LENGTH) {
        len <<= 8;
        len |= *ptr++;
    }

    if (len == AGGREGATOR_AS32_LENGTH) {
        uint32_t as32;
        memcpy(&as32, ptr, sizeof(as32));
        return beswap32(as32);
    } else {
        uint16_t as16;
        memcpy(&as16, ptr, sizeof(as16));
        return beswap16(as16);
    }
}

static inline struct in_addr getaggregatoraddress(const bgpattr_t *attr)
{
    assert(attr->code == AGGREGATOR_CODE || attr->code == AS4_AGGREGATOR_CODE);

    const byte *ptr = &attr->len;
    size_t len = *ptr++;
    if (attr->flags & ATTR_EXTENDED_LENGTH) {
        len <<= 8;
        len |= *ptr++;
    }

    struct in_addr in;

    ptr += len;
    ptr -= sizeof(in);

    memcpy(&in, ptr, sizeof(in));
    return in;
}

static inline bgpattr_t *setaggregator(bgpattr_t *attr, uint32_t as, size_t as_size, struct in_addr in)
{
    assert(attr->code == AGGREGATOR_CODE || attr->code == AS4_AGGREGATOR_CODE);

    if (unlikely(as_size != sizeof(uint16_t) && as_size != sizeof(uint32_t)))
        return NULL;

    byte *ptr = &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)];
    if (as_size == sizeof(uint32_t)) {
        uint32_t as32 = beswap32(as);
        memcpy(ptr, &as32, sizeof(as32));
        ptr += sizeof(as32);
    } else {
        uint16_t as16 = beswap16(as);
        memcpy(ptr, &as16, sizeof(as16));
        ptr += sizeof(as16);
    }

    memcpy(ptr, &in, sizeof(in));
    return attr;
}

static inline afi_t getmpafi(const bgpattr_t *attr)
{
    assert(attr->code == MP_REACH_NLRI_CODE || attr->code == MP_UNREACH_NLRI_CODE);

    const byte *ptr = &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)];

    uint16_t t;
    memcpy(&t, ptr, sizeof(t));
    return beswap16(t);
}

static inline safi_t getmpsafi(const bgpattr_t *attr)
{
    assert(attr->code == MP_REACH_NLRI_CODE || attr->code == MP_UNREACH_NLRI_CODE);

    return attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH) + sizeof(uint16_t)];
}

UBGP_API void *getmpnlri(const bgpattr_t *attr, size_t *pn);

UBGP_API void *getmpnexthop(const bgpattr_t *attr, size_t *pn);

UBGP_API bgpattr_t *putcommunities(bgpattr_t *attr, community_t c);

UBGP_API bgpattr_t *putexcommunities(bgpattr_t *attr, ex_community_t c);

UBGP_API bgpattr_t *putlargecommunities(bgpattr_t *attr, large_community_t c);

enum {
    COMMSTR_EX,
    COMMSTR_PLAIN
};

/**
 * communitytos:
 *
 * Community to string.
 */
UBGP_API char *communitytos(community_t c, int mode);

/**
 * stocommunity:
 *
 * String to community.
 */
UBGP_API community_t stocommunity(const char *s, char **eptr);

/**
 * stoexcommunity6:
 *
 * String to IPv6 specific extended community attribute.
 */
UBGP_API ex_community_t stoexcommunity6(const char *s, char **eptr);

/**
 * largecommunitytos:
 *
 * Large community to string.
 */
UBGP_API char *largecommunitytos(large_community_t c);

/**
 * stolargecommunity:
 *
 * String to large community attribute.
 */
UBGP_API large_community_t stolargecommunity(const char *s, char **eptr);

#endif

