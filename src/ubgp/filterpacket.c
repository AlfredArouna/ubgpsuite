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
#include "filterpacket.h"

#include <limits.h>
#include <stdlib.h>

UBGP_API void filter_destroy(filter_vm_t *vm)
{
    for (uint i = 0; i < vm->ntries; i++)
        patdestroy(&vm->tries[i]);

    if (vm->tries != vm->triebuf)
        free(vm->tries);
    if (vm->kp != vm->kbuf)
        free(vm->kp);
    if (vm->sp != vm->stackbuf)
        free(vm->sp);

    free(vm->code);
    free(vm->heap);
}

UBGP_API void filter_init(filter_vm_t *vm)
{
    memset(vm, 0, sizeof(*vm));
    vm->sp    = vm->stackbuf;
    vm->kp    = vm->kbuf;
    vm->tries = vm->triebuf;
    vm->stacksiz = countof(vm->stackbuf);
    vm->ksiz     = KBASESIZ;  // reserved for known variables
    vm->maxk     = countof(vm->kbuf);
    vm->ntries   = 2;  // 2 temporary tries
    vm->maxtries = countof(vm->triebuf);
    vm->funcs[VM_WITHDRAWN_INSERT_FN]         = vm_exec_withdrawn_insert;
    vm->funcs[VM_WITHDRAWN_ACCUMULATE_FN]     = vm_exec_withdrawn_accumulate;
    vm->funcs[VM_ALL_WITHDRAWN_INSERT_FN]     = vm_exec_all_withdrawn_insert;
    vm->funcs[VM_ALL_WITHDRAWN_ACCUMULATE_FN] = vm_exec_all_withdrawn_accumulate;
    vm->funcs[VM_NLRI_INSERT_FN]              = vm_exec_nlri_insert;
    vm->funcs[VM_NLRI_ACCUMULATE_FN]          = vm_exec_nlri_accumulate;
    vm->funcs[VM_ALL_NLRI_INSERT_FN]          = vm_exec_all_nlri_insert;
    vm->funcs[VM_ALL_NLRI_ACCUMULATE_FN]      = vm_exec_all_nlri_accumulate;

    patinit(&vm->tries[VM_TMPTRIE],  AF_INET);
    patinit(&vm->tries[VM_TMPTRIE6], AF_INET6);
}

