/* Copyright (C) 2019 Alpha Cogs S.R.L.
 *
 * bgpgrep is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bgpgrep is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bgpgrep.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This work is based upon work authored by the Institute of Informatics
 * and Telematics of the Italian National Research Council (IIT-CNR) licensed
 * under the BSD 3-Clause license. See AKNOWLEDGEMENT and AUTHORS for more
 * details.
 */

#include "../ubgp/bgp.h"
#include "../ubgp/dumppacket.h"
#include "../ubgp/filterintrin.h"
#include "../ubgp/hexdump.h"
#include "../ubgp/mrt.h"

#include "mrtdataread.h"
#include "progutil.h"

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PEERREF_BITSET_SIZE (UINT16_MAX / (sizeof(uint32_t) * CHAR_BIT))

enum {
    PEERREF_SHIFT = 5,
    PEERREF_MASK  = 0x1f
};

typedef enum {
    PROCESS_SUCCESS = 0,  // all good
    PROCESS_BAD,          // bad record, but keep going
    PROCESS_CORRUPTED,    // bad dump, skip the entire dump
} process_result_t;

static bool   seen_ribpi;
static ullong pkgseq;

static uint32_t peerrefs[MAX_PEERREF_BITSET_SIZE];

// packets used during analysis
static umrt_msg_s curmrt, curpi;
static ubgp_msg_s curbgp;

static ubgp_err close_bgp_packet(const char *filename)
{
    ubgp_err err = bgperror(&curbgp);
    if (err != BGP_ENOERR) {
        eprintf("%s: bad packet detected (%s)",
                filename,
                bgpstrerror(err));

        fprintf(stderr, "binary packet dump follows:\n");
        fprintf(stderr, "ASN32BIT: %s ADDPATH: %s\n",
                        isbgpasn32bit(&curbgp) ? "yes" : "no",
                        isbgpaddpath(&curbgp)  ? "yes" : "no");

        size_t n;
        const void *data = getbgpdata(&curbgp, &n);

        hexdump(stderr, data, n, "x#|1|80");
        fputc('\n', stderr);
    }

    bgpclose(&curbgp);
    return err;
}

static void report_bad_rib(const char        *filename,
                           int                err,
                           const rib_entry_t *rib)
{
    eprintf("%s: bad RIB entry for NLRI %s (%s)",
            filename,
            naddrtos(&rib->nlri, NADDR_CIDR),
            bgpstrerror(err));

    fprintf(stderr, "attributes segment dump follows:\n");
    hexdump(stderr, rib->attrs, rib->attr_length, "x#|1|80");
    fputc('\n', stderr);
}

static process_result_t processbgp4mp(const char         *filename,
                                      const mrt_header_t *hdr,
                                      filter_vm_t        *vm,
                                      mrt_dump_fmt_t      format)
{
    size_t n;
    void *data;

    const bgp4mp_header_t *bgphdr = getbgp4mpheader(&curmrt);
    if (unlikely(!bgphdr)) {
        eprintf("%s: corrupted BGP4MP header (%s)",
                filename,
                mrtstrerror(mrterror(&curmrt)));
        return PROCESS_CORRUPTED;
    }

    vm->kp[K_PEER_AS].as = bgphdr->peer_as;
    memcpy(&vm->kp[K_PEER_ADDR].addr, &bgphdr->peer_addr, sizeof(vm->kp[K_PEER_ADDR].addr));

    int res;

    uint flags     = BGPF_NOCOPY;
    size_t as_size = sizeof(uint16_t); // for state changes

    ubgp_err err = BGP_ENOERR;
    switch (hdr->subtype) {
    case BGP4MP_STATE_CHANGE_AS4:
        as_size = sizeof(uint32_t);
        FALLTHROUGH;
    case BGP4MP_STATE_CHANGE:
        printstatechange(stdout, bgphdr, "A*F*T", as_size, &vm->kp[K_PEER_ADDR].addr, vm->kp[K_PEER_AS].as, &hdr->stamp);
        break;

    case BGP4MP_MESSAGE_AS4_ADDPATH:
    case BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH:
        flags |= BGPF_ADDPATH;
        FALLTHROUGH;
    case BGP4MP_MESSAGE_AS4:
    case BGP4MP_MESSAGE_AS4_LOCAL:
        flags |= BGPF_ASN32BIT;
        FALLTHROUGH;
    case BGP4MP_MESSAGE_ADDPATH:
    case BGP4MP_MESSAGE_LOCAL_ADDPATH:
        // if BGPF_ASN32BIT is on, then we're arriving here through the case above,
        // so don't modify the flags
        flags |= (flags & BGPF_ASN32BIT) == 0 ? BGPF_ADDPATH : 0;
        FALLTHROUGH;
    case BGP4MP_MESSAGE:
    case BGP4MP_MESSAGE_LOCAL:
        data = unwrapbgp4mp(&curmrt, &n);
        if (unlikely(!data)) {
            eprintf("%s: corrupted BGP4MP message (%s)",
                    filename,
                    mrtstrerror(mrterror(&curmrt)));
            return PROCESS_CORRUPTED;
        }

        err = setbgpread(&curbgp, data, n, flags);
        if (unlikely(err != BGP_ENOERR))
            break;

        res = true;  // assume packet passes, we will only filter BGP updates
        if (getbgptype(&curbgp) == BGP_UPDATE) {
            res = bgp_filter(&curbgp, vm);
            if (res < 0 && res != VM_BAD_PACKET)
                exprintf(EXIT_FAILURE, "%s: unexpected filter failure (%s)",
                                       filename,
                                       filter_strerror(res));
        }
        if (res > 0) {
            const char *fmt = (format == MRT_DUMP_CHEX) ? "xF*T" : "rF*T";

            printbgp(stdout, &curbgp,
                             fmt,
                             &vm->kp[K_PEER_ADDR].addr,
                             vm->kp[K_PEER_AS].as, &hdr->stamp);
        }

        err = close_bgp_packet(filename);
        break;

    default:
        eprintf("%s: unhandled BGP4MP packet of subtype: %#x",
                filename,
                (uint) hdr->subtype);
        break;
    }

    if (unlikely(err != BGP_ENOERR))
        return PROCESS_BAD;

    return PROCESS_SUCCESS;
}

