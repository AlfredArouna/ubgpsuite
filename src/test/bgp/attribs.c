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

#include "../../ubgp/bgpattribs.h"
#include "../../ubgp/ubgpdef.h"
#include "test.h"

#include <CUnit/CUnit.h>

#include <errno.h>
#include <string.h>

void testcommunityconv(void)
{
    static const struct {
        const char *str;
        community_t expect;
    } comm2str[] = {
        {"PLANNED_SHUT",               COMMUNITY_PLANNED_SHUT},
        {"ROUTE_FILTER_TRANSLATED_V6", COMMUNITY_ROUTE_FILTER_TRANSLATED_V6},
        {"ROUTE_FILTER_TRANSLATED_V4", COMMUNITY_ROUTE_FILTER_TRANSLATED_V4},
        {"ROUTE_FILTER_V6",            COMMUNITY_ROUTE_FILTER_V6},
        {"ROUTE_FILTER_V4",            COMMUNITY_ROUTE_FILTER_V4},
        {"LLGR_STALE",                 COMMUNITY_LLGR_STALE},
        {"ACCEPT_OWN",                 COMMUNITY_ACCEPT_OWN},
        {"NO_LLGR",                    COMMUNITY_NO_LLGR},
        {"BLACKHOLE",                  COMMUNITY_BLACKHOLE},
        {"NO_EXPORT_SUBCONFED",        COMMUNITY_NO_EXPORT_SUBCONFED},
        {"NO_EXPORT",                  COMMUNITY_NO_EXPORT},
        {"NO_ADVERTISE",               COMMUNITY_NO_ADVERTISE},
        {"ACCEPT_OWN_NEXTHOP",         COMMUNITY_ACCEPT_OWN_NEXTHOP},
        {"NO_PEER",                    COMMUNITY_NO_PEER},

        {"0",          0},
        {"4294967295", UINT32_MAX},
        {"12345",      12345}
    };

    errno = 0;

    char *eptr;
    for (size_t i = 0; i < countof(comm2str); i++) {
        community_t c = stocommunity(comm2str[i].str, &eptr);
        CU_ASSERT_EQUAL(errno, 0);
        CU_ASSERT_EQUAL(*eptr, '\0');
        CU_ASSERT_EQUAL(c, comm2str[i].expect);
        CU_ASSERT_STRING_EQUAL(communitytos(c, COMMSTR_EX), comm2str[i].str);
    }
}

void testlargecommunityconv(void)
{
    static const struct {
        const char *str;
        large_community_t expect;
    } largecomm2str[] = {
        {"0:0:0", {0, 0, 0}},
        {
            "4294967295:4294967295:4294967295",
            {UINT32_MAX, UINT32_MAX, UINT32_MAX}
        },
        {"123:456:789", {123, 456, 789}}
    };

    char *eptr;
    for (size_t i = 0; i < countof(largecomm2str); i++) {
        large_community_t c = stolargecommunity(largecomm2str[i].str, &eptr);

        CU_ASSERT_EQUAL(*eptr, '\0');
        CU_ASSERT(memcmp(&c, &largecomm2str[i].expect, sizeof(c)) == 0);
        CU_ASSERT_STRING_EQUAL(largecommunitytos(c), largecomm2str[i].str);
    }
}

void testaspathconv(void)
{

/* FIXME
    const char *s = "10 20 30 40 50 {10 20 30} {20, 10, 0} 50 1 2";
    char *eptr;

    unsigned char buf[200];
    size_t n = stoaspath16(buf, sizeof(buf), 0, s, &eptr);
    printf("would need: %zd, now got: %zd\n", n, bgpattrhdrsize(buf) + bgpattrlen(buf));
    printf("eptr = \"%s\", errno: %s", eptr, strerror(errno));
    CU_ASSERT(n <= sizeof(buf));
    CU_ASSERT(*eptr == '\0');
*/
}

