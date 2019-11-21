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

#ifndef UBGP_HEXDUMP_H_
#define UBGP_HEXDUMP_H_

#include "funcattribs.h"
#include "ubgpdef.h"

#include <stdio.h>

/**
 * SECTION: hexdump
 * @title:   Human Readable Binary Dump
 * @include: hexdump.h
 *
 * Utilities to dump memory chunks into human readable hex dumps.
 *
 * Functions to format byte chunks into readable strings in various
 * binary encoding formats.
 *
 * Binary formatting is controlled by a mode string which determines the desired
 * encoding, in a way similar to standard printf().
 * The general mode string format is defined as follows:
 *
 * ```[Fmt][#][Sep][Group][Sep][Cols]```
 *
 * With the following semantics:
 * * `Fmt`: The desired binary format, valid values are:
 *   * `'x'` - Hexadecimal notation.
 *   * `'X'` - Hexadecimal notation, with uppercase letters.
 *   * `'o'` - Octal notation.
 *   * `'O'` - Synonym with `o`.
 *   * `'b'` - Binary notation, prints each byte as 8 bits.
 *   * `'B'` - Binary notation, this is effectively a synonym
 *     with `b`, but the format may be altered by the `#` alternative
 *     formatting flag, see below.
 *
 *     If nothing is specified, the default format is used, which is `x`.
 *
 * * `#`: Alternative formatting flag, each byte group is prepended with a
 *   sequence explicitly indicating the encoding format specified in
 *   `Fmt`. The exact formatting output depends on the `Fmt` value
 *   itself and is defined as follows:
 *   * `'x'` - Each byte group is prepended by a `0x`, indicating
 *     their hexadecimal encoding.
 *   * `'X'` - Uppercase variant of `x`, uses a `0X` prefix for each group.
 *   * `'o'` - Each group is prepended by a 0, indicating their octal encoding.
 *   * `'O'` - Synonym with `o`.
 *   * `'b'` - Each byte group is prepended by a `b`, indicating their
 *     binary encoding.
 *   * `'B'` - Uppercase variant of `b`, uses a `B` prefix for each group.
 *
 * * `Sep`: Group separator specifier, by default, groups are not separated in
 *   any way, the resulting output is a single string in the format
 *   specified by \a Fmt. This behavior can be altered with a separator
 *   marker, the accepted separator characters are:
 *   * `' '` - Add a space between each group.
 *   * `','` - Add comma followed by a space between each group.
 *   * `'/'` - Synonym with `,` for readability only.
 *   * `'|'` - Add a space followed by a pipe between each group.
 *   * `'('` - Prepend a bracket before the whole output, and
 *     close it at the end, each group is separated by a comma
 *     as if by `','`.
 *   * `'['` - Same as `'('`, but uses a square bracket.
 *   * `'{'` - Same as `'('`, but uses a curly bracket.
 *
 *   A separator should be followed by the requested size constraints
 *   defined by `Group` and `Cols`, described below.
 *
 * * `Group`: An unsigned integer value specifying how many bytes should be
 *   grouped toghether when formatting them, when the value is 1, each
 *   byte is printed separately; when the value is 2, pair of
 *   bytes are coupled toghether for printing, and so on...
 *   By default all bytes are taken as a single group and are not
 *   separated in any way, most likely, when `Sep` is specified
 *   in mode string, this value should be as well, otherwise
 *   the `Sep` modifier has no effect.
 *   As an additional feature, the `'*'` value can be used for this
 *   field, in this case, the group length is taken from the next
 *   *integer* (that is: `int`) variable in the variable argument list.
 *
 * * `Sep`: This field introduces an additional constrain on the number of
 *   columns before a newline is issued by the formatter.
 *   The only use for this field is to separate the `Group` value from
 *   the `Cols` value, specified below.
 *   The specifier must match the first `Sep`.
 *   As an additional convenience to improve readability, when a paren
 *   is specified for the original `Sep` (either `'('`, `'['`,
 *   or `'{'`), the corresponding closing paren may be used for this
 *   field.
 *
 * * `Cols`: An unsigned integer value specifying a hint for the desired column
 *   width for the produced string. Whenever this column limit is
 *   exceeded, a newline is inserted in place of the closest following
 *   whitespace character. Please note that this is not a mandatory
 *   limit, a group of bytes is never split in two lines, even when
 *   its size would exceed this value, and a comma or pipe character
 *   is never split on a new line.
 *   Like `Group`, this field can also be specified inside the argument
 *   list by a corresponding **integer** variable (that is: `int`), if
 *   a `'*'` is specified for this field.
 *
 * `hexdump.h` is guaranteed to include standard `stdio.h`, a number of
 * other standard or ubgp-specific file may be included in the
 * interest of providing additional functionality, but such inclusion
 * should not be relied upon by the includers of this file.
 */

/**
 * HEX_C_ARRAY:
 *
 * Formatting mode to obtain a valid C array of hexadecimal bytes,
 * the array is formatted in a single line.
 *
 * See hexdump(), hexdumps().
 */
#define HEX_C_ARRAY "x#{1}"

