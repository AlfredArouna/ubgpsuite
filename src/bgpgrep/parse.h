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

#ifndef UBGP_PARSE_H_
#define UBGP_PARSE_H_

#include "../ubgp/funcattribs.h"
#include "../ubgp/ubgpdef.h"

#include <stdio.h>

/**
 * Section: parse
 * @title:   Text Parsing
 * @include: parse.h
 *
 * Parse Simple parser for basic whitespace separated tokens
 *
 * The parser has a maximum token length limit, in the interest of simple and
 * rapid parsing with moderated memory usage. The implemented format only
 * supports two basic concepts:
 *
 * * Tokens: A token is any word separated by whitespaces, a number of escape
 *   sequences can be used to express special caracters inside them.
 *
 * * Comments: A hash (`#`) starts a comment that extends to the end of
 *   the line it appears in; `#` is a reserved character and
 *   it may not be used inside a token. A comment may appear
 *   anywhere in text.
 *
 * The parser recognizes the following escape sequences:
 * * `'\n'`  - newline
 * * `'\v'`  - vertical tab
 * * `'\t'`  - horizontal tab
 * * `'\r'`  - caret return
 * * `'\#'`  - hash character
 * * `'\\'`  - backslash character
 * * `'\ '`  - space character
 * * `'\n'`  - token follows after newline
 *
 * Error handling is performed via callbacks, an error callback may be
 * registered to handle parsing errors, when such error is encountered,
 * a callback is invoked with a human readable message, when available,
 * the current parsing session name and line number are also provided.
 * The user is responsible to terminate parsing under such circumstances,
 * typically by using longjmp() or similar methods.
 *
 * Since the most effective way to terminate parsing is using longjmp(),
 * extra care should be taken to avoid using Variable Length Arrays (VLA)
 * during parsing operation, since doing so may cause memory leaks.
 *
 * This file is guaranteed to include standard `stdio.h`,
 * a number of other files may be included in the interest of providing
 * API functionality, but the includer of this file should not rely on
 * any other header being included.
 */

/**
 * TOK_LEN_MAX:
 *
 * Maximum token length.
 */
#define TOK_LEN_MAX 256

/**
 * parse_err_callback_t:
 * @name: (nullable):   parsing session name, as specified by startparsing(),
 *                      may be %NULL when no parsing session information is
 *                      available (e.g. startparsing() wasn't called).
 * @lineno:             line number in which error was detected,
 *                      the handler may discretionally ignore this argument if
 *                      @name is %NULL (for example it may make little sense
 *                      to print a line number information when parsing from a
 *                      console).
 * @msg:                informative human readable parsing error message.
 * @data: (nullable):   pointer to user defined data, as provided to
 *                      startparsing().
 *
 * Error handling callback for parser.
 *
 * Defines a pointer to function with the signature expected by the parser
 * whenever a parsing-error handler is called.
 *
 * See setperrcallback()
 */
typedef void (*parse_err_callback_t)(const char *name,
                                     uint        lineno,
                                     const char *msg,
                                     void       *data);

/**
 * startparsing:
 * @name: (nullable): session name, this argument may be %NULL to indicate
 *                    that the session has no name. The contents of this
 *                    pointer are *not* copied and are expected to remain
 *                    valid throughout the parsing session.
 * @start_line:       initial line, if this value is 0,
 *                    it is silently interpreted as 1.
 * @data: (nullable): user-defined data, which is forwared to the error
 *                    handler unchanged whenever a parsing error occurs.
 *
 * Fill parsing session name and starting line.
 *
 * This function should be called whenever a new parsing session takes place,
 * in order to provide sensible information to parsing error callbacks.
 *
 * Any existing set information about previous parsing session is
 * overwritten by calling this function.
 * This function may be repeatedly called to simulate sub-sections
 * in the same parsing session.
 * The initial parsing session has %NULL name, a starting line of 1
 * and %NULL data.
 */
void startparsing(const char *name, uint start_line, void *data);

/**
 * setperrcallback:
 * @cb: (nullable): function to be called whenever a parsing error occurs,
 *                  specify %NULL to remove any installed handler.
 *
 * Register a parsing error callback, returns old callback.
 *
 * Whenever the parse() function (or any other parsing function) encounters
 * a parsing error, the provided function is called with a meaningful context
 * information to handle the parsing error. Typically the application
 * wants to print or store the parsing error for logging or notification
 * purposes.
 *
 * The registered function is responsible to decide whether the application
 * may recover from a parsing error or not.
 * If the application does not tolerate parsing errors at all, the most
 * effective way to do so is calling exit() or similar functions to
 * terminate execution from within the handler, doing so will keep
 * the parsing logic clean.
 * If the application is capable of tolerating parsing errors, then
 * there are two approaches to do so:
 * * The handler can return normally to the parer, after possibly setting
 *   some significant variable, and the parser will return special values to
 *   its caller (e.g. parse() will return %NULL, as if end-of-parse
 *   occurred), the application is thus resposible of doing the usual error
 *   checking.
 * * The handler may use longjmp() to jump back to a known setjmp()
 *   buffer, the application handles this as a parsing error and completely
 *   halts the parsing process, performing any required action.
 *   In this scenario the handler never returns to the parser and
 *   errors can be dealt with in a fashion similar to an "exception".
 *   Be careful when using this approach with Variable Length Arrays, since
 *   it may introduce unobvious memory-leak.
 *
 * Returns: previously registered error handler, %NULL if no handler
 *          was registered.
 */
