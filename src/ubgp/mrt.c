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

#include "branch.h"
#include "endian.h"
#include "mrt.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MRTGROWSTEP 256

// offsets for various MRT packet fields
enum {
    // common MRT packet offset
    TIMESTAMP_OFFSET = 0,
    TYPE_OFFSET      = TIMESTAMP_OFFSET + sizeof(uint32_t),
    SUBTYPE_OFFSET   = TYPE_OFFSET + sizeof(uint16_t),
    LENGTH_OFFSET    = SUBTYPE_OFFSET + sizeof(uint16_t),
    MESSAGE_OFFSET   = LENGTH_OFFSET + sizeof(uint32_t),
    MRT_HDRSIZ       = MESSAGE_OFFSET,

    // extended version
    MICROSECOND_TIMESTAMP_OFFSET = MESSAGE_OFFSET,
    MESSAGE_EXTENDED_OFFSET      = MICROSECOND_TIMESTAMP_OFFSET + sizeof(uint32_t),
    EXTENDED_MRT_HDRSIZ          = MESSAGE_EXTENDED_OFFSET
};

// packet reader/writer status flags
enum {
    MAX_MRT_SUBTYPE = MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH,

    F_VALID     = 1 << 0,
    F_AS32      = 1 << 1,
    F_IS_PI     = 1 << 2,
    F_NEEDS_PI  = 1 << 3,
    F_IS_EXT    = 1 << 4,
    F_IS_BGP    = 1 << 5,
    F_HAS_STATE = 1 << 6,
    F_WRAPS_BGP = 1 << 7,
    F_ADDPATH   = 1 << 8,

    F_RD   = 1 << 10,        // packet opened for read
    F_WR   = 1 << (10 + 1),  // packet opened for write
    F_RDWR = F_RD | F_WR,    // shorthand for `(F_RD | F_WR)`

    F_PE = 1 << (10 + 2),
    F_RE = 1 << (10 + 3)
};

#define SHIFT(idx) ((idx) - MRT_BGP)

static const uint16_t masktab[][MAX_MRT_SUBTYPE + 1] = {
    [SHIFT(MRT_TABLE_DUMP)][AFI_IPV4] = F_VALID | F_WRAPS_BGP,
    [SHIFT(MRT_TABLE_DUMP)][AFI_IPV6] = F_VALID | F_WRAPS_BGP,

    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_PEER_INDEX_TABLE]           = F_VALID | F_IS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_GENERIC]                = F_VALID | F_NEEDS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH]        = F_VALID | F_NEEDS_PI | F_ADDPATH,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST]           = F_VALID | F_NEEDS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST_ADDPATH]   = F_VALID | F_NEEDS_PI | F_ADDPATH,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST]         = F_VALID | F_NEEDS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST_ADDPATH] = F_VALID | F_NEEDS_PI | F_ADDPATH,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST]           = F_VALID | F_NEEDS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST_ADDPATH]   = F_VALID | F_NEEDS_PI | F_ADDPATH,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST]         = F_VALID | F_NEEDS_PI,
    [SHIFT(MRT_TABLE_DUMPV2)][MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST_ADDPATH] = F_VALID | F_NEEDS_PI | F_ADDPATH,

    [SHIFT(MRT_BGP)][MRT_BGP_NULL]         = F_VALID,
    [SHIFT(MRT_BGP)][MRT_BGP_PREF_UPDATE]  = F_VALID,
    [SHIFT(MRT_BGP)][MRT_BGP_UPDATE]       = F_VALID | F_WRAPS_BGP,
    [SHIFT(MRT_BGP)][MRT_BGP_STATE_CHANGE] = F_VALID | F_HAS_STATE,
    [SHIFT(MRT_BGP)][MRT_BGP_SYNC]         = F_VALID,
    [SHIFT(MRT_BGP)][MRT_BGP_OPEN]         = F_VALID | F_WRAPS_BGP,
    [SHIFT(MRT_BGP)][MRT_BGP_NOTIFY]       = F_VALID | F_WRAPS_BGP,
    [SHIFT(MRT_BGP)][MRT_BGP_KEEPALIVE]    = F_VALID | F_WRAPS_BGP,

    [SHIFT(MRT_BGP4MP)][BGP4MP_STATE_CHANGE]              = F_VALID | F_IS_BGP | F_HAS_STATE,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE]                   = F_VALID | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_AS4]               = F_VALID | F_AS32 | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP)][BGP4MP_STATE_CHANGE_AS4]          = F_VALID | F_AS32 | F_IS_BGP | F_HAS_STATE,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_LOCAL]             = F_VALID | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_AS4_LOCAL]         = F_VALID | F_AS32 | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_AS4_ADDPATH]       = F_VALID | F_AS32 | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_LOCAL_ADDPATH]     = F_VALID | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH,
    [SHIFT(MRT_BGP4MP)][BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH] = F_VALID | F_AS32 | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH,

    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_STATE_CHANGE]              = F_VALID | F_IS_EXT | F_IS_BGP | F_HAS_STATE,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE]                   = F_VALID | F_IS_EXT | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_AS4]               = F_VALID | F_IS_EXT | F_AS32 | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_STATE_CHANGE_AS4]          = F_VALID | F_IS_EXT | F_AS32 | F_IS_BGP | F_HAS_STATE,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_LOCAL]             = F_VALID | F_IS_EXT | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_AS4_LOCAL]         = F_VALID | F_IS_EXT | F_AS32 | F_IS_BGP | F_WRAPS_BGP,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_AS4_ADDPATH]       = F_VALID | F_IS_EXT | F_AS32 | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_LOCAL_ADDPATH]     = F_VALID | F_IS_EXT | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH,
    [SHIFT(MRT_BGP4MP_ET)][BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH] = F_VALID | F_IS_EXT | F_AS32 | F_IS_BGP | F_WRAPS_BGP | F_ADDPATH
};

