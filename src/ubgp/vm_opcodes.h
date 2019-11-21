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

// =====================================================================
// NOTE: THIS FILE HAS NO INCLUDE GUARDS, BECAUSE IT NEEDS TO COPY-PASTE
//       A STATIC OPCODE TABLE FOR A COMPUTED GOTO STATEMENT
//
// KEEP THIS TABLE IN SYNC WITH filterintrin.h
// =====================================================================

static const void *const vm_opcode_table[256] = {
    // clear array to sigill, for opcodes go below...

    // opcode table:
    [FOPC_NOP]          = &&EX_NOP,
    [FOPC_BLK]          = &&EX_BLK,
    [FOPC_ENDBLK]       = &&EX_ENDBLK,
    [FOPC_LOAD]         = &&EX_LOAD,
    [FOPC_LOADK]        = &&EX_LOADK,
    [FOPC_UNPACK]       = &&EX_UNPACK,
    [FOPC_EXARG]        = &&EX_EXARG,
    [FOPC_STORE]        = &&EX_STORE,
    [FOPC_DISCARD]      = &&EX_DISCARD,
    [FOPC_NOT]          = &&EX_NOT,
    [FOPC_CPASS]        = &&EX_CPASS,
    [FOPC_CFAIL]        = &&EX_CFAIL,

    [FOPC_SETTLE]       = &&EX_SETTLE,
    [FOPC_HASATTR]      = &&EX_HASATTR,

    [FOPC_EXACT]        = &&EX_EXACT,
    [FOPC_SUBNET]       = &&EX_SUBNET,
    [FOPC_SUPERNET]     = &&EX_SUPERNET,
    [FOPC_RELATED]      = &&EX_RELATED,
    [FOPC_PFXCONTAINS]  = &&EX_PFXCONTAINS,
    [FOPC_ADDRCONTAINS] = &&EX_ADDRCONTAINS,
    [FOPC_ASCONTAINS]   = &&EX_ASCONTAINS,

    [FOPC_ASPMATCH]     = &&EX_ASPMATCH,
    [FOPC_ASPSTARTS]    = &&EX_ASPSTARTS,
    [FOPC_ASPENDS]      = &&EX_ASPENDS,
    [FOPC_ASPEXACT]     = &&EX_ASPEXACT,

    [FOPC_COMMEXACT]    = &&EX_COMMEXACT,

    [FOPC_CALL]         = &&EX_CALL,
    [FOPC_SETTRIE]      = &&EX_SETTRIE,
    [FOPC_SETTRIE6]     = &&EX_SETTRIE6,
    [FOPC_CLRTRIE]      = &&EX_CLRTRIE,
    [FOPC_CLRTRIE6]     = &&EX_CLRTRIE6,
    [FOPC_ASCMP]        = &&EX_ASCMP,
    [FOPC_ADDRCMP]      = &&EX_ADDRCMP,
    [FOPC_PFXCMP]       = &&EX_PFXCMP,

    [OPCODES_COUNT]     = &&EX_SIGILL,

    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL, &&EX_SIGILL,
    &&EX_SIGILL
};

