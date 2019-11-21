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

#include "../../ubgp/strutil.h"

#include <cbench/cbench.h>

#include <inttypes.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

void bulltoa(cbench_state_t *state)
{
    char buf[digsof(unsigned long long) + 1];

    ullong x = ULLONG_MAX;

    while (cbench_next_iteration(state)) {
        ulltoa(buf, NULL, x);
        x--;
    }
}

void bcommsprintf(cbench_state_t *state)
{
    char buf[digsof(uint16_t) + 1 + digsof(uint16_t) + 1];

    uint32_t u = UINT32_MAX;

    while (cbench_next_iteration(state)) {
        sprintf(buf, "%"PRIu16":%"PRIu16, (uint16_t) (u >> 16), (uint16_t) (u & 0xffff));
    }
}

void bsprintf(cbench_state_t *state)
{
    char buf[digsof(ullong) + 1];

    ullong x = ULLONG_MAX;

    while (cbench_next_iteration(state)) {
        sprintf(buf, "%llu", x);
        x--;
    }
}

void bcommulltoa(cbench_state_t *state)
{
    char buf[digsof(uint16_t) + 1 + digsof(uint16_t) + 1];

    uint32_t u = UINT32_MAX;
    char *ptr;
    while (cbench_next_iteration(state)) {
        utoa(buf, &ptr, u >> 16);
        *ptr++ = ':';
        utoa(ptr, NULL, u & 0xffff);
        u--;
    }
}

void bsplit(cbench_state_t *state)
{
    while (cbench_next_iteration(state)) {
        char **str = splitstr(" ", "a b c d e f g h i j k l m n o p q r s t u v w x y z", NULL);
        free(str);
    }
}

void bjoinv(cbench_state_t *state)
{
    while (cbench_next_iteration(state)) {
        char *str = joinstrv(" ", "a", "b", "c", "d", "e", "f",
                                  "g", "h", "i", "j", "k", "l",
                                  "m", "n", "o", "p", "q", "r",
                                  "s", "t", "u", "v", "w", "x",
                                  "y", "z", (char *) NULL);
        free(str);
    }
}

void bjoin(cbench_state_t *state)
{
    static char *strarr[] = {
        "a", "b", "c", "d", "e", "f",
        "g", "h", "i", "j", "k", "l",
        "m", "n", "o", "p", "q", "r",
        "s", "t", "u", "v", "w", "x",
        "y", "z"
    };

    while (cbench_next_iteration(state)) {
        char *str = joinstr(" ", strarr, countof(strarr));
        free(str);
    }
}

