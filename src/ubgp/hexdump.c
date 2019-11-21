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

#include "branch.h"
#include "hexdump.h"

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// paranoid...
#if CHAR_BIT != 8
#error Code assumes 8-bit bytes
#endif

/**
 * hexstate_t:
 *
 * @out;      output stream
 * @actual;   data **successfully** wrote to @out in bytes
 * @wr;       data that should have been written to @out, in bytes
 * @nbytes;   consumed input bytes count
 * @col;      current column
 * @grouping: byte grouping
 * @cols:     desired column limit hint
 * @sep:      separator character
 * @format:   byte format
 * @alter:    alternative format flag
 *
 * Formatter state.
 */
typedef struct {
    FILE *out;
    size_t actual;
    size_t wr;
    size_t nbytes;
    size_t col;
    size_t grouping;
    size_t cols;
    char sep;
    char format;
    char alter;
} hexstate_t;

static int closingsep(int sep)
{
    switch (sep) {
    case '{':
        return '}';
    case '[':
        return ']';
    case '(':
        return ')';
    default:
        return sep;
    }
}

static bool isparensep(int sep)
{
    return sep == '{' || sep == '[' || sep == '(';
}

static bool ismodeformat(int c)
{
    return c == 'x' || c == 'X' || c == 'o' || c == 'O' || c == 'b' || c == 'B';
}

static bool ismodesep(int c)
{
    return c == '{' || c == '[' || c == '(' || c == '|' ||
           c == '/' || c == ',' || c == ' ';
}

static size_t formatsize(const hexstate_t *state)
{
    size_t alter_overhead = 0;
    size_t size = 0;
    switch (state->format) {
    default:
    case 'x':
    case 'X':
        alter_overhead += 2;
        size += 2;
        break;
    case 'o':
    case 'O':
        alter_overhead++;
        size += 3;
        break;
    case 'b':
    case 'B':
        alter_overhead++;
        size += 8;
        break;
    }

    if (state->grouping != SIZE_MAX)
        size *= state->grouping;

    if (state->sep == '|')
        size += 2;
    if (state->sep == ',')
        size++;

    if (state->alter == '#')
        size += alter_overhead;

    return size;
}

static void out(hexstate_t *state, int c)
{
    if (fputc(c, state->out) != EOF)
        state->actual++;

    state->wr++;
    state->col++;
    if (c == '\n')
        state->col = 0;
}

static void openparen(hexstate_t *state)
{
    if (isparensep(state->sep)) {
        if (state->col >= state->cols)
            out(state, '\n');

        out(state, state->sep);
    }
}

static void closeparen(hexstate_t *state)
{
    if (isparensep(state->sep)) {
        // newline if space + paren would exceed column limit
        out(state, (state->col + 2 > state->cols) ? '\n' : ' ');
        out(state, closingsep(state->sep));
    }
}

static void putsep(hexstate_t *state)
{
    if (state->sep == '|') {
        out(state, ' ');
        out(state, '|');
    } else if (state->sep != ' ')
        out(state, ',');
}

static void putbyte(hexstate_t *state, int byt)
{
    const char *digs = "0123456789abcdef";

    switch (state->format) {
    case 'X':
        digs = "0123456789ABCDEF";
        FALLTHROUGH;
    case 'x':
    default:
        out(state, digs[byt >> 4]);
        out(state, digs[byt & 0xf]);
        break;
    case 'O':
    case 'o':
        out(state, digs[byt >> 6]);
        out(state, digs[(byt >> 3) & 0x3]);
        out(state, digs[byt & 0x3]);
        break;
    case 'B':
    case 'b':
        for (int i = 0; i < 8; i++) {
            out(state, digs[!!(byt & 0x80)]);
            byt <<= 1;
        }
        break;
    }

    state->nbytes++;
}

static void opengroup(hexstate_t *state)
{
    // newline if space + group would exceed column limit
    if (state->col > 0)
        out(state, (state->col + formatsize(state) + 1 > state->cols) ? '\n' : ' ');

    if (state->alter != '#')
        return;

    switch (state->format) {
    default:
    case 'x':
    case 'X':
        out(state, '0');
        FALLTHROUGH;
    case 'b':
    case 'B':
        out(state, state->format);
        break;
    case 'o':
    case 'O':
        out(state, '0');
        break;
    }
}

static void dohexdump(hexstate_t *state,
                      const void *data,
                      size_t      n,
                      const char *mode,
                      va_list     vl)
{
    // parse mode string
    if (!mode)
        mode = "";

    if (ismodeformat(*mode))
        state->format = *mode++;
    if (*mode == '#')
        state->alter = *mode++;
    if (ismodesep(*mode))
        state->sep = *mode++;

    if (isdigit((uchar) *mode) || *mode == '*') {
        if (*mode == '*') {
            int len = va_arg(vl, int);
            state->grouping = MAX(len, 0);
        } else {
            state->grouping = atoll(mode);
        }

        // NOTE: this does the right thing for valid mode strings, and
        // tolerates malformed strings such as: "/*40"
        do mode++; while (isdigit((uchar) *mode));
    }
    if (state->sep != '\0' && *mode == closingsep(state->sep)) {
        mode++;
        if (*mode == '*') {
            int len = va_arg(vl, int);
            state->cols = MAX(len, 0);
        } else if (isdigit((uchar) *mode)) {
            state->cols = atoll(mode);
        }
    }

    const byte *ptr = data;
    const byte *end = ptr + n;

    // normalize arguments and apply defaults
    if (state->grouping == 0)
        state->grouping = SIZE_MAX;
    if (state->cols == 0)
        state->cols = SIZE_MAX;
    if (state->sep == '\0')
        state->sep = ' ';
    if (state->sep == '/')
        state->sep = ',';
    if (state->format == '\0')
        state->format = 'x';

    // format bytes into output
    openparen(state);

    while (ptr < end) {
        if ((state->nbytes % state->grouping) == 0) {
            if (state->nbytes > 0)
                putsep(state);

            opengroup(state);
        }

        putbyte(state, *ptr++);
    }

    closeparen(state);
}

UBGP_API size_t hexdump(FILE       *out,
                        const void *data,
                        size_t      n,
                        const char *mode,
                        ...)
{
    hexstate_t state = { .out = out };

    va_list vl;

    va_start(vl, mode);
    dohexdump(&state, data, n, mode, vl);
    va_end(vl);

    return state.actual;
}

UBGP_API size_t hexdumps(char       *dst,
                         size_t      size,
                         const void *data,
                         size_t      n,
                         const char *mode,
                         ...)
{
    char dummy;
    if (size == 0) {
        // POSIX: fmemopen() may fail with a size of 0, give it a dummy buffer
        dst  = &dummy;
        size = sizeof(dummy);
    }

    FILE *f = fmemopen(dst, size, "w");
    if (!f)
        return 0;

    setbuf(f, NULL);

    hexstate_t state = { .out = f };

    va_list vl;
    va_start(vl, mode);
    dohexdump(&state, data, n, mode, vl);
    va_end(vl);

    fclose(f);
    return state.wr + 1;  // report we need an additional byte for '\0'
}

