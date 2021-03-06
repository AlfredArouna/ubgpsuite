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

#include "../ubgp/bitops.h"
#include "../ubgp/filterintrin.h"
#include "../ubgp/filterpacket.h"
#include "../ubgp/branch.h"
#include "../ubgp/netaddr.h"
#include "../ubgp/patriciatrie.h"
#include "../ubgp/strutil.h"
#include "../ubgp/ubgpdef.h"

#include "parse.h"
#include "progutil.h"
#include "mrtdataread.h"

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <unistd.h>

static void usage(void)
{
    fprintf(stderr, "%s: MRT data reader and filtering utility\n", programnam);
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "\t%s [-cdlL] [-mM COMMSTRING] [-pP PATHEXPR] [-i ADDR] [-I FILE] [-a AS] [-A FILE] [-e PREFIX] [-E FILE] [-t ATTR_CODE] [-T FILE] [-o FILE] [FILE...]\n", programnam);
    fprintf(stderr, "\t%s [-cdlL] [-mM COMMSTRING] [-pP PATHEXPR] [-i ADDR] [-I FILE] [-a AS] [-A FILE] [-s PREFIX] [-S FILE] [-t ATTR_CODE] [-T FILE] [-o FILE] [FILE...]\n", programnam);
    fprintf(stderr, "\t%s [-cdlL] [-mM COMMSTRING] [-pP PATHEXPR] [-i ADDR] [-I FILE] [-a AS] [-A FILE] [-u PREFIX] [-U FILE] [-t ATTR_CODE] [-T FILE] [-o FILE] [FILE...]\n", programnam);
    fprintf(stderr, "\t%s [-cdlL] [-mM COMMSTRING] [-pP PATHEXPR] [-i ADDR] [-I FILE] [-a AS] [-A FILE] [-r PREFIX] [-R FILE] [-t ATTR_CODE] [-T FILE] [-o FILE] [FILE...]\n", programnam);
    fprintf(stderr, "\n");
    fprintf(stderr, "Available options:\n");
    fprintf(stderr, "\t-a <feeder AS>\n");
    fprintf(stderr, "\t\tPrint only entries coming from the given feeder AS\n");
    fprintf(stderr, "\t-A <file>\n");
    fprintf(stderr, "\t\tPrint only entries coming from the feeder ASes contained in file\n");
    fprintf(stderr, "\t-c\n");
    fprintf(stderr, "\t\tDump packets in hexadecimal C array format\n");
    fprintf(stderr, "\t-d\n");
    fprintf(stderr, "\t\tDump packet filter bytecode to stderr (debug option)\n");
    fprintf(stderr, "\t-e <subnet>\n");
    fprintf(stderr, "\t\tPrint only entries containing the exact given subnet of interest\n");
    fprintf(stderr, "\t-E <file>\n");
    fprintf(stderr, "\t\tPrint only entries containing the exact subnets of interest contained in file\n");
    fprintf(stderr, "\t-f\n");
    fprintf(stderr, "\t\tPrint only every feeder IP in the RIB provided\n");
    fprintf(stderr, "\t-i <feeder IP>\n");
    fprintf(stderr, "\t\tPrint only entries coming from a given feeder IP\n");
    fprintf(stderr, "\t-I <file>\n");
    fprintf(stderr, "\t\tPrint only entries coming from the feeder IP contained in file\n");
    fprintf(stderr, "\t-l\n");
    fprintf(stderr, "\t\tPrint only entries with a loop in its AS PATH\n");
    fprintf(stderr, "\t-L\n");
    fprintf(stderr, "\t\tPrint only entries without a loop in its AS PATH\n");
    fprintf(stderr, "\t-o <file>\n");
    fprintf(stderr, "\t\tDefine the output file to store information (defaults to stdout)\n");
    fprintf(stderr, "\t-m <communities string>\n");
    fprintf(stderr, "\t\tPrint only entries which COMMUNITY attribute contains the specified communities (the order is not relevant)\n");
    fprintf(stderr, "\t-M <communities string>\n");
    fprintf(stderr, "\t\tPrint only entries which COMMUNITY attribute does not contain the specified communities (the order is not relevant)\n");
    fprintf(stderr, "\t-p <path expression>\n");
    fprintf(stderr, "\t\tPrint only entries which AS PATH attribute matches the expression\n");
    fprintf(stderr, "\t-P <path expression>\n");
    fprintf(stderr, "\t\tPrint only entries which AS PATH attribute does not match the expression\n");
    fprintf(stderr, "\t-r <subnet>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets related to the given subnet of interest\n");
    fprintf(stderr, "\t-R <file>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets related to the subnets of interest contained in file\n");
    fprintf(stderr, "\t-s <subnet>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets included to the given subnet of interest\n");
    fprintf(stderr, "\t-S <file>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets included to the subnets of interest contained in file\n");
    fprintf(stderr, "\t-t <attribute code>\n");
    fprintf(stderr, "\t\tPrint only entries containing the attribute of interest\n");
    fprintf(stderr, "\t-T <file>\n");
    fprintf(stderr, "\t\tPrint only entries containing the attributes of interest contained in file\n");
    fprintf(stderr, "\t-u <subnet>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets including (or equal) to the given subnet of interest\n");
    fprintf(stderr, "\t-U <file>\n");
    fprintf(stderr, "\t\tPrint only entries containing subnets including (or equal) to the subnets of interest contained in file\n");
    exit(EXIT_FAILURE);
}