UBGP_API int bgp_filter(ubgp_msg_s *msg, filter_vm_t *vm)
{
// disable pedantic diagnostic about taking address label
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"

#if defined(__GNUC__) && !defined(ISOLARIO_VM_PLAIN_SWITCH)
// use computed goto to speed-up bytecode interpreter
#include "vm_opcodes.h"

#define FETCH() ip = vm->code[vm->pc++];                                           \
                np = likely(vm->pc < vm->codesiz) ? vm->code[vm->pc] : BAD_OPCODE; \
                (void) np;                                                         \
                goto *vm_opcode_table[vm_getopcode(ip)]

#define PREDICT(opcode) do {                                                     \
        if (likely(vm_getopcode(np) == FOPC_##opcode)) {                         \
            ip = np;                                                             \
            np = likely(vm->pc < vm->codesiz) ? vm->code[++vm->pc] : BAD_OPCODE; \
            goto *vm_opcode_table[FOPC_##opcode];                                \
        }                                                                        \
    } while (false)

#define DISPATCH() break

#define EXECUTE(opcode) case FOPC_##opcode: \
                        EX_##opcode

#define EXECUTE_SIGILL default: \
                       EX_SIGILL

#else
// portable C

#define FETCH() ip = vm->code[vm->pc++];                                           \
                np = likely(vm->pc < vm->codesiz) ? vm->code[vm->pc] : BAD_OPCODE; \
                (void) np

#define PREDICT(opcode) do (void) 0; while (false)

#define DISPATCH() break

#define EXECUTE(opcode) case FOPC_##opcode

#define EXECUTE_SIGILL  default
#endif

    if (setjmp(vm->except) != 0) {
        // TODO cleanup temporary patricias!
        vm_exec_settle(vm);
        return vm->error;
    }

    bytecode_t ip, np;
    int arg, exarg;
    stack_cell_t *cell;

    vm->bgp    = msg;
    vm->pc     = 0;
    vm->curblk = 0;
    vm_clearstack(vm);
    vm->dynmarker = 0;
    vm->error     = 0;
    exarg         = 0;

    vm_exec_settrie(vm, VM_TMPTRIE);
    vm_exec_settrie6(vm, VM_TMPTRIE6);
    vm_exec_clrtrie(vm);
    vm_exec_clrtrie6(vm);

    while (vm->pc < vm->codesiz) {
        FETCH();

        switch (vm_getopcode(ip)) {
        EXECUTE(NOP):
            DISPATCH();

        EXECUTE(BLK):
            vm->curblk++;
            DISPATCH();

        EXECUTE(ENDBLK):
            if (unlikely(vm->curblk == 0))
                vm_abort(vm, VM_SPURIOUS_ENDBLK);

            vm->curblk--;
            PREDICT(CPASS); // OR chain case
            PREDICT(NOT);   // usually at end of chain
            DISPATCH();

        EXECUTE(LOAD):
            vm_pushvalue(vm, vm_extendarg(vm_getarg(ip), exarg));
            exarg = 0;
            DISPATCH();

        EXECUTE(LOADK):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_loadk(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(UNPACK):
            vm_exec_unpack(vm);
            DISPATCH();

        EXECUTE(EXARG):
            arg = vm_getarg(ip);
            exarg <<= 8;
            exarg  |= arg;

            PREDICT(LOADK);
            PREDICT(LOAD);
            PREDICT(CALL);

            DISPATCH();

        EXECUTE(STORE):
            vm_exec_store(vm);
            DISPATCH();

        EXECUTE(DISCARD):
            vm_exec_discard(vm);
            DISPATCH();

        EXECUTE(NOT):
            vm_exec_not(vm);
            PREDICT(CFAIL);
            PREDICT(CPASS);
            DISPATCH();

        EXECUTE(CPASS):
            cell = vm_peek(vm);
            if (cell->value) {
                if (vm->curblk == 0)
                    goto done; // no more blocks, we're done

                // advance up to the next ENDBLK
                vm_exec_break(vm);
            } else {
                vm->si--;  // discard and proceed
            }
            DISPATCH();

        EXECUTE(CFAIL):
            cell = vm_peek(vm);
            if (cell->value) {
                cell->value = 0;  // negate existing value
                if (vm->curblk == 0)
                    goto done;    // no more blocks, we're done

                // advance up to the next ENDBLK
                vm_exec_break(vm);
            } else {
                vm->si--;  // discard and proceed
            }
            DISPATCH();

        EXECUTE(ASPMATCH):
            vm_exec_aspmatch(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(ASPSTARTS):
            vm_exec_aspstarts(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(ASPENDS):
            vm_exec_aspends(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(ASPEXACT):
            vm_exec_aspexact(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(COMMEXACT):
            vm_exec_commexact(vm);
            DISPATCH();
            
        EXECUTE(CALL):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            if (unlikely(arg >= VM_FUNCS_COUNT))
                vm_abort(vm, VM_FUNC_UNDEFINED);
            if (unlikely(!vm->funcs[arg]))
                vm_abort(vm, VM_FUNC_UNDEFINED);

            vm->funcs[arg](vm);
            exarg = 0;
            DISPATCH();

        EXECUTE(SETTLE):
            vm_exec_settle(vm);
            DISPATCH();

        EXECUTE(HASATTR):
            vm_exec_hasattr(vm, vm_getarg(ip));
            PREDICT(NOT);
            DISPATCH();

        EXECUTE(EXACT):
            vm_exec_exact(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(SUBNET):
            vm_exec_subnet(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(SUPERNET):
            vm_exec_supernet(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(RELATED):
            vm_exec_related(vm, vm_getarg(ip));
            DISPATCH();

        EXECUTE(PFXCONTAINS):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_pfxcontains(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(ADDRCONTAINS):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_addrcontains(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(ASCONTAINS):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_ascontains(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(SETTRIE):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_settrie(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(SETTRIE6):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_settrie6(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(CLRTRIE):
            vm_exec_clrtrie(vm);
            DISPATCH();

        EXECUTE(CLRTRIE6):
            vm_exec_clrtrie6(vm);
            DISPATCH();

        EXECUTE(ADDRCMP):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_addrcmp(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(PFXCMP):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_pfxcmp(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE(ASCMP):
            arg = vm_extendarg(vm_getarg(ip), exarg);
            vm_exec_ascmp(vm, arg);
            exarg = 0;
            DISPATCH();

        EXECUTE_SIGILL:
            vm_abort(vm, VM_ILLEGAL_OPCODE);
            DISPATCH();  // unreachable
        }
    }

done:
    vm_exec_settle(vm);
    if (unlikely(vm->curblk > 0))
        vm_abort(vm, VM_DANGLING_BLK);

    cell = vm_pop(vm);
    return cell->value != 0;

#undef FETCH
#undef DISPATCH
#undef PREDICT
#undef EXECUTE
#undef EXECUTE_SIGILL
#pragma GCC diagnostic pop
}

