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

#include <locale.h>
#include <stdlib.h>

#include "bench.h"

int main(void)
{
    setlocale(LC_ALL, "");

    if (cbench_initialize() != CB_SUCCESS)
        return EXIT_FAILURE;

    cbench_suite_t *suite = cbench_add_suite("core");
    if (!suite)
        goto out;

    if (!cbench_add_bench(suite, "bcommsprintf", bcommsprintf, NULL))
        goto out;

    if (!cbench_add_bench(suite, "bsprintf", bsprintf, NULL))
        goto out;

    if (!cbench_add_bench(suite, "bcommulltoa", bcommulltoa, NULL))
        goto out;

    if (!cbench_add_bench(suite, "ulltoa", bulltoa, NULL))
        goto out;

    if (!cbench_add_bench(suite, "splitstr", bsplit, NULL))
        goto out;

    if (!cbench_add_bench(suite, "joinstrv", bjoinv, NULL))
        goto out;

    if (!cbench_add_bench(suite, "joinstr", bjoin, NULL))
        goto out;

    if (!cbench_add_bench(suite, "patinsert", bpatinsert, NULL))
        goto out;
    
    if (!cbench_add_bench(suite, "bprefixeqwithmask", bprefixeqwithmask, NULL))
        goto out;
    
    if (!cbench_add_bench(suite, "bppathcompwithmask", bppathcompwithmask, NULL))
        goto out;

    cbench_run();

out:
    cbench_cleanup();
    return cbench_get_error();
}