#define CHECKBOUNDSR(ptr, end, size, errcode, retval) do { \
    if (unlikely(((ptr) + (size)) > (end))) {              \
        (msg)->err = (errcode);                            \
        return (retval);                                   \
    }                                                      \
} while (false)

#define CHECKBOUNDS(ptr, end, size, errcode) CHECKBOUNDSR(ptr, end, size, errcode, errcode)

#define CHECKTYPER(which, retval) do {                                     \
    if (unlikely((msg)->hdr.type != which))                                \
        (msg)->err = ((msg)->err != MRT_ENOERR) ? (msg)->err : MRT_EINVOP; \
    if (unlikely((msg)->err))                                              \
        return (retval);                                                   \
} while (false)

#define CHECKTYPE(which) CHECKFLAGSR(which, (msg)->err)

#define CHECKFLAGSR(which, retval) do {                                     \
    if (unlikely(((msg)->flags & (which)) != (which)))                      \
        (msg)->err = ((msg)->err != MRT_ENOERR) ? (msg)->err : MRT_EINVOP;  \
    if (unlikely((msg)->err))                                               \
        return (retval);                                                    \
} while (false)

#define CHECKFLAGS(which) CHECKFLAGSR(which, (msg)->err)

#define CHECKPEERIDXR(retval) do {                                                 \
    if (unlikely(!(msg)->peer_index))                                              \
        (msg)->err = ((msg)->err != MRT_ENOERR) ? (msg)->err : MRT_ENEEDSPEERIDX;  \
    if (unlikely((msg)->err))                                                      \
        return (retval);                                                           \
} while (false)

#define CHECKPEERIDX() CHECKPEERIDXR((msg)->err)

static uint mrtflags(const mrt_header_t *hdr)
{
    return masktab[SHIFT(hdr->type)][hdr->subtype];
}

UBGP_API umrt_err mrterror(const umrt_msg_s *msg)
{
    return msg->err;
}

UBGP_API umrt_msg_s *mrtcopy(umrt_msg_s *restrict       dst,
                             const umrt_msg_s *restrict src)
{
    // copy the initial portion of the structure, excluding the actual
    // bytes inside `src`
    size_t size = offsetof(umrt_msg_s, fastbuf);
    memcpy(dst, src, size);
    // don't allow fail path to free `src` data
    dst->pitab = NULL;
    dst->buf   = NULL;

    // now also copy the actual packet data and aux tables
    if (src->pitab) {
        // duplicate peer index table
        if (dst->picount <= countof(dst->fastpitab))
            dst->pitab = dst->fastpitab;
        else {
            dst->pitab = malloc(dst->picount * sizeof(*dst->pitab));
            if (unlikely(!dst->pitab))
                goto fail;
        }

        memcpy(dst->pitab, src->pitab, dst->picount * sizeof(*dst->pitab));
    }

    size_t n = dst->hdr.len + sizeof(dst->hdr);
    if (n <= sizeof(dst->fastbuf))
        dst->buf = dst->fastbuf;
    else {
        dst->buf = malloc(n);
        if (unlikely(!dst->buf))
            goto fail;
    }

    memcpy(dst->buf, src->buf, n);

    // reset refcount to 1
    dst->refcount = 1;
    return dst;

fail:
    if (dst->pitab != dst->fastpitab)
        free(dst->pitab);
    if (dst->buf != dst->fastbuf)
        free(dst->buf);

    return NULL;
}

// read section

static umrt_err setuppitable(umrt_msg_s *msg, umrt_msg_s *pi)
{
    if (msg->peer_index)
        return MRT_EINVOP; // TODO: may be useful to support this

    if (unlikely(!pi->pitab)) {
        // build peer index table, done once and cached for everyone referencing this

        // FIXME: this invalidates pi's peer entry iterator, it would be wise not doing this...
        //        especially to implement the next...

        // TODO this should be thread safe, should compute this and
        //      compare-exchange it into `pitab`.
        size_t count;

        umrt_err err = startpeerents(pi, &count);
        if (unlikely(err != MRT_ENOERR))
            return MRT_EBADPEERIDX;

        pi->pitab = pi->fastpitab;
        if (unlikely(count > countof(msg->fastpitab))) {
            pi->pitab = malloc(count * sizeof(*pi->pitab));
            if (unlikely(!pi->pitab))
                return MRT_ENOMEM;
        }
        for (size_t i = 0; i < count; i++) {
            pi->pitab[i] = (pi->peptr - pi->buf) - MESSAGE_OFFSET;
            nextpeerent(pi);
        }

        err = endpeerents(pi);
        if (unlikely(err != MRT_ENOERR)) {
            if (pi->pitab != pi->fastpitab)
                free(pi->pitab);

            pi->pitab = NULL;
            return MRT_EBADPEERIDX;
        }

        pi->picount = count;
    }

    // all good, mark pi as referenced, we don't need this to be
    // atomic, since if anybody tries to close this while we're
    // setting the PI table up there is no (fast) way to avoid
    // race conditions
    pi->refcount++;

    msg->peer_index = pi;
    return MRT_ENOERR;
}

