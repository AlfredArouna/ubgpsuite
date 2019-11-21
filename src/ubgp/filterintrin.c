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

#include "filterintrin.h"

#include <stddef.h>
#include <stdlib.h>

enum {
    K_GROW_STEP        = 32,
    STACK_GROW_STEP    = 128,
    HEAP_GROW_STEP     = 256,
    CODE_GROW_STEP     = 128,
    PATRICIA_GROW_STEP = 2
};

UBGP_API void vm_growstack(filter_vm_t *vm)
{
    stack_cell_t *stk = NULL;
    if (vm->sp != vm->stackbuf)
        stk = vm->sp;

    ushort stacksiz = vm->stacksiz + STACK_GROW_STEP;

    stk = realloc(stk, stacksiz * sizeof(*stk));
    if (unlikely(!stk))
        vm_abort(vm, VM_STACK_OVERFLOW);
    if (vm->sp == vm->stackbuf)
        memcpy(stk, vm->stackbuf, sizeof(vm->stackbuf));

    vm->sp       = stk;
    vm->stacksiz = stacksiz;
}

UBGP_API bool vm_growcode(filter_vm_t *vm)
{
    ushort codesiz   = vm->maxcode + CODE_GROW_STEP;
    bytecode_t *code = realloc(vm->code, codesiz * sizeof(*code));
    if (unlikely(!code))
        return false;

    vm->code    = code;
    vm->maxcode = codesiz;
    return true;
}

UBGP_API bool vm_growk(filter_vm_t *vm)
{
    ushort ksiz = vm->maxk + K_GROW_STEP;
    stack_cell_t *k = NULL;
    if (k != vm->kbuf)
        k = vm->kp;

    k = realloc(k, ksiz * sizeof(*k));
    if (unlikely(!k))
        return false;

    if (vm->kp == vm->kbuf)
        memcpy(k, vm->kp, sizeof(vm->kbuf));

    vm->kp   = k;
    vm->maxk = ksiz;
    return true;
}

UBGP_API bool vm_growtrie(filter_vm_t *vm)
{
    patricia_trie_t *tries = NULL;
    if (vm->tries != vm->triebuf)
        tries = vm->tries;

    ushort ntries = vm->maxtries + PATRICIA_GROW_STEP;
    tries = realloc(tries, ntries * sizeof(*tries));
    if (unlikely(!tries))
        return false;

    if (vm->tries == vm->triebuf)
        memcpy(tries, vm->tries, sizeof(vm->triebuf));

    vm->tries    = tries;
    vm->maxtries = ntries;
    return true;
}

UBGP_API void vm_exec_unpack(filter_vm_t *vm)
{
    stack_cell_t *cell = vm_pop(vm);
    vm_check_array(vm, cell);

    uint off   = cell->base;
    uint nels  = cell->nels;
    uint elsiz = cell->elsiz;
    while (vm->stacksiz < vm->si + nels)
        vm_growstack(vm);

    const byte *ptr = vm_heap_ptr(vm, off);
    for (uint i = 0; i < nels; i++) {
        memcpy(&vm->sp[vm->si++], ptr, elsiz);
        ptr += elsiz;
    }
}

