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

#ifndef UBGP_STRUTILS_H_
#define UBGP_STRUTILS_H_

#include "funcattribs.h"
#include "ubgpdef.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/**
 * SECTION: strutil
 * @title:   String Utility Functions
 * @include: strutil.h
 *
 * String utility functions.
 *
 * `strutil.h` is guaranteed to include `stdarg.h`, `string.h` and `stdbool.h`,
 * it may include additional standard or ubgp-specific headers in the
 * interest of providing inline implementations of its functions.
 */

static inline CHECK_NONNULL(1) ulong djb2(const char *s)
{
    ulong h = 5381;
    int c;

    const byte *ptr = (const byte *) s;
    while ((c = *ptr++) != '\0')
        h = ((h << 5) + h) + c;  // hash * 33 + c

    return h;
}

static inline CHECK_NONNULL(1) ulong memdjb2(const void *p, size_t n)
{
    ulong h = 5381;

    const byte *ptr = (const byte *) p;
    for (size_t i = 0; i < n; i++)
        h = ((h << 5) + h) + *ptr++;  // hash * 33 + c

    return h;
}

static inline CHECK_NONNULL(1) ulong sdbm(const char *s)
{
    ulong h = 0;
    int c;

    const byte *ptr = (const byte *) s;
    while ((c = *ptr++) != '\0')
        h = c + (h << 6) + (h << 16) - h;

    return h;
}

static inline CHECK_NONNULL(1) ulong memsdbm(const void *p, size_t n)
{
    ulong h = 0;

    const byte *ptr = (const byte *) p;
    for (size_t i = 0; i < n; i++)
        h = *ptr++ + (h << 6) + (h << 16) - h;

    return h;
}

static inline CHECK_NONNULL(1) char *xtoa(char *dst, char **endp, uint val)
{
    char buf[sizeof(val) * 2 + 1];

    char *ptr = buf + sizeof(buf);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789abcdef"[val & 0xf];
        val >>= 4;
    } while (val > 0);

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *itoa(char *dst, char **endp, int i)
{
    char buf[1 + digsof(i) + 1];

    char *ptr = buf + sizeof(buf);

    int val = abs(i);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[val % 10];
        val /= 10;
    } while (val > 0);

    if (i < 0)
        *--ptr = '-';

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *utoa(char *dst, char **endp, uint u)
{
    char buf[digsof(u) + 1];

    char *ptr = buf + sizeof(buf);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[u % 10];
        u /= 10;
    } while (u > 0);

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *ltoa(char *dst, char **endp, long l)
{
    char buf[1 + digsof(l) + 1];

    char *ptr = buf + sizeof(buf);

    long val = labs(l);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[val % 10];
        val /= 10;
    } while (val > 0);

    if (l < 0)
        *--ptr = '-';

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *ultoa(char *dst, char **endp, ulong u)
{
    char buf[digsof(u) + 1];

    char *ptr = buf + sizeof(buf);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[u % 10];
        u /= 10;
    } while (u > 0);

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *lltoa(char *dst, char **endp, llong ll)
{
    char buf[1 + digsof(ll) + 1];

    char *ptr = buf + sizeof(buf);

    llong val = llabs(ll);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[val % 10];
        val /= 10;
    } while (val > 0);

    if (ll < 0)
        *--ptr = '-';

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

static inline CHECK_NONNULL(1) char *ulltoa(char *dst, char **endp, ullong u)
{
    char buf[digsof(u) + 1];

    char *ptr = buf + sizeof(buf);

    *--ptr = '\0';

    char *end = ptr;
    do {
        *--ptr = "0123456789"[u % 10];
        u /= 10;
    } while (u > 0);

    size_t n = end - ptr;
    if (endp)
        *endp = &dst[n];

    return memcpy(dst, ptr, n + 1);
}

/**
 * splitstr:
 * @s: string to be splitted into substrings.
 * @delim: (nullable): delimiter string for each substring.
 * @pn: (nullable) (out): pointer to an unsigned variable where the length of
 * the returned buffer is to be stored, the terminating
 * %NULL pointer stored in the buffer is not accounted for in the returned
 * value. This argument may be %NULL if such information is unimportant.
 *
 * Split string against a delimiter.
 *
 * Returns a buffer of strings splitted against a delimiter string.
 * When such buffer is no longer useful, it must be free()d.
 *
 * Returns: a dynamically allocated and %NULL-terminated buffer of substrings
 * obtained by repeatedly splitting @str against the delimiter string
 * @delim. The returned buffer must be free()d by the caller.
 * The caller must only free() the returned pointer, each string
 * in the buffer share the same memory and is consequently free()d by
 * such call.
 */
