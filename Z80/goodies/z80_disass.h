#pragma once
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

#include "kio/kio.h"
#include "z80_goodies.h"	// wg. CpuID

enum { LegalOpcode, IllegalOpcode, WeirdOpcode };
typedef uint16 Address;
typedef uint8  Byte;
typedef uint16 Word;

inline Byte peek (const Byte* core, Address addr) { return core[addr]; }

enum { DISASS_STD=0, DISASS_ASM8080, DISASS_IXCBR2, DISASS_IXCBXH, DISASS_Z180 };

extern cstr disassemble_z80  (const Byte* core, Address& addr, int option=DISASS_STD);
extern cstr disassemble_8080 (const Byte* core, Address& addr, int option=DISASS_STD);
extern cstr disassemble_z180 (const Byte* core, Address& addr);
extern cstr disassemble		 (CpuID, const Byte* core, Address& addr, int option=DISASS_STD);

inline cstr disassemble_asm8080 (const Byte* core, Address& addr)
{
	return disassemble_8080(core,addr,DISASS_ASM8080);
}

extern int  opcode_legal_state (CpuID, const Byte* core, Address addr);

__attribute__((deprecated))
extern cstr opcode_mnemo	(CpuID, const Byte* core, Address addr);






















