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

#include "bitops.h"
#include "branch.h"
#include "endian.h"
#include "patriciatrie.h"

#include <stdlib.h>
#include <string.h>

enum {
    PATRICIA_GLUE_NODE = 1,
};

/*
 * the "public" node is trienode_t. (see patriciatrie.h).
 * It contains a prefix and a payload.
 * 
 * the actual node is a pnode_s.
*/
union pnode_s {
    trienode_t pub;                  // keep underlying structure in sync!
    struct {
        netaddr_t prefix;
        void *payload;
        union pnode_s *parent;       // the least significant bit indicates whether the node is glue
        union pnode_s *children[2];  // 0 = left, 1 = right
    };
    union pnode_s *next;             // used when the node is a free node (to create a linked list of free nodes)
};

struct nodepage_s {
    struct nodepage_s *next;
    pnode_t block[128];
};

static void pnodeinit(pnode_t *n, const netaddr_t *prefix)
{
    memset(n, 0, sizeof(*n));

    if (prefix)
        n->prefix = *prefix;
    else
        n->prefix.family = AF_UNSPEC;
}

static pnode_t *getpnodeparent(pnode_t *n)
{
    return (pnode_t *) (((uintptr_t) n->parent) & ~PATRICIA_GLUE_NODE);
}

static void setpnodeparent(pnode_t *n, pnode_t *parent)
{
    n->parent = (pnode_t *) ((uintptr_t) n->parent & PATRICIA_GLUE_NODE);
    n->parent = (pnode_t *) ((uintptr_t) n->parent | (uintptr_t) parent);
}

static int ispnodeglue(pnode_t *n)
{
    return ((uintptr_t)n->parent) & PATRICIA_GLUE_NODE;
}

static void setpnodeglue(pnode_t *n)
{
    n->parent = (pnode_t *) ((uintptr_t) n->parent | PATRICIA_GLUE_NODE);
}

static void resetpnodeglue(pnode_t* n)
{
    n->parent = (pnode_t *)((uintptr_t) n->parent & ~PATRICIA_GLUE_NODE);
}

static void patallocpage(patricia_trie_t *pt)
{
    nodepage_t *p = malloc(sizeof(*p));
    if (unlikely(!p))
        return;

    for (size_t i = 0; i < countof(p->block); i++) {
        pnode_t *n = &p->block[i];

        n->next = pt->freenodes;
        pt->freenodes = n;
    }

    p->next = pt->pages;
    pt->pages = p;
}

static pnode_t* getfreenode(patricia_trie_t *pt)
{
    pnode_t *n = pt->freenodes;
    if (!n) {
        patallocpage(pt);
        n = pt->freenodes;
        if (unlikely(!n))
            return NULL;
    }
    pt->freenodes = pt->freenodes->next;

    return n;
}

static void putfreenode(patricia_trie_t *pt, pnode_t *n)
{
    pnode_t* save = pt->freenodes;
    pt->freenodes = n;
    n->next = save;
}

UBGP_API void patinit(patricia_trie_t *pt, sa_family_t family)
{
    assert(family == AF_INET || family == AF_INET6);

    pt->head      = NULL;
    pt->maxbitlen = (family == AF_INET6) ?  128 : 32;
    pt->nprefs    = 0;
    pt->pages     = NULL;
    pt->freenodes = NULL;
}

UBGP_API void patclear(patricia_trie_t *pt)
{
    pt->head      = NULL;
    pt->nprefs    = 0;
    pt->freenodes = NULL;

    for (nodepage_t *p = pt->pages; p; p = p->next) {
        for (size_t i = 0; i < countof(p->block); i++)
            putfreenode(pt, &p->block[i]);
    }
}

UBGP_API void patdestroy(patricia_trie_t *pt)
{
    nodepage_t *ptr = pt->pages;

    while (ptr) {
        nodepage_t* next = ptr->next;
        free(ptr);

        ptr = next;
    }
}

static bool patcompwithmask(const netaddr_t *addr,
                            const netaddr_t *dest,
                            uint             mask)
{
    if (memcmp(&addr->bytes[0], &dest->bytes[0], mask / 8) == 0) {
        uint n = mask / 8;
        uint m = ((uint) (~0) << (8 - (mask % 8)));

        if (((mask & 0x7) == 0) || ((addr->bytes[n] & m) == (dest->bytes[n] & m)))
            return true;
    }

    return false;
}