UBGP_API MALLOCFUNC WARN_UNUSED CHECK_NONNULL(1)
char **splitstr(const char *s,
                const char *delim,
                size_t     *pn);

/**
 * joinstr:
 *
 * Join a string buffer on a delimiter.
 *
 * Returns: a dynamically allocated string that must be free()d by the caller.
 */
UBGP_API MALLOCFUNC WARN_UNUSED char *joinstr(const char  *delim,
                                              char       **strings,
                                              size_t       n);

/**
 * joinstrvl:
 *
 * Join a #va_list of strings on a delimiter.
 *
 * Returns: a dynamically allocated string that must be free()d by the caller.
 */
UBGP_API MALLOCFUNC WARN_UNUSED char *joinstrvl(const char *delim, va_list va);

/**
 * joinstrv:
 * @delim: (nullable): delimiter, added between each string, specifying %NULL
 * is equivalent to `""`
 * @...: variable number of `const char` pointers, this list
 * **must** be terminated by a %NULL #char pointer, as in: ```(char *) NULL```
 *
 * Join a variable list of strings on a delimiter.
 *
 * This function expects a variable number of `const char` pointers
 * and joins them on a delimiter.
 *
 * Returns: a dynamically allocated string that must be free()d by the caller.
 */
UBGP_API MALLOCFUNC WARN_UNUSED SENTINEL(0) char *joinstrv(const char *delim, ...);

/**
 * trimwhites:
 * @s: a string
 *
 * Trim leading and trailing whitespaces from string, operates in-place.
 *
 * Returns: trimmed string.
 */
UBGP_API RETURNS_NONNULL CHECK_NONNULL(1) char *trimwhites(char *s);

/**
 * strpathext:
 * @name: a filename
 *
 * Extract file extension, operates in-place.
 *
 * Returns: file extension, including leading ``'.'``, %NULL if no extension
 * is found.
 */
UBGP_API CHECK_NONNULL(1) char *strpathext(const char *name);

/**
 * strunescape:
 * @s: a string.
 *
 * Remove escape characters from @s, works in place.
 *
 * Returns: resulting string length.
 */
UBGP_API CHECK_NONNULL(1) size_t strunescape(char *s);

/**
 * strescape:
 *
 * Add escape sequences to a string, writes to destination buffer at most
 * ```2 * strlen() + 1``` bytes.
 */
UBGP_API CHECK_NONNULL(1, 2) size_t strescape(char *restrict dst, const char *restrict src);

/**
 * startswith:
 *
 * Check whether a string starts with a specific prefix.
 */
static inline CHECK_NONNULL(1, 2) bool startswith(const char *s,
                                                  const char *prefix)
{
    size_t slen = strlen(s);
    size_t preflen = strlen(prefix);
    return (slen >= preflen) && memcmp(s, prefix, preflen) == 0;
}

/**
 * endswith:
 *
 * Check whether a string ends with a specific suffix.
 */
static inline CHECK_NONNULL(1, 2) bool endswith(const char *s,
                                                const char *suffix)
{
    size_t slen = strlen(s);
    size_t suflen = strlen(suffix);
    return (slen >= suflen) && memcmp(s + slen - suflen, suffix, suflen) == 0;
}

/**
 * strupper:
 *
 * Make string uppercase, operates in-place.
 */
static inline RETURNS_NONNULL CHECK_NONNULL(1) char *strupper(char *s)
{
    char c;

    char *ptr = s;
    while ((c = *ptr) != '\0')
        *ptr++ = toupper((uchar) c);

    return s;
}

/**
 * strlower:
 *
 * Make string lowercase, operates in-place.
 */
static inline RETURNS_NONNULL CHECK_NONNULL(1) char *strlower(char *s)
{
    char c;

    char *ptr = s;
    while ((c = *ptr) != '\0')
        *ptr++ = tolower((uchar) c);

    return s;
}

#endif

