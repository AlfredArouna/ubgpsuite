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

#include "../../ubgp/endian.h"
#include "../../ubgp/netaddr.h"

#include <cbench/cbench.h>

#include <string.h>

volatile int bh;

void bprefixeqwithmask(cbench_state_t *state)
{
    netaddr_t addr, dest;
    memset(&addr, 0, sizeof(addr));
    memset(&dest, 0, sizeof(dest));
    addr.family = dest.family = AF_INET;
    addr.bitlen = dest.bitlen = 32;

    while (cbench_next_iteration(state)) {
        addr.u32[0] = beswap32(state->curiter);
        dest.u32[0] = state->curiter;
        bh = prefixeqwithmask(&addr, &dest, state->curiter % 129);
    }
}

static int patcompwithmask(const netaddr_t *addr, const netaddr_t *dest, int mask)
{
    if (memcmp(&addr->bytes[0], &dest->bytes[0], mask / 8) == 0) {
        int n = mask / 8;
        int m = ((unsigned int) (~0) << (8 - (mask % 8)));

        if (((mask & 0x7) == 0) || ((addr->bytes[n] & m) == (dest->bytes[n] & m)))
            return 1;
    }

    return 0;
}

void bppathcompwithmask(cbench_state_t *state)
{
    netaddr_t addr, dest;
    memset(&addr, 0, sizeof(addr));
    memset(&dest, 0, sizeof(dest));
    addr.family = dest.family = AF_INET;
    addr.bitlen = dest.bitlen = 32;

    while (cbench_next_iteration(state)) {
        addr.u32[0] = beswap32(state->curiter);
        dest.u32[0] = state->curiter;
        bh = patcompwithmask(&addr, &dest, state->curiter % 129);
    }
}
