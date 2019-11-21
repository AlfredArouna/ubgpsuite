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

#ifndef UBGP_BGPPARAMS_H_
#define UBGP_BGPPARAMS_H_

#include "branch.h"
#include "endian.h"
#include "netaddr.h"

#include <assert.h>
#include <stddef.h>
#include <string.h>

/**
 * SECTION: bgpparams
 * @include bgpparams.h
 * @title Constants and functions for BGP open packet parameter field
 *
 * This file exposes a fast and performance-oriented interface to BGP open
 * packet parameters, defined in relevant RFC.
 *
 * This file is guaranteed to include `stddef.h` and `stdint.h`, as well
 * as  `netaddr.h`, it may include other standard C or ubgp-specific
 * headers in the interest of providing inline versions of its functions.
 */

enum {
    PARAM_CODE_OFFSET = 0,
    PARAM_LENGTH_OFFSET = sizeof(uint8_t),

    PARAM_HEADER_SIZE = PARAM_LENGTH_OFFSET + sizeof(uint8_t),
    PARAM_LENGTH_MAX = 0xff,

    /// @brief Maximum parameter size in bytes (header included).
    PARAM_SIZE_MAX = PARAM_LENGTH_MAX + PARAM_HEADER_SIZE,

    /// @brief Maximum overall parameter chunk size in BGP open.
    PARAMS_SIZE_MAX = 0xff
};

/// @brief Constants relevant to the parameters field inside a BGP open packet.
enum {
    CAPABILITY_CODE = 2  ///< Parameter code indicating a capability list.
};

/// @brief https://www.iana.org/assignments/capability-codes/capability-codes.xhtml
enum {
    BAD_CAPABILITY_CODE = -1,                   ///< Not a valid value, error indicator.

    MULTIPROTOCOL_CODE = 1,                     ///< RFC 2858
    ROUTE_REFRESH_CODE = 2,                     ///< RFC 2918
    OUTBOUND_ROUTE_FILTERING_CODE = 3,          ///< RFC 5291
    MULTIPLE_ROUTES_TO_A_DESTINATION_CODE = 4,  ///< RFC 3107
    EXTENDED_NEXT_HOP_ENCODING_CODE = 5,        ///< RFC 5549
    EXTENDED_MESSAGE_CODE = 6,                  ///< https://tools.ietf.org/html/draft-ietf-idr-bgp-extended-messages-21
    BGPSEC_CAPABILITY_CODE = 7,                 ///< https://tools.ietf.org/html/draft-ietf-sidr-bgpsec-protocol-23
    GRACEFUL_RESTART_CODE = 64,                 ///< RFC 4724
    ASN32BIT_CODE = 65,                         ///< RFC 6793
    DYNAMIC_CAPABILITY_CODE = 67,
    MULTISESSION_BGP_CODE = 68,                 ///< https://tools.ietf.org/html/draft-ietf-idr-bgp-multisession-07
    ADD_PATH_CODE = 69,                         ///< RFC 7911
    ENHANCED_ROUTE_REFRESH_CODE = 70,           ///< RFC 7313
    LONG_LIVED_GRACEFUL_RESTART_CODE = 71,      ///< https://tools.ietf.org/html/draft-uttaro-idr-bgp-persistence-03
    FQDN_CODE = 73,                             ///< https://tools.ietf.org/html/draft-walton-bgp-hostname-capability-02
    MULTISESSION_CISCO_CODE = 131               ///< Cisco version of multisession_bgp
};

enum {
    CAPABILITY_CODE_OFFSET   = 0,
    CAPABILITY_LENGTH_OFFSET = CAPABILITY_CODE_OFFSET + sizeof(uint8_t),
    CAPABILITY_HEADER_SIZE   = CAPABILITY_LENGTH_OFFSET + sizeof(uint8_t),

    CAPABILITY_LENGTH_MAX = 0xff - CAPABILITY_HEADER_SIZE,
    CAPABILITY_SIZE_MAX   = CAPABILITY_LENGTH_MAX + CAPABILITY_HEADER_SIZE,

    ASN32BIT_LENGTH = sizeof(uint32_t),

    MULTIPROTOCOL_AFI_OFFSET      = 0,
    MULTIPROTOCOL_RESERVED_OFFSET = MULTIPROTOCOL_AFI_OFFSET + sizeof(uint16_t),
    MULTIPROTOCOL_SAFI_OFFSET     = MULTIPROTOCOL_RESERVED_OFFSET + sizeof(uint8_t),
    MULTIPROTOCOL_LENGTH          = MULTIPROTOCOL_SAFI_OFFSET + sizeof(uint8_t),

    GRACEFUL_RESTART_FLAGTIME_OFFSET = 0,
    GRACEFUL_RESTART_TUPLES_OFFSET   = GRACEFUL_RESTART_FLAGTIME_OFFSET + sizeof(uint16_t),
    GRACEFUL_RESTART_BASE_LENGTH     = GRACEFUL_RESTART_TUPLES_OFFSET
};

typedef struct {
    uint8_t code;
    uint8_t len;
    byte    data[CAPABILITY_LENGTH_MAX];
} bgpcap_t;

/// @brief Restart flags see: https://tools.ietf.org/html/rfc4724#section-3.
enum {
    RESTART_FLAG = 1 << 3 ///< Most significant bit, Restart State (R).
};

/// @brief Address Family flags, see: https://tools.ietf.org/html/rfc4724#section-3.
enum {
    FORWARDING_STATE = 1 << 7 ///< Most significant bit, Forwarding State (F).
};

