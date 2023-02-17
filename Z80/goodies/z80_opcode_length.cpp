// Copyright (c) 1995 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


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


uint opcode_length(CpuID cpuid, const Byte* core, Address a) noexcept
{
	// Calculate length [bytes] of instruction
	// op2 is only used if op1 is a prefix instruction
	// IX/IY before IX/IY/ED have no effect and are reported as length 1

	// Z180: illegal opcodes are reported as for Z80
	// basic opcodes are all same as for z80.
	// prefix CB opcodes are same as for z80; illegal SLL as for z80.
	// prefix IX/IY opcodes as for z80;       illegal opcodes not using HL as for z80.
	// prefix IXCB/IYCB opcodes as for z80;   illegal SLL and opcodes not using HL as for z80.

	uint8 op = peek(core, a);

	if (op < 0x40) // 0x00 - 0x3F:	from table
	{
		if (cpuid == Cpu8080 && (op & 7) == 0)
			return 1; // 8080: NOP and aliases => len=1
		else
			return len0[op];
	}
	if (op < 0xC0) return 1;  // 0x40 - 0xBF: ld r,r and arith/logic
	uint n = len3[op & 0x3F]; // 0xC0 .. 0xFF: from table, except prefixes
	if (n) return n;		  // 8080: 0xD9=RET: same length as Z80:EXX

	if (cpuid == Cpu8080) return 3; // 8080: Z80 prefixes are JP_NN or CALL_NN => len=3

	// Z80 or Z180:

	if (op == 0xcb) return 2; // prefix 0xCB

	uint8 op2 = peek(core, a + 1);

	if (op == 0xed) // prefix 0xED
	{
		// Z80: prefix 0xED: 2 bytes except "ld rr,(NN)" and "ld (NN),rr"
		if (cpuid != CpuZ180) return (op2 & 0xc7) == 0x43 ? 4 : 2;

		// Z180: all new opcodes use prefix 0xED.
		// They are all same length as for Z80:
		//	 4 bytes for "ld rr,(NN)" and "ld (NN),rr)"
		//   2 bytes for all other, whether legal or illegal NOP
		// except those new opcodes which require an immediate byte argument:
		//	 "out0 (N),r", "in0 r,(N)", "tst N" and "tstio N".

		if (op2 < 0x40) return 2 + (((op2 & 7) < 2) && op2 != 0x31); // add +1 for IN0_r_xN / OUT0_xN_r
		if ((op2 & 0xC7) == 0x43) return 4;							 // LD_rr_xNN / LD_xNN_rr
		if ((op2 | 0x10) == 0x74) return 3;							 // TST_n, TSTIO_xN
		return 2;
	}

	// prefix IX / IY:
	assert(op == 0xdd || op == 0xfd);

	// 0x00 - 0x3F: add +1 for dis in "inc(hl)", "dec(hl)", "ld(hl),N"
	if (op2 < 0x40) return 1 + len0[op2] + (op2 >= 0x34 && op2 <= 0x36);

	// 0x40 - 0xBF: add +1 for dis in "ari a,(hl)" and "ld r,(hl)" and "ld (hl),r" except "halt"
	if (op2 < 0xC0) return 2 + (((op2 & 0x07) == 6) != ((op2 & 0x0F8) == 0x70));

	// 0xC0 - 0xFF: from table, except prefixes:

	// prefix CB: 4 bytes
	// note: options --ixcbr2 and --ixcbxh have no effect on the opcode length
	if (op2 == 0xcb) return 4;

	// note on prefix ED, IX and IY:
	// the current IX/IY prefix has no effect and is handled like a NOP
	// so their length is 1 byte which is what we get when we add 1 (for prefix IX/IY)
	// to the value 0 from the table anyway. So no special handling for them needed.
	return 1 + len3[op2 & 0x3F];
}