enum {
    DBG_DUMP            = 1 << 0,
    ONLY_PEERS          = 1 << 1,
    MATCH_AS_PATH       = 1 << 2,
    FILTER_BY_PEER_ADDR = 1 << 3,
    FILTER_BY_PEER_AS   = 1 << 4,
    FILTER_EXACT        = 1 << 5,
    FILTER_RELATED      = 1 << 6,
    FILTER_BY_SUBNET    = 1 << 7,
    FILTER_BY_SUPERNET  = 1 << 8,
    KEEP_AS_LOOPS       = 1 << 9,
    DISCARD_AS_LOOPS    = 1 << 10,

    FILTER_MASK  = (FILTER_EXACT | FILTER_RELATED | FILTER_BY_SUBNET | FILTER_BY_SUPERNET),
    AS_LOOP_MASK = KEEP_AS_LOOPS | DISCARD_AS_LOOPS
};

enum {
    ADDRS_GROWSTEP = 128,
    ASES_GROWSTEP  = 256
};

static filter_vm_t vm;
static uint flags            = 0;
static int trie_idx          = -1;
static int trie6_idx         = -1;
static uint32_t *peer_ases   = NULL;
static uint ases_count       = 0;
static uint ases_siz         = 0;
static netaddr_t *peer_addrs = NULL;
static uint addrs_count      = 0;
static uint addrs_siz        = 0;
static mrt_dump_fmt_t format = MRT_DUMP_ROW;

// attribute of interest mask, only meaningful if attr_count > 0

enum {
    MAX_ATTRS_BITSET_SIZE = 0xff / (sizeof(uint32_t) * CHAR_BIT)
};

enum {
    ATTR_BITSET_SHIFT = 5,
    ATTR_BITSET_MASK  = 0x1f
};

static uint32_t attr_mask[MAX_ATTRS_BITSET_SIZE];
static uint     attr_count = 0;

typedef struct community_match_s {
    struct community_match_s *next;
    bool neg;
    int  kidx;
} community_match_t;

static community_match_t *community_matches = NULL;

typedef struct as_path_match_s {
    struct as_path_match_s *or_next;  // next match in OR-ed chain
    struct as_path_match_s *and_next; // next match in AND chain

    bytecode_t opcode;
    int kidx;
    bool neg;
} as_path_match_t;

static as_path_match_t *path_match_head = NULL;
static as_path_match_t *path_match_tail = NULL;

static noreturn void naddr_parse_error(const char *name,
                                       uint        lineno,
                                       const char  *msg,
                                       void        *data)
{
    USED(data);

    exprintf(EXIT_FAILURE, "%s:%u: %s", name, lineno, msg);
}

static bool add_trie_address(const char *s)
{
    netaddr_t addr;

    if (stonaddr(&addr, s) != 0)
        return false;

    void *node;
    if (addr.family == AF_INET)
        node = patinsert(&vm.tries[trie_idx], &addr, NULL);
    else
        node = patinsert(&vm.tries[trie6_idx], &addr, NULL);

    if (!node)
        exprintf(EXIT_FAILURE, "out of memory");

    return true;
}

