// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "Z80/goodies/z80_goodies.h"
#include "Z80/goodies/z80_opcodes.h"
#include "kio/kio.h"
#include "main.h"

static const uint8 cc_i8080[256] = {
#undef Z
#define Z(A, B) A
	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// NOP**,		ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// NOP**,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// NOP**,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// NOP**,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// NOP**,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// NOP**,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// NOP**,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		4,			10,			16,			6,			4,			4,			7,			4,
		4,			11,			16,			6,			4,			4,			7,			4,
		4,			10,			13,			6,			11,			11,			10,			4,
		4,			11,			13,			6,			4,			4,			7,			4,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		7,			7,			7,			7,			7,			7,			4,			7,
		4,			4,			4,			4,			4,			4,			7,			4,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		JP**,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		RET**,		JP_C,		INA,		CALL_C,		CALL**,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	CALL**,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		CALL**,		CP_N,		RST38
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	19,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	4,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	4,			Z(10,17),	11,			7,			11,
		Z(5,11),	6,			Z(10,10),	4,			Z(10,17),	17,			7,			11,
};

static const uint8 cc_i8080_on_branch[256] =
{
	#undef Z
	#define Z(A,B) B
	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// NOP**,		ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// NOP**,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// NOP**,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// NOP**,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// NOP**,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// NOP**,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// NOP**,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		4,			10,			16,			6,			4,			4,			7,			4,
		4,			11,			16,			6,			4,			4,			7,			4,
		4,			10,			13,			6,			11,			11,			10,			4,
		4,			11,			13,			6,			4,			4,			7,			4,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		7,			7,			7,			7,			7,			7,			4,			7,
		4,			4,			4,			4,			4,			4,			7,			4,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		JP**,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		RET**,		JP_C,		INA,		CALL_C,		CALL**,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	CALL**,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		CALL**,		CP_N,		RST38
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	19,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	4,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	4,			Z(10,17),	11,			7,			11,
		Z(5,11),	6,			Z(10,10),	4,			Z(10,17),	17,			7,			11,
};

static const uint8 cc_z80[256] =
{
	#undef Z
	#define Z(A,B) A
	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		Z(8,13),	10,			7,			6,			4,			4,			7,			4,
		12,			11,			7,			6,			4,			4,			7,			4,
		Z(7,12),	10,			16,			6,			4,			4,			7,			4,
		Z(7,12),	11,			16,			6,			4,			4,			7,			4,
		Z(7,12),	10,			13,			6,			11,			11,			10,			4,
		Z(7,12),	11,			13,			6,			4,			4,			7,			4,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		7,			7,			7,			7,			7,			7,			4,			7,
		4,			4,			4,			4,			4,			4,			7,			4,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	0,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	11,			Z(10,17),	0,			7,			11,
		Z(5,11),	10,			Z(10,10),	19,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	4,			Z(10,17),	0,			7,			11,
		Z(5,11),	10,			Z(10,10),	4,			Z(10,17),	11,			7,			11,
		Z(5,11),	6,			Z(10,10),	4,			Z(10,17),	0,			7,			11,
};

static const uint8 cc_z80_on_branch[256] =
{
	#undef Z
	#define Z(A,B) B
	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		4,			10,			7,			6,			4,			4,			7,			4,
		4,			11,			7,			6,			4,			4,			7,			4,
		Z(8,13),	10,			7,			6,			4,			4,			7,			4,
		12,			11,			7,			6,			4,			4,			7,			4,
		Z(7,12),	10,			16,			6,			4,			4,			7,			4,
		Z(7,12),	11,			16,			6,			4,			4,			7,			4,
		Z(7,12),	10,			13,			6,			11,			11,			10,			4,
		Z(7,12),	11,			13,			6,			4,			4,			7,			4,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		7,			7,			7,			7,			7,			7,			4,			7,
		4,			4,			4,			4,			4,			4,			7,			4,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,
		4,			4,			4,			4,			4,			4,			7,			4,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
		Z(5,11),	10,			Z(10,10),	10,			Z(10,17),	11,			7,			11,
		Z(5,11),	10,			Z(10,10),	0,			Z(10,17),	17,			7,			11,
		Z(5,11),	10,			Z(10,10),	11,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	11,			Z(10,17),	0,			7,			11,
		Z(5,11),	10,			Z(10,10),	19,			Z(10,17),	11,			7,			11,
		Z(5,11),	4,			Z(10,10),	4,			Z(10,17),	0,			7,			11,
		Z(5,11),	10,			Z(10,10),	4,			Z(10,17),	11,			7,			11,
		Z(5,11),	6,			Z(10,10),	4,			Z(10,17),	0,			7,			11,
};

static const uint8 cc_z180[256] =
{
	#undef Z
	#define Z(A,B) A

	// normal 1-byte opcodes
	// Prefix opcodes are set to 0.
	// The time for the entire opcode is given in their respective sub table.
	// Note: the original Z80 table set PFX_IX and PFX_IY to 4cc.

	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		3u,			9u,			7u,			4u,			4u,			4u,			6u,			3u,
		4u,			7u,			6u,			4u,			4u,			4u,			6u,			3u,
		Z(7u,9u),	9u,			7u,			4u,			4u,			4u,			6u,			3u,
		8u,			7u,			6u,			4u,			4u,			4u,			6u,			3u,
		Z(6u,8u),	9u,			16u, 		4u,			4u,			4u,			6u,			4u,
		Z(6u,8u),	7u,			15u, 		4u,			4u,			4u,			6u,			3u,
		Z(6u,8u),	9u,			13u, 		4u,			10u, 		10u, 		9u,			3u,
		Z(6u,8u),	7u,			12u, 		4u,			4u,			4u,			6u,			3u,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		7u,			7u,			7u,			7u,			7u,			7u,			3u,			7u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
		Z(5u,10u),	9u,			Z(6u,9u),	9u,			Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	0u,			Z(6u,16u),	16u, 		6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	10u, 		Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	3u,			Z(6u,9u),	9u,			Z(6u,16u),	0u,			6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	16u, 		Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	3u,			Z(6u,9u),	3u,			Z(6u,16u),	0u,			6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	3u,			Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	4u,			Z(6u,9u),	3u,			Z(6u,16u),	0u,			6u,			11u,
};

static const uint8 cc_z180_on_branch[256] =
{
	#undef Z
	#define Z(A,B) B

	// normal 1-byte opcodes
	// Prefix opcodes are set to 0.
	// The time for the entire opcode is given in their respective sub table.
	// Note: the original Z80 table set PFX_IX and PFX_IY to 4cc.

	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		3u,			9u,			7u,			4u,			4u,			4u,			6u,			3u,
		4u,			7u,			6u,			4u,			4u,			4u,			6u,			3u,
		Z(7u,9u),	9u,			7u,			4u,			4u,			4u,			6u,			3u,
		8u,			7u,			6u,			4u,			4u,			4u,			6u,			3u,
		Z(6u,8u),	9u,			16u, 		4u,			4u,			4u,			6u,			4u,
		Z(6u,8u),	7u,			15u, 		4u,			4u,			4u,			6u,			3u,
		Z(6u,8u),	9u,			13u, 		4u,			10u, 		10u, 		9u,			3u,
		Z(6u,8u),	7u,			12u, 		4u,			4u,			4u,			6u,			3u,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		7u,			7u,			7u,			7u,			7u,			7u,			3u,			7u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,
		4u,			4u,			4u,			4u,			4u,			4u,			6u,			4u,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
		Z(5u,10u),	9u,			Z(6u,9u),	9u,			Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	0u,			Z(6u,16u),	16u, 		6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	10u, 		Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	3u,			Z(6u,9u),	9u,			Z(6u,16u),	0u,			6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	16u, 		Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	3u,			Z(6u,9u),	3u,			Z(6u,16u),	0u,			6u,			11u,
		Z(5u,10u),	9u,			Z(6u,9u),	3u,			Z(6u,16u),	11u, 		6u,			11u,
		Z(5u,10u),	4u,			Z(6u,9u),	3u,			Z(6u,16u),	0u,			6u,			11u,
};

