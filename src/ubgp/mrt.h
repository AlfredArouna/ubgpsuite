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

#ifndef UBGP_MRT_H_
#define UBGP_MRT_H_

#include "atomics.h"
#include "bgp.h"

#include <stdarg.h>
#include <time.h>

enum {
    MRT_NULL = 0,          // Deprecated
    MRT_START = 1,         // Deprecated
    MRT_DIE = 2,           // Deprecated
    MRT_I_AM_DEAD = 3,     // Deprecated
    MRT_PEER_DOWN = 4,     // Deprecated
    MRT_BGP = 5,           // Deprecated also known as ZEBRA_BGP
    MRT_RIP = 6,           // Deprecated
    MRT_IDRP = 7,          // Deprecated
    MRT_RIPNG = 8,         // Deprecated
    MRT_BGP4PLUS = 9,      // Deprecated
    MRT_BGP4PLUS_01 = 10,  // Deprecated
    MRT_OSPFV2 = 11,
    MRT_TABLE_DUMP = 12,
    MRT_TABLE_DUMPV2 = 13,
    MRT_BGP4MP = 16,
    MRT_BGP4MP_ET = 17,
    MRT_ISIS = 32,
    MRT_ISIS_ET = 33,
    MRT_OSPFV3 = 48,
    MRT_OSPFV3_ET = 49
};

/**
 * BGP/ZEBRA BGP subtypes.
 * Deprecated by BGP4MP
 */
enum {
    MRT_BGP_NULL         = 0,
    MRT_BGP_UPDATE       = 1,
    MRT_BGP_PREF_UPDATE  = 2,
    MRT_BGP_STATE_CHANGE = 3,
    MRT_BGP_SYNC         = 4,
    MRT_BGP_OPEN         = 5,
    MRT_BGP_NOTIFY       = 6,
    MRT_BGP_KEEPALIVE    = 7
};

/**
 * @BGP4MP_STATE_CHANGE: RFC 6396
 * @BGP4MP_MESSAGE: RFC 6396
 * @BGP4MP_ENTRY: Deprecated
 * @BGP4MP_SNAPSHOT: Deprecated
 * @BGP4MP_MESSAGE_AS4: RFC 6396
 * @BGP4MP_STATE_CHANGE_AS4: RFC 6396
 * @BGP4MP_MESSAGE_LOCAL: RFC 6396
 * @BGP4MP_MESSAGE_AS4_LOCAL: RFC 6396
 * @BGP4MP_MESSAGE_ADDPATH: RFC 8050
 * @BGP4MP_MESSAGE_AS4_ADDPATH: RFC 8050
 * @BGP4MP_MESSAGE_LOCAL_ADDPATH: RFC 8050
 * @BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH: RFC 8050
 *
 * BGP4MP types.
 *
 * Further details at https://tools.ietf.org/html/rfc6396#section-4.2
 * and for extensions https://tools.ietf.org/html/rfc8050#page-2
 *
 * [IANA BGP4MP Subtype Codes](https://www.iana.org/assignments/mrt/mrt.xhtml#BGP4MP-codes)
 *
 */
enum {
    BGP4MP_STATE_CHANGE              = 0,
    BGP4MP_MESSAGE                   = 1,
    BGP4MP_ENTRY                     = 2,
    BGP4MP_SNAPSHOT                  = 3,
    BGP4MP_MESSAGE_AS4               = 4,
    BGP4MP_STATE_CHANGE_AS4          = 5,
    BGP4MP_MESSAGE_LOCAL             = 6,
    BGP4MP_MESSAGE_AS4_LOCAL         = 7,
    BGP4MP_MESSAGE_ADDPATH           = 8,
    BGP4MP_MESSAGE_AS4_ADDPATH       = 9,
    BGP4MP_MESSAGE_LOCAL_ADDPATH     = 10,
    BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH = 11
};

typedef struct {
    size_t as_size;
    uint32_t as;
    struct in_addr id;
    netaddr_t addr;
} peer_entry_t;

typedef struct {
    uint32_t seqno;
    afi_t afi;
    safi_t safi;
    netaddr_t nlri;
} rib_header_t;

