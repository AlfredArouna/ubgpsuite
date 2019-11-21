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

#ifndef UBGP_MRTDATAREAD_H_
#define UBGP_MRTDATAREAD_H_

#include "../ubgp/filterpacket.h"
#include "../ubgp/io.h"

enum {
    K_PEER_AS,
    K_PEER_ADDR
};

enum {
    // filtering functions to fill the stack with peer addresses and ASes
    MRT_ACCUMULATE_ADDRS_FN,
    MRT_ACCUMULATE_ASES_FN,
    // returns true if AS path contains loops, false otherwise
    MRT_FIND_AS_LOOPS_FN
};

typedef enum {
    MRT_NO_DUMP   = '\0',
    MRT_DUMP_CHEX = 'x',
    MRT_DUMP_ROW  = 'r'
} mrt_dump_fmt_t;

int mrtprintpeeridx(const char *filename, io_rw_t *rw, filter_vm_t *vm);

int mrtprocess(const char *filename, io_rw_t *rw, filter_vm_t *vm, mrt_dump_fmt_t format);

#endif