static const uint8 cc_z80_CB[256] =
{
	// Table for CBxx opcodes:
	// (this table is easy to compute)
	// times are stored for the entire opcode incl. CB
	// so the minimum time is 8 cycles
	// illegal opcodes are marked with **

	// RLC_B,		RLC_C,		RLC_D,		RLC_E,		RLC_H,		RLC_L,		RLC_xHL,	RLC_A,
	// RRC_B,		RRC_C,		RRC_D,		RRC_E,		RRC_H,		RRC_L,		RRC_xHL,	RRC_A,
	// RL_B,		RL_C,		RL_D,		RL_E,		RL_H,		RL_L,		RL_xHL,		RL_A,
	// RR_B,		RR_C,		RR_D,		RR_E,		RR_H,		RR_L,		RR_xHL,		RR_A,
	// SLA_B,		SLA_C,		SLA_D,		SLA_E,		SLA_H,		SLA_L,		SLA_xHL,	SLA_A,
	// SRA_B,		SRA_C,		SRA_D,		SRA_E,		SRA_H,		SRA_L,		SRA_xHL,	SRA_A,
	// SLL_B**,		SLL_C**,	SLL_D**,	SLL_E**,	SLL_H**,	SLL_L**,	SLL_xHL**,	SLL_A**,
	// SRL_B,		SRL_C,		SRL_D,		SRL_E,		SRL_H,		SRL_L,		SRL_xHL,	SRL_A,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,

	// BIT0_B,		BIT0_C,		BIT0_D,		BIT0_E,		BIT0_H,		BIT0_L,		BIT0_xHL,	BIT0_A,
	// BIT1_B,		BIT1_C,		BIT1_D,		BIT1_E,		BIT1_H,		BIT1_L,		BIT1_xHL,	BIT1_A,
	// BIT2_B,		BIT2_C,		BIT2_D,		BIT2_E,		BIT2_H,		BIT2_L,		BIT2_xHL,	BIT2_A,
	// BIT3_B,		BIT3_C,		BIT3_D,		BIT3_E,		BIT3_H,		BIT3_L,		BIT3_xHL,	BIT3_A,
	// BIT4_B,		BIT4_C,		BIT4_D,		BIT4_E,		BIT4_H,		BIT4_L,		BIT4_xHL,	BIT4_A,
	// BIT5_B,		BIT5_C,		BIT5_D,		BIT5_E,		BIT5_H,		BIT5_L,		BIT5_xHL,	BIT5_A,
	// BIT6_B,		BIT6_C,		BIT6_D,		BIT6_E,		BIT6_H,		BIT6_L,		BIT6_xHL,	BIT6_A,
	// BIT7_B,		BIT7_C,		BIT7_D,		BIT7_E,		BIT7_H,		BIT7_L,		BIT7_xHL,	BIT7_A,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,
		8,			8,			8,			8,			8,			8,			12,			8,

	// RES0_B,		RES0_C,		RES0_D,		RES0_E,		RES0_H,		RES0_L,		RES0_xHL,	RES0_A,
	// RES1_B,		RES1_C,		RES1_D,		RES1_E,		RES1_H,		RES1_L,		RES1_xHL,	RES1_A,
	// RES2_B,		RES2_C,		RES2_D,		RES2_E,		RES2_H,		RES2_L,		RES2_xHL,	RES2_A,
	// RES3_B,		RES3_C,		RES3_D,		RES3_E,		RES3_H,		RES3_L,		RES3_xHL,	RES3_A,
	// RES4_B,		RES4_C,		RES4_D,		RES4_E,		RES4_H,		RES4_L,		RES4_xHL,	RES4_A,
	// RES5_B,		RES5_C,		RES5_D,		RES5_E,		RES5_H,		RES5_L,		RES5_xHL,	RES5_A,
	// RES6_B,		RES6_C,		RES6_D,		RES6_E,		RES6_H,		RES6_L,		RES6_xHL,	RES6_A,
	// RES7_B,		RES7_C,		RES7_D,		RES7_E,		RES7_H,		RES7_L,		RES7_xHL,	RES7_A,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,

	// SET0_B,		SET0_C,		SET0_D,		SET0_E,		SET0_H,		SET0_L,		SET0_xHL,	SET0_A,
	// SET1_B,		SET1_C,		SET1_D,		SET1_E,		SET1_H,		SET1_L,		SET1_xHL,	SET1_A,
	// SET2_B,		SET2_C,		SET2_D,		SET2_E,		SET2_H,		SET2_L,		SET2_xHL,	SET2_A,
	// SET3_B,		SET3_C,		SET3_D,		SET3_E,		SET3_H,		SET3_L,		SET3_xHL,	SET3_A,
	// SET4_B,		SET4_C,		SET4_D,		SET4_E,		SET4_H,		SET4_L,		SET4_xHL,	SET4_A,
	// SET5_B,		SET5_C,		SET5_D,		SET5_E,		SET5_H,		SET5_L,		SET5_xHL,	SET5_A,
	// SET6_B,		SET6_C,		SET6_D,		SET6_E,		SET6_H,		SET6_L,		SET6_xHL,	SET6_A,
	// SET7_B,		SET7_C,		SET7_D,		SET7_E,		SET7_H,		SET7_L,		SET7_xHL,	SET7_A
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8,
		8,			8,			8,			8,			8,			8,			15,			8
};