static bool add_peer_as(const char *s)
{
    char *end;

    llong as = strtoll(s, &end, 10);
    if (*end != '\0' || s == end)
        return false;
    if (as < 0 || as > UINT32_MAX)
        return false;

    if (unlikely(ases_count == ases_siz)) {
        ases_siz += ASES_GROWSTEP;

        peer_ases = realloc(peer_ases, ases_siz * sizeof(*peer_ases));
        if (unlikely(!peer_ases))
            exprintf(EXIT_FAILURE, "out of memory");
    }

    peer_ases[ases_count++] = (uint32_t) as;
    return true;
}

static bool add_peer_address(const char *s)
{
    netaddr_t addr;
    if (inet_pton(AF_INET6, s, &addr.sin6) > 0) {
        addr.family = AF_INET6;
        addr.bitlen = 128;
    } else if (inet_pton(AF_INET, s, &addr.sin) > 0) {
        addr.family = AF_INET;
        addr.bitlen = 32;
    } else {
        return false;
    }

    if (unlikely(addrs_count == addrs_siz)) {
        addrs_siz += ADDRS_GROWSTEP;

        peer_addrs = realloc(peer_addrs, addrs_siz * sizeof(*peer_addrs));
        if (unlikely(!peer_addrs))
            exprintf(EXIT_FAILURE, "out of memory");
    }

    peer_addrs[addrs_count++] = addr;
    return true;
}

static bool add_interesting_attr(const char *s)
{
    static const struct {
        const char *name;
        int code;
    } attr_tab[] = {
        { "ORIGIN", ORIGIN_CODE },
        { "AS_PATH", AS_PATH_CODE },
        { "NEXT_HOP", NEXT_HOP_CODE },
        { "MULTI_EXIT_DISC", MULTI_EXIT_DISC_CODE },
        { "LOCAL_PREF", LOCAL_PREF_CODE },
        { "ATOMIC_AGGREGATE", ATOMIC_AGGREGATE_CODE },
        { "AGGREGATOR", AGGREGATOR_CODE },
        { "COMMUNITY", COMMUNITY_CODE },
        { "ORIGINATOR_ID", ORIGINATOR_ID_CODE },
        { "CLUSTER_LIST", CLUSTER_LIST_CODE },
        { "DPA", DPA_CODE },
        { "ADVERTISER", ADVERTISER_CODE },
        { "RCID_PATH_CLUSTER_ID", RCID_PATH_CLUSTER_ID_CODE },
        { "MP_REACH_NLRI", MP_REACH_NLRI_CODE },
        { "MP_UNREACH_NLRI_CODE", MP_UNREACH_NLRI_CODE },
        { "EXTENDED_COMMUNITY", EXTENDED_COMMUNITY_CODE },
        { "AS4_PATH", AS4_PATH_CODE },
        { "AS4_AGGREGATOR", AS4_AGGREGATOR_CODE },
        { "SAFI_SSA", SAFI_SSA_CODE },
        { "CONNECTOR", CONNECTOR_CODE },
        { "AS_PATHLIMIT", AS_PATHLIMIT_CODE },
        { "PMSI_TUNNEL", PMSI_TUNNEL_CODE },
        { "TUNNEL_ENCAPSULATION", TUNNEL_ENCAPSULATION_CODE },
        { "TRAFFIC_ENGINEERING", TRAFFIC_ENGINEERING_CODE },
        { "IPV6_ADDRESS_SPECIFIC_EXTENDED_COMMUNITY", IPV6_ADDRESS_SPECIFIC_EXTENDED_COMMUNITY_CODE },
        { "AIGP", AIGP_CODE },
        { "PE_DISTINGUISHER_LABELS", PE_DISTINGUISHER_LABELS_CODE },
        { "BGP_ENTROPY_LEVEL_CAPABILITY", BGP_ENTROPY_LEVEL_CAPABILITY_CODE },
        { "BGP_LS", BGP_LS_CODE },
        { "LARGE_COMMUNITY", LARGE_COMMUNITY_CODE },
        { "BGPSEC_PATH", BGPSEC_PATH_CODE },
        { "BGP_COMMUNITY_CONTAINER", BGP_COMMUNITY_CONTAINER_CODE },
        { "BGP_PREFIX_SID", BGP_PREFIX_SID_CODE },
        { "ATTR_SET", ATTR_SET_CODE },
        { "RESERVED", RESERVED_CODE },
        { NULL, ATTR_BAD_CODE}
    };

    int code = ATTR_BAD_CODE;

    for (uint i = 0; attr_tab[i].name != NULL; i++) {
        if (strcasecmp(attr_tab[i].name, s) == 0) {
            code = attr_tab[i].code;
            break;
        }
    }

    if (code == ATTR_BAD_CODE) {
        char *end;

        errno = 0;

        llong val = strtoll(s, &end, 10);
        if (val < 0 || val > 0xff)
            errno = ERANGE;
        if (end == s || errno != 0)
            return false;

        code = val;
    }

    if ((attr_mask[code >> ATTR_BITSET_SHIFT] & (1 << (code & ATTR_BITSET_MASK))) == 0) {
        attr_mask[code >> ATTR_BITSET_SHIFT] |= 1 << (code & ATTR_BITSET_MASK);
        attr_count++;
    }
    return true;
}