UBGP_API void vm_exec_hasattr(filter_vm_t *vm, int code)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_exec_settle(vm);

    // optimize notable attributes
    bgpattr_t *ptr = NULL;
    switch (code) {
    case ORIGIN_CODE:
        ptr = getbgporigin(vm->bgp);
        break;
    case NEXT_HOP_CODE:
        ptr = getbgpnexthop(vm->bgp);
        break;
    case AGGREGATOR_CODE:
        ptr = getbgpaggregator(vm->bgp);
        break;
    case AS4_AGGREGATOR_CODE:
        ptr = getbgpas4aggregator(vm->bgp);
        break;
    case ATOMIC_AGGREGATE_CODE:
        ptr = getbgpatomicaggregate(vm->bgp);
        break;
    case AS_PATH_CODE:
        ptr = getbgpaspath(vm->bgp);
        break;
    case AS4_PATH_CODE:
        ptr = getbgpas4path(vm->bgp);
        break;
    case MP_REACH_NLRI_CODE:
        ptr = getbgpmpreach(vm->bgp);
        break;
    case MP_UNREACH_NLRI_CODE:
        ptr = getbgpmpunreach(vm->bgp);
        break;
    case COMMUNITY_CODE:
        ptr = getbgpcommunities(vm->bgp);
        break;
    case EXTENDED_COMMUNITY_CODE:
        ptr = getbgpexcommunities(vm->bgp);
        break;
    case LARGE_COMMUNITY_CODE:
        ptr = getbgplargecommunities(vm->bgp);
        break;
    default:
        // no luck, plain iteration
        startbgpattribs(vm->bgp);
        while ((ptr = nextbgpattrib(vm->bgp)) != NULL) {
            if (ptr->code == code)
                break;
        }
        endbgpattribs(vm->bgp);
    }

    assert(ptr == NULL || ptr->code == code);

    vm_pushvalue(vm, ptr != NULL);
}

UBGP_API void vm_prepare_addr_access(filter_vm_t *vm, ushort mode)
{
    if (mode & FOPC_ACCESS_SETTLE)
        vm_exec_settle(vm);
    if (vm->access_mask == mode)
        return;

    switch (mode & ~FOPC_ACCESS_SETTLE) {
    case FOPC_ACCESS_WITHDRAWN | FOPC_ACCESS_ALL:
        startallwithdrawn(vm->bgp);
        vm->settle_func = endwithdrawn;
        break;
    case FOPC_ACCESS_WITHDRAWN:
        startwithdrawn(vm->bgp);
        vm->settle_func = endwithdrawn;
        break;
    case FOPC_ACCESS_NLRI | FOPC_ACCESS_ALL:
        startallnlri(vm->bgp);
        vm->settle_func = endnlri;
        break;
    case FOPC_ACCESS_NLRI:
        startnlri(vm->bgp);
        vm->settle_func = endnlri;
        break;
    default:
        // shouldn't be possible...
        vm_abort(vm, VM_BAD_ACCESSOR);
        break;
    }

    vm->access_mask = mode;
}

UBGP_API void vm_prepare_as_access(filter_vm_t *vm, ushort mode)
{
    if (mode & FOPC_ACCESS_SETTLE)
        vm_exec_settle(vm);
    if (vm->access_mask == mode)
        return;

    switch (mode & ~FOPC_ACCESS_SETTLE) {
    case FOPC_ACCESS_AS_PATH:
        startaspath(vm->bgp);
        break;
    case FOPC_ACCESS_AS4_PATH:
        startas4path(vm->bgp);
        break;
    case FOPC_ACCESS_REAL_AS_PATH:
        startrealaspath(vm->bgp);
        break;
    default:
        // shouldn't be possible...
        vm_abort(vm, VM_BAD_ACCESSOR);
        break;
    }

    vm->settle_func = endaspath;
    vm->access_mask = mode;
}

UBGP_API void vm_exec_exact(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_addr_access(vm, access);

    bool result = false;
    while (true) {
        netaddr_t *addr = (access & FOPC_ACCESS_NLRI) ?
                          nextnlri(vm->bgp) :
                          nextwithdrawn(vm->bgp);
        if (!addr)
            break;

        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (patsearchexact(trie, addr)) {
                result = true;
                goto done;
            }

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
            break;
        }
    }

done:
    vm_pushvalue(vm, result);
}

UBGP_API void vm_exec_subnet(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_addr_access(vm, access);

    bool result = false;
    while (true) {
        netaddr_t *addr = (access & FOPC_ACCESS_NLRI) ?
                          nextnlri(vm->bgp) :
                          nextwithdrawn(vm->bgp);
        if (!addr)
            break;

        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (patissubnetof(trie, addr)) {
                result = true;
                goto done;
            }

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
            break;
        }
    }

