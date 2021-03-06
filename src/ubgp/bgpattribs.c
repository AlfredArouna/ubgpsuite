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

#include "bgpattribs.h"
#include "strutil.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

UBGP_API void *getmpnlri(const bgpattr_t *attr, size_t *pn)
{
    assert(attr->code == MP_REACH_NLRI_CODE || attr->code == MP_UNREACH_NLRI_CODE);

    byte *ptr = (byte *) &attr->len;

    size_t len = *ptr++;
    if (attr->flags & ATTR_EXTENDED_LENGTH) {
        len <<= 8;
        len |= *ptr++;
    }

    byte *start = ptr;

    ptr += sizeof(uint16_t) + sizeof(uint8_t);
    if (attr->code == MP_REACH_NLRI_CODE) {
        ptr += *ptr + 1; // skip NEXT_HOP
        ptr++;           // skip reserved
    }
    if (likely(pn))
        *pn = len - (ptr - start);

    return ptr;
}

UBGP_API void *getmpnexthop(const bgpattr_t *attr, size_t *pn)
{
    assert(attr->code == MP_REACH_NLRI_CODE);

    byte *ptr = (byte *) &attr->data[!!(attr->flags & ATTR_EXTENDED_LENGTH)];

    ptr += sizeof(uint16_t) + sizeof(uint8_t);
    if (likely(pn))
        *pn = *ptr;

    ptr++;
    return ptr;
}

UBGP_API int stobgporigin(const char *s)
{
    if (strcasecmp(s, "i") == 0 || strcasecmp(s, "igp") == 0)
        return ORIGIN_IGP;
    else if (strcasecmp(s, "e") == 0 || strcasecmp(s, "egp") == 0)
        return ORIGIN_EGP;
    else if (strcmp(s, "?") == 0 || strcasecmp(s, "incomplete") == 0)
        return ORIGIN_INCOMPLETE;
    else
        return ORIGIN_BAD;
}

UBGP_API bgpattr_t *putasseg32(bgpattr_t *attr, int seg_type, const uint32_t *seg, size_t count)
{
    assert(attr->code == AS_PATH_CODE || attr->code == AS4_PATH_CODE);

    int extended = attr->flags & ATTR_EXTENDED_LENGTH;

    byte *ptr = &attr->len;

    size_t len   = *ptr++;
    size_t limit = ATTR_LENGTH_MAX;
    if (extended) {
        len <<= 8;
        len |= *ptr++;

        limit = ATTR_EXTENDED_LENGTH_MAX;
    }

    size_t size = count * sizeof(*seg);

    size += AS_SEGMENT_HEADER_SIZE;
    if (unlikely(len + size > limit))
        return NULL;  // would overflow attribute length
    if (unlikely(count > AS_SEGMENT_COUNT_MAX))
        return NULL;  // would overflow segment size

    ptr   += len;
    *ptr++ = seg_type;  // segment type
    *ptr++ = count;     // AS count
    for (size_t i = 0; i < count; i++) {
        uint32_t as32 = beswap32(*seg++);
        memcpy(ptr, &as32, sizeof(as32));
        ptr += sizeof(as32);
    }

    // write updated length
    len += size;
    ptr  = &attr->len;
    if (extended) {
        *ptr++ = (len >> 8);
        len &= 0xff;
    }
    *ptr++ = len;
    return attr;
}

UBGP_API bgpattr_t *putasseg16(bgpattr_t *attr, int type, const uint16_t *seg, size_t count)
{
    assert(attr->code == AS_PATH_CODE);

    byte *ptr = &attr->len;

    int extended = attr->flags & ATTR_EXTENDED_LENGTH;
    size_t len   = *ptr++;
    size_t limit = ATTR_LENGTH_MAX;
    if (extended) {
        len <<= 8;
        len |= *ptr++;

        limit = ATTR_EXTENDED_LENGTH_MAX;
    }

    size_t size = count * sizeof(*seg);
    size += AS_SEGMENT_HEADER_SIZE;
    if (unlikely(len + size > limit))
        return NULL;  // would overflow attribute length
    if (unlikely(count > AS_SEGMENT_COUNT_MAX))
        return NULL;  // would overflow segment size

    ptr   += len;
    *ptr++ = type;   // segment type
    *ptr++ = count;  // AS count
    for (size_t i = 0; i < count; i++) {
        uint16_t as16 = beswap16(*seg++);
        memcpy(ptr, &as16, sizeof(as16));
        ptr += sizeof(as16);
    }

    // write updated length
    ptr  = &attr->len;
    len += size;
    if (extended) {
        *ptr++ = (len >> 8);
        len &= 0xff;
    }
    *ptr++ = len;
    return attr;
}

