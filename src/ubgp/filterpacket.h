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

#ifndef UBGP_FILTERPACKET_H_
#define UBGP_FILTERPACKET_H_

#include "bgp.h"
#include "mrt.h"
#include "patriciatrie.h"

#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

enum {
    K_MAX = 32,  // maximum user-defined filter constants

    KBASESIZ,  // base size for known variables

    KBUFSIZ = 64,
    STACKBUFSIZ = 256,

    BLKSTACKSIZ = 32
};

enum { VM_TMPTRIE, VM_TMPTRIE6 };

enum {
    VM_FUNCS_MAX = 16,

    VM_WITHDRAWN_INSERT_FN, VM_WITHDRAWN_ACCUMULATE_FN,
    VM_ALL_WITHDRAWN_INSERT_FN, VM_ALL_WITHDRAWN_ACCUMULATE_FN,
    VM_NLRI_INSERT_FN, VM_NLRI_ACCUMULATE_FN,
    VM_ALL_NLRI_INSERT_FN, VM_ALL_NLRI_ACCUMULATE_FN,

    VM_FUNCS_COUNT
};

/**
 * wide_as_t:
 *
 * A type able to hold any AS32 value, plus %AS_ANY.
 */
typedef llong wide_as_t;

/**
 * AS_ANY:
 *
 * Special value for filter operations that matches any AS32.
 */
#define AS_ANY -1

typedef union {
    netaddr_t addr;
    wide_as_t as;
    community_t comm;
    ex_community_t excomm;
    large_community_t lrgcomm;
    int value;
    struct {
        uint base;
        uint nels;
        uint elsiz;
    };
} stack_cell_t;

typedef uint16_t bytecode_t;

typedef struct filter_vm filter_vm_t;

typedef void (*filter_func_t)(filter_vm_t *vm);

enum {
    VM_SHORTCIRCUIT_FORCE_FLAG = 1 << 2
};

struct filter_vm {
    ubgp_msg_s *bgp;
    patricia_trie_t *curtrie, *curtrie6;
    stack_cell_t *sp, *kp;  // stack and constant segment pointers
    patricia_trie_t *tries;
    filter_func_t funcs[VM_FUNCS_COUNT];
    ushort flags;        // general VM flags

    /*< private >*/
    ushort pc;
    ushort si;           // stack index
    ushort access_mask;  // current packet access mask
    ushort ntries;
    ushort maxtries;
    ushort stacksiz;
    ushort codesiz;
    ushort maxcode;
    ushort ksiz;
    ushort maxk;
    ushort curblk;
    stack_cell_t stackbuf[STACKBUFSIZ];
    stack_cell_t kbuf[KBUFSIZ];
    patricia_trie_t triebuf[2];
    bytecode_t *code;
    void *heap;
    uint highwater;
    uint dynmarker;
    uint heapsiz;
    ubgp_err (*settle_func)(ubgp_msg_s *); // SETTLE function (if not NULL has to be called to terminate the iteration)
    int error;
    jmp_buf except;
};

typedef struct {
    int opidx;
    void *pkt_data;
    size_t datasiz;
} match_result_t;

enum {
    VM_OUT_OF_MEMORY    = -1,
    VM_STACK_OVERFLOW   = -2,
    VM_STACK_UNDERFLOW  = -3,
    VM_FUNC_UNDEFINED   = -4,
    VM_K_UNDEFINED      = -5,
    VM_BAD_ACCESSOR     = -6,
    VM_TRIE_MISMATCH    = -7,
    VM_TRIE_UNDEFINED   = -8,
    VM_PACKET_MISMATCH  = -9,
    VM_BAD_PACKET       = -10,
    VM_ILLEGAL_OPCODE   = -11,
    VM_DANGLING_BLK     = -12,
    VM_SPURIOUS_ENDBLK  = -13,
    VM_SURPRISING_BYTES = -14,
    VM_BAD_ARRAY        = -15
};

static inline char *filter_strerror(int err)
{
    if (err > 0)
        return "Pass";
    if (err == 0)
        return "Fail";

    switch (err) {
    case VM_OUT_OF_MEMORY:
        return "Out of memory";
    case VM_STACK_OVERFLOW:
        return "Stack overflow";
    case VM_STACK_UNDERFLOW:
        return "Stack underflow";
    case VM_FUNC_UNDEFINED:
        return "Reference to undefined function";
    case VM_K_UNDEFINED:
        return "Reference to undefined constant";
    case VM_BAD_ACCESSOR:
        return "Illegal packet accessor";
    case VM_TRIE_MISMATCH:
        return "Trie/Prefix family mismatch";
    case VM_TRIE_UNDEFINED:
        return "Reference to undefined trie";
    case VM_PACKET_MISMATCH:
        return "Mismatched packet type for this filter";
    case VM_BAD_PACKET:
        return "Packet corruption detected";
    case VM_ILLEGAL_OPCODE:
        return "Illegal instruction";
    case VM_DANGLING_BLK:
        return "Dangling BLK at execution end";
    case VM_SPURIOUS_ENDBLK:
        return "ENDBLK with no BLK";
    case VM_SURPRISING_BYTES:
        return "Sorry, I cannot make sense of these bytes";
    case VM_BAD_ARRAY:
        return "Array access out of bounds";
    default:
        return "<Unknown error>";
    }
}

UBGP_API CHECK_NONNULL(1) void filter_init(filter_vm_t *vm);

UBGP_API CHECK_NONNULL(1, 2) void filter_dump(FILE *f, filter_vm_t *vm);

UBGP_API CHECK_NONNULL(1, 2) int bgp_filter(ubgp_msg_s *msg, filter_vm_t *vm);

UBGP_API CHECK_NONNULL(1) void filter_destroy(filter_vm_t *vm);

#endif

