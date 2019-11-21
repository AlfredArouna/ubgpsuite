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

#ifndef UBGP_CORE_BENCH_H_
#define UBGP_CORE_BENCH_H_

#include <cbench/cbench.h>

void bcommsprintf(cbench_state_t *state);

void bsprintf(cbench_state_t *state);

void bcommulltoa(cbench_state_t *state);

void bulltoa(cbench_state_t *state);

void bsplit(cbench_state_t *state);

void bjoinv(cbench_state_t *state);

void bjoin(cbench_state_t *state);

void bpatinsert(cbench_state_t *state);

void bprefixeqwithmask(cbench_state_t *state);

void bppathcompwithmask(cbench_state_t *state);

#endif