UBGP_API bgpattr_t *setmpafisafi(bgpattr_t *dst, afi_t afi, safi_t safi)
{
    assert(dst->code == MP_REACH_NLRI_CODE || dst->code == MP_UNREACH_NLRI_CODE);

    byte *ptr = &dst->len;

    ptr++;
    if (dst->flags & ATTR_EXTENDED_LENGTH)
        ptr++;

    uint16_t t = beswap16(afi);
    memcpy(ptr, &t, sizeof(t));
    ptr += sizeof(t);

    *ptr = safi;
    return dst;
}

UBGP_API bgpattr_t *putmpnexthop(bgpattr_t *dst, int family, const void *addr)
{
    assert(dst->code == MP_REACH_NLRI_CODE);
    assert(family == AF_INET || family == AF_INET6);

    byte *ptr = &dst->len;

    int extended = dst->flags & ATTR_EXTENDED_LENGTH;
    size_t len   = *ptr++;
    size_t limit = ATTR_LENGTH_MAX;
    if (extended) {
        len <<= 8;
        len |= *ptr++;

        limit = ATTR_EXTENDED_LENGTH_MAX;
    }

    size_t n = (family == AF_INET) ? sizeof(struct in_addr) : sizeof(struct in6_addr);
    if (unlikely(len + n > limit))
        return NULL;  // would overflow attribute length

    byte *nhlen = ptr + sizeof(uint16_t) + sizeof(uint8_t); // keep a pointer to NEXT_HOP length
    if (unlikely(*nhlen + n > 0xff))
        return NULL;  // would overflow next-hop length
    if (unlikely(nhlen + sizeof(uint8_t) + *nhlen != &ptr[len]))
        return NULL;  // attribute already has a NLRI field!

    memcpy(&ptr[len], addr, n);

    // write updated NEXT HOP length
    *nhlen += n;

    // write updated length
    ptr  = &dst->len;
    len += n;
    if (extended) {
        *ptr++ = (len >> 8);
        len &= 0xff;
    }
    *ptr++ = len;
    return dst;
}

UBGP_API bgpattr_t *putmpnlri(bgpattr_t *dst, const netaddr_t *addr)
{
    assert(dst->code == MP_REACH_NLRI_CODE || dst->code == MP_UNREACH_NLRI_CODE);

    byte *ptr = &dst->len;

    int extended = dst->flags & ATTR_EXTENDED_LENGTH;
    size_t len   = *ptr++;
    size_t limit = ATTR_LENGTH_MAX;
    if (extended) {
        len <<= 8;
        len |= *ptr++;

        limit = ATTR_EXTENDED_LENGTH_MAX;
    }

    size_t n = naddrsize(addr->bitlen);
    if (unlikely(len + n + 1 > limit))
        return NULL;  // would overflow attribute length

    ptr   += len;
    *ptr++ = addr->bitlen;
    memcpy(ptr, addr->bytes, n);

    // write updated length
    ptr  = &dst->len;
    len += n + 1;
    if (extended) {
        *ptr++ = (len >> 8);
        len &= 0xff;
    }
    *ptr++ = len;
    return dst;
}

UBGP_API bgpattr_t *putmpnlriap(bgpattr_t *dst, const netaddrap_t *addr)
{
    assert(dst->code == MP_REACH_NLRI_CODE || dst->code == MP_UNREACH_NLRI_CODE);

    byte *ptr = &dst->len;

    int extended = dst->flags & ATTR_EXTENDED_LENGTH;
    size_t len   = *ptr++;
    size_t limit = ATTR_LENGTH_MAX;
    if (extended) {
        len <<= 8;
        len |= *ptr++;

        limit = ATTR_EXTENDED_LENGTH_MAX;
    }

    size_t n = naddrsize(addr->pfx.bitlen);
    uint32_t netpathid = beswap32(addr->pathid);
    if (unlikely(len + n + 1 + sizeof(netpathid) > limit))
        return NULL;  // would overflow attribute length

    ptr   += len;
    memcpy(ptr, &netpathid, sizeof(netpathid));
    ptr   += sizeof(netpathid);
    *ptr++ = addr->pfx.bitlen;
    memcpy(ptr, addr->pfx.bytes, n);

    // write updated length
    ptr  = &dst->len;
    len += n + 1 + sizeof(netpathid);
    if (extended) {
        *ptr++ = (len >> 8);
        len &= 0xff;
    }
    *ptr++ = len;
    return dst;
}

