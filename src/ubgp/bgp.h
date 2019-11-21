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

#ifndef UBGP_BGP_H_
#define UBGP_BGP_H_

#include "bgpattribs.h"
#include "bgpparams.h"
#include "io.h"
#include "netaddr.h"

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * SECTION: bgp
 * @include: bgp.h
 * @title: BGP Message Decoding and Encoding
 *
 * BGP packet reading and writing routines.
 *
 * `bgp.h` is guaranteed to include POSIX `arpa/inet.h`, standard
 * `stdint.h` and ubgp-specific `netaddr.h` and `io.h`.
 * It may include additional standard and ubgp-specific
 * headers to provide its functionality.
 */

typedef enum {
    BGP_FSM_IDLE        = 1,
    BGP_FSM_CONNECT     = 2,
    BGP_FSM_ACTIVE      = 3,
    BGP_FSM_OPENSENT    = 4,
    BGP_FSM_OPENCONFIRM = 5,
    BGP_FSM_ESTABLISHED = 6
} ubgp_fsm_state;


#define BGP_VERSION   4
#define BGP_HOLD_SECS 180

#define AS_TRANS 23456

/**
 * @BGPF_DEFAULT: default flags for setbgpread(), copy the buffer.
 * @BGPF_NOCOPY:  a flag for setbgpread(), does not copy the read-buffer.
 *
 * BGP flags.
 */
enum {
    BGPF_DEFAULT      = 0,
    BGPF_NOCOPY       = 1 << 0,
    BGPF_ADDPATH      = 1 << 1,
    BGPF_ASN32BIT     = 1 << 2,
};

/**
 * @BGPF_GUESSMRT:     Guess TABLE DUMP V2 format between
 *                     %BGPF_STDMRT and %BGPF_FULLMPREACH using a heuristic
 * @BGPF_STDMRT:       Strictly standard TABLE DUMP V2
 *                     truncated MP REACH attributes with 32-bits
 *                     only AS PATH, this flag prevails over
 *                     %BGPF_GUESSMRT
 * @BGPF_FULLMPREACH:  variation over standard TABLE DUMP V2
 *                     full MP REACH attributes with 32-bits only
 *                     AS PATH
 * @BGPF_STRIPUNREACH: Strip MP UNREACH attributes from attribute list
 *                     (they shouldn't be there anyway...)
 * @BGPF_LEGACYMRT:    Legacy TABLE DUMP format: full BGP attribute list with
 *                     16-bits AS PATH this flag implies %BGPF_FULLMPREACH and
 *                     disables both %BGPF_ASN32BIT and %BGPF_ADDPATH,
 *                     this flag prevails over both %BGPF_STDMRT and %BGPF_GUESSMRT
 *
 * Flags for rebuilding original BGP from MRT attribute list.
 */
enum {
    BGPF_GUESSMRT     = 0, 
    BGPF_STDMRT       = 1 << 3,
    BGPF_FULLMPREACH  = 1 << 4,
    BGPF_STRIPUNREACH = 1 << 5,
    BGPF_LEGACYMRT    = 1 << 6
};

typedef enum {
    BGP_BADTYPE = -1,

    BGP_OPEN          = 1,
    BGP_UPDATE        = 2,
    BGP_NOTIFICATION  = 3,
    BGP_KEEPALIVE     = 4,
    BGP_ROUTE_REFRESH = 5,
    BGP_CLOSE         = 255
} ubgp_msgtype;

/**
 * ubgp_err:
 * @BGP_ENOERR:       no error (success) guaranteed to be zero
 * @BGP_EIO:          input/Output error during packet read
 * @BGP_EINVOP:       invalid operation (e.g. write while reading packet)
 * @BGP_ENOMEM:       out of memory
 * @BGP_EBADHDR:      bad BGP packet header
 * @BGP_EBADTYPE:     unrecognized BGP packet type
 * @BGP_EBADPARAMLEN: open message has invalid parameters field length
 * @BGP_EBADWDRWN:    update message has invalid Withdrawn field
 * @BGP_EBADATTR:     update message has invalid Path Attributes field
 * @BGP_EBADNLRI:     update message has invalid NLRI field
 *
 * Error codes returned by the BGP API.
 */
typedef enum {
    BGP_ENOERR = 0,
    BGP_EIO,
    BGP_EINVOP,
    BGP_ENOMEM,
    BGP_EBADHDR,
    BGP_EBADTYPE,
    BGP_EBADPARAMLEN,
    BGP_EBADWDRWN,
    BGP_EBADATTR,
    BGP_EBADNLRI
} ubgp_err;

