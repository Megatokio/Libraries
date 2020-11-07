/*	Copyright  (c)	Günter Woigk 2020 - 2020
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

enum CpuID
{
	CpuDefault=0,	// same as Z80, but not actively selected
	CpuZ80,
	Cpu8080,
	CpuZ180
};


typedef uint8  Byte;
typedef uint16 Word;

inline Byte peek (const Byte* p) { return *p; }
inline Word peek_word (const Byte* p) { return *p + 256 * *(p+1); }


/*	Calculate length [bytes] of opcode
*/
extern uint opcode_length (CpuID, const Byte* ip) noexcept;
extern uint z80_opcode_length (const Byte* ip) noexcept;
extern uint z180_opcode_length (const Byte* ip) noexcept;
extern uint i8080_opcode_length (const Byte* ip) noexcept;


/*	Calculate the "major" opcode of an instruction mnemonic.
	This is
	• byte 1 of simple instructions
	• byte 2 of instructions prefixed with ED, CB, IX or IY
	• byte 4 of instructions with prefix CB+IX or CB+IY
	note:
	• prefix ED instruction "ld hl,(NN)" is never returned,
	  because the non-prefix version is returned instead
*/
extern uint8 z80_major_opcode (cstr q) throws;


/* Calculate the minimum number of clock cycles for a Z80, Z180 or 8080 opcode
   if no wait cycles are added.
*/
extern bool opcode_can_branch (CpuID, uint8 op1, uint8 op2) noexcept;		// op2 only used if op1==0xED
extern uint clock_cycles (CpuID, uint8 op1, uint8 op2, uint8 op4) noexcept;	// dito, op4 only for IXCB/IYCB
extern uint clock_cycles_on_branch (CpuID, uint8 op1, uint8 op2) noexcept;	// op2 only used if op1==0xED

extern bool z80_opcode_can_branch(uint8 op1, uint8 op2) noexcept;			// op2 only used if op1==0xED
extern uint z80_clock_cycles(uint8 op1, uint8 op2, uint8 op4) noexcept;		// dito, op4 only for IXCB/IYCB
extern uint z80_clock_cycles_on_branch(uint8 op1, uint8 op2) noexcept;		// op2 only used if op1==0xED

extern bool z180_opcode_can_branch(uint8 op1, uint8 op2) noexcept;			// op2 only used if op1==0xED
extern uint z180_clock_cycles(uint8 op1, uint8 op2, uint8 op4) noexcept;	// dito, op4 only for IXCB/IYCB
extern uint z180_clock_cycles_on_branch(uint8 op1, uint8 op2) noexcept;		// op2 only used if op1==0xED

extern bool i8080_opcode_can_branch(uint8 op) noexcept;
extern uint i8080_clock_cycles(uint8 op) noexcept;
extern uint i8080_clock_cycles_on_branch(uint8 op) noexcept;

