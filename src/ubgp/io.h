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

#ifndef UBGP_IO_H_
#define UBGP_IO_H_

#include "funcattribs.h"
#include "ubgpdef.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct io_rw io_rw_t;

struct io_rw {
    size_t (*read) (io_rw_t *io, void *dst, size_t n);
    size_t (*write)(io_rw_t *io, const void *buf, size_t n);
    int    (*error)(io_rw_t *io);
    int    (*close)(io_rw_t *io);

    /*< private >*/
    union {
        // Unix file descriptor
        struct {
            int fd;
            int err;
        } un;

        // Direct memory I/O
        struct {
            uint  flags;
            byte *ptr, *end;
        } mem;

        // stdio.h standard FILE
        FILE *file;

        // User-defined data (generic)
        void *ptr;

        max_align_t padding;
    };
};

// memory I/O abstraction

#define IO_MEM_WRBIT  (1 << 0)
#define IO_MEM_ERRBIT (1 << 1)

UBGP_API CHECK_NONNULL(1) size_t io_mread(io_rw_t *io, void *dst, size_t n);

UBGP_API CHECK_NONNULL(1) size_t io_mwrite(io_rw_t    *io,
                                           const void *src,
                                           size_t      n);

UBGP_API CHECK_NONNULL(1) int io_merror(io_rw_t *io);

UBGP_API CHECK_NONNULL(1) int io_mclose(io_rw_t *io);

#define IO_MEM_WRINIT(dst, size) {                           \
    .mem = {                                                 \
        .flags = IO_MEM_WRBIT,                               \
        .ptr = ((byte *) (dst)),          /* writable bit */ \
        .end = ((byte *) (dst)) + (size)                     \
    },                                                       \
    .read  = io_mread,                                       \
    .write = io_mwrite,                                      \
    .error = io_merror,                                      \
    .close = io_mclose                                       \
}

#define IO_MEM_RDINIT(src, size) {       \
    .mem = {                             \
        .flags = 0,                      \
        .ptr = (byte *) (src),           \
        .end = ((byte *) (src)) + (size) \
    },                                   \
    .read  = io_mread,                   \
    .write = io_mwrite,                  \
    .error = io_merror,                  \
    .close = io_mclose                   \
}

static inline CHECK_NONNULL(1, 2) void io_mem_wrinit(io_rw_t *io,
                                                     void    *dst,
                                                     size_t   size)
{
    io->mem.ptr = (byte *) dst;
    io->mem.end = (byte *) dst + size;
    io->read  = io_mread;
    io->write = io_mwrite;
    io->error = io_merror;
    io->close = io_mclose;
}

static inline CHECK_NONNULL(1, 2) void io_mem_rdinit(io_rw_t    *io,
                                                     const void *src,
                                                     size_t      size)
{
    io->mem.ptr = (byte *) src;
    io->mem.end = (byte *) src + size;
    io->read  = io_mread;
    io->write = io_mwrite;
    io->error = io_merror;
    io->close = io_mclose;
}

// stdio FILE abstaction

UBGP_API CHECK_NONNULL(1) size_t io_fread(io_rw_t *io, void *dst, size_t n);

UBGP_API CHECK_NONNULL(1) size_t io_fwrite(io_rw_t    *io,
                                           const void *src,
                                           size_t      n);

UBGP_API CHECK_NONNULL(1) int io_ferror(io_rw_t *io);

UBGP_API CHECK_NONNULL(1) int io_fclose(io_rw_t *io);

// convenience initialization for stdio FILE
static inline CHECK_NONNULL(1, 2) void io_file_init(io_rw_t *io, FILE *file)
{
    io->file  = file;
    io->read  = io_fread;
    io->write = io_fwrite;
    io->error = io_ferror;
    io->close = io_fclose;
}

#define IO_FILE_INIT(file) { \
    .file  = (file),         \
    .read  = io_fread,       \
    .write = io_fwrite,      \
    .error = io_ferror,      \
    .close = io_fclose       \
}

// POSIX fd abstraction

UBGP_API CHECK_NONNULL(1) size_t io_fdread(io_rw_t *io, void *dst, size_t n);

UBGP_API CHECK_NONNULL(1) size_t io_fdwrite(io_rw_t    *io,
                                            const void *src,
                                            size_t      n);

UBGP_API CHECK_NONNULL(1) int io_fderror(io_rw_t *io);

UBGP_API CHECK_NONNULL(1) int io_fdclose(io_rw_t *io);

// convenience initialization for POSIX fd
static inline CHECK_NONNULL(1) void io_fd_init(io_rw_t *io, int fd)
{
    io->un.fd  = fd;
    io->un.err = 0;
    io->read   = io_fdread;
    io->write  = io_fdwrite;
    io->error  = io_fderror;
    io->close  = io_fdclose;
}

#define IO_FD_INIT(fd) {     \
    .un    = { .fd = (fd) }, \
    .read  = io_fdread,      \
    .write = io_fdwrite,     \
    .error = io_fderror,     \
    .close = io_fdclose      \
}

// compressed I/O (io_rw_t * structures are malloc()ed, but free()ed by their close() function)

UBGP_API MALLOCFUNC WARN_UNUSED CHECK_NONNULL(3)
io_rw_t *io_zopen(int fd, size_t bufsiz, const char *mode, ...);

UBGP_API MALLOCFUNC WARN_UNUSED CHECK_NONNULL(3)
io_rw_t *io_bz2open(int fd, size_t bufsiz, const char *mode, ...);

#ifdef UBGP_IO_LZ4

UBGP_API MALLOCFUNC WARN_UNUSED CHECK_NONNULL(3)
io_rw_t *io_lz4open(int fd, size_t bufsiz, const char *mode, ...);

#endif

#ifdef UBGP_IO_XZ

UBGP_API MALLOCFUNC WARN_UNUSED CHECK_NONNULL(3)
io_rw_t *io_xzopen(int fd, size_t bufsiz, const char *mode, ...);

#endif

#endif
