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

#include "../ubgp/branch.h"

#include "parse.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *name;
    void *data;
    uint lineno;
    parse_err_callback_t err;

    char buf[TOK_LEN_MAX + 1];
    char unget[TOK_LEN_MAX + 1];
} parser_t;

// initialize as specified by the default parsing session
static _Thread_local parser_t parser = { NULL, NULL, 1, NULL };

void parsingerr(const char *msg, ...)
{
    // ** don't use VLA **
    // parser.err() might longjmp() causing leaks!
    if (parser.err) {
        char buf[4096];
        va_list vl;

        va_start(vl, msg);
        int n = vsnprintf(buf, sizeof(buf), msg, vl);
        va_end(vl);

        // add strerror if msg ends with :
        int nmsg = strlen(msg);
        if (nmsg > 0 && msg[nmsg - 1] == ':')
            snprintf(buf + n, sizeof(buf) - n, ": %s", strerror(errno));

        // always report line 0 if session is unavailable
        uint lineno = (parser.name) ? parser.lineno : 0;
        parser.err(parser.name, lineno, buf, parser.data);
    }
}

parse_err_callback_t setperrcallback(parse_err_callback_t cb)
{
    parse_err_callback_t old = parser.err;

    parser.err = cb;
    return old;
}

void startparsing(const char *name, unsigned int start_line, void *data)
{
    if (start_line < 1)
        start_line = 1;  // line 0 makes little sense...

    parser.name = name;
    parser.lineno = start_line;
	parser.data = data;
}

void skiptonextline(FILE *f)
{
    parser.unget[0] = '\0';  // don't waste time parsing it back

    const char *tok;
    uint curline = parser.lineno;
    while ((tok = parse(f)) != NULL) {
        if (curline != parser.lineno)
            break;
    }

    ungettoken(tok);
}

char *parse(FILE *f)
{
    if (parser.unget[0] != '\0') {
        strcpy(parser.buf, parser.unget);
        parser.unget[0] = '\0';
        return parser.buf;
    }

    int c;
    do {
        c = getc_unlocked(f);

        // skip to newline in case of comment
        if (c == '#')
            skiptonextline(f);
        if (c == '\n' || c == EOF)
            parser.lineno++;

    } while (isspace((uchar) c) || c == '\0');

    int i = 0;
    ungetc(c, f);
    while (true) {
        c = getc_unlocked(f);
        if (isspace((uchar) c) || c == EOF || c == '\0' || c == '#') {
            ungetc(c, f);
            break;
        }
        if (c == '\\') {
            // escape sequence
            c = getc_unlocked(f);
            switch (c) {
            case '\n':
                continue; // token follows after newline

            case '#':
            case '\\':
            case ' ':
                break;
            case 'n':
                c = '\n';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            case 'r':
                c = '\r';
                break;
            case EOF:
                parsingerr("EOF after '\\'!");
                continue;  // skip bad escape sequence
            default:
                parsingerr("bad escape sequence '\\%c'", c);
                continue;  // skip bad escape sequence
            }
        }
        if (unlikely(i == TOK_LEN_MAX)) {
            parsingerr("'%.*s'...: token too long", i, parser.buf);
            ungetc(c, f);
            break;
        }

        parser.buf[i++] = c;
    }

    parser.buf[i] = '\0';
    return (i > 0) ? parser.buf : NULL;
}

void ungettoken(const char *tok)
{
    if (likely(tok))
        strcpy(parser.unget, tok);
}

char *expecttoken(FILE *f, const char *what)
{
    char *tok = parse(f);
    if (!tok) {
        parsingerr("unexpected end of parse");
        return NULL;
    } else if (what && strcmp(tok, what) != 0) {
        parsingerr("expecting '%s', got '%s'", what, tok);
        return NULL;
    } else {
        return tok;
    }
}

int iexpecttoken(FILE *f)
{
    char *tok = expecttoken(f, NULL);
    char *eptr;

    if (!tok)
        return 0;

    errno = 0;
    long v = strtol(tok, &eptr, 10);
    if (tok == eptr || *eptr != '\0') {
        parsingerr("got '%s', but integer value expected", tok);
        return 0;
    }

    // handle out of range integers
    if (unlikely(v < INT_MIN || v > INT_MAX))
        errno = ERANGE;

    if (errno != 0) {
        parsingerr("got '%s': %s", tok, strerror(errno));
        return 0;
    } else {
        return (int) v;
    }
}

llong llexpecttoken(FILE *f)
{
    char *tok = expecttoken(f, NULL);
    char *eptr;

    if (!tok)
        return 0;

    errno = 0;
    llong v = strtoll(tok, &eptr, 10);
    if (tok == eptr || *eptr != '\0') {
        parsingerr("got '%s', but integer value expected", tok);
        return 0;
    }

    if (errno != 0) {
        parsingerr("got '%s':", tok);
        return 0;
    }

    return v;
}

double fexpecttoken(FILE *f)
{
    char *tok = expecttoken(f, NULL);
    char *eptr;

    if (!tok)
        return 0.0;

    errno = 0;
    double v = strtod(tok, &eptr);
    if (tok == eptr || *eptr != '\0') {
        parsingerr("got '%s', but floating point value expected", tok);
        return 0.0;
    } else if (errno != 0) {
        parsingerr("'%s':", tok);
        return 0.0;
    } else {
        return v;
    }
}