done:
    vm_pushvalue(vm, result);
}

UBGP_API void vm_exec_supernet(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_addr_access(vm, access);

    bool result = false;
    while (true) {
        netaddr_t *addr = (access & FOPC_ACCESS_NLRI) ?
                    nextnlri(vm->bgp) :
                    nextwithdrawn(vm->bgp);
        if (!addr)
            break;

        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (patissupernetof(trie, addr)) {
                result = true;
                goto done;
            }

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
            break;
        }
    }

done:
    vm_pushvalue(vm, result);
}


UBGP_API void vm_exec_related(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_addr_access(vm, access);

    bool result = false;
    while (true) {
        netaddr_t *addr = (access & FOPC_ACCESS_NLRI) ?
                    nextnlri(vm->bgp) :
                    nextwithdrawn(vm->bgp);
        if (!addr)
            break;

        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (patisrelatedof(trie, addr)) {
                result = true;
                goto done;
            }

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
            break;
        }
    }

done:
    vm_pushvalue(vm, result);
}

UBGP_API void vm_exec_aspmatch(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_as_access(vm, access);

    uint32_t asbuf[vm->si];
    int buffered = 0;

    as_pathent_t *ent;
    while (true) {
        int i;

        for (i = 0; i < vm->si; i++) {
            stack_cell_t *cell = &vm->sp[i];
            if (i == buffered) {
                // read one more AS from packet
                ent = nextaspath(vm->bgp);
                if (!ent) {
                    // doesn't match
                    vm_clearstack(vm);
                    vm->sp[vm->si++].value = false;
                    return;
                }

                asbuf[buffered++] = ent->as;
            }
            if (cell->as != asbuf[i] && cell->as != AS_ANY)
                break;
        }

        if (i == vm->si) {
            // successfuly matched
            vm_clearstack(vm);
            vm->sp[vm->si++].value = true;
            return;
        }

        // didn't match, consume the first element in asbuf and continue trying
        memmove(asbuf, asbuf + 1, (buffered - 1) * sizeof(*asbuf));
        buffered--;
    }
}

UBGP_API void vm_exec_aspstarts(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_as_access(vm, access);

    int i;
    for (i = 0; i < vm->si; i++) {
        as_pathent_t *ent = nextaspath(vm->bgp);
        if (!ent)
            break;
        if (vm->sp[i].as != ent->as && vm->sp[i].as != AS_ANY)
            break;  // does not match
    }

    bool value = (i == vm->si);

    vm_clearstack(vm);
    vm->sp[vm->si++].value = value;
}

UBGP_API void vm_exec_aspends(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_as_access(vm, access);

    uint32_t asbuf[vm->si];
    int n = 0;

    as_pathent_t *ent;
    while ((ent = nextaspath(vm->bgp)) != NULL) {
        if (n == vm->si) {
            // slide AS path window
            memmove(asbuf, asbuf + 1, (n - 1) * sizeof(*asbuf));
            n--;
        }

        asbuf[n++] = ent->as;
    }

    int length_match = (n == vm->si);

    vm_clearstack(vm);

    if (!length_match) {
        vm->sp[vm->si++].value = false;
        return;
    }

    // NOTE: technically we cleared the stack, but we know it was N elements long before...
    int i;
    for (i = 0; i < n; i++) {
        if (asbuf[i] != vm->sp[i].as && vm->sp[i].as != AS_ANY)
            break;
    }

    vm->sp[vm->si++].value = (i == n);
}

UBGP_API void vm_exec_aspexact(filter_vm_t *vm, uint access)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    vm_prepare_as_access(vm, access);

    as_pathent_t *ent;
    int i;
    for (i = 0; i < vm->si; i++) {
        ent = nextaspath(vm->bgp);
        if (!ent)
            break;
        if (vm->sp[i].as != ent->as && vm->sp[i].as != AS_ANY)
            break;  // does not match
    }

    ent = nextaspath(vm->bgp);

    bool value = (ent == NULL && i == vm->si);

    vm_clearstack(vm);
    vm->sp[vm->si++].value = value;
}

