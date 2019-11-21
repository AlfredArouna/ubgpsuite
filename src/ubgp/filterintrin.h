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

#ifndef UBGP_FILTERINTRIN_H_
#define UBGP_FILTERINTRIN_H_

#include "filterpacket.h"
#include "netaddr.h"

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <string.h>


/**
 * SECTION: filterintrin
 * @title: Filter VM Intrinsics
 * @include: filterintrin.h
 *
 * Filter Virtual Machine intrinsics, low level access to the packet
 * filtering engine.
 */

/**
 * @FOPC_NOP:      NOP - no operation, does nothing
 * @FOPC_BLK:      NONE - push a new block in block stack
 * @FOPC_ENDBLK:   NONE - mark the end of the latest opened block
 * @FOPC_LOAD:     PUSH - direct value load
 * @FOPC_LOADK:    PUSH - load from constants environment
 * @FOPC_UNPACK:   POP-PUSH - unpack an array constant into stack.
 * @FOPC_EXARG:    NOP - extends previous operation's argument range
 * @FOPC_STORE:    POP - store address into current trie (v4 or v6 depends on address)
 * @FOPC_DISCARD:  POP - remove address from current trie (v4 or v6 depends on address)
 * @FOPC_NOT:      POP-PUSH - pops stack topmost value, negates it and pushes it back.
 * @FOPC_CPASS:    POP  pops topmost stack element and terminates with PASS if value is %true.
 * @FOPC_CFAIL:    POP - pops topmost stack element and terminates with FAIL if value is %false.
 * @FOPC_SETTLE:   NONE - forcefully close an iteration sequence
 * @FOPC_HASATTR:  push %true if attribute is present, %false otherwise.
 * @FOPC_EXACT:    pops the entire stack and verifies that at least one *address* has an *exact* relationship with
 *                 the addresses stored inside the current tries, pushes a boolean result.
 *                 This opcode expects that the entire stack is composed of cells containing #netaddr_t.
 *                 Stack operation mode is POPA-PUSH, this opcode has an
 *                 announce/withdrawn accessor argument.
 * @FOPC_PFXCONTAINS: POPA-PUSH - prefix contained
 * @FOPC_ASPMATCH: pops the entire stack and verifies that each AS in the stack
 *                 appears within the PATH field identified by this instruction
 *                 argument. This opcode expects that the entire stack is
 *                 composed of cells containing #wide_as_t.
 *
 * @note Stack operation mode is POPA-PUSH, this opcode has an AS PATH accessor argument.
 *
 * @FOPC_CALL: ??? - call a function
 *
 * Filter Virtual Machine opcodes.
 */
enum {
    BAD_OPCODE = -1,

    FOPC_NOP,

    FOPC_BLK,
    FOPC_ENDBLK,
    FOPC_LOAD,
    FOPC_LOADK,
    FOPC_UNPACK,
    FOPC_EXARG,
    FOPC_STORE,
    FOPC_DISCARD,
    FOPC_NOT,
    FOPC_CPASS,
    FOPC_CFAIL,

    FOPC_SETTLE,

    FOPC_HASATTR,

    FOPC_EXACT,
    FOPC_SUBNET,
    FOPC_SUPERNET,
    FOPC_RELATED,

    FOPC_PFXCONTAINS,
    FOPC_ADDRCONTAINS,
    FOPC_ASCONTAINS,

    FOPC_ASPMATCH,
    FOPC_ASPSTARTS,
    FOPC_ASPENDS,
    FOPC_ASPEXACT,
    
    FOPC_COMMEXACT,

    FOPC_CALL,
    FOPC_SETTRIE,
    FOPC_SETTRIE6,
    FOPC_CLRTRIE,
    FOPC_CLRTRIE6,
    FOPC_PFXCMP,
    FOPC_ADDRCMP,
    FOPC_ASCMP,

    OPCODES_COUNT
};

// operator accessors (8 bits)
enum {
    FOPC_ACCESS_SETTLE        = 1 << 7,  // General flag to rewind the iterator (equivalent to call settle from the beginning)

    // NLRI/Withdrawn accessors
    FOPC_ACCESS_NLRI          = 1 << 0,
    FOPC_ACCESS_WITHDRAWN     = 1 << 1,
    FOPC_ACCESS_ALL           = 1 << 2,

