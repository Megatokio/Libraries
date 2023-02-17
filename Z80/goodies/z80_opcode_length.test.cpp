// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "kio/kio.h"
#include "main.h"
#include "z80_goodies.h"
#include "z80_opcodes.h"


static const uint8 i8080_len[256] = {
	1, 3, 1, 1, 1, 1, 2, 1, //	NOP,		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	1, 1, 1, 1, 1, 1, 2, 1, //	NOP/EX_AF_AF,ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	1, 3, 1, 1, 1, 1, 2, 1, //	NOP/DJNZ,	LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	1, 1, 1, 1, 1, 1, 2, 1, //	NOP/JR, 	ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	1, 3, 3, 1, 1, 1, 2, 1, //	NOP/JR_NZ,	LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	1, 1, 3, 1, 1, 1, 2, 1, //	NOP/JR_Z,	ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	1, 3, 3, 1, 1, 1, 2, 1, //	NOP/JR_NC,	LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	1, 1, 3, 1, 1, 1, 2, 1, //	NOP/JR_C,	ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ld
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ld
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ari
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ari

	1, 1, 3, 3, 3, 1, 2, 1, //	RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	1, 1, 3, 3, 3, 3, 2, 1, //	RET_Z,		RET,		JP_Z,		JP/PFX_CB,	CALL_Z,		CALL,		ADC_N,		RST08,
	1, 1, 3, 2, 3, 1, 2, 1, //	RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	1, 1, 3, 2, 3, 3, 2, 1, //	RET_C,		EXX,		JP_C,		INA,		CALL_C,		CALL/PFX_IX, SBC_N,		RST18,
	1, 1, 3, 1, 3, 1, 2, 1, //	RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	1, 1, 3, 1, 3, 3, 2, 1, //	RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	CALL/PFX_ED, XOR_N,		RST28,
	1, 1, 3, 1, 3, 1, 2, 1, //	RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	1, 1, 3, 1, 3, 3, 2, 1, //	RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		CALL/PFX_IY, CP_N,		RST38
};

static const uint8 z80_len[256] = {
	1, 3, 1, 1, 1, 1, 2, 1, //	NOP,		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	1, 1, 1, 1, 1, 1, 2, 1, //	EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	2, 3, 1, 1, 1, 1, 2, 1, //	DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	2, 1, 1, 1, 1, 1, 2, 1, //	JR, 		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	2, 3, 3, 1, 1, 1, 2, 1, //	JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	2, 1, 3, 1, 1, 1, 2, 1, //	JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	2, 3, 3, 1, 1, 1, 2, 1, //	JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	2, 1, 3, 1, 1, 1, 2, 1, //	JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,

	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ld
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ld
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ari
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // ari

	1, 1, 3, 3, 3, 1, 2, 1, //	RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	1, 1, 3, 0, 3, 3, 2, 1, //	RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	1, 1, 3, 2, 3, 1, 2, 1, //	RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	1, 1, 3, 2, 3, 0, 2, 1, //	RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	1, 1, 3, 1, 3, 1, 2, 1, //	RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	1, 1, 3, 1, 3, 0, 2, 1, //	RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	1, 1, 3, 1, 3, 1, 2, 1, //	RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	1, 1, 3, 1, 3, 0, 2, 1, //	RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
};

static const uint8 z80_ix_len[256] =
{
	2,4,2,2,2,2,3,2, //	NOP,		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	2,2,2,2,2,2,3,2, //	EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	3,4,2,2,2,2,3,2, //	DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	3,2,2,2,2,2,3,2, //	JR, 		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	3,4,4,2,2,2,3,2, //	JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	3,2,4,2,2,2,3,2, //	JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	3,4,4,2,3,3,4,2, //	JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	3,2,4,2,2,2,3,2, //	JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,

	2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2,	// ld
	2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 3,3,3,3,3,3,2,3, 2,2,2,2,2,2,3,2,	// ld
	2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2,	// ari
	2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2, 2,2,2,2,2,2,3,2,	// ari

	// 0xCB: excluded (own list)
	// 0xDD, 0xED, 0xFD: size = 1 because the 0xDD/0xFD prefix has no effect

	2,2,4,4,4,2,3,2, //	RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	2,2,4,0,4,4,3,2, //	RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	2,2,4,3,4,2,3,2, //	RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	2,2,4,3,4,1,3,2, //	RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	2,2,4,2,4,2,3,2, //	RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	2,2,4,2,4,1,3,2, //	RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	2,2,4,2,4,2,3,2, //	RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	2,2,4,2,4,1,3,2, //	RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
};

