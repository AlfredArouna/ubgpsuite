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

#include "bgpparams.h"

UBGP_API size_t getgracefulrestarttuples(afi_safi_t     *dst,
                                         size_t          n,
                                         const bgpcap_t *cap)
{
    assert(cap->code == GRACEFUL_RESTART_CODE);

    const afi_safi_t *src = (const afi_safi_t *) &cap->data[GRACEFUL_RESTART_TUPLES_OFFSET];

    // XXX: signal non-zeroed flags as an error or mask them?
    size_t size = cap->len - GRACEFUL_RESTART_TUPLES_OFFSET;
    size /= sizeof(*src);
    if (n > size)
        n = size;

    // copy and swap bytes
    for (size_t i = 0; i < n; i++) {
        dst[i].afi   = beswap16(src[i].afi);
        dst[i].safi  = src[i].safi;
        dst[i].flags = src[i].flags;
    }
    return size;
}

UBGP_API size_t getaddpathtuples(afi_safi_t* dst, size_t n, const bgpcap_t *cap)
{
    assert(cap->code == ADD_PATH_CODE);

    const afi_safi_t *src = (const afi_safi_t *) cap->data;

    size_t size = cap->len / sizeof(*src);
    if (n > size)
        n = size;

    // copy and swap bytes
    for (size_t i = 0; i < n; i++) {
        dst[i].afi   = beswap16(src[i].afi);
        dst[i].safi  = src[i].safi;
        dst[i].flags = src[i].flags;
    }
    return size;
}