static process_result_t processzebra(const char         *filename,
                                     const mrt_header_t *hdr,
                                     filter_vm_t        *vm,
                                     mrt_dump_fmt_t      format)
{
    size_t n;
    void *data;

    const zebra_header_t *zhdr = getzebraheader(&curmrt);
    if (unlikely(!zhdr)) {
        eprintf("%s: corrupted Zebra BGP header (%s)",
                filename,
                mrtstrerror(mrterror(&curmrt)));
        return PROCESS_CORRUPTED;
    }

    vm->kp[K_PEER_AS].as = zhdr->peer_as;
    memcpy(&vm->kp[K_PEER_ADDR].addr, &zhdr->peer_addr, sizeof(vm->kp[K_PEER_ADDR].addr));

    int res;

    ubgp_err err = BGP_ENOERR;
    switch (hdr->subtype) {
    case MRT_BGP_STATE_CHANGE:
        // FIXME printstatechange(stdout, bgphdr, "A*F*T", sizeof(uint16_t), &vm->kp[K_PEER_ADDR].addr, vm->kp[K_PEER_AS].as, &hdr->stamp);
        break;


    case MRT_BGP_NULL:
    case MRT_BGP_PREF_UPDATE:
    case MRT_BGP_SYNC:
    case MRT_BGP_OPEN:
    case MRT_BGP_NOTIFY:
    case MRT_BGP_KEEPALIVE:
        break;
    
    case MRT_BGP_UPDATE:
        data = unwrapzebra(&curmrt, &n);
        if (unlikely(!data)) {
            eprintf("%s: corrupted Zebra BGP message (%s)",
                    filename,
                    mrtstrerror(mrterror(&curmrt)));
            return PROCESS_CORRUPTED;
        }

        setbgpwrite(&curbgp, BGP_UPDATE, BGPF_DEFAULT);
        setbgpdata(&curbgp, data, n);
        if (unlikely(!bgpfinish(&curbgp, NULL)))
            break;

        res = bgp_filter(&curbgp, vm);
        if (res < 0 && res != VM_BAD_PACKET) {
            exprintf(EXIT_FAILURE, "%s: unexpected filter failure (%s)",
                                   filename,
                                   filter_strerror(res));
        }
        if (res > 0) {
            const char *fmt = (format == MRT_DUMP_CHEX) ? "xF*T" : "rF*T";

            printbgp(stdout, &curbgp,
                             fmt,
                             &vm->kp[K_PEER_ADDR].addr,
                             vm->kp[K_PEER_AS].as, &hdr->stamp);
        }

        err = close_bgp_packet(filename);
        break;

    default:
        eprintf("%s: unhandled Zebra BGP packet of subtype: %#x",
                filename,
                (uint) hdr->subtype);
        break;
    }

    if (unlikely(err != BGP_ENOERR))
        return PROCESS_BAD;

    return PROCESS_SUCCESS;
}

static bool istrivialfilter(filter_vm_t *vm)
{
    return vm->codesiz == 1 && vm->code[0] == vm_makeop(FOPC_LOAD, true);
}

enum {
    DONT_DUMP_RIBS,
    DUMP_RIBS
};

enum {
    TABLE_DUMP_SUBTYPE_MARKER = -1  // a marker subtype, see processtabledump()
};

static void refpeeridx(uint16_t idx)
{
    peerrefs[idx >> PEERREF_SHIFT] |= 1 << (idx & PEERREF_MASK);
}