    // AS access
    FOPC_ACCESS_AS_PATH       = 1 << 0,
    FOPC_ACCESS_AS4_PATH      = 1 << 1,
    FOPC_ACCESS_REAL_AS_PATH  = 1 << 2,
    
    // community access
    FOPC_ACCESS_COMM          = 1 << 0
};

/**
 * vm_growstack:
 * @vm: an initialized #filter_vm_t.
 *
 * Grow @vm stack segment.
 *
 * This function is called only during VM execution stage,
 * hence it doesn't return a value on failure, but rather
 * triggers vm_abort().
 */
UBGP_API CHECK_NONNULL(1) void vm_growstack(filter_vm_t *vm);

/**
 * vm_growcode:
 * @vm: an initialized #filter_vm_t.
 *
 * Grow @vm bytecode instructions segment.
 *
 * Returns: %true on success, %false on out of memory.
 * On failure @vm is left unchanged.
 */
UBGP_API CHECK_NONNULL(1) bool vm_growcode(filter_vm_t *vm);

/**
 * vm_growk:
 * @vm: an initialized #filter_vm_t.
 *
 * Grow @vm constant segment.
 *
 * Returns: %true on success, %false on out of memory.
 * On failure @vm is left unchanged.
 */
UBGP_API CHECK_NONNULL(1) bool vm_growk(filter_vm_t *vm);

/**
 * vm_growtrie:
 * @vm: an initialized #filter_vm_t.
 *
 * Grow @vm allocated PATRICIA trie segment.
 *
 * Returns: %true on success, %false on out of memory.
 * On failure @vm is left unchanged.
 */
UBGP_API CHECK_NONNULL(1) bool vm_growtrie(filter_vm_t *vm);

static inline noreturn CHECK_NONNULL(1) void vm_abort(filter_vm_t *vm,
                                                      int          error)
{
    vm->error = error;
    longjmp(vm->except, -1);
}

static inline bytecode_t vm_makeop(int opcode, int arg)
{
    return ((arg << 8) & 0xff00) | (opcode & 0xff);
}

static inline int vm_getopcode(bytecode_t code)
{
    return code & 0xff;
}

static inline int vm_getarg(bytecode_t code)
{
    return code >> 8;
}

static inline int vm_extendarg(int arg, int exarg)
{
    return ((exarg << 8) | arg) & 0x7fffffff;
}

/// @brief Reserve one constant (avoids the user custom section).
static inline CHECK_NONNULL(1) int vm_newk(filter_vm_t *vm)
{
    if (unlikely(vm->ksiz == vm->maxk)) {
        if (unlikely(!vm_growk(vm)))
            return -1;
    }
    return vm->ksiz++;
}

static inline int vm_newtrie(filter_vm_t *vm, sa_family_t family)
{
    if (unlikely(vm->ntries == vm->maxtries)) {
        if (unlikely(!vm_growtrie(vm)))
            return -1;
    }

    int idx = vm->ntries++;
    patinit(&vm->tries[idx], family);
    return idx;
}

/**
 * vm_emit:
 * @vm:     an inactive #filter_vm_t
 * @opcode: an instruction
 *
 * Emit one bytecode operation.
 *
 * Returns: %true on success, %false on out of memory.
 */
static inline bool vm_emit(filter_vm_t *vm, bytecode_t opcode)
{
    if (unlikely(vm->codesiz == vm->maxcode)) {
        if (unlikely(!vm_growcode(vm)))
            return false;
    }

    vm->code[vm->codesiz++] = opcode;
    return true;
}

UBGP_API CHECK_NONNULL(1) void vm_emit_ex(filter_vm_t *vm,
                                          int          opcode,
                                          int          idx);

// Virtual Machine dynamic memory:

typedef enum { VM_HEAP_PERM, VM_HEAP_TEMP } vm_heap_zone_t;

#define VM_BAD_HEAP_PTR -1

UBGP_API CHECK_NONNULL(1) intptr_t vm_heap_alloc(filter_vm_t    *vm,
                                                 size_t          size,
                                                 vm_heap_zone_t  zone);

/**
 * Only valid for the last allocated %VM_HEAP_TEMP chunk!!!
 */
