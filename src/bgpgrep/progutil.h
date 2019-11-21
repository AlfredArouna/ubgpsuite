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

#ifndef UBGP_PROGUTIL_H_
#define UBGP_PROGUTIL_H_

#include "../ubgp/funcattribs.h"

#include <stdarg.h>
#include <stdnoreturn.h>

/**
 * SECTION: progutil
 * @title:   Common CLI application Utility Functions
 * @include: progutil.h
 *
 * General utilities for command line tools.
 *
 * This header is guaranteed to include standard `stdarg.h` and `stdnoreturn.h`,
 * it may include other standard, POSIX or ubgpsuite-specific headers, but
 * includers should not rely on it.
 */

/**
 + programnam:
 *
 * External variable referencing the name of this program.
 *
 * This variable is initially %NULL, and is only set by setprogramnam().
 */
extern const char *programnam;

/**
 * setprogramnam:
 * @argv0: This argument should always be the main() function
 *         `argv[0]` value, it is used to retrieve the name
 *         of this program.
 *
 * Initialize the #programnam variable.
 *
 * Extracts the program name from the first command line argument
 * (`argv[0]`), saving it to #programnam.
 * Functions in this group use its value to format diagnostic messages.
 *
 * **This function may alter the contents of @argv0**
 * and #programnam shall reference a substring of this argument.
 * Hence the program should rely no more on the contents of the
 * argument string.
 */
CHECK_NONNULL(1) void setprogramnam(char *argv0);

/**
 * eprintf:
 * @fmt message format string.
 * @... formatting arguments.
 *
 * Error printf().
 *
 * Formats and reports an error to #stderr.
 * This convenience function prepends the program name to the message, and
 * automatically appends strerror() if the format string terminates
 * with ':'. This function automatically appends a newline to every message.
 */
CHECK_PRINTF(1, 2) CHECK_NONNULL(1) void eprintf(const char *fmt, ...);

/**
 * evprintf:
 *
 * eprintf() variant with explicit #va_list.
 */
CHECK_NONNULL(1) void evprintf(const char *fmt, va_list va);

/**
 * exvprintf:
 *
 * exprintf() variant with explicit #va_list.
 */
CHECK_NONNULL(2) noreturn void exvprintf(int code, const char *fmt, va_list va);

/**
 * exprintf:
 * @code: exit code
 * @fmt:  message format string
 * @...:  format message arguments
 *
 * Error printf() and exit().
 *
 * Behaves like eprintf(), once the message is printed, this function
 * exit()s with the specified error code.
 */
CHECK_PRINTF(2, 3) CHECK_NONNULL(2) noreturn void exprintf(int code,
                                                           const char *fmt,
                                                           ...);

#endif