/**
 * rib_entry_t:
 * @peer_idx:    peer index number, always 0 for TABLE_DUMP
 * @attr_length: attrs field length in bytes
 * @seqno:       entry sequence number, same as rib_header_t seqno
 *               when TABLE_DUMPV2, sequence entry information
 *               for TABLE_DUMP (TABLE_DUMP sequence numbers are
 *               16-bits wide, wrapping is very likely)
 * @originated:
 * @nlri:        same as rib_header_t nlri when TABLE_DUMPV2
 * @pathid:      only meaningful for ADDPATH TABLE_DUMPV2 subtypes, 0 otherwise
 * @peer:        PEER_INDEX information when TABLE_DUMPV2, available
 *               peer information when TABLE_DUMP
 * @attrs:
 */
typedef struct {
    uint16_t peer_idx;
    uint16_t attr_length;
    uint32_t seqno;
    time_t originated;
    netaddr_t nlri;
    uint32_t pathid;
    peer_entry_t *peer;
    bgpattr_t *attrs;
} rib_entry_t;

/**
 * @MRT_TABLE_DUMPV2_PEER_INDEX_TABLE: RFC6396
 * @MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST: RFC6396
 * @MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST: RFC6396
 * @MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST: RFC6396
 * @MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST: RFC6396
 * @MRT_TABLE_DUMPV2_RIB_GENERIC: RFC6396
 * @MRT_TABLE_DUMPV2_GEO_PEER_TABLE: RFC6397
 * @MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST_ADDPATH: RFC8050
 * @MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST_ADDPATH: RFC8050
 * @MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST_ADDPATH: RFC8050
 * @MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST_ADDPATH: RFC8050
 * @MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH: RFC8050
 *
 * Further details at https://tools.ietf.org/html/rfc6396#section-4.3
 *
 * See [IANA table dumpv2 Subtype Codes](https://www.iana.org/assignments/mrt/mrt.xhtml#table-dump-v2-subtype-codes)
 */
enum {
    MRT_TABLE_DUMPV2_PEER_INDEX_TABLE = 1,
    MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST = 2,
    MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST = 3,
    MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST = 4,
    MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST = 5,
    MRT_TABLE_DUMPV2_RIB_GENERIC = 6,
    MRT_TABLE_DUMPV2_GEO_PEER_TABLE = 7,
    MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST_ADDPATH = 8,
    MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST_ADDPATH = 9,
    MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST_ADDPATH = 10,
    MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST_ADDPATH = 11,
    MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH = 12
};

/**
 * umrt_err:
 * @MRT_NOTPEERIDX:     packet isn't a PEER_INDEX
 * @MRT_ENOERR:         no error (success) guaranteed to be zero
 * @MRT_EIO:            IO error during packet read
 * @MRT_EINVOP:         invalid operation (e.g. write while reading packet)
 * @MRT_ENOMEM:         out of memory
 * @MRT_EBADHDR:        bad MRT packet header
 * @MRT_EBADTYPE:       bad MRT packet type
 * @MRT_EBADBGP4MPHDR:  bad BGP4MP header
 * @MRT_EBADZEBRAHDR:   bad Zebra BGP header
 * @MRT_EBADPEERIDXHDR: bad PEER_INDEX header
 * @MRT_EBADPEERIDX:    error encountered parsing associated Peer Index
 * @MRT_ENEEDSPEERIDX:  packet needs a peer index, but none was provided
 * @MRT_ERIBNOTSUP:     unsupported RIB entry encountered, according to RFC6396:
 *                      "An implementation that does not recognize particular
 *                      AFI and SAFI values SHOULD discard the remainder
 *                      of the MRT record."
 * @MRT_EBADRIBENT:     corrupted or truncated RIB entry
 * @MRT_EAFINOTSUP:     AFI not supported
 * @MRT_ETYPENOTSUP:    MRT type is not supported
 *
 * MRT API error codes.
 */
typedef enum {
    // recoverable errors
    MRT_NOTPEERIDX = -1,
    // unrecoverable
    MRT_ENOERR = 0,
    MRT_EIO,
    MRT_EINVOP,
    MRT_ENOMEM,
    MRT_EBADHDR,
    MRT_EBADTYPE,
    MRT_EBADBGP4MPHDR,
    MRT_EBADZEBRAHDR,
    MRT_EBADPEERIDXHDR,
    MRT_EBADPEERIDX,
    MRT_ENEEDSPEERIDX,
    MRT_ERIBNOTSUP,
    MRT_EBADRIBENT,
    MRT_EAFINOTSUP,
    MRT_ETYPENOTSUP
} umrt_err;