/// @brief ADD_PATH SEND/RECEIVE flags
enum {
    ADD_PATH_RX = 1 << 0,
    ADD_PATH_TX = 1 << 1
};

typedef struct {
    afi_t   afi;
    safi_t  safi;
    uint8_t flags;
} afi_safi_t;

static_assert(sizeof(afi_safi_t) ==  4, "Unsupported platform");

static inline CHECK_NONNULL(1) uint32_t getasn32bit(const bgpcap_t *cap)
{
    assert(cap->code == ASN32BIT_CODE);
    assert(cap->len  == ASN32BIT_LENGTH);

    uint32_t asn32bit;
    memcpy(&asn32bit, cap->data, sizeof(asn32bit));
    return beswap32(asn32bit);
}

static inline CHECK_NONNULL(1) bgpcap_t *setasn32bit(bgpcap_t *cap, uint32_t as)
{
    assert(cap->code == ASN32BIT_CODE);
    assert(cap->len  == ASN32BIT_LENGTH);

    as = beswap32(as);
    memcpy(cap->data, &as, sizeof(as));
    return cap;
}

static inline CHECK_NONNULL(1) bgpcap_t *setmultiprotocol(bgpcap_t *cap,
                                                          afi_t     afi,
                                                          safi_t    safi)
{
    assert(cap->code == MULTIPROTOCOL_CODE);
    assert(cap->len  == MULTIPROTOCOL_LENGTH);

    uint16_t t = beswap16(afi);
    byte  *ptr = cap->data;
    memcpy(ptr, &t, sizeof(t));
    ptr += sizeof(t);

    *ptr++ = 0;  // reserved
    *ptr   = safi;
    return cap;
}

static inline CHECK_NONNULL(1) afi_safi_t getmultiprotocol(const bgpcap_t *cap)
{
    assert(cap->code == MULTIPROTOCOL_CODE);
    assert(cap->len == MULTIPROTOCOL_LENGTH);

    afi_safi_t r;
    uint16_t t;

    memcpy(&t, &cap->data[MULTIPROTOCOL_AFI_OFFSET], sizeof(t));

    r.afi   = beswap16(t);
    r.safi  = cap->data[MULTIPROTOCOL_SAFI_OFFSET];
    r.flags = 0;
    return r;
}

static inline CHECK_NONNULL(1) bgpcap_t *setgracefulrestart(bgpcap_t *cap,
                                                            uint      flags,
                                                            uint      secs)
{
    assert(cap->code == GRACEFUL_RESTART_CODE);

    // RFC mandates that any other restart flag is reserved and zeroed
    flags &= RESTART_FLAG;

    uint16_t flagtime = ((flags & 0x000f) << 12) | (secs & 0x0fff);
    flagtime = beswap16(flagtime);

    memcpy(cap->data, &flagtime, sizeof(flagtime));
    return cap;
}

static inline CHECK_NONNULL(1)
bgpcap_t *putgracefulrestarttuple(bgpcap_t *cap,
                                  afi_t     afi,
                                  safi_t    safi,
                                  uint      flags)
{
    assert(cap->code == GRACEFUL_RESTART_CODE);

    // RFC mandates that any other flag is reserved and zeroed out
    flags &= FORWARDING_STATE;
    afi_safi_t t = {
        .afi   = beswap16(afi),
        .safi  = safi,
        .flags = flags
    };

    assert(cap->len + sizeof(t) <= CAPABILITY_LENGTH_MAX);

    memcpy(&cap->data[cap->len], &t, sizeof(t));  // append tuple
    cap->len += sizeof(t);
    return cap;
}

static inline CHECK_NONNULL(1) uint getgracefulrestarttime(const bgpcap_t *cap)
{
    assert(cap->code == GRACEFUL_RESTART_CODE);

    uint16_t flagtime;
    memcpy(&flagtime, cap->data, sizeof(flagtime));
    flagtime = beswap16(flagtime);
    return flagtime & 0x0fff;
}

static inline CHECK_NONNULL(1) uint getgracefulrestartflags(const bgpcap_t *cap)
{
    assert(cap->code == GRACEFUL_RESTART_CODE);

    uint16_t flagtime;
    memcpy(&flagtime, cap->data, sizeof(flagtime));
    flagtime = beswap16(flagtime);

    // XXX: signal non-zeroed flags as an error or mask them?
    return flagtime >> 12;
}

/**
 * getgracefulrestarttuples:
 *
 * Get tuples from a graceful restart capability into @dst.
 */
UBGP_API CHECK_NONNULL(3) size_t getgracefulrestarttuples(afi_safi_t     *dst,
                                                          size_t          n,
                                                          const bgpcap_t *cap);

static inline CHECK_NONNULL(1) bgpcap_t *putaddpathtuple(bgpcap_t *cap,
                                                         afi_t     afi,
                                                         safi_t    safi,
                                                         uint      flags)
{
    assert(cap->code == ADD_PATH_CODE);

    afi_safi_t t = {
        .afi   = beswap16(afi),
        .safi  = safi,
        .flags = flags
    };

    assert(cap->len + sizeof(t) <= CAPABILITY_LENGTH_MAX);

    memcpy(&cap->data[cap->len], &t, sizeof(t));  // append tuple
    cap->len += sizeof(t);
    return cap;
}

UBGP_API CHECK_NONNULL(3) size_t getaddpathtuples(afi_safi_t     *dst,
                                                  size_t          n,
                                                  const bgpcap_t *cap);

#endif