parse_err_callback_t setperrcallback(parse_err_callback_t cb);

/**
 * parse:
 * @f: input file to be parsed, this function does not impose
 *     the caller to provide the same source for each call,
 *     you may freely change the source.
 *
 * Return next token, or %NULL on end of parse.
 *
 * This function should be called repeatedly on an input file to
 * parse each token, on end-of-file %NULL will be returned.
 *
 * Returns: pointer to a statically (thread-local) allocated storage,
 *          that is guaranteed to remain valid at least up to the next call
 *          to this function. The returned pointer must not be free()d.
 *          On read error, %NULL is returned, as if %EOF was encountered,
 *          the caller may distinguish such situations by invoking ferror()
 *          on @f.
 */
CHECK_NONNULL(1) char *parse(FILE *f);

/**
 * ungettoken:
 * @tok: (nullable): token to be placed back into the parser,
 *                   this argument can be %NULL, and if it is, this function
 *                   has no effect.
 *                   Behavior is undefined if @tok is not %NULL and it was
 *                   not returned by a previous call to parse() or any other
 *                   parsing function.
 *
 * Place token back into the stream.
 *
 * This function makes possible to place back a parsed token into
 * the parsing stream, the next parsing call shall return it as if it
 * was encountered for the first time.
 */
void ungettoken(const char *tok);

/**
 * expecttoken:
 * @f:                input source, same considerations as parse() apply.
 * @what: (nullable): optional string that requires the next token to match
 *                    exactly this string. If this argument is not %NULL,
 *                    then encountering a token which does not compare
 *                    as equal to this string is considered a parsing error.
 *
 * Expect a token (%NULL to expect any token).
 *
 * Behaves like parse(), but additionally requires a token to exist in the
 * input source.
 * This function shall consider an end of parse condition as a parsing error,
 * and call the parsing error handler registered by the latest call to
 * setperrcallback() with the appropriate arguments.
 * If @what is not %NULL, then this function also ensures that the token
 * matches exactly (as in strcmp()) the provided argument string.
 *
 * Returns: next token in input source, %NULL on error: in particular,
 *          if no error handler is currently registered, %NULL is returned
 *          to signal an unexpected end of parse or token mismatch.
 *          The same storage considerations as parse() hold for the
 *          returned buffer.
 */
CHECK_NONNULL(1) char *expecttoken(FILE *f, const char *what);

/**
 * iexpecttoken:
 * @f: input source, same considerations as parse() apply.
 *
 * Expect integer value.
 *
 * Behaves like expecttoken(), but additionally considers a parsing error
 * encountering a token which is an invalid or out of range base 10 integer
 * value.
 *
 * Returns: parsed integer token, 0 is returned on parsing error.
 *          Since 0 is a valid integer token, the error handler should take
 *          action to inform the caller that a bad token was encountered to
 *          disambiguate this event.
 */
CHECK_NONNULL(1) int iexpecttoken(FILE *f);

/**
 * llexpecttoken:
 *
 * Behaves like iexpecttoken(), but allows #llong values.
 */
CHECK_NONNULL(1) llong llexpecttoken(FILE *f);

/**
 * fexpecttoken:
 * @f: input source, same considerations as parse() apply.
 *
 * Expect floating point value.
 *
 * Behaves like expecttoken(), but additionally considers a parsing error
 * encountering a token which is not an invalid floating point value.
 *
 * Returns: parsed floating point token, `0.0` is returned on parsing error.
 *          Since 0.0 is a valid floating point token, the error handler should
 *          take action to inform the caller that a bad token was encountered to
 *          disambiguate this event.
 */
CHECK_NONNULL(1) double fexpecttoken(FILE *f);

/**
 * skiptonextline:
 * @f: input source, same considerations as parse() apply.
 *
 * Skip remaining tokens in this line.
 *
 * Advances until a newline character is encountered, effectively skipping
 * any token in the current line.
 * Any token fed to ungettoken() is discarded.
 *
 * In the event that a token spans over multiple lines (e.g. it contains the
 * `'\n'` escape sequence), such token is considered to be in the next line,
 * so that same token will be the one returned by the subsequent call to
 * parse().
 */
CHECK_NONNULL(1) void skiptonextline(FILE *f);

/**
 * parsingerr:
 * @msg: error format string.
 * @...: format arguments.
 *
 * Trigger a parsing error at the current position.
 *
 * This function can be called to signal a parsing error at the current
 * position, it is especially useful when performing additional checks
 * on token returned by this API.
 * The message is formatted in a printf()-like fashion.
 */
CHECK_PRINTF(1, 2) CHECK_NONNULL(1) void parsingerr(const char *msg, ...);

#endif