static void parse_file(const char  *filename,
                       bool       (*read_callback)(const char *))
{
    FILE *f = fopen(filename, "r");
    if (!f)
        exprintf(EXIT_FAILURE, "cannot open '%s':", filename);

    setperrcallback(naddr_parse_error);
    startparsing(filename, 1, NULL);

    char *tok;
    while ((tok = parse(f)) != NULL) {
        if (!read_callback(tok))
            parsingerr("bad entry: %s", tok);
    }

    setperrcallback(NULL);

    if (fclose(f) != 0)
        exprintf(EXIT_FAILURE, "read error while parsing: %s:", filename);
}

static void mrt_accumulate_addrs(filter_vm_t *vm)
{
    for (uint i = 0; i < addrs_count; i++)
        vm_pushaddr(vm, &peer_addrs[i]);
}

static void mrt_accumulate_ases(filter_vm_t *vm)
{
    for (uint i = 0; i < ases_count; i++)
        vm_pushas(vm, peer_ases[i]);
}

enum {
    ASPATHSIZ = 32
};

static void mrt_find_as_loops(filter_vm_t *vm)
{

    vm_exec_settle(vm);  // force iterators settle (not really necessary... but whatever)

    uint max       = 0;
    uint n         = 0;
    intptr_t vaddr = 0;
    uint32_t *path = NULL;

    startrealaspath(vm->bgp);

    as_pathent_t *ent;
    while ((ent = nextaspath(vm->bgp)) != NULL) {
        if (unlikely(n == max)) {
            max += ASPATHSIZ;

            vaddr = vm_heap_grow(vm, vaddr, max * sizeof(*path));
            if (unlikely(vaddr == VM_BAD_HEAP_PTR))
                vm_abort(vm, VM_OUT_OF_MEMORY);

            path = vm_heap_ptr(vm, vaddr);  // update pointer
        }
        path[n++] = ent->as;
    }

    if (endaspath(vm->bgp) != BGP_ENOERR)
        vm_abort(vm, VM_BAD_PACKET);

    bool has_loop = false;
    for (uint i = 2; i < n - 1 && !has_loop; i++) {
        if (path[i] == path[i - 1])
            continue;  // prepending
        if (path[i] == AS_TRANS)
            continue;

        for (uint j = 0; j < i - 2; j++) {
            if (path[j] == path[i] && path[j] != AS_TRANS) {
                has_loop = true;
                break;
            }
        }
    }

    vm_heap_return(vm, max * sizeof(*path));  // don't need temporary memory anymore

    vm_pushvalue(vm, has_loop);
}