static umrt_err endpending(umrt_msg_s *msg)
{
    // small optimization for common case
    if (likely((msg->flags & (F_PE | F_RE)) == 0))
        return msg->err;

    // only one flag can be set
    if (msg->flags & F_RE)
        return endribents(msg);

    assert(msg->flags & F_PE);
    return endpeerents(msg);
}

UBGP_API bool ismrtext(const umrt_msg_s *msg)
{
    return (msg->flags & (F_RD | F_IS_EXT)) == (F_RD | F_IS_EXT);
}

UBGP_API bool isbgpwrapper(const umrt_msg_s* msg)
{
    return (msg->flags & (F_RD | F_WRAPS_BGP)) == (F_RD | F_WRAPS_BGP);
}

UBGP_API bool ismrtrib(const umrt_msg_s *msg)
{
    return (msg->flags & (F_RD | F_NEEDS_PI)) == (F_RD | F_NEEDS_PI);
}

UBGP_API bool ismrtasn32bit(const umrt_msg_s *msg)
{
    return (msg->flags & F_AS32) != 0;
}

UBGP_API bool ismrtaddpath(const umrt_msg_s *msg)
{
    return (msg->flags & F_ADDPATH) != 0;
}

UBGP_API umrt_err setmrtpi(umrt_msg_s *msg, umrt_msg_s *pi)
{
    if (likely((msg->flags & F_NEEDS_PI) && (pi->flags & F_IS_PI)))
        return setuppitable(msg, pi);

    return MRT_EINVOP;
}


UBGP_API umrt_err setmrtread(umrt_msg_s *msg, const void *data, size_t n)
{
    io_rw_t io = IO_MEM_RDINIT(data, n);
    return setmrtreadfrom(msg, &io);
}

UBGP_API umrt_err setmrtreadfd(umrt_msg_s *msg, int fd)
{
    io_rw_t io = IO_FD_INIT(fd);
    return setmrtreadfrom(msg, &io);
}

UBGP_API umrt_err setmrtreadfrom(umrt_msg_s *msg, io_rw_t *io)
{
    byte hdr[MRT_HDRSIZ];

    size_t n = io->read(io, hdr, sizeof(hdr));
    if (unlikely(n != sizeof(hdr)))
        return (n > 0) ? MRT_EBADHDR : MRT_EIO;  // either we couldn't fetch a complete header or there are no bytes left

    // decode header into msg->hdr for easy access
    memset(&msg->hdr, 0, sizeof(msg->hdr));

    uint32_t time;
    memcpy(&time, &hdr[TIMESTAMP_OFFSET], sizeof(time));
    msg->hdr.stamp.tv_sec = beswap32(time);

    // read type and subtype
    uint16_t type, subtype;
    memcpy(&type, &hdr[TYPE_OFFSET], sizeof(type));
    memcpy(&subtype, &hdr[SUBTYPE_OFFSET], sizeof(subtype));
    type    = beswap16(type);
    subtype = beswap16(subtype);

    // don't accept absurd values
    if (unlikely(type < MRT_BGP || type > MRT_BGP4MP_ET))
        return MRT_ETYPENOTSUP;
    if (unlikely(subtype >= countof(masktab[0])))
        return MRT_EBADHDR;

    msg->hdr.type    = type;
    msg->hdr.subtype = subtype;

    uint32_t len;
    memcpy(&len, &hdr[LENGTH_OFFSET], sizeof(len));
    msg->hdr.len = beswap32(len);
    uint flags = mrtflags(&msg->hdr);
    if (unlikely((flags & F_VALID) == 0))
        return MRT_EBADHDR;

    // populate message buffer
    msg->buf = msg->fastbuf;
    n        = msg->hdr.len + sizeof(hdr);
    if (unlikely(n > sizeof(msg->fastbuf)))
        msg->buf = malloc(n);
    if (unlikely(!msg->buf))
        return MRT_ENOMEM;

    // copy header over
    memcpy(msg->buf, hdr, sizeof(hdr));
    // copy leftover packet
    if (unlikely(io->read(io, &msg->buf[MRT_HDRSIZ], msg->hdr.len) != msg->hdr.len))
        return io->error(io) ? MRT_EIO : MRT_EBADHDR;

    // be the very first to reference this message (no need for atomicity)
    msg->refcount = 1;

    // read extended timestamp if necessary
    if (flags & F_IS_EXT) {
        uint32_t usec;

        memcpy(&usec, &msg->buf[MICROSECOND_TIMESTAMP_OFFSET], sizeof(usec));
        msg->hdr.stamp.tv_nsec = beswap32(usec) * 1000ull;
    }

    msg->flags      = flags | F_RD;
    msg->err        = MRT_ENOERR;
    msg->bufsiz     = n;
    msg->peer_index = NULL;
    msg->pitab      = NULL;

    return MRT_ENOERR;
}