/**
 * HEX_C_ARRAY_N:
 * @cols: integer column limit hint
 *
 * Formatting mode to obtain a valid C array of hexadecimal bytes,
 * an attempt is made to keep the array within a specific column limit.
 *
 * **This macro should only be used as `mode` argument of hexdump() or
 *   hexdumps().**
 */
#define HEX_C_ARRAY_N(cols) "x#{1}*", (int) (cols)

/**
 * OCT_C_ARRAY:
 *
 * Formatting mode to obtain a valid C array of octal bytes,
 * the array is formatted in a single line.
 *
 * See hexdump(), hexdumps().
 */
#define OCT_C_ARRAY "o#{1}"

/**
 * OCT_C_ARRAY_N:
 * @cols: integer column limit hint
 *
 * Formatting mode to obtain a valid C array of octal bytes,
 * an attempt is made to keep the array within a specific column limit.
 *
 * **This macro should only be used as a `mode` argument of hexdump() or
 *   hexdumps().**
 */
#define OCT_C_ARRAY_N(cols) "o#{1}*", (int) (cols)

/**
 * HEX_PLAIN:
 *
 * Formatting mode for a comma-separated string of hexadecimal bytes
 * prepended by the `"0x"` prefix.
 *
 * See hexdump(), hexdumps().
 */
#define HEX_PLAIN "x#/1"

/**
 * OCT_PLAIN:
 *
 * Formatting mode for a comma-separated string of octal bytes.
 *
 * See hexdump(), hexdumps().
 */
#define OCT_PLAIN "o/1"

/**
 * BINARY_PLAIN:
 *
 * Formatting mode for a comma-separated string of bytes encoded into
 * binary digits.
 *
 * See hexdump(), hexdumps().
 */
#define BINARY_PLAIN "b/1"

/**
 * hexdump:
 * @out: #FILE handle where the formatted string has to be
 * written, must not be %NULL.
 * @data: (nullable): data chunk that has to be dumped, must reference at least
 * @n bytes, may be %NULL if @n is 0.
 * @n: number of bytes to be formatted to @out.
 * @mode: (nullable): mode string determining output format, may be %NULL to
 * use default formatting (equivalent to empty string).
 * @...: additional @mode arguments, see `Group` and `Cols` documentation of
 * module's mode strings.
 *
 * Hexadecimal dump to file stream.
 *
 * Formats a data chunk to a standard #FILE in a human readable binary dump,
 * as specified by a mode string.
 *
 * Returns: number of bytes successfully written to @out, which shall not
 * exceed @n and may only be less than @n in presence of an I/O error.
 */
UBGP_API CHECK_NONNULL(1) size_t hexdump(FILE       *out,
                                         const void *data,
                                         size_t      n,
                                         const char *mode,
                                         ...);

/**
 * hexdumps:
 * @dst: (nullable): destination string where the formatted string has to be
 * written, may be %NULL only if @size is 0.
 * @size: maximum number of characters that may be written to @dst, this
 * function shall never attempt to write more than @size characters to @dst,
 * this quantity includes the terminating `NUL` (`'\0'`) character.
 * The resulting string is always `NUL`-terminated, even when truncation
 * occurred, except when @size is 0.
 * @data: (nullable): data chunk that has to be dumped, must reference at least
 * @n bytes, may be %NULL if ```n == 0```.
 * @n: number of bytes in @data to be formatted into @dst.
 * @mode: (nullable): mode string determining output format, may be %NULL to
 * use default formatting (equivalent to empty string).
 * @...: additional @mode arguments, see `Group` and `Cols` documentation of
 * module's mode strings.
 *
 * Hexadecimal dump to char string.
 *
 * Formats a data chunk directly into a string as a human readable binary dump,
 * following the rules specified by a mode string.
 *
 * The caller may be able to determine the appropriate size for the
 * destination buffer by calling this function with a %NULL destination
 * and zero @size in advance, using the returned value to allocate a
 * large enough buffer. However, when trivial formatting is enough,
 * it is possible to determine such size a-priori, for example a buffer
 * of size 2 * @n + 1 is always large enough for a contiguous
 * hexadecimal dump with no hex prefix. Similar considerations can be done
 * for simple space, comma or pipe separated dumps (for example
 * allocating a possible upperbound of 4 * @n + 1 `char` buffer).
 * Obvious adjustments of these calculations have to be performed for
 * binary and octal dumps, and when prefix or grouping is requested.
 * Still, the afore mentioned approach is valuable for non-trivial
 * formatting and has the obvious advantage of not requiring careful
 * thinking on caller's behalf.
 *
 * Returns: number of characters necessary to hold the entire dump, including
 * the terminating `NUL` (`'\0'`) character. If the returned value is
 * greater than @size, then a truncation occurred, the caller may
 * then allocate a buffer of the appropriate size and call this
 * function again to obtain the entire dump.
 * A return value of 0 indicates an out of memory error.
 */
UBGP_API size_t hexdumps(char       *dst,
                         size_t      size,
                         const void *data,
                         size_t      n,
                         const char *mode,
                         ...);

#endif