static const uint8 cc_z180_CB[256] =
{
	// Table for CBxx opcodes:
	// (this table is easy to compute)
	// times are for the entire opcode incl. CB
	// illegal opcodes SLL are trapped by the Z80180 and marked with **

	// RLC_B,		RLC_C,		RLC_D,		RLC_E,		RLC_H,		RLC_L,		RLC_xHL,	RLC_A,
	// RRC_B,		RRC_C,		RRC_D,		RRC_E,		RRC_H,		RRC_L,		RRC_xHL,	RRC_A,
	// RL_B,		RL_C,		RL_D,		RL_E,		RL_H,		RL_L,		RL_xHL,		RL_A,
	// RR_B,		RR_C,		RR_D,		RR_E,		RR_H,		RR_L,		RR_xHL,		RR_A,
	// SLA_B,		SLA_C,		SLA_D,		SLA_E,		SLA_H,		SLA_L,		SLA_xHL,	SLA_A,
	// SRA_B,		SRA_C,		SRA_D,		SRA_E,		SRA_H,		SRA_L,		SRA_xHL,	SRA_A,
	// SLL_B**,		SLL_C**,	SLL_D**,	SLL_E**,	SLL_H**,	SLL_L**,	SLL_xHL**,	SLL_A**,
	// SRL_B,		SRL_C,		SRL_D,		SRL_E,		SRL_H,		SRL_L,		SRL_xHL,	SRL_A,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,
		0,			0,			0,			0,			0,			0,			0,			0,
		7u,			7u,			7u,			7u,			7u,			7u,			13u, 		7u,

	// BIT0_B,		BIT0_C,		BIT0_D,		BIT0_E,		BIT0_H,		BIT0_L,		BIT0_xHL,	BIT0_A,
	// BIT1_B,		BIT1_C,		BIT1_D,		BIT1_E,		BIT1_H,		BIT1_L,		BIT1_xHL,	BIT1_A,
	// BIT2_B,		BIT2_C,		BIT2_D,		BIT2_E,		BIT2_H,		BIT2_L,		BIT2_xHL,	BIT2_A,
	// BIT3_B,		BIT3_C,		BIT3_D,		BIT3_E,		BIT3_H,		BIT3_L,		BIT3_xHL,	BIT3_A,
	// BIT4_B,		BIT4_C,		BIT4_D,		BIT4_E,		BIT4_H,		BIT4_L,		BIT4_xHL,	BIT4_A,
	// BIT5_B,		BIT5_C,		BIT5_D,		BIT5_E,		BIT5_H,		BIT5_L,		BIT5_xHL,	BIT5_A,
	// BIT6_B,		BIT6_C,		BIT6_D,		BIT6_E,		BIT6_H,		BIT6_L,		BIT6_xHL,	BIT6_A,
	// BIT7_B,		BIT7_C,		BIT7_D,		BIT7_E,		BIT7_H,		BIT7_L,		BIT7_xHL,	BIT7_A,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,
		6u,			6u,			6u,			6u,			6u,			6u,			9u,			6u,

	// RES0_B,		RES0_C,		RES0_D,		RES0_E,		RES0_H,		RES0_L,		RES0_xHL,	RES0_A,
	// RES1_B,		RES1_C,		RES1_D,		RES1_E,		RES1_H,		RES1_L,		RES1_xHL,	RES1_A,
	// RES2_B,		RES2_C,		RES2_D,		RES2_E,		RES2_H,		RES2_L,		RES2_xHL,	RES2_A,
	// RES3_B,		RES3_C,		RES3_D,		RES3_E,		RES3_H,		RES3_L,		RES3_xHL,	RES3_A,
	// RES4_B,		RES4_C,		RES4_D,		RES4_E,		RES4_H,		RES4_L,		RES4_xHL,	RES4_A,
	// RES5_B,		RES5_C,		RES5_D,		RES5_E,		RES5_H,		RES5_L,		RES5_xHL,	RES5_A,
	// RES6_B,		RES6_C,		RES6_D,		RES6_E,		RES6_H,		RES6_L,		RES6_xHL,	RES6_A,
	// RES7_B,		RES7_C,		RES7_D,		RES7_E,		RES7_H,		RES7_L,		RES7_xHL,	RES7_A,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,

	// SET0_B,		SET0_C,		SET0_D,		SET0_E,		SET0_H,		SET0_L,		SET0_xHL,	SET0_A,
	// SET1_B,		SET1_C,		SET1_D,		SET1_E,		SET1_H,		SET1_L,		SET1_xHL,	SET1_A,
	// SET2_B,		SET2_C,		SET2_D,		SET2_E,		SET2_H,		SET2_L,		SET2_xHL,	SET2_A,
	// SET3_B,		SET3_C,		SET3_D,		SET3_E,		SET3_H,		SET3_L,		SET3_xHL,	SET3_A,
	// SET4_B,		SET4_C,		SET4_D,		SET4_E,		SET4_H,		SET4_L,		SET4_xHL,	SET4_A,
	// SET5_B,		SET5_C,		SET5_D,		SET5_E,		SET5_H,		SET5_L,		SET5_xHL,	SET5_A,
	// SET6_B,		SET6_C,		SET6_D,		SET6_E,		SET6_H,		SET6_L,		SET6_xHL,	SET6_A,
	// SET7_B,		SET7_C,		SET7_D,		SET7_E,		SET7_H,		SET7_L,		SET7_xHL,	SET7_A
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
		6u,			6u,			6u,			6u,			6u,			6u,			13u, 		6u,
};

static const uint8 cc_z80_XYCB[256] =
{
	// Table for opcode after DDCB / FDCB:
	// legal ones are only those with memory access (XY+dis)
	// all other opcodes are illegal.
	// Depending on the exact Z80 type they behave like --ixcbr2 or --ixcbxh.
	// In any case the major opcode is the 4th byte of the whole opcode and thus all 4 bytes are always read
	// and the offset calculation is done before the major opcode is seen.
	// In --ixcbr2 mode the calculation of ix+dis and load/store is always done so all illegals must take the whole time.
	// In --ixcbxh mode the calculation of ix+dis might be aborted after the 4th byte is read, but that's unlikely,
	//		and then the memory is probably not read or written which would save the time for this cycle(s).	<-- TODO!

	// RLC_B,		RLC_C,		RLC_D,		RLC_E,		RLC_H,		RLC_L,		RLC_xHL,	RLC_A,
	// RRC_B,		RRC_C,		RRC_D,		RRC_E,		RRC_H,		RRC_L,		RRC_xHL,	RRC_A,
	// RL_B,		RL_C,		RL_D,		RL_E,		RL_H,		RL_L,		RL_xHL,		RL_A,
	// RR_B,		RR_C,		RR_D,		RR_E,		RR_H,		RR_L,		RR_xHL,		RR_A,
	// SLA_B,		SLA_C,		SLA_D,		SLA_E,		SLA_H,		SLA_L,		SLA_xHL,	SLA_A,
	// SRA_B,		SRA_C,		SRA_D,		SRA_E,		SRA_H,		SRA_L,		SRA_xHL,	SRA_A,
	// SLL_B,		SLL_C,		SLL_D,		SLL_E,		SLL_H,		SLL_L,		SLL_xHL,	SLL_A,
	// SRL_B,		SRL_C,		SRL_D,		SRL_E,		SRL_H,		SRL_L,		SRL_xHL,	SRL_A,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,

	// BIT0_B,		BIT0_C,		BIT0_D,		BIT0_E,		BIT0_H,		BIT0_L,		BIT0_xHL,	BIT0_A,
	// BIT1_B,		BIT1_C,		BIT1_D,		BIT1_E,		BIT1_H,		BIT1_L,		BIT1_xHL,	BIT1_A,
	// BIT2_B,		BIT2_C,		BIT2_D,		BIT2_E,		BIT2_H,		BIT2_L,		BIT2_xHL,	BIT2_A,
	// BIT3_B,		BIT3_C,		BIT3_D,		BIT3_E,		BIT3_H,		BIT3_L,		BIT3_xHL,	BIT3_A,
	// BIT4_B,		BIT4_C,		BIT4_D,		BIT4_E,		BIT4_H,		BIT4_L,		BIT4_xHL,	BIT4_A,
	// BIT5_B,		BIT5_C,		BIT5_D,		BIT5_E,		BIT5_H,		BIT5_L,		BIT5_xHL,	BIT5_A,
	// BIT6_B,		BIT6_C,		BIT6_D,		BIT6_E,		BIT6_H,		BIT6_L,		BIT6_xHL,	BIT6_A,
	// BIT7_B,		BIT7_C,		BIT7_D,		BIT7_E,		BIT7_H,		BIT7_L,		BIT7_xHL,	BIT7_A,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,
		20,			20,			20,			20,			20,			20,			20,			20,

	// RES0_B,		RES0_C,		RES0_D,		RES0_E,		RES0_H,		RES0_L,		RES0_xHL,	RES0_A,
	// RES1_B,		RES1_C,		RES1_D,		RES1_E,		RES1_H,		RES1_L,		RES1_xHL,	RES1_A,
	// RES2_B,		RES2_C,		RES2_D,		RES2_E,		RES2_H,		RES2_L,		RES2_xHL,	RES2_A,
	// RES3_B,		RES3_C,		RES3_D,		RES3_E,		RES3_H,		RES3_L,		RES3_xHL,	RES3_A,
	// RES4_B,		RES4_C,		RES4_D,		RES4_E,		RES4_H,		RES4_L,		RES4_xHL,	RES4_A,
	// RES5_B,		RES5_C,		RES5_D,		RES5_E,		RES5_H,		RES5_L,		RES5_xHL,	RES5_A,
	// RES6_B,		RES6_C,		RES6_D,		RES6_E,		RES6_H,		RES6_L,		RES6_xHL,	RES6_A,
	// RES7_B,		RES7_C,		RES7_D,		RES7_E,		RES7_H,		RES7_L,		RES7_xHL,	RES7_A,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,

	// SET0_B,		SET0_C,		SET0_D,		SET0_E,		SET0_H,		SET0_L,		SET0_xHL,	SET0_A,
	// SET1_B,		SET1_C,		SET1_D,		SET1_E,		SET1_H,		SET1_L,		SET1_xHL,	SET1_A,
	// SET2_B,		SET2_C,		SET2_D,		SET2_E,		SET2_H,		SET2_L,		SET2_xHL,	SET2_A,
	// SET3_B,		SET3_C,		SET3_D,		SET3_E,		SET3_H,		SET3_L,		SET3_xHL,	SET3_A,
	// SET4_B,		SET4_C,		SET4_D,		SET4_E,		SET4_H,		SET4_L,		SET4_xHL,	SET4_A,
	// SET5_B,		SET5_C,		SET5_D,		SET5_E,		SET5_H,		SET5_L,		SET5_xHL,	SET5_A,
	// SET6_B,		SET6_C,		SET6_D,		SET6_E,		SET6_H,		SET6_L,		SET6_xHL,	SET6_A,
	// SET7_B,		SET7_C,		SET7_D,		SET7_E,		SET7_H,		SET7_L,		SET7_xHL,	SET7_A
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
		23,			23,			23,			23,			23,			23,			23,			23,
};