UBGP_API trienode_t *patinsert(patricia_trie_t *pt,
                               const netaddr_t *prefix,
                               int             *inserted)
{
    int dummy;
    if (!inserted)
        inserted = &dummy;

    pnode_t *n;

    *inserted = PREFIX_ALREADY_PRESENT;

    if (pt->head == NULL) {
        pnode_t *n = getfreenode(pt);
        if (unlikely(!n))
            return NULL;

        pnodeinit(n, prefix);
        pt->head = n;
        pt->nprefs++;
        *inserted = PREFIX_INSERTED;

        return &n->pub;
    }

    n = pt->head;
    uint maxbits = pt->maxbitlen;

    while (n->prefix.bitlen < prefix->bitlen || ispnodeglue(n)) {
        int bit = (n->prefix.bitlen < maxbits) && (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        if (n->children[bit != 0] == NULL)
            break;

        n = n->children[bit != 0];
    }

    byte *test_addr = n->prefix.bytes;
    uint check_bit  = MIN(n->prefix.bitlen, prefix->bitlen);
    uint differ_bit = 0;

#if 1
    // unoptimized version
    uint r;
    for (uint i = 0, z = 0; z < check_bit; i++, z += 8) {
        if ((r = (prefix->bytes[i] ^ n->prefix.bytes[i])) == 0) {
            differ_bit = z + 8;
            continue;
        }

        uint j;
        for (j = 0; j < 8; j++) {
            if (r & (0x80 >> j))
                break;
        }

        differ_bit = z + j;
        break;
    }
#else
    /* TODO possible optimization like this:

      example 32 bit portion in different endianness
     LSB    (visit using LSB->MSB)        MSB (LE)
        01000000 00000000 00000100 00000000
     MSB    (visit using MSB->LSB)        LSB (BE)

        leftmost bit is:
        -> 32 - bsr32() (BE)
        -> bsf32() - 1  (LE)
    */

    for (uint i = 0, z = 0; z < check_bit; i++, z += 32) {
        uint32_t r;
        if ((r = (prefix->u32[i] ^ n->prefix.u32[i])) == 0) {
            differ_bit = z + 32;
            continue;
        }

        uint j = (ENDIAN_NATIVE == ENDIAN_BIG) ?  32 - bsr32(r) : bsf32(r) - 1; // clz(beswap32(r));
        differ_bit = z + j;
        break;
    }

#endif

    if (differ_bit > check_bit)
        differ_bit = check_bit;

    pnode_t* parent = getpnodeparent(n);
    while (parent && parent->prefix.bitlen >= differ_bit) {
        n = parent;
        parent = getpnodeparent(n);
    }

    if (differ_bit == prefix->bitlen && n->prefix.bitlen == prefix->bitlen) {
        if (ispnodeglue(n))
            return &n->pub;

        pt->nprefs++;
        n->prefix = *prefix;
        resetpnodeglue(n);

        return &n->pub;
    }

    pnode_t *newnode = getfreenode(pt);
    if (unlikely(!newnode))
        return NULL;

    pnodeinit(newnode, prefix);
    pt->nprefs++;

    if (n->prefix.bitlen == differ_bit) {
        setpnodeparent(newnode, n);

        int bit = (n->prefix.bitlen < maxbits) && (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        n->children[bit != 0] = newnode;

        *inserted = PREFIX_INSERTED;

        return &newnode->pub;
    }

    if (n->prefix.bitlen == differ_bit) {
        int bit = (prefix->bitlen < maxbits) && (test_addr[prefix->bitlen >> 3] & (0x80 >> (prefix->bitlen & 0x07)));
        newnode->children[bit] = n;
        setpnodeparent(newnode, n);

        if (!getpnodeparent(n)) {
            pt->head = newnode;
        } else if (getpnodeparent(n)->children[1] == n) {
            int b = (getpnodeparent(n)->children[1] == n);
            getpnodeparent(n)->children[b] = n;
        }
        setpnodeparent(n, newnode);
    } else {
        pnode_t *glue = getfreenode(pt);
        if (unlikely(!glue))
            return NULL;

        pnodeinit(glue, NULL);
        glue->prefix.bitlen = differ_bit;
        setpnodeglue(glue);
        setpnodeparent(glue, getpnodeparent(n));

        int bit = (differ_bit < maxbits) && (prefix->bytes[differ_bit >> 3] & (0x80 >> (differ_bit & 0x07)));

        glue->children[bit] = newnode;
        glue->children[!bit] = n;

        setpnodeparent(newnode, glue);
        if (!getpnodeparent(n)) {
            pt->head = glue;
        } else {
            int bit = (getpnodeparent(n)->children[1] == n);
            getpnodeparent(n)->children[bit] = glue;
        }

        setpnodeparent(n, glue);
    }

    *inserted = PREFIX_INSERTED;
    return &newnode->pub;
}

UBGP_API trienode_t *patsearchexact(const patricia_trie_t *pt,
                                    const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *n = pt->head;

    while (n->prefix.bitlen < prefix->bitlen) {
        int bit = (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        n = n->children[bit != 0];
        if (!n)
            return NULL;
    }

    if (n->prefix.bitlen > prefix->bitlen || ispnodeglue(n))
        return NULL;

    if (patcompwithmask(&n->prefix, prefix, prefix->bitlen))
        return &n->pub;

    return NULL;
}

UBGP_API trienode_t *patsearchbest(const patricia_trie_t *pt,
                                   const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *n = pt->head;
    pnode_t *last = NULL;

    while (n->prefix.bitlen < prefix->bitlen) {
        if (!ispnodeglue(n)) {
            if ((n->prefix.bitlen <= prefix->bitlen) && (patcompwithmask(&n->prefix, prefix, n->prefix.bitlen)))
                last = n;
            else
                break;
        }

        int bit = (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        n = n->children[bit != 0];

        if (!n)
            break;
    }

    return &last->pub;
}

UBGP_API void *patremove(patricia_trie_t *pt, const netaddr_t *prefix)
{
    pnode_t *n = (pnode_t*)patsearchexact(pt, prefix);

    pt->nprefs--;

    if (!n)
        return NULL;

    void *payload = n->payload;

    if (n->children[0] && n->children[1]) {
        putfreenode(pt, n);
        setpnodeglue(n);
        return payload;
    }

    pnode_t *parent;
    pnode_t *child;

    if (!n->children[0] && !n->children[1]) {
        parent = getpnodeparent(n);
        putfreenode(pt, n);

        if (!parent) {
            pt->head = NULL;
            return payload;
        }

        int bit = (parent->children[1] == n);
        parent->children[bit] = NULL;
        child = parent->children[!bit];

        if (!ispnodeglue(parent))
            return payload;

        // if here, the parent is glue then we need to remove the parent too
        if (!getpnodeparent(parent)) {
            pt->head = child;
        } else {
            int bit = (getpnodeparent(parent)->children[1] == parent);
            getpnodeparent(parent)->children[bit] = child;
        }
        setpnodeparent(child, getpnodeparent(parent));
        putfreenode(pt, parent);

        return payload;
    }

    int bit = (n->children[1] != NULL);
    child = n->children[bit];

    parent = getpnodeparent(n);
    setpnodeparent(child, parent);

    putfreenode(pt, n);

    if (!parent) {
        pt->head = child;
        return payload;
    }

    bit = (parent->children[1] == n);
    parent->children[bit] = child;

    return payload;
}

UBGP_API trienode_t **patgetsupernetsof(const patricia_trie_t *pt,
                                        const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *n = pt->head;

    trienode_t **res = malloc((prefix->bitlen + 2) * sizeof(*res));
    if (unlikely(!res))
        return NULL;

    uint i = 0;

    while (n && n->prefix.bitlen < prefix->bitlen) {
        if (!ispnodeglue(n)) {
            if (n->prefix.bitlen < pt->maxbitlen && patcompwithmask(&n->prefix, prefix, n->prefix.bitlen)) {
                res[i++] = &n->pub;
            } else {
                res[i] = NULL;
                return res;
            }
        }

        int bit = (n->prefix.bitlen < pt->maxbitlen) && (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        n = n->children[bit != 0];
    }

    if (n && !ispnodeglue(n) && n->prefix.bitlen <= prefix->bitlen && patcompwithmask(&n->prefix, prefix, prefix->bitlen))
        res[i++] = &n->pub;

    res[i] = NULL;

    return res;
}

UBGP_API bool patissubnetof(const patricia_trie_t *pt, const netaddr_t *prefix)
{
    pnode_t *n = pt->head;
    while (n && n->prefix.bitlen < prefix->bitlen) {
        if (!ispnodeglue(n))
           return n->prefix.bitlen < pt->maxbitlen && patcompwithmask(&n->prefix, prefix, n->prefix.bitlen);

        int bit = (n->prefix.bitlen < pt->maxbitlen) && (prefix->bytes[n->prefix.bitlen >> 3] & (0x80 >> (n->prefix.bitlen & 0x07)));
        n = n->children[bit];
    }

    return n && !ispnodeglue(n) && n->prefix.bitlen <= prefix->bitlen && patcompwithmask(&n->prefix, prefix, prefix->bitlen);
}

UBGP_API trienode_t **patgetsubnetsof(const patricia_trie_t *pt,
                                      const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *start = pt->head;

    while (start && start->prefix.bitlen < prefix->bitlen) {
        int bit = prefix->bytes[start->prefix.bitlen >> 3] & (0x80 >> (start->prefix.bitlen & 0x07));
        start = start->children[bit != 0];
    }

    uint64_t n;
    if (prefix->family == AF_INET || prefix->bitlen >= 96) {
        n = (1ull << (pt->maxbitlen - prefix->bitlen));
        if (pt->nprefs < n)
            n = pt->nprefs;
    } else {
        n = pt->nprefs;
    }

    n++;

    trienode_t **res = malloc(n * sizeof(*res));
    if (unlikely(!res))
        return NULL;

    uint i = 0;

    pnode_t *node;

    pnode_t *stack[pt->maxbitlen+1];
    pnode_t **sp = stack;
    pnode_t *next = start;
    while ((node = next)) {
        if (!ispnodeglue(node)) {
            if (patcompwithmask(&node->prefix, prefix, prefix->bitlen))
                res[i++] = &node->pub;
            else
                break;
        }

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];

            next = next->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(sp--);
        } else {
            next = NULL;
        }
    }

    res[i] = NULL;

    return res;
}

UBGP_API bool patissupernetof(const patricia_trie_t *pt,
                              const netaddr_t       *prefix)
{
    pnode_t *start = pt->head;
    while (start && start->prefix.bitlen < prefix->bitlen) {
        int bit = prefix->bytes[start->prefix.bitlen >> 3] & (0x80 >> (start->prefix.bitlen & 0x07));
        start = start->children[bit != 0];
    }

    pnode_t *node;

    pnode_t *stack[pt->maxbitlen+1];
    pnode_t **sp = stack;
    pnode_t *next = start;
    while ((node = next)) {
        if (!ispnodeglue(node)) {
            if (patcompwithmask(&node->prefix, prefix, prefix->bitlen))
                return true;

            break;
        }

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];

            next = next->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(sp--);
        } else {
            next = NULL;
        }
    }

    return false;
}

UBGP_API trienode_t **patgetrelatedof(const patricia_trie_t *pt,
                                      const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *start = pt->head;

    uint64_t n;
    if (prefix->family == AF_INET || prefix->bitlen >= 96) {
        n = (1ull << (pt->maxbitlen - prefix->bitlen));
        if (pt->nprefs < n)
            n = pt->nprefs;
    } else {
        n = pt->nprefs;
    }

    n += prefix->bitlen + 3;

    trienode_t **res = malloc(n * sizeof(*res));
    if (unlikely(!res))
        return NULL;

    uint i = 0;
    while (start && start->prefix.bitlen < prefix->bitlen) {
        if (!ispnodeglue(start)) {
            if (patcompwithmask(&start->prefix, prefix, start->prefix.bitlen))
                res[i++] = &start->pub;
            else
                return res;
        }

        int bit = prefix->bytes[start->prefix.bitlen >> 3] & (0x80 >> (start->prefix.bitlen & 0x07));
        start = start->children[bit != 0];
    }

    pnode_t *node;
    pnode_t *stack[pt->maxbitlen+1];
    pnode_t **sp = stack;
    pnode_t *next = start;
    while ((node = next)) {
        if (!ispnodeglue(node)) {
            if (patcompwithmask(&node->prefix, prefix, prefix->bitlen))
                res[i++] = &node->pub;
            else
                break;
        }

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];

            next = next->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(sp--);
        } else {
            next = NULL;
        }
    }

    res[i] = NULL;

    return res;
}

