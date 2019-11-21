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

#ifndef UBGP_TEST_UTIL_H_
#define UBGP_TEST_UTIL_H_

#include <CUnit/CUnit.h>
#include <stdio.h>
#include <string.h>

#define CU_ASSERT_VERBOSE(cond, line, file, func, fatal, fmt, ...) \
    do { \
        char buf__[4096]; \
        int off__ = snprintf(buf__, sizeof(buf__), "%s: ", #cond); \
        snprintf(buf__ + off__, sizeof(buf__) - off__, fmt, ## __VA_ARGS__); \
        CU_assertImplementation((cond), line, buf__, file, func, fatal); \
    } while (0)


#define CU_ASSERT_EX(cond, fmt, ...) \
    CU_ASSERT_VERBOSE(cond, __LINE__, __FILE__, __func__, 0, fmt, ## __VA_ARGS__)


#define CU_ASSERT_STRING_EQUAL_EX(r, s, fmt, ...) \
    CU_ASSERT_VERBOSE(strcmp(r, s) == 0, __LINE__, __FILE__, __func__, 0, fmt, ## __VA_ARGS__)

#endif