static const uint8 cc_z180_XYCB[256] =
{
	// Table for opcode after DDCB / FDCB:
	// Legal ones are only those with memory access (XY+dis).
	// All other opcodes are illegal and trapped by the Z80180.
	// Timing is for all 4 opcode bytes: DD/FD, CB, dis and opcode.

	// RLC_B,		RLC_C,		RLC_D,		RLC_E,		RLC_H,		RLC_L,		RLC_xHL,	RLC_A,
	// RRC_B,		RRC_C,		RRC_D,		RRC_E,		RRC_H,		RRC_L,		RRC_xHL,	RRC_A,
	// RL_B,		RL_C,		RL_D,		RL_E,		RL_H,		RL_L,		RL_xHL,		RL_A,
	// RR_B,		RR_C,		RR_D,		RR_E,		RR_H,		RR_L,		RR_xHL,		RR_A,
	// SLA_B,		SLA_C,		SLA_D,		SLA_E,		SLA_H,		SLA_L,		SLA_xHL,	SLA_A,
	// SRA_B,		SRA_C,		SRA_D,		SRA_E,		SRA_H,		SRA_L,		SRA_xHL,	SRA_A,
	// SLL_B,		SLL_C,		SLL_D,		SLL_E,		SLL_H,		SLL_L,		SLL_xHL,	SLL_A,
	// SRL_B,		SRL_C,		SRL_D,		SRL_E,		SRL_H,		SRL_L,		SRL_xHL,	SRL_A,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			19u, 		0,

	// BIT0_B,		BIT0_C,		BIT0_D,		BIT0_E,		BIT0_H,		BIT0_L,		BIT0_xHL,	BIT0_A,
	// BIT1_B,		BIT1_C,		BIT1_D,		BIT1_E,		BIT1_H,		BIT1_L,		BIT1_xHL,	BIT1_A,
	// BIT2_B,		BIT2_C,		BIT2_D,		BIT2_E,		BIT2_H,		BIT2_L,		BIT2_xHL,	BIT2_A,
	// BIT3_B,		BIT3_C,		BIT3_D,		BIT3_E,		BIT3_H,		BIT3_L,		BIT3_xHL,	BIT3_A,
	// BIT4_B,		BIT4_C,		BIT4_D,		BIT4_E,		BIT4_H,		BIT4_L,		BIT4_xHL,	BIT4_A,
	// BIT5_B,		BIT5_C,		BIT5_D,		BIT5_E,		BIT5_H,		BIT5_L,		BIT5_xHL,	BIT5_A,
	// BIT6_B,		BIT6_C,		BIT6_D,		BIT6_E,		BIT6_H,		BIT6_L,		BIT6_xHL,	BIT6_A,
	// BIT7_B,		BIT7_C,		BIT7_D,		BIT7_E,		BIT7_H,		BIT7_L,		BIT7_xHL,	BIT7_A,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,
		0,			0,			0,			0,			0,			0,			15u, 		0,

	// RES0_B,		RES0_C,		RES0_D,		RES0_E,		RES0_H,		RES0_L,		RES0_xHL,	RES0_A,
	// RES1_B,		RES1_C,		RES1_D,		RES1_E,		RES1_H,		RES1_L,		RES1_xHL,	RES1_A,
	// RES2_B,		RES2_C,		RES2_D,		RES2_E,		RES2_H,		RES2_L,		RES2_xHL,	RES2_A,
	// RES3_B,		RES3_C,		RES3_D,		RES3_E,		RES3_H,		RES3_L,		RES3_xHL,	RES3_A,
	// RES4_B,		RES4_C,		RES4_D,		RES4_E,		RES4_H,		RES4_L,		RES4_xHL,	RES4_A,
	// RES5_B,		RES5_C,		RES5_D,		RES5_E,		RES5_H,		RES5_L,		RES5_xHL,	RES5_A,
	// RES6_B,		RES6_C,		RES6_D,		RES6_E,		RES6_H,		RES6_L,		RES6_xHL,	RES6_A,
	// RES7_B,		RES7_C,		RES7_D,		RES7_E,		RES7_H,		RES7_L,		RES7_xHL,	RES7_A,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,

	// SET0_B,		SET0_C,		SET0_D,		SET0_E,		SET0_H,		SET0_L,		SET0_xHL,	SET0_A,
	// SET1_B,		SET1_C,		SET1_D,		SET1_E,		SET1_H,		SET1_L,		SET1_xHL,	SET1_A,
	// SET2_B,		SET2_C,		SET2_D,		SET2_E,		SET2_H,		SET2_L,		SET2_xHL,	SET2_A,
	// SET3_B,		SET3_C,		SET3_D,		SET3_E,		SET3_H,		SET3_L,		SET3_xHL,	SET3_A,
	// SET4_B,		SET4_C,		SET4_D,		SET4_E,		SET4_H,		SET4_L,		SET4_xHL,	SET4_A,
	// SET5_B,		SET5_C,		SET5_D,		SET5_E,		SET5_H,		SET5_L,		SET5_xHL,	SET5_A,
	// SET6_B,		SET6_C,		SET6_D,		SET6_E,		SET6_H,		SET6_L,		SET6_xHL,	SET6_A,
	// SET7_B,		SET7_C,		SET7_D,		SET7_E,		SET7_H,		SET7_L,		SET7_xHL,	SET7_A
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
		0,			0,			0,			0,			0,			0,			19u, 		0,
};

static const uint8 cc_z80_XY[256] =
{
	// Opcodes with prefix DD and FD use the IX resp. IY register instead of HL.
	// times are given for the entire opcode incl. DD/FD
	// so the minimum time for opcodes which behave different after DD/FD is 8 cycles.
	// 4 indicates that the DD/FD prefix has no effect on this opcode and the preceding DD/FD
	// is effectively a NOP after which this byte starts a new opcode
	// 0 for PFX_CB which combines to a DDCB…/FDCB… opcode and is covered in table cc_XYCB.
	// illegal opcodes are marked with **

	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H**,	DEC_H**,	LD_H_N**,	DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L**,	DEC_L**,	LD_L_N**,	CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			15,			4,			4,			4,			4,			4,			4,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			15,			4,			4,			4,			4,			4,			4,
		4,			14,			20,			10,			8,			8,			11,			4,
		4,			15,			20,			10,			8,			8,			11,			4,
		4,			4,			4,			4,			23,			23,			19,			4,
		4,			15,			4,			4,			4,			4,			4,			4,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H**,	LD_B_L**,	LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H**,	LD_C_L**,	LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H**,	LD_D_L**,	LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H**,	LD_E_L**,	LD_E_xHL,	LD_E_A,
	// LD_H_B**,	LD_H_C**,	LD_H_D**,	LD_H_E**,	LD_H_H**,	LD_H_L**,	LD_H_xHL,	LD_H_A**,
	// LD_L_B**,	LD_L_C**,	LD_L_D**,	LD_L_E**,	LD_L_H**,	LD_L_L**,	LD_L_xHL,	LD_L_A**,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H**,	LD_A_L**,	LD_A_xHL,	LD_A_A,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		8,			8,			8,			8,			8,			8,			19,			8,
		8,			8,			8,			8,			8,			8,			19,			8,
		19,			19,			19,			19,			19,			19,			4,			19,
		4,			4,			4,			4,			8,			8,			19,			4,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H**,	ADD_L**,	ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H**,	ADC_L**,	ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H**,	SUB_L**,	SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H**,	SBC_L**,	SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H**,	AND_L**,	AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H**,	XOR_L**,	XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H**,		OR_L**,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H**,		CP_L**,		CP_xHL,		CP_A,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,
		4,			4,			4,			4,			8,			8,			19,			4,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			4,			4,			0,			4,			4,			4,			4,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			14,			4,			23,			4,			15,			4,			4,
		4,			8,			4,			4,			4,			4,			4,			4,
		4,			4,			4,			4,			4,			4,			4,			4,
		4,			10,			4,			4,			4,			4,			4,			4,
};