UBGP_API bool patisrelatedof(const patricia_trie_t *pt,
                             const netaddr_t       *prefix)
{
    pnode_t *start = pt->head;
    while (start && start->prefix.bitlen < prefix->bitlen) {
        if (!ispnodeglue(start) && patcompwithmask(&start->prefix, prefix, start->prefix.bitlen))
            return true;

        int bit = prefix->bytes[start->prefix.bitlen >> 3] & (0x80 >> (start->prefix.bitlen & 0x07));
        start = start->children[bit != 0];
    }

    pnode_t *node;
    pnode_t *stack[pt->maxbitlen+1];
    pnode_t **sp = stack;
    pnode_t *next = start;
    while ((node = next)) {
        if (!ispnodeglue(node) && patcompwithmask(&node->prefix, prefix, prefix->bitlen))
            return true;

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];

            next = next->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(sp--);
        } else {
            next = NULL;
        }
    }

    return false;
}

UBGP_API u128 patcoverage(const patricia_trie_t *pt)
{
    u128 coverage = U128_ZERO;

    pnode_t *n;
    pnode_t *stack[pt->maxbitlen + 1];
    pnode_t **sp = stack;
    pnode_t *next = pt->head;

    while ((n = next)) {
        if (!ispnodeglue(n) && n->prefix.bitlen != 0) {
            coverage = u128add(coverage, u128shl(U128_ONE, (pt->maxbitlen - n->prefix.bitlen)));
            if (sp != stack)
                next = *(--sp);
            else
                next = NULL;

            continue;
        }

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];
            next = n->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(--sp);
        } else {
            next = NULL;
        }
    }

    return coverage;
}

