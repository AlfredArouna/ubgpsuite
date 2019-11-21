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
#include "../../ubgp/patriciatrie.h"

#include <cbench/cbench.h>

void bpatinsert(cbench_state_t *state)
{
    netaddr_t addr;
    addr.family = AF_INET;
    addr.bitlen = 32;
    memset(addr.bytes, 0, sizeof(addr.bytes));

    patricia_trie_t trie;

    patinit(&trie, AF_INET);

    while (cbench_next_iteration(state)) {
        addr.u32[0] = beswap32(state->curiter);
        patinsert(&trie, &addr, NULL);
    }

    patdestroy(&trie);
}
