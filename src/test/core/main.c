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

#include "../../ubgp/ubgpdef.h"
#include "test.h"

#include <CUnit/Basic.h>

#include <stdlib.h>

int main(void)
{
    if (CU_initialize_registry() != CUE_SUCCESS)
        return CU_get_error();

    CU_pSuite suite = CU_add_suite("core", NULL, NULL);
    if (!suite)
        goto error;

    if (!CU_add_test(suite, "simple u128 iteration", testu128iter))
        goto error;

    if (!CU_add_test(suite, "u128 to string and string to u128 conversion", testu128conv))
        goto error;

    if (!CU_add_test(suite, "simple hexdump", testhexdump))
        goto error;

    if (!CU_add_test(suite, "test for splitstr() and joinstr()", testsplitjoinstr))
        goto error;

    if (!CU_add_test(suite, "test for joinstrv()", testjoinstrv))
        goto error;

    if (!CU_add_test(suite, "test for trimwhites()", testtrimwhites))
        goto error;

    if (!CU_add_test(suite, "test for strunescape()", teststrunescape))
        goto error;

    if (!CU_add_test(suite, "test netaddr", testnetaddr))
        goto error;

    if (!CU_add_test(suite, "test testprefixeqwithmask", testprefixeqwithmask))
        goto error;

    if (!CU_add_test(suite, "test patricia base", testpatbase))
        goto error;

    if (!CU_add_test(suite, "test patricia get functions", testpatgetfuncs))
        goto error;

    if (!CU_add_test(suite, "test patricia check functions", testpatcheckfuncs))
        goto error;

    if (!CU_add_test(suite, "test patricia iterator", testpatiterator))
        goto error;

    if (!CU_add_test(suite, "test patricia coverage", testpatcoverage))
        goto error;

    if (!CU_add_test(suite, "test patricia get first subnets", testpatgetfirstsubnets))
        goto error;
    
    if (!CU_add_test(suite, "test patricia problem", testpatproblem))
        goto error;

    if (!CU_add_test(suite, "test abstract I/O with Zlib", testzio))
        goto error;

    if (!CU_add_test(suite, "test abstract I/O with bz2", testbz2))
        goto error;

#ifdef UBGP_IO_XZ

    if (!CU_add_test(suite, "test abstract I/O with LZMA", testxz))
        goto error;

#endif
#ifdef UBGP_IO_LZ4

    if (!CU_add_test(suite, "test abstract I/O with LZ4", testlz4))
        goto error;

#endif

    if (!CU_add_test(suite, "test abstract I/O with LZ4 by performing small writes and reads", testlz4smallwrites))
        goto error;

    if (!CU_add_test(suite, "test bgp dump packet row", testbgpdumppacketrow))
        goto error;

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    uint num_failures = CU_get_number_of_failures();
    CU_cleanup_registry();
    return (num_failures == 0) ? EXIT_SUCCESS : EXIT_FAILURE;

error:
    CU_cleanup_registry();
    return CU_get_error();
}
