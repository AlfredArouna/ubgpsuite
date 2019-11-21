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

#include "../../ubgp/io.h"
#include "../test_util.h"
#include "test.h"

#include <CUnit/CUnit.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef io_rw_t *(*open_func_t)(int fd, size_t bufsiz, const char *mode, ...);

static void write_and_read(const char  *where,
                           open_func_t  open_func,
                           const char  *what)
{
    int fd = open(where, O_CREAT | O_TRUNC | O_WRONLY, 0666);
    CU_ASSERT_TRUE_FATAL(fd >= 0);

    size_t len = strlen(what);

    io_rw_t *io = open_func(fd, 0, "w");
    CU_ASSERT_PTR_NOT_NULL_FATAL(io);

    size_t n = io->write(io, what, len);
    CU_ASSERT_TRUE_FATAL(n == len);
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    int err = io->close(io);
    CU_ASSERT_TRUE_FATAL(err == 0);

    fd = open(where, O_RDONLY);
    CU_ASSERT_TRUE_FATAL(fd >= 0);

    char buf[len + 1];
    io = open_func(fd, 0, "r");
    CU_ASSERT_PTR_NOT_NULL_FATAL(io);

    n = io->read(io, buf, len);
    CU_ASSERT_TRUE_FATAL(n == len);
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    buf[len] = '\0';
    CU_ASSERT_STRING_EQUAL(buf, what);

    err = io->close(io);
    CU_ASSERT_TRUE_FATAL(err == 0);

    unlink(where);
}

#define DEFAULT_STRING "the quick brown fox jumps over the lazy dog\n"

void testzio(void)
{
    write_and_read("miao.Z", io_zopen, DEFAULT_STRING);
}

void testbz2(void)
{
    write_and_read("miao.bz2", io_bz2open, DEFAULT_STRING);
}

#ifdef UBGP_IO_XZ

void testxz(void)
{
    write_and_read("miao.xz", io_xzopen, DEFAULT_STRING);
}

#endif
#ifdef UBGP_IO_LZ4

void testlz4(void)
{
    write_and_read("miao.lz4", io_lz4open, DEFAULT_STRING);
}

#endif

void testlz4smallwrites(void)
{
    const char *filename    = "hello.txt";
    const char *test_string = "hello to everyone";

    int fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, 0666);
    CU_ASSERT_TRUE_FATAL(fd >= 0);

    io_rw_t *io = io_lz4open(fd, 0, "w*", 0);
    CU_ASSERT_PTR_NOT_NULL_FATAL(io);
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);
    char c = 'd';

    size_t n = io->write(io, &c, sizeof(c));
    CU_ASSERT_TRUE_FATAL(n == sizeof(c));
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    n = io->write(io, " ", 1);
    CU_ASSERT_TRUE_FATAL(n == 1);
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    n = io->write(io, test_string, strlen(test_string));
    CU_ASSERT_TRUE_FATAL(n == strlen(test_string));
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    int err = io->close(io);
    CU_ASSERT_TRUE_FATAL(err == 0);

    fd = open(filename, O_RDONLY);
    CU_ASSERT_TRUE_FATAL(fd >= 0);

    io = io_lz4open(fd, 0, "r");
    CU_ASSERT_PTR_NOT_NULL_FATAL(io);
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    char ci;
    char buf[strlen(test_string) + 1];

    n = io->read(io, &ci, sizeof(ci));
    CU_ASSERT_TRUE_FATAL(n == sizeof(ci));
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);
    CU_ASSERT_TRUE(ci == c);

    n = io->read(io, &ci, sizeof(ci));
    CU_ASSERT_TRUE_FATAL(n == sizeof(ci));
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);
    CU_ASSERT_TRUE(ci == ' ');

    n = io->read(io, buf, strlen(test_string));
    CU_ASSERT_TRUE_FATAL(n == strlen(test_string));
    CU_ASSERT_TRUE_FATAL(io->error(io) == 0);

    buf[strlen(test_string)] = '\0';
    CU_ASSERT_STRING_EQUAL(buf, test_string);

    err = io->close(io);
    CU_ASSERT_TRUE_FATAL(err == 0);

    unlink(filename);
}