UBGP_API void vm_exec_commexact(filter_vm_t *vm)
{
    if (getbgptype(vm->bgp) != BGP_UPDATE)
        vm_abort(vm, VM_PACKET_MISMATCH);

    startcommunities(vm->bgp, COMMUNITY_CODE);

    bool seen[vm->si];
    int seen_count = 0;

    memset(seen, 0, vm->si * sizeof(*seen));

    community_t *comm;
    while ((comm = nextcommunity(vm->bgp)) != NULL && seen_count != vm->si) {
        for (int i = 0; i < vm->si; i++) {
            if (*comm == vm->sp[i].comm) {
                if (!seen[i]) {
                    seen[i] = true;
                    seen_count++;
                }

                break;
            }
        }
    }

    endcommunities(vm->bgp);

    int value = (seen_count == vm->si);

    vm_clearstack(vm);
    vm->sp[vm->si++].value = value;
}

UBGP_API void vm_emit_ex(filter_vm_t *vm, int opcode, int idx)
{
    // NOTE: paranoid, assumes 8 bit bytes... as the whole ubgp does

    // find the most significant byte
    int msb = (sizeof(idx) - 1) * 8;
    while (msb > 0) {
        if ((idx >> msb) & 0xff)
            break;

        msb -= 8;
    }

    // emit value most-significant byte first
    while (msb > 0) {
        vm_emit(vm, vm_makeop(FOPC_EXARG, (idx >> msb) & 0xff));
        msb -= 8;
    }
    // emit the instruction with the least-signficiant byte
    vm_emit(vm, vm_makeop(opcode, idx & 0xff));
}

static void *vm_heap_ensure(filter_vm_t *vm, size_t aligned_size)
{
    size_t used = vm->highwater;
    used += vm->dynmarker;
    if (likely(vm->heapsiz - used >= aligned_size))
        return vm->heap;

    aligned_size += used;
    aligned_size += HEAP_GROW_STEP;
    void *heap = realloc(vm->heap, aligned_size);
    if (unlikely(!heap))
        return NULL;

    vm->heap    = heap;
    vm->heapsiz = aligned_size;
    return heap;
}

UBGP_API intptr_t vm_heap_alloc(filter_vm_t    *vm,
                                size_t          size,
                                vm_heap_zone_t  zone)
{
    // align allocation
    size += sizeof(max_align_t) - 1;
    size -= (size & (sizeof(max_align_t) - 1));

    intptr_t ptr;
    if (unlikely(!vm_heap_ensure(vm, size)))
        return VM_BAD_HEAP_PTR;

    switch (zone) {
    case VM_HEAP_PERM:
        if (unlikely(vm->dynmarker > 0)) {
            assert(false);
            return VM_BAD_HEAP_PTR; // illegal!
        }

        ptr = vm->highwater;
        vm->highwater += size;
        return ptr;

    case VM_HEAP_TEMP:
        ptr = vm->highwater;
        ptr += vm->dynmarker;

        vm->dynmarker += size;
        return ptr;

    default:
        // should never happen
        assert(false);
        return VM_BAD_HEAP_PTR;
    }
}

UBGP_API intptr_t vm_heap_grow(filter_vm_t *vm, intptr_t addr, size_t newsize)
{
    newsize += sizeof(max_align_t) - 1;
    newsize -= (newsize & (sizeof(max_align_t) - 1));

    if (addr == 0)
        addr = vm->highwater; // never did a VM_HEAP_TEMP alloc before

    size_t oldsize = vm->highwater + vm->dynmarker - addr;
    if (unlikely(newsize < oldsize))
        return addr;

    size_t amount = newsize - oldsize;
    if (unlikely(!vm_heap_ensure(vm, amount)))
        return VM_BAD_HEAP_PTR;

    vm->dynmarker += amount;
    return addr;
}