static inline CHECK_NONNULL(1) void vm_heap_return(filter_vm_t* vm,
                                                   size_t       size)
{
    // align allocation
    size += sizeof(max_align_t) - 1;
    size -= (size & (sizeof(max_align_t) - 1));

    assert(vm->dynmarker >= size);

    vm->dynmarker -= size;
}

/**
 * Only valid for the last allocated %VM_HEAP_TEMP chunk!!!
 */
UBGP_API CHECK_NONNULL(1) intptr_t vm_heap_grow(filter_vm_t *vm,
                                                intptr_t     addr,
                                                size_t       newsize);


// General Virtual Machine operations:

static inline CHECK_NONNULL(1) void vm_clearstack(filter_vm_t *vm)
{
    vm->si = 0;
}

static inline CHECK_NONNULL(1) stack_cell_t *vm_peek(filter_vm_t *vm)
{
    if (unlikely(vm->si == 0))
        vm_abort(vm, VM_STACK_UNDERFLOW);

    return &vm->sp[vm->si - 1];
}

static inline CHECK_NONNULL(1) stack_cell_t *vm_pop(filter_vm_t *vm)
{
    if (unlikely(vm->si == 0))
        vm_abort(vm, VM_STACK_UNDERFLOW);

    return &vm->sp[--vm->si];
}

static inline CHECK_NONNULL(1, 2) void vm_push(filter_vm_t        *vm,
                                               const stack_cell_t *cell)
{
    if (unlikely(vm->si == vm->stacksiz))
        vm_growstack(vm);

    memcpy(&vm->sp[vm->si++], cell, sizeof(*cell));
}

static inline CHECK_NONNULL(1, 2) void vm_pushaddr(filter_vm_t     *vm,
                                                   const netaddr_t *addr)
{
    // especially optimized, since it's very common
    if (unlikely(vm->si == vm->stacksiz))
        vm_growstack(vm);

    memcpy(&vm->sp[vm->si++].addr, addr, sizeof(*addr));
}

static inline CHECK_NONNULL(1) void vm_pushvalue(filter_vm_t *vm, int value)
{
    // optimized, since it's common
    if (unlikely(vm->si == vm->stacksiz))
        vm_growstack(vm);

    vm->sp[vm->si++].value = value;
}

static inline CHECK_NONNULL(1) void vm_pushas(filter_vm_t *vm, wide_as_t as)
{
    if (unlikely(vm->si == vm->stacksiz))
        vm_growstack(vm);

    vm->sp[vm->si++].as  = as;
}

