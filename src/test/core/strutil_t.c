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
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>
#include <stdlib.h>


void testtrimwhites(void)
{
    const struct {
        const char *input;
        const char *expect;
    } table[] = {
        {
            "nowhites",
            "nowhites"
        }, {
            "",
            ""
        }, {
            "           ",
            ""
        }, {
            "     onlyleading",
            "onlyleading"
        }, {
            "onlytrailing     ",
            "onlytrailing"
        }, {
            "     both       ",
            "both"
        }, {
            " mixed inside string too     ",
            "mixed inside string too"
        }
    };
    for (size_t i = 0; i < countof(table); i++) {
        char buf[strlen(table[i].input) + 1];

        strcpy(buf, table[i].input);
        CU_ASSERT_STRING_EQUAL(trimwhites(buf), table[i].expect);
    }
}


void testjoinstrv(void)
{
    char *joined = joinstrv(" ", "a", "fine", "sunny", "day", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "a fine sunny day");
    free(joined);

    joined = joinstrv(" not ", "this is", "funny", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "this is not funny");
    free(joined);

    joined = joinstrv(" ", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "");
    free(joined);

    joined = joinstrv(" ", "trivial", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "trivial");
    free(joined);

    joined = joinstrv("", "no", " changes", " to", " be", " seen", " here", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "no changes to be seen here");
    free(joined);

    joined = joinstrv(NULL, "no", " changes", " here", " either", (char *) NULL);
    CU_ASSERT_STRING_EQUAL(joined, "no changes here either");
    free(joined);
}

void testsplitjoinstr(void)
{
    const struct {
        const char *input;
        const char *delim;
        size_t n;
        const char *expected[16];
    } table[] = {
        {
            "a whitespace separated string",
            " ", 4,
            { "a", "whitespace", "separated", "string" }
        }, {
            "",
            NULL, 0
        }, {
            "",
            "", 0
        }
    };
    for (size_t i = 0; i < countof(table); i++) {
        size_t n;
        char **s = splitstr(table[i].input, table[i].delim, &n);

        CU_ASSERT_EQUAL_FATAL(n, table[i].n);
        for (size_t j = 0; j < n; j++)
            CU_ASSERT_STRING_EQUAL(s[j], table[i].expected[j]);

        CU_ASSERT_PTR_EQUAL(s[n], NULL);

        char *sj = joinstr(table[i].delim, s, n);
        CU_ASSERT_STRING_EQUAL(table[i].input, sj);

        free(s);
        free(sj);
    }
}

#define ESCAPED "\\\"\\\\\\/\\b\\f\\n\\r\\t\\v"
#define ESCAPED_BACK "\\\"\\\\\\/\\b\\f\\n\\r\\t\\n"
#define UNESCAPED "\"\\/\b\f\n\r\t\v"

void teststrunescape(void)
{
    char buf2[256];
    char buf[256];

    strcpy(buf, ESCAPED);
    size_t len = strunescape(buf);
    CU_ASSERT_EQUAL_FATAL(len, strlen(buf));
    CU_ASSERT_STRING_EQUAL(buf, UNESCAPED);
    strescape(buf2, buf);
    CU_ASSERT_STRING_EQUAL(buf2, ESCAPED_BACK);
}