/*
static size_t appendsegment(byte **pptr, byte *end, int type, const void *ases, size_t as_size, size_t count)
{
    if (count == 0)
        return 0;

    byte *ptr = *pptr;

    size_t size = count * as_size;
    size_t total = AS_SEGMENT_HEADER_SIZE + size;
    if (likely(ptr + total <= end)) {
        *ptr++ = type;
        *ptr++ = count;
        memcpy(ptr, ases, size);

        ptr += size;
    } else {
        ptr = end; // don't append anymore
    }

    *pptr = ptr;
    return total;
}

enum {
    PARSE_AS_NONE = -1,
    PARSE_AS_ERR  = -2
};

static long long parseas(const char *s, size_t as_size, char **eptr)
{
        unsigned long long as_max = (1ull << as_size * CHAR_BIT) - 1;

        int err = errno;  // don't pollute errno

        errno = 0;

        long long val = strtoll(s, eptr, 10);
        if (s == *eptr) {
            // this is not an error per se,
            // we just made it to the end of the parsable string
            errno = err;  // some POSIX set this to EINVAL
            return PARSE_AS_NONE;
        }
        if (val < 0 || (unsigned long long) val > as_max)
            errno = ERANGE; // range error to us
        if (errno != 0)
            return PARSE_AS_ERR;

        errno = err;  // all good, restore errno
        return val;
}

static size_t parseasset(byte **pptr, byte *end, size_t as_size, const char *s, char **eptr)
{
    union {
        uint32_t as32[AS_SEGMENT_COUNT_MAX];
        uint16_t as16[AS_SEGMENT_COUNT_MAX];
    } segbuf;

    size_t count = 0;
    size_t len   = 0;
    bool commas  = false;

    char *epos;
    while (true) {
        while (isspace(*s)) s++;

        char c = *s++;
        if (c == '}') {
            s++;
            break;
        }

        if (c == ',') {
            commas |= (count == 1);   // enable commas only after first AS
            if (!commas) {
                // unexpected comma
                errno = EINVAL;
                goto out;
            }

            s++;
            while (isspace(*s)) s++;

            if (*s == '\0') {
                // unexpected end of string
                errno = EINVAL;
                goto out;
            }

        } else if (commas) {
            // missing comma
            errno = EINVAL;
            goto out;
        }

        if (unlikely(count == AS_SEGMENT_COUNT_MAX)) {
            errno = ERANGE;
            goto out;
        }

        long long as = parseas(s, as_size, &epos);
        if (as == PARSE_AS_ERR)
            goto out;
        if (as == PARSE_AS_NONE)
            break;

        if (as_size == sizeof(uint32_t))
            segbuf.as32[count++] = as;
        else
            segbuf.as16[count++] = as;

        s = epos;
    }

    if (count == 0) {
        errno = EINVAL;
        ptr   = end;
        goto out;
    }

    // output segment
    len = appendsegment(pptr, end, AS_SEGMENT_SEQ, &segbuf, as_size, count);

out:
    return len;
}

static size_t parseaspath(byte **pptr, byte *end, const char *s, size_t as_size, char **eptr)
{
    union {
        uint32_t as32[AS_SEGMENT_COUNT_MAX];
        uint16_t as16[AS_SEGMENT_COUNT_MAX];
    } segbuf;

    size_t count = 0;
    size_t len = 0;

    char *epos;
    while (true) {
        while (isspace(*s)) s++;

        if (*s == '\0')
            break;

        if (*s == '{') {
            // AS set segment
            s++;

            // append SEQ segment so far, NOP if count == 0
            len += appendsegment(pptr, end, AS_SEGMENT_SEQ, &segbuf, as_size, count);
            count = 0;

            // go on parsing the SET segment
            len += parseasset(pptr, end, as_size, s, &epos);
        } else {
            // additional AS sequence segment entry
            if (unlikely(count == AS_SEGMENT_COUNT_MAX) {
                errno = ERANGE;
                goto out;
            }

            long long as = parseas(s, as_size, &epos);
            if (as == PARSE_AS_ERR)
                goto out;
            if (as == PARSE_AS_NONE)
                break;

            if (as_size == sizeof(uint32_t))
                segbuf.as32[count++] = as;
            else
                segbuf.as16[count++] = as;
        }

        s = epos;
    }

    // flush last sequence
    len += appendsegment(pptr, end, AS_SEGMENT_SEQ, &segbuf, as_size, count);

out:
    *eptr = (char *) s;
    return len;
}

size_t stoaspath(bgpattr_t *attr, size_t n, int code, int flags, size_t as_size, const char *s, char **eptr)
{
    char *dummy;
    size_t off = 0;

    if (!eptr)
        eptr = &dummy;

    byte *ptr = &attr->code;
    byte *end = ptr + n;
    if (ptr < end)
        *ptr++ = code;
    if (ptr < end)
        *ptr++ = flags;
    if (ptr < end)
        *ptr++ = 0;
    if ((flags & ATTR_EXTENDED_LENGTH) != 0 && ptr < end)
        *ptr++ = 0;

    size_t len  = hdrsize;
    len        += parseaspath(&ptr, end, s, sizeof(uint32_t), eptr);
    return len;
}

size_t stoaspath16(bgpattr_t *attr, size_t n, int flags, const char *s, char **eptr)
{
    char *dummy;
    size_t off = 0;

    if (!eptr)
        eptr = &dummy;

    size_t hdrsize = ATTR_HEADER_SIZE;
    if (flags & ATTR_EXTENDED_LENGTH)
        hdrsize = ATTR_EXTENDED_HEADER_SIZE;

    if (n >= hdrsize)
        makeaspath(buf, flags);

    off += hdrsize;
    parseaspath(buf, &off, n, s, sizeof(uint16_t), eptr);
    return off;
}

size_t stoas4path(bgpattr_t *attr, size_t n, int flags, const char *s, char **eptr)
{
    char *dummy;
    size_t off = 0;

    if (!eptr)
        eptr = &dummy;

    size_t hdrsize = ATTR_HEADER_SIZE;
    if (flags & ATTR_EXTENDED_LENGTH)
        hdrsize = ATTR_EXTENDED_HEADER_SIZE;

    if (n >= hdrsize)
        makeas4path(buf, flags);

    off += hdrsize;
    parseaspath(buf, &off, n, s, sizeof(uint32_t), eptr);
    return off;
}
*/