static inline CHECK_NONNULL(1) void vm_exec_loadk(filter_vm_t *vm, int kidx)
{
    if (unlikely(kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    vm_push(vm, &vm->kp[kidx]);
}

/**
 * vm_exec_break:
 * @vm: a #filter_vm_t in execution mode
 *
 * Breaks from the current `BLK`, leaves `vm->pc` at the corresponding `ENDBLK`,
 * `vm->pc` is assumed to be inside the `BLK`.
 */
static inline CHECK_NONNULL(1) void vm_exec_break(filter_vm_t *vm)
{
    uint nblk = 1; // encountered blocks

    while (vm->pc < vm->codesiz) {
        if (vm->code[vm->pc] == FOPC_ENDBLK)
            nblk--;
        if (vm->code[vm->pc] == FOPC_BLK)
            nblk++;

        if (nblk == 0)
            break;

        vm->pc++;
    }
}

static inline CHECK_NONNULL(1) void vm_exec_not(filter_vm_t *vm)
{
    stack_cell_t *cell = vm_peek(vm);
    cell->value = !cell->value;
}

static inline CHECK_NONNULL(1) void *vm_heap_ptr(filter_vm_t *vm,
                                                 intptr_t     offset)
{
    assert(offset >= 0);

    return (byte *) vm->heap + offset;  // duh...
}

static inline CHECK_NONNULL(1, 2) void vm_check_array(filter_vm_t        *vm,
                                                      const stack_cell_t *arr)
{
    size_t bound = arr->base;
    bound += arr->nels * arr->elsiz;

    if (unlikely(arr->elsiz > sizeof(stack_cell_t) || bound > vm->heapsiz))
        vm_abort(vm, VM_BAD_ARRAY);
}

UBGP_API CHECK_NONNULL(1) void vm_exec_unpack(filter_vm_t *vm);

static inline CHECK_NONNULL(1) void vm_exec_store(filter_vm_t *vm)
{
    stack_cell_t *cell    = vm_pop(vm);
    netaddr_t *addr       = &cell->addr;
    patricia_trie_t *trie = vm->curtrie;
    switch (addr->family) {
    case AF_INET6:
        trie = vm->curtrie6;
        FALLTHROUGH;
    case AF_INET:
        if (!patinsert(trie, addr, NULL))
            vm_abort(vm, VM_OUT_OF_MEMORY);

        break;
    default:
        vm_abort(vm, VM_SURPRISING_BYTES); // should never happen
        break;
    }
}

static inline CHECK_NONNULL(1) void vm_exec_discard(filter_vm_t *vm)
{
    stack_cell_t *cell    = vm_pop(vm);
    netaddr_t *addr       = &cell->addr;
    patricia_trie_t *trie = vm->curtrie;
    switch (addr->family) {
    case AF_INET6:
        trie = vm->curtrie6;
        FALLTHROUGH;
    case AF_INET:
        patremove(trie, addr);
        break;
    default:
        vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
        break;
    }
}

static inline CHECK_NONNULL(1) void vm_exec_clrtrie(filter_vm_t *vm)
{
    patclear(vm->curtrie);
}

static inline CHECK_NONNULL(1) void vm_exec_clrtrie6(filter_vm_t *vm)
{
    patclear(vm->curtrie6);
}

static inline CHECK_NONNULL(1) void vm_exec_settrie(filter_vm_t *vm, int trie)
{
    if (unlikely((uint) trie >= vm->ntries))
        vm_abort(vm, VM_TRIE_UNDEFINED);

    vm->curtrie = &vm->tries[trie];
    if (unlikely(vm->curtrie->maxbitlen != 32))
        vm_abort(vm, VM_TRIE_MISMATCH);
}

static inline CHECK_NONNULL(1) void vm_exec_settrie6(filter_vm_t *vm, int trie6)
{
    if (unlikely((uint) trie6 >= vm->ntries))
        vm_abort(vm, VM_TRIE_UNDEFINED);

    vm->curtrie6 = &vm->tries[trie6];
    if (unlikely(vm->curtrie6->maxbitlen != 128))
        vm_abort(vm, VM_TRIE_MISMATCH);
}

static inline CHECK_NONNULL(1) void vm_exec_ascmp(filter_vm_t *vm, int kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = vm_peek(vm);
    stack_cell_t *b = &vm->kp[kidx];

    a->value = (a->as == b->as);
}

static inline CHECK_NONNULL(1) void vm_exec_addrcmp(filter_vm_t *vm, int kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = vm_peek(vm);
    stack_cell_t *b = &vm->kp[kidx];
    a->value = naddreq(&a->addr, &b->addr);
}

static inline CHECK_NONNULL(1) void vm_exec_pfxcmp(filter_vm_t *vm, int kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = vm_peek(vm);
    stack_cell_t *b = &vm->kp[kidx];
    a->value = prefixeq(&a->addr, &b->addr);
}

static inline CHECK_NONNULL(1) void vm_exec_settle(filter_vm_t *vm)
{
    if (vm->settle_func) {
        vm->settle_func(vm->bgp);
        vm->settle_func = NULL;

        vm->access_mask = 0;
    }
}

UBGP_API CHECK_NONNULL(1) void vm_exec_hasattr(filter_vm_t *vm, int code);

static inline CHECK_NONNULL(1)
void vm_exec_all_withdrawn_insert(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startallwithdrawn(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextwithdrawn(vm->bgp)) != NULL) {
        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            FALLTHROUGH;
        case AF_INET:
            if (!patinsert(trie, addr, NULL))
                vm_abort(vm, VM_OUT_OF_MEMORY);

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES);  // should never happen
            break;
        }
    }

    if (unlikely(endwithdrawn(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1)
void vm_exec_all_withdrawn_accumulate(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startallwithdrawn(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextwithdrawn(vm->bgp)) != NULL)
        vm_pushaddr(vm, addr);

    if (unlikely(endwithdrawn(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1)
void vm_exec_withdrawn_accumulate(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startwithdrawn(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextwithdrawn(vm->bgp)) != NULL)
        vm_pushaddr(vm, addr);

    if (unlikely(endwithdrawn(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline void CHECK_NONNULL(1) vm_exec_withdrawn_insert(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startwithdrawn(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextwithdrawn(vm->bgp)) != NULL) {
        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            FALLTHROUGH;
        case AF_INET:
            if (!patinsert(trie, addr, NULL))
                vm_abort(vm, VM_OUT_OF_MEMORY);

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES); // should never happen
            break;
        }
    }

    if (unlikely(endwithdrawn(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1) void vm_exec_all_nlri_insert(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startallnlri(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextnlri(vm->bgp)) != NULL) {
        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (!patinsert(trie, addr, NULL))
                vm_abort(vm, VM_OUT_OF_MEMORY);

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES); // should never happen
            break;
        }
    }

    if (unlikely(endnlri(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1)
void vm_exec_all_nlri_accumulate(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startallnlri(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextnlri(vm->bgp)) != NULL)
        vm_pushaddr(vm, addr);

    if (unlikely(endnlri(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1)
void vm_exec_nlri_insert(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startnlri(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextnlri(vm->bgp)) != NULL) {
        patricia_trie_t *trie = vm->curtrie;
        switch (addr->family) {
        case AF_INET6:
            trie = vm->curtrie6;
            // fallthrough
        case AF_INET:
            if (!patinsert(trie, addr, NULL))
                vm_abort(vm, VM_OUT_OF_MEMORY);

            break;
        default:
            vm_abort(vm, VM_SURPRISING_BYTES); // should never happen
            break;
        }
    }

    if (unlikely(endnlri(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

static inline CHECK_NONNULL(1)
void vm_exec_nlri_accumulate(filter_vm_t *vm)
{
    if (unlikely(getbgptype(vm->bgp) != BGP_UPDATE))
        vm_abort(vm, VM_PACKET_MISMATCH);

    startnlri(vm->bgp);

    const netaddr_t *addr;
    while ((addr = nextnlri(vm->bgp)) != NULL)
        vm_pushaddr(vm, addr);

    if (unlikely(endnlri(vm->bgp) != BGP_ENOERR))
        vm_abort(vm, VM_BAD_PACKET);
}

UBGP_API CHECK_NONNULL(1) void vm_exec_exact(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_subnet(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_supernet(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_related(filter_vm_t *vm, uint access);

static inline CHECK_NONNULL(1) void vm_exec_pfxcontains(filter_vm_t *vm,
                                                        int          kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = &vm->kp[kidx];
    while (vm->si > 0) {
        stack_cell_t *b = vm_pop(vm);

        if (prefixeq(&a->addr, &b->addr)) {
            vm_clearstack(vm);
            vm_pushvalue(vm, true);
            return;
        }
    }

    vm_pushvalue(vm, false);
}

static inline CHECK_NONNULL(1) void vm_exec_addrcontains(filter_vm_t *vm,
                                                         int          kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = &vm->kp[kidx];
    while (vm->si > 0) {
        stack_cell_t *b = vm_pop(vm);

        if (naddreq(&a->addr, &b->addr)) {
            vm_clearstack(vm);
            vm_pushvalue(vm, true);
            return;
        }
    }

    vm_pushvalue(vm, false);
}

static inline CHECK_NONNULL(1) void vm_exec_ascontains(filter_vm_t *vm,
                                                       int          kidx)
{
    if (unlikely((uint) kidx >= vm->ksiz))
        vm_abort(vm, VM_K_UNDEFINED);

    stack_cell_t *a = &vm->kp[kidx];
    while (vm->si > 0) {
        stack_cell_t *b = vm_pop(vm);
        if (a->as == b->as) {
            vm_clearstack(vm);
            vm_pushvalue(vm, true);
            return;
        }
    }

    vm_pushvalue(vm, false);
}

UBGP_API CHECK_NONNULL(1) void vm_exec_aspmatch(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_aspstarts(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_aspends(filter_vm_t *vm, uint access);
UBGP_API CHECK_NONNULL(1) void vm_exec_aspexact(filter_vm_t *vm, uint access);

UBGP_API CHECK_NONNULL(1) void vm_exec_commexact(filter_vm_t *vm);

#endif

