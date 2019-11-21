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

#ifndef UBGP_PATRICIA_TRIE_H_
#define UBGP_PATRICIA_TRIE_H_

#include "funcattribs.h"
#include "netaddr.h"
#include "u128.h"

/**
 * SECTION: patriciatrie
 * @title: Patricia Trie
 * @include: patriciatrie.h
 *
 * A (binary) Patricia Trie implementation.
 */

enum {
    PREFIX_INSERTED,
    PREFIX_ALREADY_PRESENT,
};

enum {
    SUPERNET_ITERATOR,
    SUBNET_ITERATOR,
};

typedef struct {
    netaddr_t prefix;
    void *payload;
} trienode_t;

typedef union pnode_s pnode_t;
typedef struct nodepage_s nodepage_t;

/**
 * patricia_trie_t:
 *
 * The patricia memory is allocated in pages.
 * A list of pages is maintained.
 * Each page is a block of 256 nodes.
 * Each node can be free or not. Each node has the possibility to be in a list
 * of free nodes. When no more free nodes are available, a new page is allocated.
*/
typedef struct patricia_trie {
    /*< private >*/
    pnode_t* head;
    uint maxbitlen;
    uint nprefs;
    nodepage_t* pages;
    pnode_t* freenodes;
} patricia_trie_t;

UBGP_API CHECK_NONNULL(1) void patinit(patricia_trie_t* pt, sa_family_t family);

/**
 * patclear:
 * @pt: a #patricia_trie_t.
 *
 * Clear a Patricia Trie *without* freeing its memory.
 *
 * This is essentially a faster way to remove every node in the trie, while
 * preserving the allocated memory for further use.
 *
 * Calling this function invalidates any node in the Patricia Trie,
 * the user is responsible to perform any memory management to free()
 * node resources stored in the `payload` field of each node, whenever
 * necessary.
 */
UBGP_API CHECK_NONNULL(1) void patclear(patricia_trie_t *pt);

/**
 + patdestroy:
 * @pt: a #patricia_trie_t
 *
 * Destroy a Patricia Trie, free()ing any allocated memory.
 *
 * This function *does not* free() any memory contained in the
 * `payload` field of each node, the user of the Patricia Trie is
 * responsible for any memory management of such field, more importantly
 * such memory management *must* be done before calling this function 
 */
UBGP_API CHECK_NONNULL(1) void patdestroy(patricia_trie_t* pt);

UBGP_API CHECK_NONNULL(1, 2) trienode_t* patinsert(patricia_trie_t *pt,
                                                   const netaddr_t *prefix,
                                                   int *inserted);

UBGP_API CHECK_NONNULL(1, 2)
trienode_t* patsearchexact(const patricia_trie_t *pt, const netaddr_t *prefix);

UBGP_API CHECK_NONNULL(1, 2)
trienode_t* patsearchbest(const patricia_trie_t *pt, const netaddr_t *prefix);

UBGP_API CHECK_NONNULL(1, 2)
void* patremove(patricia_trie_t *pt, const netaddr_t *prefix);

/**
 + patgetsubnetsof:
 *
 * Supernets of a prefix
 *
 * Returns: supernets of the provided prefix, if present, the provided prefix
 *          is returned into the result.
 *          If not %NULL, the returned value must bee free()d by the caller
 */
UBGP_API CHECK_NONNULL(1, 2)
trienode_t** patgetsupernetsof(const patricia_trie_t *pt,
                               const netaddr_t *prefix);

/**
 * patissupernetof:
 *
 * Check if supernets of a prefix.
 *
 * Returns: %true if the provided prefix is supernet of any patricia prefix,
 *          %false otherwise.
 *
 */
UBGP_API PUREFUNC CHECK_NONNULL(1, 2)
bool patissupernetof(const patricia_trie_t *pt, const netaddr_t *prefix);

/**
 * patgetsubnetsof:
 *
 * Subnets of a prefix.
 *
 * Returns: subnets of the provided prefix.
 *          If present, the provided prefix is returned into the result.
 *          If not %NULL, the returned value must bee free()d by the caller
 *
 */
UBGP_API CHECK_NONNULL(1, 2)
trienode_t** patgetsubnetsof(const patricia_trie_t *pt,
                             const netaddr_t       *prefix);

/**
 * patissubnetof:
 *
 * Check subnet of a prefix.
 *
 * Returns: %true if the provided prefix is subnet of any patricia prefix,
 *          %false otherwise
 *
 */
UBGP_API PUREFUNC CHECK_NONNULL(1, 2)
bool patissubnetof(const patricia_trie_t *pt, const netaddr_t *prefix);

/**
 * patgetrelatedof:
 *
 * Related of a prefix.
 *
 * Returns: related prefixes of the provided prefix.
 *          If present, the provided prefix is returned into the result. FIXME?
 *          If not %NULL, the returned value must bee free()d by the caller
 */
UBGP_API CHECK_NONNULL(1, 2)
trienode_t** patgetrelatedof(const patricia_trie_t *pt, const netaddr_t *prefix);

/**
 * patisrelatedof:
 * @pt:     an initialized #patricia_trie_t
 * @prefix: a network prefix
 *
 * Check if related of a prefix
 *
 * Returns: %true if the provided prefix is related to any of the patricia
 *          prefixes, %false otherwise
 */
UBGP_API PUREFUNC CHECK_NONNULL(1, 2)
bool patisrelatedof(const patricia_trie_t *pt, const netaddr_t *prefix);

/**
 * patcoverage:
 * pt: a patricia trie.
 *
 * Coverage of prefixes.
 * **The default route is ignored. **
 *
 * Returns: amount of address space covered by the prefixes insrted into the
 *          patricia
 */
UBGP_API PUREFUNC CHECK_NONNULL(1) u128 patcoverage(const patricia_trie_t *pt);

/**
 * patgetfirstsubnetsof:
 * @pt:     a patricia trie.
 * @prefix: a prefix.
 *
 * Get the first subnets of a given @prefix.
 *
 * Returns: the first-level subnets of @prefix.
 */
UBGP_API CHECK_NONNULL(1, 2)
trienode_t** patgetfirstsubnetsof(const patricia_trie_t *pt,
                                  const netaddr_t       *prefix);

/* Iterator */

typedef struct {
    /*< private >*/
    pnode_t *stack[129];
    pnode_t **sp;
    pnode_t *curr;
} patiterator_t;

UBGP_API void patiteratorinit(patiterator_t *state, const patricia_trie_t *pt);

UBGP_API trienode_t* patiteratorget(patiterator_t *state);

UBGP_API void patiteratornext(patiterator_t *state);

UBGP_API PUREFUNC bool patiteratorend(const patiterator_t *state);

#endif