static const uint8 cc_z180_XY[256] =
{
	// Opcodes with prefix DD and FD use the IX resp. IY register instead of HL.
	// Times are given for the entire opcode incl. DD/FD.
	// Legal opcodes are only those which would use register HL if not prefixed with DD or FD.
	// All illegal opcodes are trapped by the Z80180.

	// NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	// EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	// DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	// JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			10u, 		0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			10u, 		0,			0,			0,			0,			0,			0,
	// JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	// JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	// JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	// JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,
		0,			12u,		19u, 		7u,			0,			0,			0,			0,
		0,			10u, 		18u, 		7u,			0,			0,			0,			0,
		0,			0,			0,			0,			18u, 		18u, 		15u, 		0,
		0,			10u, 		0,			0,			0,			0,			0,			0,

	// LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	// LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	// LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	// LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	// LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	// LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	// LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	// LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		15u, 		15u, 		15u, 		15u, 		15u, 		15u, 		0,			15u,
		0,			0,			0,			0,			0,			0,			14u, 		0,

	// ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	// ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	// SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	// SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	// AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	// XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	// OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	// CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,
		0,			0,			0,			0,			0,			0,			14u, 		0,

	// RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	// RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	// RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	// RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	// RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	// RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	// RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	// RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			12u, 		0,			19u, 		0,			14u, 		0,			0,
		0,			6u,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			7u,			0,			0,			0,			0,			0,			0,
};

static const uint8 cc_z80_ED[256] =
{
	#undef Z
	#define Z(A,B) A

	// Table for EDxx opcodes:
	// times are stored for the entire opcode incl. ED
	// so the minimum time is 8 cycles
	// illegal non-NOP opcodes are marked with **
	// illegal NOPs are marked with their opcode

	// ED00,		ED01,		ED02,		ED03,		ED04,		ED05,		ED06,		ED07,
	// ED08,		ED09,		ED0A,		ED0B,		ED0C,		ED0D,		ED0E,		ED0F,
	// ED10,		ED11,		ED12,		ED13,		ED14,		ED15,		ED16,		ED17,
	// ED18,		ED19,		ED1A,		ED1B,		ED1C,		ED1D,		ED1E,		ED1F,
	// ED20,		ED21,		ED22,		ED23,		ED24,		ED25,		ED26,		ED27,
	// ED28,		ED29,		ED2A,		ED2B,		ED2C,		ED2D,		ED2E,		ED2F,
	// ED30,		ED31,		ED32,		ED33,		ED34,		ED35,		ED36,		ED37,
	// ED38,		ED39,		ED3A,		ED3B,		ED3C,		ED3D,		ED3E,		ED3F,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,

	// IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	// IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	NEG**,		RETI,		IM0**,		LD_R_A,
	// IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	NEG**,		RETI**,		IM_1,		LD_A_I,
	// IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	NEG**,		RETI**,		IM_2,		LD_A_R,
	// IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	NEG**,		RETI**,		IM0**,		RRD,
	// IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	NEG**,		RETI**,		IM0**,		RLD,
	// IN_F_xC, 	OUT_xC_0,	SBC_HL_SP,	LD_xNN_SP,	NEG**,		RETI**,		IM1**,		ED77,
	// IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	NEG**,		RETI**,		IM2**,		ED7F,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			18,
		12,			12,			15,			20,			8,			14,			8,			18,
		12,			12,			15,			20,			8,			14,			8,			8,
		12,			12,			15,			20,			8,			14,			8,			8,

	// ED80,		ED81,		ED82,		ED83,		ED84,		ED85,		ED86,		ED87,
	// ED88,		ED89,		ED8A,		ED8B,		ED8C,		ED8D,		ED8E,		ED8F,
	// ED90,		ED91,		ED92,		ED93,		ED94,		ED95,		ED96,		ED97,
	// ED98,		ED99,		ED9A,		ED9B,		ED9C,		ED9D,		ED9E,		ED9F,
	// LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	// LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	// LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	// LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		16,			16,			16,			16,			8,			8,			8,			8,
		16,			16,			16,			16,			8,			8,			8,			8,
		Z(16,21),	Z(16,21),	Z(16,21),	Z(16,21),	8,			8,			8,			8,
		Z(16,21),	Z(16,21),	Z(16,21),	Z(16,21),	8,			8,			8,			8,

	// EDC0,		EDC1,		EDC2,		EDC3,		EDC4,		EDC5,		EDC6,		EDC7,
	// EDC8,		EDC9,		EDCA,		EDCB,		EDCC,		EDCD,		EDCE,		EDCF,
	// EDD0,		EDD1,		EDD2,		EDD3,		EDD4,		EDD5,		EDD6,		EDD7,
	// EDD8,		EDD9,		EDDA,		EDDB,		EDDC,		EDDD,		EDDE,		EDDF,
	// EDE0,		EDE1,		EDE2,		EDE3,		EDE4,		EDE5,		EDE6,		EDE7,
	// EDE8,		EDE9,		EDEA,		EDEB,		EDEC,		EDED,		EDEE,		EDEF,
	// EDF0,		EDF1,		EDF2,		EDF3,		EDF4,		EDF5,		EDF6,		EDF7,
	// EDF8,		EDF9,		EDFA,		EDFB,		EDFC,		EDFD,		EDFE,		EDFF
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8
};

