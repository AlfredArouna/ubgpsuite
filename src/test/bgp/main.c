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

    CU_pSuite suite = CU_add_suite("bgp", NULL, NULL);
    if (!suite)
        goto error;

    if (!CU_add_test(suite, "test for simple open packet creation", testopencreate))
        goto error;

    if (!CU_add_test(suite, "test for simple open packet read", testopenread))
        goto error;

    if (!CU_add_test(suite, "test for string to community", testcommunityconv))
        goto error;

    if (!CU_add_test(suite, "test for string to large community conversion", testlargecommunityconv))
        goto error;

    if (!CU_add_test(suite, "test for string to AS path conversion", testaspathconv))
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
