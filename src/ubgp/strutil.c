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
#include "strutil.h"

#include <stdbool.h>
#include <stdlib.h>

static size_t substrcnt(const char *s, const char *substr, size_t *pn)
{
    if (unlikely(!substr))
        substr = "";

    size_t n = 0;
    size_t count = 0;

    const char *ptr = s;
    size_t step = strlen(substr);
    while (*ptr != '\0') {
        const char *tok = strstr(ptr, substr);
        if (!tok) {
            n += strlen(ptr);
            break;
        }

        count++;

        n += tok - ptr;
        ptr = tok + step;
    }

    if (likely(pn))
        *pn = n;

    return count;
}

UBGP_API char **splitstr(const char *s, const char *delim, size_t *pn)
{
    if (unlikely(!delim))
        delim = "";

    size_t size;
    size_t n = substrcnt(s, delim, &size);

    char **res = malloc((n + 2) * sizeof(*res) + size + n + 1);
    if (unlikely(!res))
        return NULL;

    char *buf = (char *) (res + n + 2);
    const char *ptr = s;
    size_t step = strlen(delim);

    size_t i = 0;
    while (*ptr != '\0') {
        const char *tok = strstr(ptr, delim);
        size_t len = (tok) ? (size_t) (tok - ptr) : strlen(ptr);

        memcpy(buf, ptr, len);

        res[i++] = buf;
        buf += len;

        *buf++ = '\0';
        if (!tok)
            break;

        ptr = tok + step;
    }

    res[i] = NULL;
    if (pn)
        *pn = i;

    return res;
}

UBGP_API char *joinstr(const char *delim, char **strings, size_t n)
{
    if (unlikely(!delim))
        delim = "";

    size_t counts[n];
    size_t dlen = strlen(delim);
    size_t bufsiz = 0;
    for (size_t i = 0; i < n; i++) {
        size_t len = strlen(strings[i]);
        counts[i] = len;
        bufsiz += len;
    }

    bufsiz += dlen * (n - 1);

    char *buf = malloc(bufsiz + 1);
    if (unlikely(!buf))
        return NULL;

    char *ptr = buf;
    char *end = buf + bufsiz;
    for (size_t i = 0; i < n; i++) {
        size_t len = counts[i];
        memcpy(ptr, strings[i], len);
        ptr += len;
        if (ptr < end) {
            memcpy(ptr, delim, dlen);
            ptr += dlen;
        }
    }

    *ptr = '\0';
    return buf;
}

UBGP_API char *joinstrvl(const char *delim, va_list va)
{
    if (unlikely(!delim))
        delim = "";

    va_list vc;
    size_t bufsiz = 0;
    size_t n      = 0;
    size_t dlen   = strlen(delim);

    va_copy(vc, va);
    while (true) {
        const char *s = va_arg(vc, const char *);
        if (!s)
            break;

        bufsiz += strlen(s);
        n++;
    }
    va_end(vc);

    if (likely(n > 0))
        bufsiz += dlen * (n - 1);

    char *buf = malloc(bufsiz + 1);
    if (unlikely(!buf))
        return NULL;

    char *ptr = buf;
    char *end = buf + bufsiz;
    for (size_t i = 0; i < n; i++) {
        const char *s = va_arg(va, const char *);
        size_t n = strlen(s);

        memcpy(ptr, s, n);
        ptr += n;
        if (ptr < end) {
            memcpy(ptr, delim, dlen);
            ptr += dlen;
        }
    }

    *ptr = '\0';
    return buf;
}

UBGP_API char *joinstrv(const char *delim, ...)
{
    va_list va;

    va_start(va, delim);
    char *s = joinstrvl(delim, va);
    va_end(va);
    return s;
}

UBGP_API char *trimwhites(char *s)
{
    char *start = s;
    while (isspace(*start)) start++;

    if (unlikely(*start == '\0')) {
        *s = '\0';
        return s;
    }

    size_t n = strlen(start);
    char *end = start + n - 1;
    while (isspace(*end)) end--;

    n = (end - start) + 1;
    memmove(s, start, n);
    s[n] = '\0';
    return s;
}

UBGP_API char *strpathext(const char *name)
{
    const char *ext = NULL;

    int c;
    while ((c = *name) != '\0') {
       if (c == '/')
           ext = NULL;
       if (c == '.')
           ext = name;

        name++;
    }
    if (!ext)
        ext = name;

    return (char *) ext;
}

UBGP_API size_t strunescape(char *s)
{
    static const char unescapechar[CHAR_MAX + 1] = {
        ['"']  = '"',
        ['\\'] = '\\',
        ['/']  = '/',  // allows embedding JSON in a <script>
        ['b']  = '\b',
        ['f']  = '\f',
        ['n']  = '\n',
        ['r']  = '\r',
        ['t']  = '\t',
        ['v']  = '\v'  // not really JSON, but still...
    };

    char c;

    char *dst = s;
    char *cur = s;
    while ((c = *cur++) != '\0') {
        if (c == '\\') {
            int e = unescapechar[MAX(*cur, 0)];  // deal with chars < 0
            if (e != '\0') {
                c = e;
                cur++;
            }
        }
        *dst++ = c;
    }

    *dst = '\0';
    return dst - s;
}

UBGP_API size_t strescape(char *restrict dst, const char *restrict src)
{
    static const char escapechar[CHAR_MAX + 1] = {
        ['"']  = '"',
        ['\\'] = '\\',
        ['/']  = '/',  // allows embedding JSON in a <script>
        ['\b'] = 'b',
        ['\f'] = 'f',
        ['\n'] = 'n',
        ['\r'] = 'r',
        ['\t'] = 't',
        ['\v'] = 'n'   // paranoid, remap \v to \n, incorrect but still better than nothing
    };

    // TODO escape chars < ' ' as '\u000'hex(c)

    int c;

    char *ptr = dst;
    while ((c = *src++) != '\0') {
        char e = escapechar[MAX(c, 0)];  // deal with chars < 0
        if (e != '\0') {
            *ptr++ = '\\';
            c = e;
        }

        *ptr++ = c;
    }

    *ptr = '\0';
    return ptr - dst;
}