static void setup_filter(void)
{
    if (flags & FILTER_BY_PEER_AS) {
        vm_emit(&vm, vm_makeop(FOPC_CALL, MRT_ACCUMULATE_ASES_FN));
        vm_emit(&vm, vm_makeop(FOPC_ASCONTAINS, K_PEER_AS));
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }
    if (flags & FILTER_BY_PEER_ADDR) {
        vm_emit(&vm, vm_makeop(FOPC_CALL, MRT_ACCUMULATE_ADDRS_FN));
        vm_emit(&vm, vm_makeop(FOPC_ADDRCONTAINS, K_PEER_ADDR));
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }
    if (attr_count > 0) {
        // filter by attribute of interest
        uint n = 0;

        vm_emit(&vm, FOPC_BLK);
        for (uint i = 0; i < 256 && n < attr_count; i++) {
            if (attr_mask[i >> ATTR_BITSET_SHIFT] & (1 << (i & ATTR_BITSET_MASK))) {
                vm_emit(&vm, vm_makeop(FOPC_HASATTR, i));
                n++;

                if (n < attr_count)
                    vm_emit(&vm, FOPC_CPASS);
            }
        }

        vm_emit(&vm, FOPC_ENDBLK);
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }
    if (community_matches) {
        // filter by community, each community_match_t is ORed
        vm_emit(&vm, FOPC_BLK);
        for (community_match_t *i = community_matches; i; i = i->next) {
            vm_emit(&vm, vm_makeop(FOPC_LOADK, i->kidx));
            vm_emit(&vm, FOPC_UNPACK);
            vm_emit(&vm, FOPC_COMMEXACT);
            if (i->neg)
                vm_emit(&vm, FOPC_NOT);
            if (i->next)
                vm_emit(&vm, FOPC_CPASS);
        }
        vm_emit(&vm, FOPC_ENDBLK);
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }

    if (path_match_head) {
        // include the AS PATH filtering logic
        vm_emit(&vm, FOPC_BLK);
        for (as_path_match_t *i = path_match_head; i; i = i->or_next) {
            // compile the AND chain
            vm_emit(&vm, FOPC_BLK);
            for (as_path_match_t *j = i; j; j = j->and_next) {
                uint access = FOPC_ACCESS_REAL_AS_PATH;
                if (j == i)
                    access |= FOPC_ACCESS_SETTLE; // rewind the AS PATH on first test

                vm_emit(&vm, vm_makeop(FOPC_LOADK, j->kidx));
                vm_emit(&vm, FOPC_UNPACK);
                vm_emit(&vm, vm_makeop(j->opcode, access));
                if (j->and_next) {  // omit the last AND term for optimization
                    vm_emit(&vm, FOPC_NOT);
                    vm_emit(&vm, FOPC_CFAIL);
                }
            }
            vm_emit(&vm, FOPC_ENDBLK);
            if (i->neg)
                vm_emit(&vm, FOPC_NOT);

            if (i->or_next)
                vm_emit(&vm, FOPC_CPASS);
            /*
            if (i->or_next) {                // omit the conditional on last OR term
                vm_emit(&vm, FOPC_CPASS);  // if the block was successful, the entire OR succeeded
            }*/
        }

        vm_emit(&vm, FOPC_ENDBLK);
        // must fail if none of the OR clauses was satisfied
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }

    if (flags & FILTER_MASK) {
        // only one filter may be set (otherwise it's an option conflict)
        vm_emit(&vm, vm_makeop(FOPC_SETTRIE,  trie_idx));
        vm_emit(&vm, vm_makeop(FOPC_SETTRIE6, trie6_idx));

        bytecode_t opcode;
        if (flags & FILTER_EXACT)
            opcode = FOPC_EXACT;
        if (flags & FILTER_RELATED)
            opcode = FOPC_RELATED;
        if (flags & FILTER_BY_SUBNET)
            opcode = FOPC_SUBNET;
        if (flags & FILTER_BY_SUPERNET)
            opcode = FOPC_SUPERNET;

        vm_emit(&vm, FOPC_BLK);
        vm_emit(&vm, vm_makeop(opcode, FOPC_ACCESS_SETTLE | FOPC_ACCESS_ALL | FOPC_ACCESS_NLRI));
        vm_emit(&vm, FOPC_CPASS);
        vm_emit(&vm, vm_makeop(opcode, FOPC_ACCESS_SETTLE | FOPC_ACCESS_ALL | FOPC_ACCESS_WITHDRAWN));
        vm_emit(&vm, FOPC_ENDBLK);
        vm_emit(&vm, FOPC_NOT);
        vm_emit(&vm, FOPC_CFAIL);
    }
    if (flags & AS_LOOP_MASK) {
        // also call the AS loop filtering logic...
        vm_emit(&vm, vm_makeop(FOPC_CALL, MRT_FIND_AS_LOOPS_FN));
        if (flags & KEEP_AS_LOOPS)
            vm_emit(&vm, FOPC_NOT);

        vm_emit(&vm, FOPC_CFAIL);
    }
    vm_emit(&vm, vm_makeop(FOPC_LOAD, true));
}

static bool iswildcard(char c)
{
    return c == '*' || c == '?';
}

