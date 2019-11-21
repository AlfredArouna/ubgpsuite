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

#ifndef UBGP_ATOMICS_H_
#define UBGP_ATOMICS_H_

#ifdef __GNUC__

typedef int atomic_word       __attribute__((__mode__(__word__)));
typedef unsigned atomic_uword __attribute__((__mode__(__word__)));

#define ATOMIC_INCR(x) (__atomic_fetch_add(&(x), 1, __ATOMIC_ACQ_REL) + 1)
#define ATOMIC_DECR(x) (__atomic_fetch_sub(&(x), 1, __ATOMIC_ACQ_REL) - 1)

#else
/* rely on stdatomic, not as portable as we'd like to, unfortunately */

#include <stdatomic.h>

#if defined(ATOMIC_LONG_LOCK_FREE)
typedef atomic_long  atomic_word;
typedef atomic_ulong atomic_uword;
#else
typedef atomic_int  atomic_word;
typedef atomic_uint atomic_uword;
#endif

#define ATOMIC_INCR(x) (atomic_fetch_add_explicit(&(x), 1, memory_order_acq_rel) + 1)
#define ATOMIC_DECR(x) (atomic_fetch_sub_explicit(&(x), 1, memory_order_acq_rel) - 1)

#endif

#endif