static inline const char *mrtstrerror(umrt_err err)
{
    switch (err) {
    case MRT_NOTPEERIDX:
        return "Not Peer Index message";
    case MRT_ENOERR:
        return "Success";
    case MRT_EIO:
        return "I/O error";
    case MRT_EINVOP:
        return "Invalid operation";
    case MRT_ENOMEM:
        return "Out of memory";
    case MRT_EBADHDR:
        return "Bad MRT header";
    case MRT_EBADTYPE:
        return "Bad MRT packet type";
    case MRT_EBADBGP4MPHDR:
        return "Bad BGP4MP header";
    case MRT_EBADZEBRAHDR:
        return "Bad Zebra BGP header";
    case MRT_EBADPEERIDXHDR:
        return "Bad Peer Index header";
    case MRT_EBADPEERIDX:
        return "Bad Peer Index message";
    case MRT_ENEEDSPEERIDX:
        return "No peer index provided";
    case MRT_ERIBNOTSUP:
        return "Unsupported RIB entry";
    case MRT_EAFINOTSUP:
        return "Unsupported AFI";
    case MRT_EBADRIBENT:
        return "Corrupted or truncated RIB entry";
    case MRT_ETYPENOTSUP:
        return "Unsupported MRT packet type";
    default:
        return "Unknown error";
    }
}

// header section

typedef struct {
    struct timespec stamp;
    int type, subtype;
    size_t len;
} mrt_header_t;

#define MRTBUFSIZ       4096
#define MRTPRESRVBUFSIZ 512

typedef struct {
    uint32_t peer_as, local_as;
    netaddr_t peer_addr, local_addr;
    uint16_t  iface;
    uint16_t old_state, new_state;  // Only meaningful for BGP4MP_STATE_CHANGE*
} bgp4mp_header_t;

typedef struct {
    uint16_t peer_as;
    netaddr_t peer_addr;
    union {
        struct {
            uint16_t old_state, new_state;  // Only meaningful for MRT_BGP_STATE_CHANGE
        };
        struct {
            uint16_t local_as;
            netaddr_t local_addr;
        };
    };
} zebra_header_t;

/**
 * umrt_msg_s:
 *
 * Packet reader/writer global status structure.
 */
typedef struct umrt_msg {
    /*< private >*/

    uint16_t flags;      // General status flags.
    int16_t err;         // Last error code.
    uint32_t bufsiz;     // Packet buffer capacity

    atomic_uword refcount;       // messages referencing this message itself.
    struct umrt_msg *peer_index;

    mrt_header_t hdr;
    union {
        struct {
            peer_entry_t pe;  // Current peer entry
            byte *peptr;      // Raw peer entry pointer in current packet
        };
        struct {
            rib_header_t ribhdr;
            rib_entry_t ribent;
            peer_entry_t ribpe;
            byte *reptr;       // Raw RIB entry pointer in current packet
        };

        bgp4mp_header_t bgp4mphdr;

        zebra_header_t zebrahdr;
    };

    byte *buf;  // Packet buffer base.

    uint32_t *pitab;
    uint16_t picount;

    // these fields must come at the end of this struct!

    byte fastbuf[MRTBUFSIZ];  // Fast buffer to avoid malloc()s.
    union {
        uint32_t fastpitab[MRTPRESRVBUFSIZ / sizeof(uint32_t)];
        byte     prsvbuf[MRTPRESRVBUFSIZ];
    };
} umrt_msg_s;

UBGP_API CHECK_NONNULL(1) PUREFUNC bool ismrtext(const umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) PUREFUNC bool isbgpwrapper(const umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) PUREFUNC bool ismrtasn32bit(const umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) PUREFUNC bool ismrtaddpath(const umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) PUREFUNC bool ismrtrib(const umrt_msg_s *msg);


/**
 * mrtcopy:
 * @dst: copy destination
 * @src: copy source
 *
 * Copies @src into @dst, arguments must not overlap.
 *
 * Returns: copied message, %NULL on out of memory.
 */
UBGP_API umrt_msg_s *mrtcopy(umrt_msg_s *restrict       dst,
                             const umrt_msg_s *restrict src);

