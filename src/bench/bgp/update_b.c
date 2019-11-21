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
#include "bench.h"

static ubgp_msg_s curmsg;

void bupdategen(cbench_state_t *state)
{
    byte buf[256];

    bgpattr_t *attr = (bgpattr_t *) buf;

    const uint32_t asseq[] = { 1, 2, 3, 4, 5, 6, 7, 9, 11 };
    const uint32_t asset[] = { 22, 0x11111, 93495 };

    while (cbench_next_iteration(state)) {
        setbgpwrite(&curmsg, BGP_UPDATE, BGPF_DEFAULT);

        startbgpattribs(&curmsg);
        {
            attr->code  = ORIGIN_CODE;
            attr->flags = DEFAULT_ORIGIN_FLAGS;
            attr->len   = ORIGIN_LENGTH;
            setorigin(attr, ORIGIN_IGP);
            putbgpattrib(&curmsg, attr);

            attr->code  = AS_PATH_CODE;
            attr->flags = DEFAULT_AS_PATH_FLAGS;
            attr->len   = 0;
            putasseg32(attr, AS_SEGMENT_SEQ, asseq, countof(asseq));
            putasseg32(attr, AS_SEGMENT_SET, asset, countof(asset));
            putbgpattrib(&curmsg, attr);
        }
        endbgpattribs(&curmsg);
        bgpfinish(&curmsg, NULL);
        bgpclose(&curmsg);
    }
}