static bool isdelim(char c)
{
    return isspace((uchar) c) || c == '$' || c == '\0' || c == '^';
}

static char *skip_spaces(const char *ptr)
{
    while (isspace((uchar) *ptr)) ptr++;

    return (char *) ptr;
}

static void parse_as_match_expr(const char *expr, bool negate)
{
    int opcode = FOPC_ASPMATCH;

    wide_as_t buf[strlen(expr) / 2 + 1];  // wild upperbound

    const char *ptr = skip_spaces(expr);
    if (*ptr == '^') {
        opcode = FOPC_ASPSTARTS;
        ptr = skip_spaces(ptr + 1);
    }

    as_path_match_t *expr_head = NULL;
    as_path_match_t *expr_tail = NULL;
    while (true) {
        // parse an AS match expressions, expressions are ANDed such as "^1 2 3 * 4 ? ? 5 6 * 7$"

        int count = 0;
        while (true) {
            // this splits the full expressions into subexpressions
            // ptr *DOES NOT* reference a space here!
            errno = 0;

            if (*ptr == '\0')
                break;  // we're done
            if (iswildcard(*ptr) && !isdelim(*(ptr + 1)))
                exprintf(EXIT_FAILURE, "%s: wildcard '%c' must be delimiter separated", expr, *ptr);

            if (*ptr == '*') {
                // flush operations at this point
                ptr = skip_spaces(ptr + 1);
                break;
            }

            llong as;
            if (*ptr == '?') {
                as = AS_ANY;
                ptr++;
            } else {
                char* eptr;

                as = strtoll(ptr, &eptr, 10);
                if (ptr == eptr) {
                    uint len = ptr - expr;
                    const char *at = expr;
                    if (len == 0) {
                        at = "[expression start]";
                        len = strlen(at);
                    }

                    exprintf(EXIT_FAILURE, "%s: expecting AS number after: '%.*s'", expr, len, at);
                }
                if (as < 0 || as > UINT32_MAX)
                    errno = ERANGE;
                if (errno != 0)
                    exprintf(EXIT_FAILURE, "%s: AS number '%lld':", expr, as);

                ptr = eptr;
            }

            // buffer the AS
            buf[count++] = as;
            // ... and position to the next token
            ptr = skip_spaces(ptr);

            if (*ptr == '$') {
                opcode = (opcode == FOPC_ASPSTARTS) ? FOPC_ASPEXACT : FOPC_ASPENDS;
                ptr = skip_spaces(ptr + 1);
                if (*ptr != '\0')
                    exprintf(EXIT_FAILURE, "%s: expecting expression end after '$'", expr);
            }
        }

        if (count == 0) {
            if (*ptr == '$')
                break; // expression with a terminating ?

            exprintf(EXIT_FAILURE, "empty AS match expression");
        }

        // parsed one match, add it to the AND chain of the current expression
        // we'll compile it to bytecode later
        as_path_match_t *match = malloc(sizeof(*match));
        if (unlikely(!match))
            exprintf(EXIT_FAILURE, "out of memory");

        // populate AS match subexpression
        match->opcode = opcode;
        // add the AS path segment to VM heap
        intptr_t heapptr = vm_heap_alloc(&vm, count * sizeof(*buf), VM_HEAP_PERM);
        if (unlikely(heapptr == VM_BAD_HEAP_PTR))
            exprintf(EXIT_FAILURE, "out of memory");

        memcpy(vm_heap_ptr(&vm, heapptr), buf, count * sizeof(*buf));

        // generate a K constant with the heap array address
        int kidx = vm_newk(&vm);
        if (kidx == -1)
            exprintf(EXIT_FAILURE, "out of memory");

        vm.kp[kidx].base  = heapptr;
        vm.kp[kidx].elsiz = sizeof(*buf);
        vm.kp[kidx].nels  = count;

        match->kidx = kidx;
        match->neg = negate;
        
        match->or_next = NULL;
        if (expr_tail)
            expr_tail->and_next = match;
        else
            expr_head = match;

        expr_tail = match;

        if (*ptr == '\0')
            break;  // done parsing the entire OR-chain

        // reset match semantics:
        opcode = FOPC_ASPMATCH;
    }

    // add to the global OR chain, in case more expressions are provided
    if (path_match_tail)
        path_match_tail->or_next = expr_head;
    else
        path_match_head = expr_head;

    path_match_tail = expr_head;
}

