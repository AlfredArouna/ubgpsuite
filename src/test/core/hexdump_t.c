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

#include "../../ubgp/hexdump.h"
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>

void testhexdump(void)
{
    const struct {
        byte input[16];
        size_t input_size;
        const char *format;
        const char *expected;
    } table[] = {
        {{0x40, 0x01, 0x01, 0x01}, 4, "x", "40010101"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x#{1}", "{ 0x40, 0x01, 0x01, 0x01 }"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x# 1", "0x40 0x01 0x01 0x01"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x# 1 9", "0x40 0x01\n0x01 0x01"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x#|1", "0x40 | 0x01 | 0x01 | 0x01"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x 1", "40 01 01 01"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "x|1", "40 | 01 | 01 | 01"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "b", "01000000000000010000000100000001"},
        {{0x40, 0x01, 0x01, 0x01}, 4, "b# 1", "b01000000 b00000001 b00000001 b00000001"},
        {{0x40, 0x01, 0x01, 0x01}, 4, BINARY_PLAIN, "01000000, 00000001, 00000001, 00000001"},
        {{0x40, 0x01, 0x01, 0x01}, 4, HEX_C_ARRAY, "{ 0x40, 0x01, 0x01, 0x01 }"},
        {{0x40, 0x01, 0x01, 0x01}, 4, HEX_PLAIN, "0x40, 0x01, 0x01, 0x01"},
    };
    
    for (size_t i = 0; i < countof(table); i++) {
        char output[256];

        size_t n = hexdumps(output, sizeof(output), table[i].input, table[i].input_size, table[i].format);
        CU_ASSERT_EX(strlen(table[i].expected)+1 == n, "with i = %zu", i);
        CU_ASSERT_STRING_EQUAL_EX(output, table[i].expected,
                                  "with i = %zu: \"%s\" != \"%s\"",
                                  i, output, table[i].expected);
    }
}

