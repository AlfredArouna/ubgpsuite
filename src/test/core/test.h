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

#ifndef UBGP_CORE_TEST_H_
#define UBGP_CORE_TEST_H_

void testu128iter(void);

void testu128conv(void);

void testhexdump(void);

void testjoinstrv(void);

void testsplitjoinstr(void);

void testtrimwhites(void);

void teststrunescape(void);

void testnetaddr(void);

void testprefixeqwithmask(void);

void testpatbase(void);

void testpatgetfuncs(void);

void testpatcheckfuncs(void);

void testpatiterator(void);

void testpatcoverage(void);

void testpatgetfirstsubnets(void);

void testzio(void);

void testbz2(void);

#ifdef UBGP_IO_XZ

void testxz(void);

#endif
#ifdef UBGP_IO_LZ4

void testlz4(void);

#endif

void testlz4smallwrites(void);

void testbgpdumppacketrow(void);

void testpatproblem(void);

#endif