static const uint8 z80_ed_len[256] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 4, 2, 2, 2, 2, // IN_B_xC,	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,	RETN,	IM_0,	LD_I_A,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_C_xC,	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	ED4C,	RETI,	ED4E,	LD_R_A,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_D_xC,	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	ED54,	ED55,	IM_1,	LD_A_I,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_E_xC,	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	ED5C,	ED5D,	IM_2,	LD_A_R,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_H_xC,	OUT_xC_H,	SBC_HL_HL,	ED_xNN_HL,	ED64,	ED65,	ED66,	RRD,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_L_xC,	OUT_xC_L,	ADC_HL_HL,	ED_HL_xNN,	ED6C,	ED6D,	ED6E,	RLD,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_F_xC,	NOP,		SBC_HL_SP,	LD_xNN_SP,	ED74,	ED75,	ED76,	ED77,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_A_xC,	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	ED7C,	ED7D,	ED7E,	ED7F,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

static const uint8 z180_ed_len[256] = {
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_B_xN  OUT0_xN_B	 NOP  TST_B	  NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_C_xN  OUT0_xN_C	 NOP  TST_C	  NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_D_xN  OUT0_xN_D	 NOP  TST_D	  NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_E_xN  OUT0_xN_E	 NOP  TST_E	  NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_H_xN  OUT0_xN_H	 NOP  TST_H	  NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_L_xN  OUT0_xN_L	 NOP  TST_L	  NOP  NOP  NOP  NOP
	3, 2, 2, 2, 2, 2, 2, 2, //	IN0_F_xN  ED31		 NOP  TST_xHL NOP  NOP  NOP  NOP
	3, 3, 2, 2, 2, 2, 2, 2, //	IN0_A_xN  OUT0_xN_A	 NOP  TST_A	  NOP  NOP  NOP  NOP

	2, 2, 2, 4, 2, 2, 2, 2, // IN_B_xC, OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,	RETN,	IM_0,	LD_I_A,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_C_xC, OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	ED4C,	RETI,	ED4E,	LD_R_A,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_D_xC, OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	ED54,	ED55,	IM_1,	LD_A_I,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_E_xC, OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	ED5C,	ED5D,	IM_2,	LD_A_R,
	2, 2, 2, 4, 3, 2, 2, 2, // IN_H_xC, OUT_xC_H,	SBC_HL_HL,	ED_xNN_HL,	TST_N,	ED65,	ED66,	RRD,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_L_xC, OUT_xC_L,	ADC_HL_HL,	ED_HL_xNN,	ED6C,	ED6D,	ED6E,	RLD,
	2, 2, 2, 4, 3, 2, 2, 2, // IN_F_xC, ED71		SBC_HL_SP,	LD_xNN_SP,	TSTIO,	ED75,	ED76,	ED77,
	2, 2, 2, 4, 2, 2, 2, 2, // IN_A_xC, OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	ED7C,	ED7D,	ED7E,	ED7F,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,

	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
};