static inline const char *bgpstrerror(ubgp_err err)
{
    switch (err) {
    case BGP_ENOERR:
        return "Success";
    case BGP_EIO:
        return "I/O error";
    case BGP_EINVOP:
        return "Invalid operation";
    case BGP_ENOMEM:
        return "Out of memory";
    case BGP_EBADHDR:
        return "Bad BGP header";
    case BGP_EBADTYPE:
        return "Bad BGP packet type";
    case BGP_EBADPARAMLEN:
        return "Oversized or inconsistent BGP open parameters length";
    case BGP_EBADWDRWN:
        return "Oversized or inconsistent BGP update Withdrawn length";
    case BGP_EBADATTR:
        return "Malformed attribute list";
    case BGP_EBADNLRI:
        return "Oversized or inconsistent BGP update NLRI field";
    default:
        return "Unknown BGP error";
    }
}

typedef struct {
    uint8_t  version;
    uint16_t hold_time;
    uint16_t my_as;
    struct in_addr iden;
} bgp_open_t;

typedef struct {
    size_t as_size;
    int type, segno;
    uint32_t as;
} as_pathent_t;

/**
 * BGPBUFSIZ:
 *
 * A good initial buffer size to store a BGP message.
 *
 * A buffer with this size should be enough to store most (if not all)
 * BGP messages without any reallocation, but code should be ready to
 * reallocate buffer to a larger size if necessary.
 *
 * See also: [BGP extended messages draft](https://tools.ietf.org/html/draft-ietf-idr-bgp-extended-messages-24)
 */
#define BGPBUFSIZ 4096

/**
 * ubgp_msg_s:
 * BGP message structure.
 *
 * A structure encapsulating all the relevant status used to read or write a
 * BGP message.
 *
 * This structure must be considered opaque, no field in this structure
 * to be accessed directly, use the appropriate functions instead!
 */
typedef struct {
    /*< private >*/

    uint16_t  flags;   // General status flags.
    uint16_t  pktlen;  // Actual packet length.
    uint16_t  bufsiz;  // Packet buffer capacity
    int16_t   err;     // Last error code.
    byte     *buf;     // Packet buffer base.

    // Relevant status for each BGP packet.
    union {
        // BGP open specific fields
        struct {
            byte *pptr;    // Current parameter pointer
            byte *params;  // Pointer to parameters base

            bgp_open_t opbuf;  // Convenience field for reading
        };
        // BGP update specific fields
        struct {
            byte *ustart;   // Current update message field starting pointer.
            byte *uptr;     // Current update message field pointer.
            byte *uend;     // Current update message field ending pointer.

            // Following fields are mutually exclusive
            union {
                // Read-specific fields.
                struct {
                    netaddrap_t pfxbuf;  // Convenience field for reading prefixes.
                    union {
                        struct {
                            byte *asptr, *asend;
                            byte *as4ptr, *as4end;
                            uint8_t seglen;
                            uint8_t segi;
                            int16_t ascount;
                            as_pathent_t asp;
                        };
                        struct {
                            byte *nhptr, *nhend;
                            byte *mpnhptr, *mpnhend;
                            short mpfamily, mpbitlen;
                            struct in_addr nhbuf;
                        };
                        struct {
                            int ccode;
                            union {
                                community_t comm;
                                ex_community_t excomm;
                                ex_community_v6_t excomm6;
                                large_community_t lcomm;
                            } cbuf;
                        };
                    };

                    uint16_t offtab[16];  // Notable attributes offset table.
                };

                // write-specific fields.
                struct {
                    // Preserved buffer pointer, either fastpresbuf or malloc()ed
                    byte *presbuf;
                    // Fast preserve buffer, to avoid malloc()s.
                    byte fastpresbuf[96];
                };
            };
        };
    };

    byte fastbuf[BGPBUFSIZ];  // Fast buffer to avoid malloc()s.
} ubgp_msg_s;

UBGP_API bool isbgpasn32bit(ubgp_msg_s *msg);

