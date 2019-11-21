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

#include "../../ubgp/u128.h"

#include <CUnit/CUnit.h>

#include <stdlib.h>

void testu128iter(void)
{
    uint64_t expect = 0;
    for (u128 i = U128_ZERO; u128cmpu(i, 100) < 0; i = u128addu(i, 1)) {
        CU_ASSERT(u128cmpu(i, expect) == 0);
        CU_ASSERT(u128equ(i, expect));
        expect++;
    }
}

enum {
    CONV_SCALE = 2,
    CONV_STEP = 7
};

void testu128conv(void)
{
    u128 u;
    char *s;

    u128 limit = u128divu(U128_MAX, CONV_SCALE);
    limit = u128subu(limit, CONV_STEP);

    for (u = U128_ZERO; u128cmp(u, limit) < 0; u = u128muladdu(u, CONV_SCALE, CONV_STEP)) {
        s = u128tos(u, 10);
        CU_ASSERT(u128cmp(u, stou128(s, NULL, 10)) == 0);
        CU_ASSERT(u128eq(u, stou128(s, NULL, 10)));

        s = u128tos(u, 2);
        CU_ASSERT(u128cmp(u, stou128(s, NULL, 2)) == 0);
        CU_ASSERT(u128eq(u, stou128(s, NULL, 2)));

        s = u128tos(u, 8);
        CU_ASSERT(u128cmp(u, stou128(s, NULL, 8)) == 0);
        CU_ASSERT(u128eq(u, stou128(s, NULL, 8)));

        s = u128tos(u, 16);
        CU_ASSERT(u128cmp(u, stou128(s, NULL, 16)) == 0);
        CU_ASSERT(u128eq(u, stou128(s, NULL, 16)));

        s = u128tos(u, 36);
        CU_ASSERT(u128cmp(u, stou128(s, NULL, 36)) == 0);
        CU_ASSERT(u128eq(u, stou128(s, NULL, 36)));
    }

    u = U128_MAX;
    s = u128tos(u, 10);
    CU_ASSERT(u128cmp(u, stou128(s, NULL, 10)) == 0);
    CU_ASSERT(u128eq(u, stou128(s, NULL, 10)));

    s = u128tos(u, 2);
    CU_ASSERT(u128cmp(u, stou128(s, NULL, 2)) == 0);
    CU_ASSERT(u128eq(u, stou128(s, NULL, 2)));

    s = u128tos(u, 8);
    CU_ASSERT(u128cmp(u, stou128(s, NULL, 8)) == 0);
    CU_ASSERT(u128eq(u, stou128(s, NULL, 8)));

    s = u128tos(u, 16);
    CU_ASSERT(u128cmp(u, stou128(s, NULL, 16)) == 0);
    CU_ASSERT(u128eq(u, stou128(s, NULL, 16)));

    s = u128tos(u, 36);
    CU_ASSERT(u128cmp(u, stou128(s, NULL, 36)) == 0);
    CU_ASSERT(u128eq(u, stou128(s, NULL, 36)));
}