void test_z80_opcode_length(uint& num_tests, uint& num_errors)
{
	logIn("test z80_opcode_length");

	// i8080:
	TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {uint8(i), 0, 0, 0};
		assert(opcode_length(Cpu8080, z) == i8080_len[i]);
	}
	END

		// z80 no pfx:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {uint8(i), 0, 0, 0};
		if (z80_len[i]) assert(opcode_length(CpuZ80, z) == z80_len[i]);
	}
	END

		// z180 no pfx:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {uint8(i), 0, 0, 0};
		if (z80_len[i]) assert(opcode_length(CpuZ180, z) == z80_len[i]);
	}
	END

		// z80 PFX_CB:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {0xcb, uint8(i), 0, 0};
		assert(opcode_length(CpuZ80, z) == 2);
	}
	END

		// z180 PFX_CB:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {0xcb, uint8(i), 0, 0};
		assert(opcode_length(CpuZ180, z) == 2);
	}
	END

		// z80 IX/IY + CB:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {uint8(i & 1 ? 0xdd : 0xfd), 0xcb, 0, uint8(i)};
		assert(opcode_length(CpuZ80, z) == 4);
	}
	END

		// z180 IX/IY + CB:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {uint8(i & 1 ? 0xfd : 0xdd), 0xcb, 0, uint8(i)};
		assert(opcode_length(CpuZ180, z) == 4); // sll as for z80 too
	}
	END

		// z80 IX/IY:
		TRY for (uint i = 0; i < 256; i++)
	{
		if (i == 0xcb) continue;
		uint8 z[4] = {uint8(i & 1 ? 0xdd : 0xfd), uint8(i), 0, 0};
		if (opcode_length(CpuZ80, z) != z80_ix_len[i]) logline("z80: ix,%02x failed", i);
		assert(opcode_length(CpuZ80, z) == z80_ix_len[i]);
	}
	END

		// z180 IX/IY:
		// note: illegal opcodes are reported as for z80
		TRY for (uint i = 0; i < 256; i++)
	{
		if (i == 0xcb) continue;
		uint8 z[4] = {uint8(i & 1 ? 0xfd : 0xdd), uint8(i), 0, 0};
		if (opcode_length(CpuZ180, z) != z80_ix_len[i]) logline("z180: ix,%02x failed", i);
		assert(opcode_length(CpuZ180, z) == z80_ix_len[i]);
	}
	END

		// z80 PFX_ED:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {0xed, uint8(i), 0, 0};
		assert(opcode_length(CpuZ80, z) == z80_ed_len[i]);
	}
	END

		// z180 PFX_ED:
		TRY for (uint i = 0; i < 256; i++)
	{
		uint8 z[4] = {0xed, uint8(i), 0, 0};
		if (opcode_length(CpuZ180, z) != z180_ed_len[i]) logline("z180: ed,%02x failed", i);
		assert(opcode_length(CpuZ180, z) == z180_ed_len[i]);
	}
	END

		TRY static const Byte ld_a_xix[] = {0xdd, LD_A_xHL, 0};
	assert(opcode_length(CpuZ180, ld_a_xix) == 3);
	static const Byte ld_xix_a[] = {0xdd, LD_xHL_A, 0};
	assert(opcode_length(CpuZ180, ld_xix_a) == 3);
	static const Byte ld_b_xix[] = {0xdd, LD_B_xHL, 0};
	assert(opcode_length(CpuZ180, ld_b_xix) == 3);
	static const Byte ld_xix_b[] = {0xdd, LD_xHL_B, 0};
	assert(opcode_length(CpuZ180, ld_xix_b) == 3);
	static const Byte ld_c_xix[] = {0xdd, LD_C_xHL, 0};
	assert(opcode_length(CpuZ180, ld_c_xix) == 3);
	static const Byte ld_xix_c[] = {0xdd, LD_xHL_C, 0};
	assert(opcode_length(CpuZ180, ld_xix_c) == 3);
	static const Byte ld_ix_halt[] = {0xdd, HALT, 0};
	assert(opcode_length(CpuZ180, ld_ix_halt) == 2);
	static const Byte or_xix[] = {0xdd, OR_xHL, 0};
	assert(opcode_length(CpuZ180, or_xix) == 3);
	static const Byte xor_xix[] = {0xdd, XOR_xHL, 0};
	assert(opcode_length(CpuZ180, xor_xix) == 3);
	static const Byte or_xh[] = {0xdd, OR_H, 0};
	assert(opcode_length(CpuZ80, or_xh) == 2);
	static const Byte xor_xl[] = {0xdd, XOR_L, 0};
	assert(opcode_length(CpuZ80, xor_xl) == 2);
	END
}