static void parse_communities(const char *expr, bool negate)
{
    community_t buf[strlen(expr)];  // wild upperbound

    const char *ptr = expr;
    char *eptr;
    size_t count = 0;
    while (true) {
        ptr = skip_spaces(ptr);
        if (*ptr == '\0')
            break;

        community_t c = stocommunity(ptr, &eptr);
        if (ptr == eptr)
            exprintf(EXIT_FAILURE, "bad community string: '%s' at %c", expr, *ptr);

        // do not take the same community twice, it would have no effect,
        // and it would confuse the VM
        uint i;
        for (i = 0; i < count; i++) {
            if (buf[i] == c)
                break;
        }
        if (i == count)
            buf[count++] = c;  // ok, this community is unique

        ptr = eptr;
    }

    if (count == 0)
        exprintf(EXIT_FAILURE, "empty community match expression");
        
    community_match_t *m = malloc(sizeof(*m));
    if (unlikely(!m))
        exprintf(EXIT_FAILURE, "out of memory");

    // add the community segment to VM heap
    intptr_t heapptr = vm_heap_alloc(&vm, count * sizeof(*buf), VM_HEAP_PERM);
    if (unlikely(heapptr == VM_BAD_HEAP_PTR))
        exprintf(EXIT_FAILURE, "out of memory");

    memcpy(vm_heap_ptr(&vm, heapptr), buf, count * sizeof(*buf));

    // generate a K constant with the heap array address
    int kidx = vm_newk(&vm);
    if (kidx == -1)
        exprintf(EXIT_FAILURE, "out of memory");

    vm.kp[kidx].base  = heapptr;
    vm.kp[kidx].elsiz = sizeof(*buf);
    vm.kp[kidx].nels  = count;

    m->neg  = negate;
    m->kidx = kidx;
    m->next = community_matches;
    community_matches = m;
}