UBGP_API bool isbgpaddpath(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpread(ubgp_msg_s *msg,
                                              const void *data,
                                              size_t      n,
                                              uint        flags);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpreadfd(ubgp_msg_s *msg,
                                                int         fd,
                                                uint        flags);

UBGP_API CHECK_NONNULL(1, 2) ubgp_err setbgpreadfrom(ubgp_msg_s *msg,
                                                     io_rw_t    *io,
                                                     uint        flags);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpwrite(ubgp_msg_s   *msg,
                                               ubgp_msgtype  type,
                                               uint          flags);

UBGP_API CHECK_NONNULL(1) ubgp_msgtype getbgptype(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1, 2) ubgp_err rebuildbgpfrommrt(ubgp_msg_s *msg, const void *nlri, const void *data, size_t n, uint flags);

UBGP_API CHECK_NONNULL(1) size_t getbgplength(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) void *getbgpdata(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpdata(ubgp_msg_s *msg, const void *data, size_t size);

UBGP_API CHECK_NONNULL(1) ubgp_err bgperror(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) void *bgpfinish(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err bgpclose(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) bgp_open_t *getbgpopen(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpopen(ubgp_msg_s *msg, const bgp_open_t *open);

UBGP_API CHECK_NONNULL(1) void *getbgpparams(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpparams(ubgp_msg_s *msg, const void *params, size_t n);

UBGP_API CHECK_NONNULL(1) ubgp_err startbgpcaps(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) bgpcap_t *nextbgpcap(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1, 2) ubgp_err putbgpcap(ubgp_msg_s *msg, const bgpcap_t *cap);

UBGP_API CHECK_NONNULL(1) ubgp_err endbgpcaps(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err setwithdrawn(ubgp_msg_s *msg, const void *data, size_t n);

UBGP_API CHECK_NONNULL(1) void *getwithdrawn(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err startallwithdrawn(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startwithdrawn(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startmpunreachnlri(ubgp_msg_s *msg);

/**
 * nextwithdrawn:
 *
 * This function returns either a #netaddr_t or a #netaddrap_t,
 * depending on the package being ADDPATH enabled.
 * The returned pointer may always be used as #netaddr_t.
 */
UBGP_API CHECK_NONNULL(1) void *nextwithdrawn(ubgp_msg_s *msg);
/**
 * putwithdrawn:
 *
 * @p may be either a #netaddr_t or a #netaddrap_t, depending on
 * the package being ADDPATH enabled.
 */
UBGP_API CHECK_NONNULL(1, 2) ubgp_err putwithdrawn(ubgp_msg_s *msg, const void *p);

UBGP_API CHECK_NONNULL(1) ubgp_err endwithdrawn(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err setbgpattribs(ubgp_msg_s *msg, const void *data, size_t n);

UBGP_API CHECK_NONNULL(1) void *getbgpattribs(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err startbgpattribs(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) bgpattr_t *nextbgpattrib(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1, 2) ubgp_err putbgpattrib(ubgp_msg_s *msg, const bgpattr_t *attr);

UBGP_API CHECK_NONNULL(1) ubgp_err endbgpattribs(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err setnlri(ubgp_msg_s *msg, const void *data, size_t n);

UBGP_API CHECK_NONNULL(1) void *getnlri(ubgp_msg_s *msg, size_t *pn);

UBGP_API CHECK_NONNULL(1) ubgp_err startallnlri(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startnlri(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startmpreachnlri(ubgp_msg_s *msg);

/**
 * putnlri:
 *
 * @p may be either a #netaddr_t or a #netaddrap_t, depending on
 * the package being ADDPATH enabled.
 */
UBGP_API CHECK_NONNULL(1, 2) ubgp_err putnlri(ubgp_msg_s *msg, const void *p);

/**
 * nextnlri:
 *
 * This function returns either a #netaddr_t or a #netaddrap_t,
 * depending on the package being ADDPATH enabled.
 * The returned pointer may always be used as #netaddr_t.
 */
UBGP_API CHECK_NONNULL(1) void *nextnlri(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err endnlri(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startaspath(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startas4path(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startrealaspath(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) as_pathent_t *nextaspath(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err endaspath(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startnhop(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) netaddr_t *nextnhop(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err endnhop(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err startcommunities(ubgp_msg_s *msg, int code);

UBGP_API CHECK_NONNULL(1) void *nextcommunity(ubgp_msg_s *msg);

UBGP_API CHECK_NONNULL(1) ubgp_err endcommunities(ubgp_msg_s *msg);

// utility functions for update packages, direct access to notable attributes

UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgporigin(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpnexthop(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpaggregator(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpatomicaggregate(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpas4aggregator(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getrealbgpaggregator(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpaspath(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpas4path(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpmpreach(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpmpunreach(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpcommunities(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgpexcommunities(ubgp_msg_s *msg);
UBGP_API CHECK_NONNULL(1) bgpattr_t *getbgplargecommunities(ubgp_msg_s *msg);

#endif
