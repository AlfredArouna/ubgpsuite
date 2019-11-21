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

#ifndef UBGP_DUMPPACKET_H_
#define UBGP_DUMPPACKET_H_

#include "funcattribs.h"
#include "bgp.h"
#include "mrt.h"

#include <stdarg.h>
#include <stdio.h>

UBGP_API CHECK_NONNULL(1, 2, 3) void printbgpv(FILE       *out,
                                               ubgp_msg_s *pkt,
                                               const char *fmt,
                                               va_list     va);

UBGP_API CHECK_NONNULL(1, 2, 3) void printbgp(FILE       *out,
                                              ubgp_msg_s *pkt,
                                              const char *fmt,
                                              ...);

UBGP_API CHECK_NONNULL(1, 2, 3) void printpeerentv(FILE               *out,
                                                   const peer_entry_t *ent,
                                                   const char         *fmt,
                                                   va_list             va);

UBGP_API CHECK_NONNULL(1, 2, 3) void printpeerent(FILE               *out,
                                                  const peer_entry_t *ent,
                                                  const char         *fmt,
                                                  ...);

UBGP_API CHECK_NONNULL(1, 2, 3)
void printstatechangev(FILE                  *out,
                       const bgp4mp_header_t *bgphdr,
                       const char            *fmt,
                       va_list                va);

UBGP_API CHECK_NONNULL(1, 2, 3)
void printstatechange(FILE                  *out,
                      const bgp4mp_header_t *bgphdr,
                      const char            *fmt,
                      ...);

#endif