static bgpattr_t *appendcommunities(bgpattr_t *attr, const void *commptr, size_t bytes)
{
    byte *ptr = &attr->len;
    int extended       = attr->flags & ATTR_EXTENDED_LENGTH;

    size_t limit = ATTR_LENGTH_MAX;
    size_t len   = *ptr++;
    if (extended) {
        limit = ATTR_EXTENDED_LENGTH_MAX;
        len <<= 8;
        len |= *ptr++;
    }
    if (unlikely(len + bytes > limit))
        return NULL;

    memcpy(ptr + len, commptr, bytes);
    len += bytes;

    ptr = &attr->len;
    if (extended) {
        *ptr++  = len >> 8;
        len    &= 0xff;
    }

    *ptr = len;
    return attr;
}

UBGP_API bgpattr_t *putcommunities(bgpattr_t *attr, community_t c)
{
    assert(attr->code == COMMUNITY_CODE);

    c = beswap32(c);
    return appendcommunities(attr, &c, sizeof(c));
}

UBGP_API bgpattr_t *putexcommunities(bgpattr_t *attr, ex_community_t c)
{
    assert(attr->code == EXTENDED_COMMUNITY_CODE);

    return appendcommunities(attr, &c, sizeof(c));
}

UBGP_API bgpattr_t *putlargecommunities(bgpattr_t *attr, large_community_t c)
{
    assert(attr->code == LARGE_COMMUNITY_CODE);

    c.global  = beswap32(c.global);
    c.hilocal = beswap32(c.hilocal);
    c.lolocal = beswap32(c.lolocal);
    return appendcommunities(attr, &c, sizeof(c));
}

