/*	Copyright  (c)	Günter Woigk 1995 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/


#include "kio/kio.h"
#include "z80_goodies.h"


static uchar const len0[64] = // opcodes 0x00 - 0x3F
{
	1,3,1,1,1,1,2,1, //	NOP,		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	1,1,1,1,1,1,2,1, //	EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	2,3,1,1,1,1,2,1, //	DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	2,1,1,1,1,1,2,1, //	JR, 		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	2,3,3,1,1,1,2,1, //	JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	2,1,3,1,1,1,2,1, //	JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	2,3,3,1,1,1,2,1, //	JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	2,1,3,1,1,1,2,1, //	JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
};

static uchar const len3[64] = // opcodes 0xC0 - 0xFF: prefixes are 0
{
	1,1,3,3,3,1,2,1, //	RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	1,1,3,0,3,3,2,1, //	RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	1,1,3,2,3,1,2,1, //	RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	1,1,3,2,3,0,2,1, //	RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	1,1,3,1,3,1,2,1, //	RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	1,1,3,1,3,0,2,1, //	RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	1,1,3,1,3,1,2,1, //	RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	1,1,3,1,3,0,2,1, //	RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
};


uint i8080_opcode_length (const Byte* ip) noexcept
{
	// Calculate length [bytes] of instruction

	uint8 op = peek(ip);

	switch (op>>6)
	{
	case 0:
		if ((op&7)==0) return 1;	// NOP and aliases => len=1
		else return len0[op];
	default:
		return 1;					// ld r,r and arith a,r
	case 3:
		uint n = len3[op-0xc0];		// 0xD9=RET: same length as Z80:EXX
		return n ? n : 3;			// prefixes are JP_NN or CALL_NN => len=3
	}
}

uint z80_opcode_length (const Byte* ip) noexcept
{
	// Calculate length of instruction
	// op2 is only used if op1 is a prefix instruction
	// IX/IY before IX/IY/ED have no effect and are reported as length 1

	uint8 op = peek(ip);

	// 0x00 - 0x3F:	from table
	if (op < 0x40) return len0[op];

	// 0x40 - 0xBF: ld r,r and arith/logic
	if (op < 0xC0) return 1;

	// 0xC0 .. 0xFF: from table, except prefixes
	uint n = len3[op & 0x3F];
	if (n) return n;

	// prefix 0xCB:
	if (op == 0xcb) return 2;

	// prefix 0xED: 2 bytes except "ld rr,(NN)" and "ld (NN),rr"
	if (op == 0xed) return (peek(ip+1) & 0xc7) == 0x43 ? 4 : 2;

	// prefix IX / IY:
	assert(op==0xdd || op==0xfd);
	op = peek(ip+1);

	// 0x00 - 0x3F: add +1 for dis in "inc(hl)", "dec(hl)", "ld(hl),N"
	if (op < 0x40) return 1 + len0[op] + (op>=0x34 && op<=0x36);

	// 0x40 - 0xBF: add +1 for dis in "ari a,(hl)" and "ld r,(hl)" and "ld (hl),r" except "halt"
	if (op < 0xC0) return 2 + (((op&0x07)==6) != ((op&0x0F8)==0x70));

	// 0xC0 - 0xFF: from table, except prefixes:

	// prefix CB: 4 bytes
	// note: options --ixcbr2 and --ixcbxh have no effect on the opcode length
	if (op==0xcb) return 4;

	// prefix ED, IX and IY:
	// the current IX/IY prefix has no effect and is handled like a NOP
	// so their length is 1 byte which is what we get when we add 1 (for prefix IX/IY)
	// to the value 0 from the table anyway. So no special handling for them needed.
	return 1 + len3[op&0x3F];
}

uint z180_opcode_length (const Byte* ip) noexcept
{
	// Calculate length [bytes] of instruction
	// op2 is only used if op1 is a prefix instruction
	// illegal opcodes are reported as for Z80

	// basic opcodes are all same as for z80.
	// prefix CB opcodes are same as for z80; illegal SLL as for z80.
	// prefix IX/IY opcodes as for z80;       illegal opcodes not using HL as for z80.
	// prefix IXCB/IYCB opcodes as for z80;   illegal SLL and opcodes not using HL as for z80.

	if (peek(ip) != 0xED) return z80_opcode_length(ip);

	// all new opcodes use prefix 0xED.
	// They are all same length as for z80:
	//	 4 bytes for "ld rr,(NN)" and "ld (NN),rr)"
	//   2 bytes for all other, whether legal or illegal NOP
	// except those new opcodes which require an immediate byte argument:
	//	 "out0 (N),r", "in0 r,(N)", "tst N" and "tstio N".

	uint8 op = peek(ip+1);
	if (op < 0x40) return 2 + (((op&7) < 2) && op != 0x31); // add +1 for IN0_r_xN / OUT0_xN_r
	if ((op & 0xC7) == 0x43) return 4;		// LD_rr_xNN / LD_xNN_rr
	if ((op | 0x10) == 0x74) return 3;		// TST_n, TSTIO_xN
	return 2;
}

uint cpu_opcode_length (CpuID cpuid, const Byte* ip) noexcept
{
	switch(cpuid)
	{
	default:
	case CpuZ80:  return z80_opcode_length(ip);
	case Cpu8080: return i8080_opcode_length(ip);
	case CpuZ180: return z180_opcode_length(ip);
	}
}