static bool ispeeridxref(uint16_t idx)
{
    return (peerrefs[idx >> PEERREF_SHIFT] & (1 << (idx & PEERREF_MASK))) != 0;
}

static process_result_t processtabledump(const char         *filename,
                                         const mrt_header_t *hdr,
                                         filter_vm_t        *vm,
                                         mrt_dump_fmt_t      format)
{
    const rib_entry_t *rib;

    uint ribflags = BGPF_GUESSMRT | BGPF_STRIPUNREACH;
    uint subtype  = hdr->subtype;
    if (hdr->type == MRT_TABLE_DUMP) {
        // expect attribute lists to be encoded in the appropriate format (disables BGPF_GUESSMRT)
        ribflags |= BGPF_LEGACYMRT;
        // we are dealing with the deprecated TABLE_DUMP format, remap
        // subtype to a spoecial value so we can deal with them in an
        // uniform way without any code duplication
        subtype = TABLE_DUMP_SUBTYPE_MARKER;
    }

    switch (subtype) {
    case MRT_TABLE_DUMPV2_PEER_INDEX_TABLE:
        if (unlikely(seen_ribpi)) {
            eprintf("%s: bad RIB dump, duplicated PEER_INDEX_TABLE, skipping rest of file", filename);
            return PROCESS_CORRUPTED;
        }
        if (unlikely(pkgseq != 0))
            eprintf("%s: warning, PEER_INDEX_TABLE is not the first record in file", filename);

        if (unlikely(!mrtcopy(&curpi, &curmrt)))
            exprintf(EXIT_FAILURE, "out of memory");

        seen_ribpi = true;
        break;

    case MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST_ADDPATH:
    case MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST_ADDPATH:
    case MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST_ADDPATH:
    case MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST_ADDPATH:
    case MRT_TABLE_DUMPV2_RIB_GENERIC_ADDPATH:
        ribflags |= BGPF_ADDPATH;
        FALLTHROUGH;
    case MRT_TABLE_DUMPV2_RIB_IPV4_MULTICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV4_UNICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV6_MULTICAST:
    case MRT_TABLE_DUMPV2_RIB_IPV6_UNICAST:
    case MRT_TABLE_DUMPV2_RIB_GENERIC:
        // every TABLE_DUMPV2 subtype need a peer index
        if (unlikely(!seen_ribpi)) {
            eprintf("%s: warning, TABLE_DUMPV2 RIB with no PEER_INDEX_TABLE, skipping record", filename);
            return PROCESS_BAD;
        }

        setribpi(&curmrt, &curpi);
        FALLTHROUGH;

    case TABLE_DUMP_SUBTYPE_MARKER:
        startribents(&curmrt, NULL);
        while ((rib = nextribent(&curmrt)) != NULL) {
            int res = true;  // assume packet passes
            bool must_close_bgp = false;

            // we want to avoid rebuilding a BGP packet in case we don't want to dump it
            // or we don't want to filter it (think about a peer-index dump without any filtering)
            if (format != MRT_NO_DUMP || !istrivialfilter(vm)) {
                vm->kp[K_PEER_AS].as = rib->peer->as;
                memcpy(&vm->kp[K_PEER_ADDR].addr, &rib->peer->addr, sizeof(vm->kp[K_PEER_ADDR].addr));

                if (rib->peer->as_size == sizeof(uint32_t))
                    ribflags |= BGPF_ASN32BIT;

                ubgp_err err;
                if (ribflags & BGPF_ADDPATH) {
                    netaddrap_t addrap;
                    addrap.pfx    = rib->nlri;
                    addrap.pathid = rib->pathid;

                    err = rebuildbgpfrommrt(&curbgp, &addrap, rib->attrs, rib->attr_length, ribflags);
                } else {
                    err = rebuildbgpfrommrt(&curbgp, &rib->nlri, rib->attrs, rib->attr_length, ribflags);
                }
                if (err != BGP_ENOERR) {
                    report_bad_rib(filename, err, rib);
                    continue;
                }

                must_close_bgp = true;
                res = bgp_filter(&curbgp, vm);
                if (res < 0 && res != VM_BAD_PACKET)
                    exprintf(EXIT_FAILURE, "%s: unexpected filter failure (%s)",
                                           filename,
                                           filter_strerror(res));
            }

            if (res > 0) {
                // update peer index references
                refpeeridx(rib->peer_idx);
                // dump BGP if needed
                if (format != MRT_NO_DUMP) {
                    const char *fmt = (format == MRT_DUMP_ROW) ? "#rF*t" : "#xF*t";

                    printbgp(stdout, &curbgp,
                                     fmt,
                                     &vm->kp[K_PEER_ADDR].addr,
                                     vm->kp[K_PEER_AS].as,
                                     rib->originated);
                }
            }

            if (must_close_bgp)
                close_bgp_packet(filename);
        }

        endribents(&curmrt);
        break;

    default:
        // we can only encounter this fot TABLE_DUMPV2 subtypes
        eprintf("%s: unhandled TABLE_DUMPV2 packet of subtype: %#x", filename, (uint) hdr->subtype);
        break;
    }

    return PROCESS_SUCCESS;
}