// header section

UBGP_API umrt_err setmrtheaderv(umrt_msg_s *msg, const mrt_header_t *hdr, va_list va)
{
    USED(msg), USED(hdr), USED(va);
    return MRT_ENOERR;  // TODO
}

UBGP_API umrt_err setmrtheader(umrt_msg_s *msg, const mrt_header_t *hdr, ...)
{
    va_list va;

    va_start(va, hdr);
    umrt_err err = setmrtheaderv(msg, hdr, va);
    va_end(va);
    return err;
}

UBGP_API mrt_header_t *getmrtheader(umrt_msg_s *msg)
{
    CHECKFLAGSR(F_RD, NULL);

    return &msg->hdr;
}

UBGP_API umrt_err mrtclose(umrt_msg_s *msg)
{
    umrt_err err = mrterror(msg);
    if (msg->peer_index)
        mrtclose(msg->peer_index);  // will free() it if this is the last ref

    /* MRT messages are refcounted, this is due to the fact that a
     * peer index table may be shared across multiple other messages,
     * don't touch anything if the refcount doesn't go to 0
     */
    if (ATOMIC_DECR(msg->refcount) == 0) {
        if (msg->flags & F_IS_PI && msg->pitab != msg->fastpitab)
            free(msg->pitab);
        if (unlikely(msg->buf != msg->fastbuf))
            free(msg->buf);
    }
    return err;
}

// Peer Index

UBGP_API struct in_addr getpicollector(umrt_msg_s *msg)
{
    struct in_addr addr = {0};
    CHECKFLAGSR(F_IS_PI, addr);

    memcpy(&addr, &msg->buf[MESSAGE_OFFSET], sizeof(addr));
    return addr;
}

UBGP_API size_t getpiviewname(umrt_msg_s *msg, char *buf, size_t n)
{
    CHECKFLAGS(F_IS_PI);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;
    CHECKBOUNDS(ptr, end, sizeof(struct in_addr) + sizeof(uint16_t), MRT_EBADPEERIDXHDR);

    ptr += sizeof(struct in_addr);

    uint16_t len;
    memcpy(&len, ptr, sizeof(len));
    len = beswap16(len);
    ptr += sizeof(len);

    CHECKBOUNDS(ptr, end, len, MRT_EBADPEERIDXHDR);

    if (n > (size_t) len + 1)
        n = len + 1;

    if (n > 0) {
        memcpy(buf, ptr, n - 1);
        buf[n - 1] = '\0';
    }
    return len;
}

