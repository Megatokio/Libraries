/*	Copyright  (c)	Günter Woigk 1996 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY,
	NOT EVEN THE IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
	A PARTICULAR PURPOSE, AND IN NO EVENT SHALL THE COPYRIGHT HOLDER
	BE LIABLE FOR ANY DAMAGES ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#pragma once

#include "kio/kio.h"
#include "CpuID.h"

typedef uint16 Address;
typedef uint8  Byte;
typedef uint16 Word;

enum __attribute__((deprecated)) { LegalOpcode, IllegalOpcode, WeirdOpcode };	// deprecated: use enum below
enum
{
	LEGAL_OPCODE,
	UNDOCUMENTED_OPCODE,// undocumented opcodes which have a useful new effect:
						// z80: SLL, use of XH, XL, YH and YL, IXCBR2 or IXCBXH if option ON

	DEPRECATED_OPCODE,	// opcode aliases and undocumented opcodes which have no useful new effect:
						// 8080: opcode aliases reused by z80; z80: 0xED aliases for RETI, IM_x

	ILLEGAL_OPCODE		// unhandled ops, undocumented nops, ops with uncertain effect:
						// z180: all; z80: 0xED nops, IX/IY with no effect, IXCBR2 or IXCBXH if option OFF
};

inline Byte peek (const Byte* core, Address addr) { return core[addr]; }

// get info about the legal state of an opcode:
// return: LEGAL_OPCODE .. ILLEGAL_OPCODE
extern int z80_opcode_validity (CpuID, const Byte* core, Address addr);

// disassemble 8080, Z80 or Z180 opcode using Z80 syntax:
extern cstr disassemble (CpuID, const Byte* core, Address& addr);

// disassemble 8080 opcode using 8080 or Z80 syntax:
extern cstr disassemble_8080 (const Byte* core, Address& addr, bool asm8080=yes);




__attribute__((deprecated))	// use z80_opcode_validity()
inline int opcode_legal_state (CpuID id, const Byte* core, Address addr) { return z80_opcode_validity(id,core,addr); }

__attribute__((deprecated)) // use real disassemble()
extern cstr opcode_mnemo (CpuID, const Byte* core, Address addr);






