UBGP_API trienode_t **patgetfirstsubnetsof(const patricia_trie_t *pt,
                                           const netaddr_t       *prefix)
{
    if (!pt->head)
        return NULL;

    pnode_t *node = pt->head;

    while (node && node->prefix.bitlen < prefix->bitlen) {
        int bit = prefix->bytes[node->prefix.bitlen >> 3] & (0x80 >> (node->prefix.bitlen & 0x07));
        node = node->children[bit != 0];
    }

    pnode_t *stack[pt->maxbitlen + 1];
    pnode_t **sp = stack;
    pnode_t *next = node;

    uint64_t n;
    if (prefix->family == AF_INET || prefix->bitlen >= 96) {
        n = (1ull << (pt->maxbitlen - prefix->bitlen));
        if (pt->nprefs < n)
            n = pt->nprefs;
    } else {
        n = pt->nprefs;
    }

    n++;

    trienode_t **res = malloc(n * sizeof(*res));
    if (unlikely(!res))
        return NULL;

    uint i = 0;
    while ((node = next)) {
        if (!ispnodeglue(node) && node->prefix.bitlen != 0) {
            res[i++] = &node->pub;
            if (sp != stack)
                next = *(--sp);
            else
                next = NULL;

            continue;
        }

        if (next->children[0]) {
            if (next->children[1])
                *sp++ = next->children[1];
            next = node->children[0];
        } else if (next->children[1]) {
            next = next->children[1];
        } else if (sp != stack) {
            next = *(--sp);
        } else {
            next = NULL;
        }
    }

    res[i] = NULL;

    return res;
}

/* Iterator */

static void patiteratormovenext(patiterator_t *state)
{
    pnode_t *l = state->curr->children[0];
    pnode_t *r = state->curr->children[1];

    if (l) {
        if (r)
            *state->sp++ = r;

        state->curr = l;
    } else if (r) {
        state->curr = r;
    } else if (state->sp != state->stack) {
        state->curr = *(--state->sp);
    } else {
        state->curr = NULL;
    }
}

UBGP_API void patiteratorskipglue(patiterator_t *state)
{
    while (state->curr && ispnodeglue(state->curr))
        patiteratormovenext(state);
}

UBGP_API void patiteratorinit(patiterator_t *state, const patricia_trie_t *pt)
{
    state->sp = state->stack;
    state->curr = pt->head;
    patiteratorskipglue(state);
}

UBGP_API trienode_t *patiteratorget(patiterator_t *state)
{
    return &state->curr->pub;
}

UBGP_API void patiteratornext(patiterator_t *state)
{
    patiteratormovenext(state);
    patiteratorskipglue(state);
}

UBGP_API bool patiteratorend(const patiterator_t *state)
{
    return (state->curr == NULL);
}