static const uint8 cc_z80_ED_on_branch[256] =
{
	#undef Z
	#define Z(A,B) B

	// Table for EDxx opcodes:
	// times are stored for the entire opcode incl. ED
	// so the minimum time is 8 cycles
	// illegal non-NOP opcodes are marked with **
	// illegal NOPs are marked with their opcode

	// ED00,		ED01,		ED02,		ED03,		ED04,		ED05,		ED06,		ED07,
	// ED08,		ED09,		ED0A,		ED0B,		ED0C,		ED0D,		ED0E,		ED0F,
	// ED10,		ED11,		ED12,		ED13,		ED14,		ED15,		ED16,		ED17,
	// ED18,		ED19,		ED1A,		ED1B,		ED1C,		ED1D,		ED1E,		ED1F,
	// ED20,		ED21,		ED22,		ED23,		ED24,		ED25,		ED26,		ED27,
	// ED28,		ED29,		ED2A,		ED2B,		ED2C,		ED2D,		ED2E,		ED2F,
	// ED30,		ED31,		ED32,		ED33,		ED34,		ED35,		ED36,		ED37,
	// ED38,		ED39,		ED3A,		ED3B,		ED3C,		ED3D,		ED3E,		ED3F,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,

	// IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	// IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	NEG**,		RETI,		IM0**,		LD_R_A,
	// IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	NEG**,		RETI**,		IM_1,		LD_A_I,
	// IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	NEG**,		RETI**,		IM_2,		LD_A_R,
	// IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	NEG**,		RETI**,		IM0**,		RRD,
	// IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	NEG**,		RETI**,		IM0**,		RLD,
	// IN_F_xC, 	OUT_xC_0,	SBC_HL_SP,	LD_xNN_SP,	NEG**,		RETI**,		IM1**,		ED77,
	// IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	NEG**,		RETI**,		IM2**,		ED7F,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			9,
		12,			12,			15,			20,			8,			14,			8,			18,
		12,			12,			15,			20,			8,			14,			8,			18,
		12,			12,			15,			20,			8,			14,			8,			8,
		12,			12,			15,			20,			8,			14,			8,			8,

	// ED80,		ED81,		ED82,		ED83,		ED84,		ED85,		ED86,		ED87,
	// ED88,		ED89,		ED8A,		ED8B,		ED8C,		ED8D,		ED8E,		ED8F,
	// ED90,		ED91,		ED92,		ED93,		ED94,		ED95,		ED96,		ED97,
	// ED98,		ED99,		ED9A,		ED9B,		ED9C,		ED9D,		ED9E,		ED9F,
	// LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	// LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	// LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	// LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		16,			16,			16,			16,			8,			8,			8,			8,
		16,			16,			16,			16,			8,			8,			8,			8,
		Z(16,21),	Z(16,21),	Z(16,21),	Z(16,21),	8,			8,			8,			8,
		Z(16,21),	Z(16,21),	Z(16,21),	Z(16,21),	8,			8,			8,			8,

	// EDC0,		EDC1,		EDC2,		EDC3,		EDC4,		EDC5,		EDC6,		EDC7,
	// EDC8,		EDC9,		EDCA,		EDCB,		EDCC,		EDCD,		EDCE,		EDCF,
	// EDD0,		EDD1,		EDD2,		EDD3,		EDD4,		EDD5,		EDD6,		EDD7,
	// EDD8,		EDD9,		EDDA,		EDDB,		EDDC,		EDDD,		EDDE,		EDDF,
	// EDE0,		EDE1,		EDE2,		EDE3,		EDE4,		EDE5,		EDE6,		EDE7,
	// EDE8,		EDE9,		EDEA,		EDEB,		EDEC,		EDED,		EDEE,		EDEF,
	// EDF0,		EDF1,		EDF2,		EDF3,		EDF4,		EDF5,		EDF6,		EDF7,
	// EDF8,		EDF9,		EDFA,		EDFB,		EDFC,		EDFD,		EDFE,		EDFF
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8,
		8,			8,			8,			8,			8,			8,			8,			8
};

static const uint8 cc_z180_ED[256] =
{
	#undef Z
	#define Z(A,B) A

	// Table for EDxx opcodes:
	// times are for the entire opcode incl. ED
	// illegal opcodes are trapped by the Z80180

	// IN0_B_xN, 	OUT0_xN_B, 	ED02,		ED03,		TST_B,		ED05,		ED06,		ED07,
	// IN0_C_xN, 	OUT0_xN_C, 	ED0A,		ED0B,		TST_C,		ED0D,		ED0E,		ED0F,
	// IN0_D_xN, 	OUT0_xN_D, 	ED12,		ED13,		TST_D,		ED15,		ED16,		ED17,
	// IN0_E_xN, 	OUT0_xN_E, 	ED1A,		ED1B,		TST_E,		ED1D,		ED1E,		ED1F,
	// IN0_H_xN, 	OUT0_xN_H, 	ED22,		ED23,		TST_H,		ED25,		ED26,		ED27,
	// IN0_L_xN, 	OUT0_xN_L, 	ED2A,		ED2B,		TST_L,		ED2D,		ED2E,		ED2F,
	// IN0_F_xN, 	ED31,		ED32,		ED33,		TST_xHL,	ED35,		ED36,		ED37,
	// IN0_A_xN, 	OUT0_xN_A,	ED3A,		ED3B,		TST_A,		ED3D,		ED3E,		ED3F,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		0,			0,			0,			10u, 		0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,

	// IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	// IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	MLT_BC,		RETI,		ED4E,		LD_R_A,
	// IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	ED54,		ED55,		IM_1,		LD_A_I,
	// IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	MLT_DE,		ED5D,		IM_2,		LD_A_R,
	// IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	TST_N,		ED65,		ED66,		RRD,
	// IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	MLT_HL,		ED6D,		ED6E,		RLD,
	// IN_F_xC, 	ED71,		SBC_HL_SP,	LD_xNN_SP,	TSTIO,		ED75,		SLP,		ED77,		note: in f,(c) is legal
	// IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	MLT_SP,		ED7D,		ED7E,		ED7F,
		9u,			10u, 		10u, 		19u, 		6u,			12u, 		6u,			6u,
		9u,			10u, 		10u, 		18u, 		17u, 		12u, 		0,			6u,
		9u,			10u, 		10u, 		19u, 		0,			0,			6u,			6u,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			6u,			6u,
		9u,			10u, 		10u, 		19u, 		9u,			0,			0,			16u,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			0,			16u,
		9u,			12, 		10u, 		19u, 		12u, 		0,			8u,			0,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			0,			0,

	// ED80,		ED81,		ED82,		OTIM,		ED84,		ED85,		ED86,		ED87,
	// ED88,		ED89,		ED8A,		OTDM,		ED8C,		ED8D,		ED8E,		ED8F,
	// ED90,		ED91,		ED92,		OTIMR,		ED94,		ED95,		ED96,		ED97,
	// ED98,		ED99,		ED9A,		OTDMR,		ED9C,		ED9D,		ED9E,		ED9F,
	// LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	// LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	// LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	// LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
		0,			0,			0,			14u, 		0,			0,			0,			0,
		0,			0,			0,			14u, 		0,			0,			0,			0,
		0,			0,			0,			Z(14u,16u),	0,			0,			0,			0,
		0,			0,			0,			Z(14u,16u),	0,			0,			0,			0,
		12u, 		12u, 		12u, 		12u, 		0,			0,			0,			0,
		12u, 		12u, 		12u, 		12u, 		0,			0,			0,			0,
		Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	0,			0,			0,			0,
		Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	0,			0,			0,			0,

	// EDC0,		EDC1,		EDC2,		EDC3,		EDC4,		EDC5,		EDC6,		EDC7,
	// EDC8,		EDC9,		EDCA,		EDCB,		EDCC,		EDCD,		EDCE,		EDCF,
	// EDD0,		EDD1,		EDD2,		EDD3,		EDD4,		EDD5,		EDD6,		EDD7,
	// EDD8,		EDD9,		EDDA,		EDDB,		EDDC,		EDDD,		EDDE,		EDDF,
	// EDE0,		EDE1,		EDE2,		EDE3,		EDE4,		EDE5,		EDE6,		EDE7,
	// EDE8,		EDE9,		EDEA,		EDEB,		EDEC,		EDED,		EDEE,		EDEF,
	// EDF0,		EDF1,		EDF2,		EDF3,		EDF4,		EDF5,		EDF6,		EDF7,
	// EDF8,		EDF9,		EDFA,		EDFB,		EDFC,		EDFD,		EDFE,		EDFF
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
};