UBGP_API void *getpeerents(umrt_msg_s *msg, size_t *pcount, size_t *pn)
{
    CHECKFLAGSR(F_IS_PI, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;

    CHECKBOUNDSR(ptr, end, sizeof(struct in_addr) + sizeof(uint16_t), MRT_EBADPEERIDXHDR, NULL);

    ptr += sizeof(struct in_addr);  // collector id

    // view name
    uint16_t len;
    memcpy(&len, ptr, sizeof(len));
    len = beswap16(len);
    ptr += sizeof(len);

    CHECKBOUNDSR(ptr, end, len + sizeof(uint16_t), MRT_EBADPEERIDXHDR, NULL);

    ptr += len;

    // peer count
    if (pcount) {
        memcpy(&len, ptr, sizeof(len));
        *pcount = beswap16(len);
    }
    ptr += sizeof(len);

    if (pn)
        *pn = end - ptr;

    return ptr;
}

UBGP_API umrt_err startpeerents(umrt_msg_s *msg, size_t *pcount)
{
    CHECKFLAGS(F_IS_PI);

    endpending(msg);

    msg->peptr = getpeerents(msg, pcount, NULL);
    msg->flags |= F_PE;
    return MRT_ENOERR;
}

enum {
    PT_IPV6 = 1 << 0,
    PT_AS32 = 1 << 1
};

static byte *decodepeerent(peer_entry_t *dst, const byte *ptr)
{
    uint flags = *ptr++;

    // TODO check bounds
    memcpy(&dst->id, ptr, sizeof(dst->id));
    ptr += sizeof(dst->id);
    if (flags & PT_IPV6) {
        dst->addr.family = AF_INET6;
        dst->addr.bitlen = 128;
        memcpy(&dst->addr.sin6, ptr, sizeof(dst->addr.sin6));
        ptr += sizeof(dst->addr.sin6);
    } else {
        dst->addr.family = AF_INET;
        dst->addr.bitlen = 32;
        memcpy(&dst->addr.sin, ptr, sizeof(dst->addr.sin));
        ptr += sizeof(dst->addr.sin);
    }
    if (flags & PT_AS32) {
        uint32_t as;

        dst->as_size = sizeof(as);
        memcpy(&as, ptr, sizeof(as));
        dst->as = beswap32(as);
    } else {
        uint16_t as;

        dst->as_size = sizeof(as);
        memcpy(&as, ptr, sizeof(as));
        dst->as = beswap16(as);
    }
    return (byte *) ptr + dst->as_size;
}

UBGP_API peer_entry_t *nextpeerent(umrt_msg_s *msg)
{
    CHECKFLAGSR(F_PE, NULL);

    // NOTE: RFC 6396 "The Length field does not include the length of the MRT Common Header."
    byte *end = msg->buf + msg->hdr.len + MESSAGE_OFFSET;
    if (msg->peptr == end)
        return NULL;

    msg->peptr = decodepeerent(&msg->pe, msg->peptr);
    return &msg->pe;
}


UBGP_API umrt_err endpeerents(umrt_msg_s *msg)
{
    CHECKFLAGS(F_PE);

    msg->flags &= ~F_PE;
    return MRT_ENOERR;
}

// RIB entries

UBGP_API umrt_err setribpi(umrt_msg_s *msg, umrt_msg_s *pi)
{
    if (unlikely(msg->err != MRT_ENOERR || pi->err != MRT_ENOERR))
        return MRT_EINVOP;
    if (unlikely((pi->flags & F_IS_PI) == 0))
        return MRT_NOTPEERIDX;

    return setuppitable(msg, pi);
}

static void *getribents_v2(umrt_msg_s *msg, size_t *pcount, size_t*pn)
{
    CHECKFLAGSR(F_NEEDS_PI, NULL);
    CHECKPEERIDXR(NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;

    CHECKBOUNDSR(ptr, end, sizeof(uint32_t), MRT_EBADRIBENT, NULL);

    uint32_t seqno;
    memcpy(&seqno, ptr, sizeof(seqno));
    seqno = beswap32(seqno);
    ptr += sizeof(seqno);

    uint16_t afi;
    uint8_t safi;
    switch (msg->hdr.subtype) {
    case MRT_TABLE_DUMPV2_RIB_GENERIC:
    case MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH:
        CHECKBOUNDSR(ptr, end, sizeof(afi) + sizeof(safi), MRT_EBADRIBENT, NULL);

        memcpy(&afi, ptr, sizeof(afi));
        afi = beswap16(afi);
        ptr += sizeof(afi);
        safi = *ptr++;
        break;
    case MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST_ADDPATH:
        afi = AFI_IPV4;
        safi = SAFI_UNICAST;
        break;
    case MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST_ADDPATH:
        afi = AFI_IPV4;
        safi = SAFI_MULTICAST;
        break;
    case MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST_ADDPATH:
        afi = AFI_IPV6;
        safi = SAFI_UNICAST;
        break;
    case MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST_ADDPATH:
        afi = AFI_IPV6;
        safi = SAFI_MULTICAST;
        break;
    default:
        goto unsup;
    }
    if (unlikely(safi != SAFI_UNICAST && safi != SAFI_MULTICAST))
        goto unsup;

    sa_family_t fam;
    switch (afi) {
    case AFI_IPV4:
        fam = AF_INET;
        break;
    case AFI_IPV6:
        fam = AF_INET6;
        break;
    default:
        goto unsup;
    }

    CHECKBOUNDSR(ptr, end, sizeof(uint8_t), MRT_EBADRIBENT, NULL);

    size_t bitlen = *ptr++;
    msg->ribhdr.seqno = seqno;
    msg->ribhdr.afi = afi;
    msg->ribhdr.safi = safi;

    memset(&msg->ribhdr.nlri, 0, sizeof(msg->ribhdr.nlri));
    msg->ribhdr.nlri.family = fam;
    msg->ribhdr.nlri.bitlen = bitlen;

    size_t n = naddrsize(bitlen);

    CHECKBOUNDSR(ptr, end, n + sizeof(uint16_t), MRT_EBADRIBENT, NULL);

    memcpy(msg->ribhdr.nlri.bytes, ptr, n);
    ptr += n;

    uint16_t count;
    if (pcount) {
        memcpy(&count, ptr, sizeof(count));
        *pcount = beswap16(count);
    }
    ptr += sizeof(count);

    if (pn)
        *pn = end - ptr;

    return ptr;

unsup:
    msg->err = MRT_ERIBNOTSUP;
    return NULL;
}

static void *getribents_legacy(umrt_msg_s *msg, size_t *pcount, size_t *pn)
{
    CHECKTYPER(MRT_TABLE_DUMP, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;

    if (pcount) {
        // TABLE_DUMP doesn't provide the rib count, so we must
        // quickly scan the packet to find it out
        size_t hdrlen = 2 * sizeof(uint16_t) + 2 + sizeof(uint32_t) + 2 * sizeof(uint16_t);
        if (msg->hdr.subtype == AFI_IPV6)
            hdrlen += 2 * 4;
        else
            hdrlen += 2 * 16;

        size_t n = 0;

        byte *cur = ptr;
        while (cur != end) {
            uint16_t attr_len;

            CHECKBOUNDSR(cur, end, hdrlen, MRT_EBADRIBENT, NULL);

            cur += hdrlen - sizeof(attr_len);
            memcpy(&attr_len, cur, sizeof(attr_len));
            attr_len = beswap16(attr_len);
            cur += sizeof(attr_len);

            CHECKBOUNDSR(cur, end, attr_len, MRT_EBADRIBENT, NULL);

            cur += attr_len;
            n++;
        }

        *pcount = n;
    }

    if (pn)
        *pn = end - ptr;

    return ptr;
}

UBGP_API void *getribents(umrt_msg_s *msg, size_t *pcount, size_t *pn)
{
    if (msg->hdr.type == MRT_TABLE_DUMPV2)
        return getribents_v2(msg, pcount, pn);
    else
        return getribents_legacy(msg, pcount, pn);
}

UBGP_API umrt_err setribents(umrt_msg_s *msg, const void *buf, size_t n)
{
    CHECKFLAGS(F_WR | F_NEEDS_PI);
    CHECKPEERIDX();

    // TODO implement
    USED(buf), USED(n);
    return MRT_ENOERR;
}

UBGP_API rib_header_t *startribents(umrt_msg_s *msg, size_t *pcount)
{
    endpending(msg);

    msg->reptr = getribents(msg, pcount, NULL);
    msg->flags |= F_RE;
    return &msg->ribhdr;
}

static rib_entry_t *nextribent_legacy(umrt_msg_s *msg)
{
    byte *end = msg->buf + MESSAGE_OFFSET + msg->hdr.len;
    if (msg->reptr == end)
        return NULL;

    size_t hdrsize = 2 * sizeof(uint16_t) + 2 * sizeof(uint8_t) + sizeof(uint32_t) + 2 * sizeof(uint16_t);
    if (msg->hdr.subtype == AFI_IPV6)
        hdrsize += 2 * 16;
    else
        hdrsize += 2 * 4;

    CHECKBOUNDSR(msg->reptr, end, hdrsize, MRT_EBADRIBENT, NULL);

    uint16_t view, seqno;

    memcpy(&view, msg->reptr, sizeof(view));
    view = beswap16(view);
    msg->reptr += sizeof(view);
    memcpy(&seqno, msg->reptr, sizeof(seqno));
    seqno = beswap16(seqno);
    msg->reptr += sizeof(seqno);

    if (msg->hdr.subtype == AFI_IPV6) {
        msg->ribent.nlri.family = AF_INET6;
        memcpy(&msg->ribent.nlri.bytes, msg->reptr, 16);
        msg->reptr += 16;
    } else {
        msg->ribent.nlri.family = AF_INET;
        memcpy(&msg->ribent.nlri.bytes, msg->reptr, 4);
        msg->reptr += 4;
    }

    msg->ribent.nlri.bitlen = *msg->reptr++;

    msg->reptr++; // status, SHOULD be 1

    uint32_t originated;

    memcpy(&originated, msg->reptr, sizeof(originated));
    originated = beswap32(originated);
    msg->reptr += sizeof(originated);

    if (msg->hdr.subtype == AFI_IPV6) {
        msg->ribpe.addr.family = AF_INET6;
        memcpy(&msg->ribpe.addr.bytes, msg->reptr, 16);
        msg->reptr += 16;

        msg->ribpe.addr.bitlen = 128;
    } else {
        msg->ribpe.addr.family = AF_INET;
        memcpy(&msg->ribpe.addr.bytes, msg->reptr, 4);
        msg->reptr += 4;

        msg->ribpe.addr.bitlen = 32;
    }


    uint16_t as, attr_len;

    memcpy(&as, msg->reptr, sizeof(as));
    as = beswap16(as);
    msg->reptr += sizeof(as);

    memcpy(&attr_len, msg->reptr, sizeof(attr_len));
    attr_len = beswap16(attr_len);
    msg->reptr += sizeof(attr_len);

    CHECKBOUNDSR(msg->reptr, end, attr_len, MRT_EBADRIBENT, NULL);

    // populate ribent
    msg->ribent.peer_idx    = 0;
    msg->ribent.attr_length = attr_len;
    msg->ribent.seqno       = seqno;
    msg->ribent.originated  = (time_t) originated;
    msg->ribent.pathid      = 0;
    msg->ribent.attrs       = (bgpattr_t *) msg->reptr;

    msg->reptr += attr_len;

    // populate any other peer entry field
    msg->ribpe.as_size   = sizeof(uint16_t);
    msg->ribpe.as        = as;
    msg->ribpe.id.s_addr = 0;

    msg->ribent.peer = &msg->ribpe;

    return &msg->ribent;
}

static rib_entry_t *nextribent_v2(umrt_msg_s *msg)
{
    umrt_msg_s *pi = msg->peer_index;

    byte *end = msg->buf + MESSAGE_OFFSET + msg->hdr.len;
    if (msg->reptr == end)
        return NULL;

    uint16_t idx;
    uint32_t originated;

    CHECKBOUNDSR(msg->reptr, end, sizeof(idx) + sizeof(originated), MRT_EBADRIBENT, NULL);

    memcpy(&idx, msg->reptr, sizeof(idx));
    idx = beswap16(idx);
    if (idx >= pi->picount) {
        msg->err = MRT_EBADPEERIDX;
        return NULL;
    }

    msg->reptr += sizeof(idx);

    memcpy(&originated, msg->reptr, sizeof(originated));
    originated = beswap32(originated);
    msg->reptr += sizeof(originated);

    uint32_t pathid = 0;
    if (msg->flags & F_ADDPATH) {
        CHECKBOUNDSR(msg->reptr, end, sizeof(pathid), MRT_EBADRIBENT, NULL);

        memcpy(&pathid, msg->reptr, sizeof(pathid));
        pathid = beswap32(pathid);
        msg->reptr += sizeof(pathid);
    }

    uint16_t attr_len;

    CHECKBOUNDSR(msg->reptr, end, sizeof(attr_len), MRT_EBADRIBENT, NULL);

    memcpy(&attr_len, msg->reptr, sizeof(attr_len));
    attr_len = beswap16(attr_len);
    msg->reptr += sizeof(attr_len);

    CHECKBOUNDSR(msg->reptr, end, attr_len, MRT_EBADRIBENT, NULL);

    msg->ribent.peer_idx = idx;
    msg->ribent.originated = (time_t) originated;
    memcpy(&msg->ribent.nlri, &msg->ribhdr.nlri, sizeof(msg->ribent.nlri));
    msg->ribent.pathid = pathid;
    msg->ribent.attr_length = attr_len;
    msg->ribent.attrs = (bgpattr_t *) msg->reptr;

    msg->reptr += attr_len;

    // decode peer entry
    byte *peer_ent = &pi->buf[msg->peer_index->pitab[idx] + MESSAGE_OFFSET];
    decodepeerent(&msg->ribpe, peer_ent);
    msg->ribent.peer = &msg->ribpe;
    return &msg->ribent;
}

UBGP_API rib_entry_t *nextribent(umrt_msg_s *msg)
{
    CHECKFLAGSR(F_RE, NULL);

    if (msg->hdr.type == MRT_TABLE_DUMPV2)
        return nextribent_v2(msg);
    else
        return nextribent_legacy(msg);
}

UBGP_API umrt_err endribents(umrt_msg_s *msg)
{
    CHECKFLAGS(F_RE);

    msg->flags |= F_RE;
    return MRT_ENOERR;
}

UBGP_API bgp4mp_header_t *getbgp4mpheader(umrt_msg_s *msg)
{
    CHECKFLAGSR(F_RD | F_IS_BGP, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;
    if (msg->flags & F_IS_EXT)
        ptr = &msg->buf[MESSAGE_EXTENDED_OFFSET];

    bgp4mp_header_t *hdr = &msg->bgp4mphdr;
    memset(hdr, 0, sizeof(*hdr));
    if (msg->flags & F_AS32) {
        CHECKBOUNDSR(ptr, end, 2 * sizeof(uint32_t), MRT_EBADBGP4MPHDR, NULL);

        memcpy(&hdr->peer_as, ptr, sizeof(hdr->peer_as));
        hdr->peer_as = beswap32(hdr->peer_as);
        ptr += sizeof(hdr->peer_as);
        memcpy(&hdr->local_as, ptr, sizeof(hdr->local_as));
        hdr->local_as = beswap32(hdr->local_as);
        ptr += sizeof(hdr->local_as);
    } else {
        uint16_t as;

        CHECKBOUNDSR(ptr, end, 2 * sizeof(uint16_t), MRT_EBADBGP4MPHDR, NULL);

        memcpy(&as, ptr, sizeof(as));
        hdr->peer_as = beswap16(as);
        ptr += sizeof(as);
        memcpy(&as, ptr, sizeof(as));
        hdr->local_as = beswap16(as);
        ptr += sizeof(as);
    }

    CHECKBOUNDSR(ptr, end, 2 * sizeof(uint16_t), MRT_EBADBGP4MPHDR, NULL);

    memcpy(&hdr->iface, ptr, sizeof(hdr->iface));
    hdr->iface = beswap16(hdr->iface);
    ptr += sizeof(hdr->iface);

    uint16_t afi;
    memcpy(&afi, ptr, sizeof(afi));
    afi = beswap16(afi);
    ptr += sizeof(afi);
    switch (afi) {
    case AFI_IPV4:
        CHECKBOUNDSR(ptr, end, 2 * sizeof(struct in_addr), MRT_EBADBGP4MPHDR, NULL);

        makenaddr(&hdr->peer_addr, AF_INET, ptr, 32);
        ptr += sizeof(struct in_addr);
        makenaddr(&hdr->local_addr, AF_INET, ptr, 32);
        ptr += sizeof(struct in_addr);
        break;
    case AFI_IPV6:
        CHECKBOUNDSR(ptr, end, 2 * sizeof(struct in6_addr), MRT_EBADBGP4MPHDR, NULL);

        makenaddr(&hdr->peer_addr, AF_INET6, ptr, 128);
        ptr += sizeof(struct in6_addr);
        makenaddr(&hdr->local_addr, AF_INET6, ptr, 128);
        ptr += sizeof(struct in6_addr);
        break;
    default:
        msg->err = MRT_EAFINOTSUP;
        return NULL;
    }

    if (msg->flags & F_HAS_STATE) {
        CHECKBOUNDSR(ptr, end, 2 * sizeof(uint16_t), MRT_EBADBGP4MPHDR, NULL);  // FIXME BADSTATECHANGE

        memcpy(&hdr->old_state, ptr, sizeof(hdr->old_state));
        hdr->old_state = beswap16(hdr->old_state);
        ptr += sizeof(hdr->old_state);
        memcpy(&hdr->new_state, ptr, sizeof(hdr->new_state));
        hdr->new_state = beswap16(hdr->new_state);
    } else {
        hdr->old_state = hdr->new_state = 0;
    }

    return hdr;
}

UBGP_API void *unwrapbgp4mp(umrt_msg_s *msg, size_t *pn)
{
    CHECKFLAGSR(F_RD | F_WRAPS_BGP, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;
    if (msg->flags & F_IS_EXT)
        ptr = &msg->buf[MESSAGE_EXTENDED_OFFSET];

    uint16_t afi;
    size_t total_as_size = msg->flags & F_AS32 ? 2 * sizeof(uint32_t) : 2 * sizeof(uint16_t);
    CHECKBOUNDSR(ptr, end, total_as_size + sizeof(uint16_t) + sizeof(afi), MRT_EBADBGP4MPHDR, NULL);

    ptr += total_as_size;
    ptr += sizeof(uint16_t);

    memcpy(&afi, ptr, sizeof(afi));
    afi = beswap16(afi);
    ptr += sizeof(afi);
    switch (afi) {
    case AFI_IPV4:
        CHECKBOUNDSR(ptr, end, 2 * sizeof(struct in_addr), MRT_EBADBGP4MPHDR, NULL);
        ptr += 2 * sizeof(struct in_addr);
        break;
    case AFI_IPV6:
        CHECKBOUNDSR(ptr, end, 2 * sizeof(struct in6_addr), MRT_EBADBGP4MPHDR, NULL);
        ptr += 2 * sizeof(struct in6_addr);
        break;
    default:
        msg->err = MRT_EAFINOTSUP;
        return NULL;
    }

    if (likely(pn))
        *pn = end - ptr;

    return ptr;
}

UBGP_API zebra_header_t *getzebraheader(umrt_msg_s *msg)
{
    CHECKTYPER(MRT_BGP, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;

    CHECKBOUNDSR(ptr, end, sizeof(uint16_t) + IPV4_SIZE, MRT_EBADZEBRAHDR, NULL);

    memcpy(&msg->zebrahdr.peer_as, ptr, sizeof(msg->zebrahdr.peer_as));
    msg->zebrahdr.peer_as = beswap16(msg->zebrahdr.peer_as);
    ptr += sizeof(msg->zebrahdr.peer_as);

    msg->zebrahdr.peer_addr.family = AF_INET;
    msg->zebrahdr.peer_addr.bitlen = IPV4_BIT;
    memcpy(&msg->zebrahdr.peer_addr.bytes, ptr, IPV4_SIZE);
    ptr += IPV4_SIZE;

    if (msg->flags & F_WRAPS_BGP) {
        CHECKBOUNDSR(ptr, end, sizeof(uint16_t) + IPV4_SIZE, MRT_EBADZEBRAHDR, NULL);

        memcpy(&msg->zebrahdr.local_as, ptr, sizeof(msg->zebrahdr.local_as));
        msg->zebrahdr.local_as = beswap16(msg->zebrahdr.local_as);
        ptr += sizeof(msg->zebrahdr.local_as);

        msg->zebrahdr.local_addr.family = AF_INET;
        msg->zebrahdr.local_addr.bitlen = IPV4_BIT;
        memcpy(&msg->zebrahdr.local_addr.bytes, ptr, IPV4_SIZE);
        ptr += IPV4_SIZE;
    } else if (msg->flags & F_HAS_STATE) {
        CHECKBOUNDSR(ptr, end, 2 * sizeof(uint16_t), MRT_EBADZEBRAHDR, NULL);

        memcpy(&msg->zebrahdr.old_state, ptr, sizeof(msg->zebrahdr.old_state));
        msg->zebrahdr.old_state = beswap16(msg->zebrahdr.old_state);
        ptr += sizeof(msg->zebrahdr.old_state);

        memcpy(&msg->zebrahdr.new_state, ptr, sizeof(msg->zebrahdr.new_state));
        msg->zebrahdr.new_state = beswap16(msg->zebrahdr.new_state);
        ptr += sizeof(msg->zebrahdr.new_state);
    } else {
        msg->err = MRT_EINVOP;
        return NULL;
    }

    return &msg->zebrahdr;
}

UBGP_API void *unwrapzebra(umrt_msg_s *msg, size_t *pn)
{
    CHECKTYPER(MRT_BGP, NULL);
    CHECKFLAGSR(F_WRAPS_BGP | F_RD, NULL);

    byte *ptr = &msg->buf[MESSAGE_OFFSET];
    byte *end = ptr + msg->hdr.len;

    size_t hdrsize = 2 * sizeof(uint16_t) + 2 * IPV4_SIZE;
    CHECKBOUNDSR(ptr, end, hdrsize, MRT_EBADZEBRAHDR, NULL);

    ptr += hdrsize;
    if (pn)
        *pn = end - ptr;

    return ptr;
}