int main(int argc, char **argv)
{
    setprogramnam(argv[0]);

    // setup VM environment
    filter_init(&vm);

    trie_idx  = vm_newtrie(&vm, AF_INET);
    trie6_idx = vm_newtrie(&vm, AF_INET6);

    vm.funcs[MRT_ACCUMULATE_ADDRS_FN] = mrt_accumulate_addrs;
    vm.funcs[MRT_ACCUMULATE_ASES_FN]  = mrt_accumulate_ases;
    vm.funcs[MRT_FIND_AS_LOOPS_FN] = mrt_find_as_loops;

    // parse command line
    int c;
    while ((c = getopt(argc, argv, "A:a:cdE:e:fi:I:lLm:M:o:p:P:R:r:S:s:t:T:U:u:")) != -1) {
        switch (c) {
        case 'a':
            if (!add_peer_as(optarg))
                exprintf(EXIT_FAILURE, "'%s': bad AS number", optarg);

            flags |= FILTER_BY_PEER_AS;
            break;

        case 'A':
            parse_file(optarg, add_peer_as);
            flags |= FILTER_BY_PEER_AS;
            break;

        case 'c':
            format = MRT_DUMP_CHEX;
            break;

        case 'd':
            flags |= DBG_DUMP;
            break;

        case 'o':
            if (!freopen(optarg, "w", stdout))
                exprintf(EXIT_FAILURE, "cannot open '%s':", optarg);

            break;

        case 'f':
            flags |= ONLY_PEERS;
            break;

        case 'E':
        case 'U':
        case 'R':
        case 'S':
            if (c == 'E')
                flags |= FILTER_EXACT;
            if (c == 'U')
                flags |= FILTER_BY_SUPERNET;
            if (c == 'R')
                flags |= FILTER_RELATED;
            if (c == 'S')
                flags |= FILTER_BY_SUBNET;

            if (popcnt(flags & FILTER_MASK) != 1)
                exprintf(EXIT_FAILURE, "conflicting options in filter");

            parse_file(optarg, add_trie_address);
            break;

        case 'e':
        case 'u':
        case 'r':
        case 's':
            if (c == 'e')
                flags |= FILTER_EXACT;
            if (c == 'u')
                flags |= FILTER_BY_SUPERNET;
            if (c == 'r')
                flags |= FILTER_RELATED;
            if (c == 's')
                flags |= FILTER_BY_SUBNET;

            if (popcnt(flags & FILTER_MASK) != 1)
                exprintf(EXIT_FAILURE, "conflicting options in filter");

            if (!add_trie_address(optarg))
                exprintf(EXIT_FAILURE, "bad address: %s", optarg);

            break;

        case 'p':
        case 'P':
            parse_as_match_expr(optarg, c == 'P');
            break;

        case 'm':
        case 'M':
            parse_communities(optarg, c == 'M');
            break;

        case 'i':
            if (!add_peer_address(optarg))
                exprintf(EXIT_FAILURE, "'%s': bad peer address", optarg);

            flags |= FILTER_BY_PEER_ADDR;
            break;

        case 'I':
            parse_file(optarg, add_peer_address);
            flags |= FILTER_BY_PEER_ADDR;
            break;

        case 'l':
            flags &= ~DISCARD_AS_LOOPS;
            flags |= KEEP_AS_LOOPS;
            break;

        case 'L':
            flags &= ~KEEP_AS_LOOPS;
            flags |= DISCARD_AS_LOOPS;
            break;

        case 't':
            if (!add_interesting_attr(optarg))
                exprintf(EXIT_FAILURE, "'%s': bad attribute code", optarg);

            break;

        case 'T':
            parse_file(optarg, add_interesting_attr);
            break;

        case '?':
        default:
            usage();
            break;
        }
    }

    setup_filter();
    if (flags & DBG_DUMP)
        filter_dump(stderr, &vm);

    if (optind == argc) {
        // no file arguments, process stdin
        // we apply an innocent trick to simulate a "-" argument
        // NOTE argv will *NOT* be NULL terminated anymore
        argv[argc] = "-";
        argc++;
    }

    // apply to required files
    uint nerrors = 0;
    for (int i = optind; i < argc; i++) {
        io_rw_t io;
        int fd;

        io_rw_t *iop = NULL;

        char *ext = strpathext(argv[i]);
        if (strcasecmp(ext, ".gz") == 0 || strcasecmp(ext, ".z") == 0) {
            fd = open(argv[i], O_RDONLY);
            if (fd >= 0)
                iop = io_zopen(fd, BUFSIZ, "r");

        } else if (strcasecmp(ext, ".bz2") == 0) {
            fd = open(argv[i], O_RDONLY);
            if (fd >= 0)
                iop = io_bz2open(fd, BUFSIZ, "r");

#ifdef UBGP_IO_XZ
        } else if (strcasecmp(ext, ".xz") == 0) {
            fd = open(argv[i], O_RDONLY);
            if (fd >= 0)
                iop = io_xzopen(fd, BUFSIZ, "r");
#endif

        } else if (strcmp(argv[i], "-") == 0) {
            io_file_init(&io, stdin);
            iop = &io;
            fd = STDIN_FILENO;

            // rename argument to (stdin) to improve logging quality
            argv[i] = "(stdin)";
        } else {
            FILE *file = fopen(argv[i], "rb");

            fd = -1;
            if (file) {
                io_file_init(&io, file);
                iop = &io;

                fd = fileno(file);
            }
        }

        if (fd == -1) {
            eprintf("cannot open '%s':", argv[i]);
            nerrors++;
            continue;
        }
        if (!iop) {
            eprintf("'%s': not a valid %s file", argv[i], ext);
            nerrors++;
            continue;
        }

#ifdef _POSIX_ADVISORY_INFO
        if (fd != STDIN_FILENO)
            posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL);
#endif

        int res;
        if (flags & ONLY_PEERS)
            res = mrtprintpeeridx(argv[i], iop, &vm);
        else
            res = mrtprocess(argv[i], iop, &vm, format);

        if (res != 0)
            nerrors++;

        if (fd != STDIN_FILENO)
            iop->close(iop);
    }

    // cleanup and exit
    filter_destroy(&vm);
    free(peer_ases);
    free(peer_addrs);
    while (path_match_head) {
        as_path_match_t *t = path_match_head;
        while (t->and_next) {
            as_path_match_t *tn = t->and_next;

            t->and_next = tn->and_next;
            free(tn);
        }

        path_match_head = t->or_next;

        free(t);
    }
    while (community_matches) {
        community_match_t *t = community_matches;
        community_matches = t->next;
        free(t);
    }

    if (fflush(stdout) != 0)
        exprintf(EXIT_FAILURE, "could not write to output file:");

    return (nerrors == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