int mrtprintpeeridx(const char* filename, io_rw_t* rw, filter_vm_t *vm)
{
    int retval = 0;

    seen_ribpi = false;
    pkgseq     = 0;
    memset(peerrefs, 0, sizeof(peerrefs));

    while (true) {
        bool prev_seen_rib_pi = seen_ribpi;

        umrt_err err = setmrtreadfrom(&curmrt, rw);
        if (unlikely(err == MRT_EIO))
            break;
        if (unlikely(err != MRT_ENOERR)) {
            eprintf("%s: corrupted packet: %s", filename, mrtstrerror(err));  // FIXME better reporting
            continue;
        }

        const mrt_header_t *hdr = getmrtheader(&curmrt);
        if (hdr != NULL && hdr->type == MRT_TABLE_DUMPV2) {
            process_result_t result = processtabledump(filename, hdr, vm, MRT_NO_DUMP);
            if (result != PROCESS_SUCCESS)
                retval = -1;  // packet is not well formed, so propagate error to the caller
            if (unlikely(result == PROCESS_CORRUPTED))
                goto done;    // we must skip the whole packet
        }

        // don't close this message if it is our rib pi
        if ((seen_ribpi ^ prev_seen_rib_pi) == 0) {
            err = mrtclose(&curmrt);
            if (unlikely(err != MRT_ENOERR))
                eprintf("%s: corrupted packet: %s", filename, mrtstrerror(err));  // FIXME better reporting
        }
    }

    if (seen_ribpi) {
        startpeerents(&curpi, NULL);

        peer_entry_t *pe;
        uint16_t idx = 0;
        while ((pe = nextpeerent(&curpi))) {
            if (ispeeridxref(idx))
                printpeerent(stdout, pe, "r");

            idx++;
        }

        endpeerents(&curpi);
    }

done:
    if (seen_ribpi)
        mrtclose(&curpi);

    if (rw->error(rw)) {
        eprintf("%s: read error or corrupted data, skipping rest of file", filename);
        retval = -1;
    }

    return retval;
}

int mrtprocess(const char     *filename,
               io_rw_t        *rw,
               filter_vm_t    *vm,
               mrt_dump_fmt_t  format)
{
    seen_ribpi = false;
    pkgseq     = 0;
    // don't care about peerrefs

    int retval = 0;
    while (true) {
        bool prev_seen_rib_pi = seen_ribpi;

        umrt_err err = setmrtreadfrom(&curmrt, rw);
        if (err == MRT_EIO)
            break;
        if (unlikely(err != MRT_ENOERR)) {
            eprintf("%s: corrupted packet: %s", filename, mrtstrerror(err));  // FIXME better reporting
            retval = -1;  // propagate this error to the caller
            continue;
        }

        const mrt_header_t *hdr = getmrtheader(&curmrt);

        process_result_t result = PROCESS_BAD;  // assume bad record unless stated otherwise
        if (hdr != NULL) {
            switch (hdr->type) {
            case MRT_BGP:
                result = processzebra(filename, hdr, vm, format);
                break;

            case MRT_TABLE_DUMP:
            case MRT_TABLE_DUMPV2:
                result = processtabledump(filename, hdr, vm, format);
                break;

            case MRT_BGP4MP:
            case MRT_BGP4MP_ET:
                result = processbgp4mp(filename, hdr, vm, format);
                break;

            default:
                // skip packet, but not necessarily wrong
                eprintf("%s: unhandled MRT packet of type: %#x", filename, (uint) hdr->type);
                result = PROCESS_SUCCESS;
                break;
            }
        }

        // don't close message if this is our rib pi
        if ((seen_ribpi ^ prev_seen_rib_pi) == 0) {
            err = mrtclose(&curmrt);
            if (unlikely(err != MRT_ENOERR)) {
                eprintf("%s: corrupted packet: %s", filename, mrtstrerror(err));  // FIXME better reporting
                retval = -1;  // propagate this error to the caller
            }
        }
        if (result != PROCESS_SUCCESS)
            retval = -1;  // packet is not well formed, so propagate error to the caller
        if (unlikely(result == PROCESS_CORRUPTED))
            break;        // we must skip the whole packet
    }

    if (seen_ribpi)
        mrtclose(&curpi);

    if (rw->error(rw)) {
        eprintf("%s: read error or corrupted data, skipping rest of file", filename);
        retval = -1;
    }

    return retval;
}
