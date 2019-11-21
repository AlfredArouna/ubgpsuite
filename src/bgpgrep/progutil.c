/* Copyright (C) 2019 Alpha Cogs S.R.L.
 *
 * bgpgrep is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bgpgrep is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bgpgrep.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This work is based upon work authored by the Institute of Informatics
 * and Telematics of the Italian National Research Council (IIT-CNR) licensed
 * under the BSD 3-Clause license. See AKNOWLEDGEMENT and AUTHORS for more
 * details.
 */

#include "progutil.h"

#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *programnam = NULL;

void setprogramnam(char *argv0)
{
    programnam = basename(argv0);
}

void evprintf(const char *fmt, va_list va)
{
    if (programnam) {
        fputs(programnam, stderr);
        fputs(": ", stderr);
    }

    vfprintf(stderr, fmt, va);
    if (fmt[strlen(fmt) - 1] == ':') {
        fputc(' ', stderr);
        fputs(strerror(errno), stderr);
    }

    fputc('\n', stderr);
}

void eprintf(const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    evprintf(fmt, va);
    va_end(va);
}

void exvprintf(int code, const char *fmt, va_list va)
{
    evprintf(fmt, va);
    exit(code);
}

void exprintf(int code, const char *fmt, ...)
{
    va_list va;

    va_start(va, fmt);
    exvprintf(code, fmt, va);
    va_end(va);
}