static const uint8 cc_z180_ED_on_branch[256] =
{
	#undef Z
	#define Z(A,B) B

	// Table for EDxx opcodes:
	// times are for the entire opcode incl. ED
	// illegal opcodes are trapped by the Z80180

	// IN0_B_xN, 	OUT0_xN_B, 	ED02,		ED03,		TST_B,		ED05,		ED06,		ED07,
	// IN0_C_xN, 	OUT0_xN_C, 	ED0A,		ED0B,		TST_C,		ED0D,		ED0E,		ED0F,
	// IN0_D_xN, 	OUT0_xN_D, 	ED12,		ED13,		TST_D,		ED15,		ED16,		ED17,
	// IN0_E_xN, 	OUT0_xN_E, 	ED1A,		ED1B,		TST_E,		ED1D,		ED1E,		ED1F,
	// IN0_H_xN, 	OUT0_xN_H, 	ED22,		ED23,		TST_H,		ED25,		ED26,		ED27,
	// IN0_L_xN, 	OUT0_xN_L, 	ED2A,		ED2B,		TST_L,		ED2D,		ED2E,		ED2F,
	// IN0_F_xN, 	ED31,		ED32,		ED33,		TST_xHL,	ED35,		ED36,		ED37,
	// IN0_A_xN, 	OUT0_xN_A,	ED3A,		ED3B,		TST_A,		ED3D,		ED3E,		ED3F,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,
		12u, 		0,			0,			0,			10u, 		0,			0,			0,
		12u, 		13u,		0,			0,			7u,			0,			0,			0,

	// IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	// IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	MLT_BC,		RETI,		ED4E,		LD_R_A,
	// IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	ED54,		ED55,		IM_1,		LD_A_I,
	// IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	MLT_DE,		ED5D,		IM_2,		LD_A_R,
	// IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	TST_N,		ED65,		ED66,		RRD,
	// IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	MLT_HL,		ED6D,		ED6E,		RLD,
	// IN_F_xC, 	ED71,		SBC_HL_SP,	LD_xNN_SP,	TSTIO,		ED75,		SLP,		ED77,		note: in f,(c) is legal
	// IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	MLT_SP,		ED7D,		ED7E,		ED7F,
		9u,			10u, 		10u, 		19u, 		6u,			12u, 		6u,			6u,
		9u,			10u, 		10u, 		18u, 		17u, 		12u, 		0,			6u,
		9u,			10u, 		10u, 		19u, 		0,			0,			6u,			6u,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			6u,			6u,
		9u,			10u, 		10u, 		19u, 		9u,			0,			0,			16u,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			0,			16u,
		9u,			12, 		10u, 		19u, 		12u, 		0,			8u,			0,
		9u,			10u, 		10u, 		18u, 		17u, 		0,			0,			0,

	// ED80,		ED81,		ED82,		OTIM,		ED84,		ED85,		ED86,		ED87,
	// ED88,		ED89,		ED8A,		OTDM,		ED8C,		ED8D,		ED8E,		ED8F,
	// ED90,		ED91,		ED92,		OTIMR,		ED94,		ED95,		ED96,		ED97,
	// ED98,		ED99,		ED9A,		OTDMR,		ED9C,		ED9D,		ED9E,		ED9F,
	// LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	// LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	// LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	// LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
		0,			0,			0,			14u, 		0,			0,			0,			0,
		0,			0,			0,			14u, 		0,			0,			0,			0,
		0,			0,			0,			Z(14u,16u),	0,			0,			0,			0,
		0,			0,			0,			Z(14u,16u),	0,			0,			0,			0,
		12u, 		12u, 		12u, 		12u, 		0,			0,			0,			0,
		12u, 		12u, 		12u, 		12u, 		0,			0,			0,			0,
		Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	0,			0,			0,			0,
		Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	Z(12u,14u),	0,			0,			0,			0,

	// EDC0,		EDC1,		EDC2,		EDC3,		EDC4,		EDC5,		EDC6,		EDC7,
	// EDC8,		EDC9,		EDCA,		EDCB,		EDCC,		EDCD,		EDCE,		EDCF,
	// EDD0,		EDD1,		EDD2,		EDD3,		EDD4,		EDD5,		EDD6,		EDD7,
	// EDD8,		EDD9,		EDDA,		EDDB,		EDDC,		EDDD,		EDDE,		EDDF,
	// EDE0,		EDE1,		EDE2,		EDE3,		EDE4,		EDE5,		EDE6,		EDE7,
	// EDE8,		EDE9,		EDEA,		EDEB,		EDEC,		EDED,		EDEE,		EDEF,
	// EDF0,		EDF1,		EDF2,		EDF3,		EDF4,		EDF5,		EDF6,		EDF7,
	// EDF8,		EDF9,		EDFA,		EDFB,		EDFC,		EDFD,		EDFE,		EDFF
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
		0,			0,			0,			0,			0,			0,			0,			0,
};


static void test_8080 (uint& num_tests, uint& num_errors)
{
	logIn("i8080_clock_cycles");

	Byte core[20] = {0,0,0,0,0,0,0,0,0,0,1,2,3,4};

	TRY
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[10] = Byte(op);
		assert(clock_cycles(Cpu8080,core,10) == cc_i8080[op]);
		assert(clock_cycles_on_branch(Cpu8080,core,10) == cc_i8080_on_branch[op]);
		assert(opcode_can_branch(Cpu8080,core,10) == ((cc_i8080[op]!=cc_i8080_on_branch[op]) || (op&0307)==JP_NZ));
	}
	END
	logNl();
}

