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
#include "ubgpdef.h"
#include "vt100.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

UBGP_API bool isvt100tty(int fd)
{
    static const char *const known_terms[] = {
      "xterm",         "xterm-color",     "xterm-256color",
      "screen",        "screen-256color", "tmux",
      "tmux-256color", "rxvt-unicode",    "rxvt-unicode-256color",
      "linux",         "cygwin"
    };

    const char *term = getenv("TERM");
    if (unlikely(!term))
        return false;

    size_t i;
    for (i = 0; i < countof(known_terms); i++) {
        if (strcmp(term, known_terms[i]) == 0)
            break;
    }

    return i != countof(known_terms) && isatty(fd);
}

