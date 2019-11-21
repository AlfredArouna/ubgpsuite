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

#include "../../ubgp/netaddr.h"
#include "../../ubgp/ubgpdef.h"
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>

void testnetaddr(void)
{
    struct {
        const char* ip;
        const char* cidr;
        ushort bitlen;
        short family;
    } table[] = {
        {"127.0.0.1", "127.0.0.1/32", 32, AF_INET},
        {"8.2.0.0", "8.2.0.0/16", 16, AF_INET},
        {"::", "::/0", 0, AF_INET6},
        {"2a00:1450:4002:800::2002", "2a00:1450:4002:800::2002/127", 127, AF_INET6},
        {"2a00:1450:4002:800::2003", "2a00:1450:4002:800::2003/128", 128, AF_INET6},
        {"2001:67c:1b08:3:1::1", "2001:67c:1b08:3:1::1/128", 128, AF_INET6}
    };

    netaddr_t prefix;
    int res;

    for (size_t i = 0; i < countof(table); i++) {
        res = stonaddr(&prefix, table[i].cidr);
        CU_ASSERT_EQUAL(res, 0);
        CU_ASSERT_EQUAL(prefix.family, table[i].family);
        CU_ASSERT_EQUAL(prefix.bitlen, table[i].bitlen);
        CU_ASSERT_STRING_EQUAL(naddrtos(&prefix, NADDR_CIDR), table[i].cidr);
        CU_ASSERT_STRING_EQUAL(naddrtos(&prefix, NADDR_PLAIN), table[i].ip);

        netaddr_t cloned;
        makenaddr(&cloned, table[i].family, &prefix.sin, prefix.bitlen);

        if ((table[i].bitlen == 32 && table[i].family == AF_INET) || (table[i].bitlen == 128 && table[i].family == AF_INET6)) {
            res = stonaddr(&prefix, table[i].ip); // NB
            CU_ASSERT_EQUAL(res, 0);
            CU_ASSERT_EQUAL(prefix.family, table[i].family);
            CU_ASSERT_EQUAL(prefix.bitlen, table[i].bitlen);
            CU_ASSERT_STRING_EQUAL(naddrtos(&prefix, NADDR_CIDR), table[i].cidr);
            CU_ASSERT_STRING_EQUAL(naddrtos(&prefix, NADDR_PLAIN), table[i].ip);
        }

        CU_ASSERT_EQUAL(cloned.family, table[i].family);
        CU_ASSERT_EQUAL(cloned.bitlen, table[i].bitlen);
        CU_ASSERT_STRING_EQUAL(naddrtos(&cloned, NADDR_CIDR), table[i].cidr);
        CU_ASSERT_STRING_EQUAL(naddrtos(&cloned, NADDR_PLAIN), table[i].ip);
    }
}

void testprefixeqwithmask(void)
{
    netaddr_t p;
    stonaddr(&p, "2a00::");
    
    netaddr_t q;
    stonaddr(&q, "8a00::");
    
    netaddr_t r;
    stonaddr(&r, "8a00::1");
    
    for (uint i = 0; i <= 128; i++) {
        CU_ASSERT_EX(prefixeqwithmask(&p, &p, i) == 1, "with mask %d", i);
        CU_ASSERT(prefixeqwithmask(&p, &q, i) == 0 || i == 0);
        CU_ASSERT(prefixeqwithmask(&p, &r, i) == 0 || i == 0 || i == 128);
    }
}
