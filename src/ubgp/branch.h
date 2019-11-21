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

#ifndef UBGP_BRANCH_H_
#define UBGP_BRANCH_H_

/**
 * SECTION:  branch
 * @include: branch.h
 * @title:   Compiler-specific branching hints
 *
 * Macros to mark code branches as likely or unlikely, and various
 * code flow related hints.
 *
 * Correct use of these macros gives an opportunity optimizer to make effective
 * use of instruction cache.
 *
 * **Misusing these compiler hints may produce suboptimal code, or even
 *   lead to undefined results, when in doubt leave the task to the compiler!**
 */

/**
 * likely:
 * @guard: A boolean expression.
 *
 * Marks a branch as highly likely.
 *
 * If the compiler honors the hint, generated assembly is optimized for
 * the case in which @guard is %true.
 */
/**
 * unlikely:
 * @guard: A boolean expression.
 *
 * Marks a branch as highly unlikely.
 *
 * If the compiler honors the hint, generated assembly is optimized
 * for the case in which @guard is %false.
 */
/**
 * UNREACHABLE:
 *
 * Marks a code path as unreachable.
 *
 * The compiler is free to assume that the code path following this
 * marker shall never be reached.
 *
 * **If the code path is actually reachable, then subtle, unexpected and
 *   typically catastrophic events could very well occur at runtime.**
 */
/**
 * FALLTHROUGH:
 *
 * Explicitly mark a switch branch to fall-through the next `case` label.
 *
 * This is merely a readability macro, on some compilers it disables pedantic
 * warning generation about case fall-throughs.
 */
/**
 * NODEFAULT:
 *
 * Marks a `switch` statement as having no `default` case, meaning that
 * the compiler is free to assume that **every** possible value of the `switch`
 * guard is handled explicitly using `case` labels.
 *
 * **If this assumption isn't actually true, then tragic consequences may happen
 *   at runtime.**
 */
#ifdef __GNUC__
#define likely(guard)   __builtin_expect(!!(guard), 1)
#define unlikely(guard) __builtin_expect(!!(guard), 0)
#define UNREACHABLE     __builtin_unreachable()
#define FALLTHROUGH     __attribute__((__fallthrough__))
#endif

#ifndef likely
#define likely(guard) (guard)
#endif
#ifndef unlikely
#define unlikely(guard) (guard)
#endif
#ifndef UNREACHABLE
#define UNREACHABLE ((void) 0)
#endif
#ifndef FALLTHROUGH
#define FALLTHROUGH /* FALLTHROUGH */ ((void) 0)
#endif

#define NODEFAULT default: do { UNREACHABLE; break; } while (0)

#endif

