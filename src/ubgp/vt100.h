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

#ifndef UBGP_VT100_H_
#define UBGP_VT100_H_

#include "funcattribs.h"

#include <stdbool.h>

/**
 * Section:  vt100
 * @title    VT100 Macros for ANSI VT100 consoles
 * @include: vt100.h
 *
 * ANSI VT100 compliant console escape codes.
 *
 * Escape sequences that trigger various advanced VT100 console features.
 */

#define VTBLC "\x1b(0\x6d\x1b(B"  // ANSI VT100 bottom left corner
#define VTBRC "\x1b(0\x6a\x1b(B"  // ANSI VT100 bottom left corner
#define VTTLC "\x1b(0\x6c\x1b(B"  // ANSI VT100 top left corner
#define VTTRC "\x1b(0\x6b\x1b(B"  // ANSI VT100 top right corner

#define VTVLN "\x1b(0\x78\x1b(B"  // ANSI VT100 vertical line
#define VTHLN "\x1b(0\x71\x1b(B"  // ANSI VT100 horizontal line

#define VTBLD "\x1b[1m"           // ANSI VT100 bold
#define VTLIN "\x1b[2m"           // ANSI VT100 low intensity
#define VTITL "\x1b[3m"           // ANSI VT100 italics
#define VTRST "\x1b[0m"           // ANSI VT100 reset

#define VTRED "\x1b[31m"          // ANSI VT100 foreground to red
#define VTGRN "\x1b[32m"          // ANSI VT100 foreground to green
#define VTYLW "\x1b[33m"          // ANSI VT100 foreground to yellow
#define VTBLU "\x1b[34m"          // ANSI VT100 foreground to blue
#define VTMGN "\x1b[35m"          // ANSI VT100 foreground to magenta
#define VTCYN "\x1b[36m"          // ANSI VT100 foreground to cyan
#define VTWHT "\x1b[37m"          // ANSI VT100 foreground to white

#define VTREDB "\x1b[41m"          // ANSI VT100 background to red
#define VTGRNB "\x1b[42m"          // ANSI VT100 background to green
#define VTYLWB "\x1b[43m"          // ANSI VT100 background to yellow
#define VTBLUB "\x1b[44m"          // ANSI VT100 background to blue
#define VTMGNB "\x1b[45m"          // ANSI VT100 background to magenta
#define VTCYNB "\x1b[46m"          // ANSI VT100 background to cyan
#define VTWHTB "\x1b[47m"          // ANSI VT100 background to white

/**
 * isvt100tty:
 * @fd: a file descriptor.
 *
 * Check whether a Unix file descriptor belongs to a VT100 enabled TTY.
 *
 * Returns: %true if `fd` refers to a VT100 enabled console, %false otherwise.
 */
UBGP_API bool isvt100tty(int fd);

#endif