/**
 * setmrtpi:
 * @msg: an #umrt_msg_s
 * @msg: an #umrt_msg_s of type %MRT_TABLE_DUMPV2 and subtype
 *       %MRT_TABLE_DUMPV2_PEER_INDEX_TABLE.
 *
 * Set MRT message reference PEER_INDEX table.
 */
UBGP_API CHECK_NONNULL(1, 2) umrt_err setmrtpi(umrt_msg_s *msg, umrt_msg_s *pi);

UBGP_API CHECK_NONNULL(1) PUREFUNC umrt_err mrterror(const umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) umrt_err mrtclose(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) umrt_err setmrtread(umrt_msg_s *msg, const void *data, size_t n);

UBGP_API CHECK_NONNULL(1) umrt_err setmrtreadfd(umrt_msg_s *msg, int fd);

UBGP_API CHECK_NONNULL(1, 2) umrt_err setmrtreadfrom(umrt_msg_s *msg, io_rw_t *io);

// header

UBGP_API CHECK_NONNULL(1) mrt_header_t *getmrtheader(umrt_msg_s *msg);

// Peer Index

UBGP_API CHECK_NONNULL(1) struct in_addr getpicollector(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) size_t getpiviewname(umrt_msg_s *msg, char *buf, size_t n);

UBGP_API CHECK_NONNULL(1) umrt_err setpeerents(umrt_msg_s *msg, const void *buf, size_t n);

/**
 * getpeerents:
 * @msg:                a #umrt_msg_s
 * @pcount: (nullable): if not %NULL, storage where peer entries count
 *                      should be stored
 * @pn: (nullable):     if not %NULL, storage where peer entries chunk size
 *                      (in bytes) should be stored
 *
 * Retrieve raw bytes of peer entities field.
 *
 * Returns: raw peer entities bytes inside @msg, %NULL on error.
 */
UBGP_API CHECK_NONNULL(1) void *getpeerents(umrt_msg_s *msg, size_t *pcount, size_t *pn);

UBGP_API CHECK_NONNULL(1) umrt_err startpeerents(umrt_msg_s *msg, size_t *pcount);

UBGP_API CHECK_NONNULL(1) peer_entry_t *nextpeerent(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1, 2) umrt_err putpeerent(umrt_msg_s *msg, const peer_entry_t *pe);

UBGP_API CHECK_NONNULL(1) umrt_err endpeerents(umrt_msg_s *msg);

// RIB subtypes

UBGP_API CHECK_NONNULL(1, 2) umrt_err setribpi(umrt_msg_s *msg, umrt_msg_s *pi);

UBGP_API CHECK_NONNULL(1) umrt_err setribents(umrt_msg_s *msg, const void *buf, size_t n);

/**
 * getribents:
 * @msg:                a #umrt_msg_s
 * @pcount: (nullable): if not %NULL, storage where RIB entries count
 *                      should be stored
 * @pn: (nullable):     if not %NULL, storage where RIB entries chunk size
 *                      (in bytes) should be stored
 *
 * Retrieve raw bytes of peer entities field.
 *
 * Returns: raw RIB entities bytes inside @msg, %NULL on error.
 */
UBGP_API CHECK_NONNULL(1) void *getribents(umrt_msg_s *msg, size_t *pcount, size_t *pn);

UBGP_API CHECK_NONNULL(1) rib_header_t *startribents(umrt_msg_s *msg, size_t *pcount);

UBGP_API CHECK_NONNULL(1) rib_entry_t *nextribent(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1, 2, 5)
umrt_err putribent(umrt_msg_s        *msg,
                   const rib_entry_t *pe,
                   uint16_t           idx,
                   time_t             seconds,
                   const bgpattr_t   *attrs,
                   size_t             attrs_size);

UBGP_API CHECK_NONNULL(1) umrt_err endribents(umrt_msg_s *msg);

// BGP4MP

UBGP_API CHECK_NONNULL(1) bgp4mp_header_t *getbgp4mpheader(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) void *unwrapbgp4mp(umrt_msg_s *msg, size_t *pn);

// ZEBRA BGP

UBGP_API CHECK_NONNULL(1) zebra_header_t *getzebraheader(umrt_msg_s *msg);

UBGP_API CHECK_NONNULL(1) void *unwrapzebra(umrt_msg_s *msg, size_t *pn);

#endif