static void test_z80 (uint& num_tests, uint& num_errors)
{
	logIn("z80_clock_cycles");

	Byte core[20] = {0,0,0,0,0,0,0,0,0,0,1,2,3,4};

	TRY
	logline("simple opcodes");
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		if (cc_z80[op] == 0) continue;
		core[10] = Byte(op);
		assert(clock_cycles(CpuZ80,core,10) == cc_z80[op]);
		assert(clock_cycles_on_branch(CpuZ80,core,10) == cc_z80_on_branch[op]);
		assert(clock_cycles(CpuDefault,core,10) == cc_z80[op]);
		assert(clock_cycles_on_branch(CpuDefault,core,10) == cc_z80_on_branch[op]);
		assert(clock_cycles(CpuZ80_ixcbr2,core,10) == cc_z80[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbr2,core,10) == cc_z80_on_branch[op]);
		assert(clock_cycles(CpuZ80_ixcbxh,core,10) == cc_z80[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbxh,core,10) == cc_z80_on_branch[op]);
		assert(opcode_can_branch(CpuZ80,core,10) == ((cc_z80[op]!=cc_z80_on_branch[op]) || (op&0307)==JP_NZ));
	}
	END
	logNl();

	TRY
	logline("prefix CB opcodes");
	core[10] = PFX_CB;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[11] = Byte(op);
		assert(clock_cycles(CpuZ80,core,10) == cc_z80_CB[op]);
		assert(clock_cycles_on_branch(CpuZ80,core,10) == cc_z80_CB[op]);
		assert(clock_cycles(CpuDefault,core,10) == cc_z80_CB[op]);
		assert(clock_cycles_on_branch(CpuDefault,core,10) == cc_z80_CB[op]);
		assert(clock_cycles(CpuZ80_ixcbr2,core,10) == cc_z80_CB[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbr2,core,10) == cc_z80_CB[op]);
		assert(clock_cycles(CpuZ80_ixcbxh,core,10) == cc_z80_CB[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbxh,core,10) == cc_z80_CB[op]);
		assert(opcode_can_branch(CpuZ80,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix IX opcodes");
	core[10] = PFX_IX;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		if (op==0xcb) continue;
		core[10] ^= PFX_IX ^ PFX_IY;
		core[11] = Byte(op);
		core[12]++;
		assert(clock_cycles(CpuZ80,core,10) == cc_z80_XY[op]);
		assert(clock_cycles_on_branch(CpuZ80,core,10) == cc_z80_XY[op]);
		assert(clock_cycles(CpuDefault,core,10) == cc_z80_XY[op]);
		assert(clock_cycles_on_branch(CpuDefault,core,10) == cc_z80_XY[op]);
		assert(clock_cycles(CpuZ80_ixcbr2,core,10) == cc_z80_XY[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbr2,core,10) == cc_z80_XY[op]);
		assert(clock_cycles(CpuZ80_ixcbxh,core,10) == cc_z80_XY[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbxh,core,10) == cc_z80_XY[op]);
		assert(opcode_can_branch(CpuZ80,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix IXCB opcodes");
	core[10] = PFX_IX;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[10] ^= PFX_IX ^ PFX_IY;
		core[11] = PFX_CB;
		core[12]++;
		core[13] = Byte(op);
		assert(clock_cycles(CpuZ80,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles_on_branch(CpuZ80,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles(CpuDefault,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles_on_branch(CpuDefault,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles(CpuZ80_ixcbr2,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbr2,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles(CpuZ80_ixcbxh,core,10) == cc_z80_XYCB[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbxh,core,10) == cc_z80_XYCB[op]);
		assert(opcode_can_branch(CpuZ80,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix ED opcodes");
	core[10] = PFX_ED;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[11] = Byte(op);
		assert(clock_cycles(CpuZ80,core,10) == cc_z80_ED[op]);
		assert(clock_cycles_on_branch(CpuZ80,core,10) == cc_z80_ED_on_branch[op]);
		assert(clock_cycles(CpuDefault,core,10) == cc_z80_ED[op]);
		assert(clock_cycles_on_branch(CpuDefault,core,10) == cc_z80_ED_on_branch[op]);
		assert(clock_cycles(CpuZ80_ixcbr2,core,10) == cc_z80_ED[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbr2,core,10) == cc_z80_ED_on_branch[op]);
		assert(clock_cycles(CpuZ80_ixcbxh,core,10) == cc_z80_ED[op]);
		assert(clock_cycles_on_branch(CpuZ80_ixcbxh,core,10) == cc_z80_ED_on_branch[op]);
		assert(opcode_can_branch(CpuZ80,core,10) == (cc_z80_ED[op]!=cc_z80_ED_on_branch[op]));
	}
	END
	logNl();
}

static void test_z180 (uint& num_tests, uint& num_errors)
{
	logIn("z180_clock_cycles");

	Byte core[20] = {0,0,0,0,0,0,0,0,0,0,1,2,3,4};

	TRY
	logline("simple opcodes");
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		if (cc_z180[op] == 0) continue;
		core[10] = Byte(op);
		assert(clock_cycles(CpuZ180,core,10) == cc_z180[op]);
		assert(clock_cycles_on_branch(CpuZ180,core,10) == cc_z180_on_branch[op]);
		assert(opcode_can_branch(CpuZ180,core,10) == (cc_z180[op]!=cc_z180_on_branch[op]));
	}
	END
	logNl();

	TRY
	logline("prefix CB opcodes");
	core[10] = PFX_CB;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[11] = Byte(op);
		assert(clock_cycles(CpuZ180,core,10) == cc_z180_CB[op]);
		assert(clock_cycles_on_branch(CpuZ180,core,10) == cc_z180_CB[op]);
		assert(opcode_can_branch(CpuZ180,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix IX opcodes");
	core[10] = PFX_IX;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		if (op==0xcb) continue;
		core[10] ^= PFX_IX ^ PFX_IY;
		core[11] = Byte(op);
		core[12]++;
		assert(clock_cycles(CpuZ180,core,10) == cc_z180_XY[op]);
		assert(clock_cycles_on_branch(CpuZ180,core,10) == cc_z180_XY[op]);
		assert(opcode_can_branch(CpuZ180,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix IXCB opcodes");
	core[10] = PFX_IX;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[10] ^= PFX_IX ^ PFX_IY;
		core[11] = PFX_CB;
		core[12]++;
		core[13] = Byte(op);
		assert(clock_cycles(CpuZ180,core,10) == cc_z180_XYCB[op]);
		assert(clock_cycles_on_branch(CpuZ180,core,10) == cc_z180_XYCB[op]);
		assert(opcode_can_branch(CpuZ180,core,10) == no);
	}
	END
	logNl();

	TRY
	logline("prefix ED opcodes");
	core[10] = PFX_ED;
	for (uint op=0; op<256; op++)
	{
		log("%02X ",op);
		core[11] = Byte(op);
		assert(clock_cycles(CpuZ180,core,10) == cc_z180_ED[op]);
		assert(clock_cycles_on_branch(CpuZ180,core,10) == cc_z180_ED_on_branch[op]);
		assert(opcode_can_branch(CpuZ180,core,10) == (cc_z180_ED[op]!=cc_z180_ED_on_branch[op]));
	}
	END
	logNl();
}

static void test_z180_deprecated (uint& num_tests, uint& num_errors)
{
	logIn("z180_clock_cycles ((deprecated))");

	TRY
	assert(z180_opcode_can_branch(JP,0)==no);
	assert(z180_opcode_can_branch(JP_M,0)==yes);
	assert(z180_opcode_can_branch(PFX_ED,OUTI)==no);
	assert(z180_opcode_can_branch(PFX_ED,OTIR)==yes);
	assert(z180_opcode_can_branch(PFX_ED,OTIM)==no);
	assert(z180_opcode_can_branch(PFX_ED,OTIMR)==yes);
	assert(z180_opcode_can_branch(JR,0)==no);
	assert(z180_opcode_can_branch(JR_NC,0)==yes);
	assert(z180_opcode_can_branch(CALL,0)==no);
	assert(z180_opcode_can_branch(CALL_PE,0)==yes);
	assert(z180_opcode_can_branch(RET,0)==no);
	assert(z180_opcode_can_branch(RET_Z,0)==yes);
	END

	TRY
	assert(z180_clock_cycles(JP,0,0)==9);
	assert(z180_clock_cycles(JP_Z,0,0)==6);
	assert(z180_clock_cycles_on_branch(JP,0)==9);

	assert(z180_clock_cycles(CALL,0,0)==16);
	assert(z180_clock_cycles(CALL_Z,0,0)==6);
	assert(z180_clock_cycles_on_branch(CALL_Z,0)==16);

	assert(z180_clock_cycles(PFX_ED,LDIR,0)==12);
	assert(z180_clock_cycles_on_branch(PFX_ED,LDIR)==14);

	assert(z180_clock_cycles(RET_Z,0,0)==5);
	assert(z180_clock_cycles(LD_xNN_A,0,0)==13);
	assert(z180_clock_cycles(PFX_CB,SRL_C,0)==7);
	assert(z180_clock_cycles(PFX_CB,SRA_xHL,0)==13);
	assert(z180_clock_cycles(PFX_CB,BIT7_xHL,0)==9);
	assert(z180_clock_cycles(PFX_CB,SET7_xHL,0)==13);
	assert(z180_clock_cycles(PFX_CB,RES7_xHL,0)==13);
	assert(z180_clock_cycles(PFX_CB,SET6_C,0)==6);

	assert(z180_clock_cycles(PFX_ED,IN0_C_xN,0)==12);
	assert(z180_clock_cycles(PFX_ED,TST_xHL,0)==10);
	assert(z180_clock_cycles(PFX_ED,IN_F_xC,0)==9);

	assert(z180_clock_cycles(PFX_IX,INC_xHL,0)==18);
	assert(z180_clock_cycles(PFX_IY,LD_xHL_N,0)==15);

	assert(z180_clock_cycles(PFX_IX,PFX_CB,SRL_xHL)==19);
	assert(z180_clock_cycles(PFX_IX,PFX_CB,BIT4_xHL)==15);
	END

	TRY
	assert(clock_cycles(CpuZ180,PFX_ED,MLT_SP,0)==17);
	assert(clock_cycles(CpuZ180,PFX_IX,LD_E_xHL,0)==14);
	assert(clock_cycles(CpuZ180,PFX_IY,PFX_CB,SET4_xHL)==19);
	END
}





void test_z80_clock_cycles (uint& num_tests, uint& num_errors)
{
	logIn("test z80_clock_cycles");

	test_8080(num_tests, num_errors);
	test_z80 (num_tests, num_errors);
	test_z180(num_tests, num_errors);
	test_z180_deprecated(num_tests, num_errors);
}































