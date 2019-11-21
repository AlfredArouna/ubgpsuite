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

#include "../../ubgp/bgp.h"
#include "../../ubgp/bgpattribs.h"
#include "../../ubgp/dumppacket.h"
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>
#include <arpa/inet.h>


static ubgp_msg_s curbgp;

void testbgpdumppacketrow(void)
{
    setbgpwrite(&curbgp, BGP_UPDATE, BGPF_DEFAULT);

    byte buf[255];

    startbgpattribs(&curbgp);

        bgpattr_t *origin = (bgpattr_t *) buf;
        origin->code = ORIGIN_CODE;
        origin->len = ORIGIN_LENGTH;
        origin->flags = DEFAULT_ORIGIN_FLAGS;

        setorigin(origin, ORIGIN_IGP);
        putbgpattrib(&curbgp, origin);

        struct sockaddr_in nh;
        inet_pton(AF_INET, "1.2.3.4", &nh);
        bgpattr_t *nexthop = (bgpattr_t *)buf;
        nexthop->code = NEXT_HOP_CODE;
        nexthop->len = NEXT_HOP_LENGTH;
        nexthop->flags = DEFAULT_NEXT_HOP_FLAGS;
        setnexthop(nexthop, nh.sin_addr);
        putbgpattrib(&curbgp, nexthop);

        bgpattr_t *aspath = (bgpattr_t *)buf;
        aspath->code = AS_PATH_CODE;
        aspath->len = 0;
        aspath->flags = DEFAULT_AS_PATH_FLAGS;

        uint32_t as[] = {2598, 137, 3356};
        putasseg32(aspath, AS_SEGMENT_SEQ, as, 3);
        putbgpattrib(&curbgp, aspath);

    endbgpattribs(&curbgp);

    struct in_addr nlri_s;
    inet_pton(AF_INET, "10.0.0.0", &nlri_s);
    netaddr_t nlri;
    makenaddr(&nlri, AF_INET, &nlri_s, 8);

    startnlri(&curbgp);
        putnlri(&curbgp, &nlri);
    endnlri(&curbgp);

    size_t pktlen;
    bgpfinish(&curbgp, &pktlen);

    printbgp(stdout, &curbgp, "r");

    bgpclose(&curbgp);
}