static const struct {
    const char *str;
    community_t community;
} str2wellknown[] = {
    {"PLANNED_SHUT", COMMUNITY_PLANNED_SHUT},
    {"ACCEPT_OWN_NEXTHOP", COMMUNITY_ACCEPT_OWN_NEXTHOP},  // NOTE: must come BEFORE "ACCEPT_OWN", see stocommunity()
    {"ACCEPT_OWN", COMMUNITY_ACCEPT_OWN},
    {"ROUTE_FILTER_TRANSLATED_V4", COMMUNITY_ROUTE_FILTER_TRANSLATED_V4},
    {"ROUTE_FILTER_V4", COMMUNITY_ROUTE_FILTER_V4},
    {"ROUTE_FILTER_TRANSLATED_V6", COMMUNITY_ROUTE_FILTER_TRANSLATED_V6},
    {"ROUTE_FILTER_V6", COMMUNITY_ROUTE_FILTER_V6},
    {"LLGR_STALE", COMMUNITY_LLGR_STALE},
    {"NO_LLGR", COMMUNITY_NO_LLGR},
    {"BLACKHOLE", COMMUNITY_BLACKHOLE},
    {"NO_EXPORT_SUBCONFED", COMMUNITY_NO_EXPORT_SUBCONFED},  // NOTE: must come BEFORE "NO_EXPORT", see stocommunity()
    {"NO_EXPORT", COMMUNITY_NO_EXPORT} ,
    {"NO_ADVERTISE", COMMUNITY_NO_ADVERTISE},
    {"NO_PEER", COMMUNITY_NO_PEER}
};

UBGP_API char *communitytos(community_t c, int mode)
{
    static _Thread_local char buf[digsof(uint16_t) + 1 + digsof(uint16_t) + 1];

    if (mode == COMMSTR_EX) {
        for (size_t i = 0; i < countof(str2wellknown); i++) {
            if (str2wellknown[i].community == c)
                return (char *) str2wellknown[i].str;
        }
    }

    char *ptr;

    utoa(buf, &ptr, c >> 16);
    *ptr++ = ':';
    utoa(ptr, NULL, c & 0xffff);
    return buf;
}

UBGP_API char *largecommunitytos(large_community_t c)
{
    static _Thread_local char buf[3 * digsof(uint32_t) + 2 + 1];

    char *ptr;

    ultoa(buf, &ptr, c.global);
    *ptr++ = ':';
    ultoa(ptr, &ptr, c.hilocal);
    *ptr++ = ':';
    ultoa(ptr, NULL, c.lolocal);
    return buf;
}

static uint32_t parsecommfield(const char *s, char **eptr, ullong max)
{
    // don't accept sign before field
    const char *ptr = s;
    while (isspace((uchar) *ptr)) ptr++;

    if (!isdigit((uchar) *ptr)) {
        // no conversion, invalid community
        *eptr = (char *) s;
        return 0;
    }
    if (*ptr == '0') {
        // RFC only allows exactly one 0 for community values
        *eptr = (char *) (ptr + 1);
        return 0;
    }

    ullong val = strtoull(ptr, eptr, 10);
    if (val > max) {
        errno = ERANGE;
        return UINT32_MAX;
    }

    // since isdigit(*ptr) is true, a conversion certainly took place,
    // and we don't need to restore errno, since EINVAL can't happen

    return (uint32_t) val;
}

UBGP_API community_t stocommunity(const char *s, char **eptr)
{
    for (size_t i = 0; i < countof(str2wellknown); i++) {
        if (startswith(s, str2wellknown[i].str)) {
            if (eptr)
                *eptr = (char *) (s + strlen(str2wellknown[i].str));

            return str2wellknown[i].community;
        }
    }

    char *epos;
    uint32_t upper = parsecommfield(s, &epos, UINT16_MAX);
    
    if (s == epos || *epos != ':') {
        if (eptr)
            *eptr = epos;
        return 0;
    }

    s = epos + 1; // skip ":"

    uint32_t lower = parsecommfield(s, &epos, UINT16_MAX);
    
    if (eptr)
        *eptr = epos;

    return (upper << 16) | lower;
}

UBGP_API large_community_t stolargecommunity(const char *s, char **eptr)
{
    const large_community_t zero = {0};

    large_community_t comm;

    const char *ptr = s;
    uint32_t *dst = &comm.global;

    char *epos;
    for (int i = 0; i < 3; i++) {
        *dst++ = parsecommfield(ptr, &epos, UINT32_MAX);
        if (ptr == epos) {
            // no conversion happened, restore position and bail out
            epos = (char *) s;
            comm = zero;
            break;
        }

        ptr = epos;
        if (i != 2 && *ptr != ':') {
            // bad format, restore position and bail out
            epos = (char *) s;
            comm = zero;
            break;
        }

        ptr++;
    }

    if (eptr)
        *eptr = epos;

    return comm;
}

/* TODO
ex_community_v6_t stoexcommunityv6(const char *s, char **eptr)
{
}
*/

