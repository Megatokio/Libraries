// Copyright (c) 1996 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "z80_goodies.h"
namespace Z80
{ // wg. enum collissions
#include "z80_opcodes.h"
}

typedef uchar Mnemo[3];
typedef uchar IXCBMnemo[4];


// ----	Z80 opcode definitions ------------------------------------------------------

enum { // Instruction:

	// Z80:
	NIX,
	NOP,
	LD,
	INC,
	DEC,
	RLCA,
	EX,
	ADD,
	ADC,
	SUB,
	SBC,
	AND,
	XOR,
	OR,
	CP, // <-- DO NOT REORDER!
	RLC,
	RRC,
	RL,
	RR,
	SLA,
	SRA,
	SLL,
	SRL, // <-- DO NOT REORDER!
	RRCA,
	DJNZ,
	RLA,
	JR,
	RRA,
	DAA,
	CPL,
	HALT,
	SCF,
	CCF,
	IN,
	OUT,
	NEG,
	RETN,
	IM,
	RETI,
	RRD,
	RLD,
	LDI,
	CPI,
	INI,
	BIT,
	RES,
	SET,
	OUTI,
	LDD,
	CPD,
	IND,
	OUTD,
	LDIR,
	CPIR,
	INIR,
	OTIR,
	LDDR,
	CPDR,
	INDR,
	OTDR,
	RET,
	POP,
	JP,
	CALL,
	PUSH,
	RST,
	PFX,
	EXX,
	DI,
	EI,

	// Z180 new opcodes:
	IN0,
	OUT0,
	TST,
	TSTIO,
	MLT,
	OTIM,
	OTDM,
	OTIMR,
	OTDMR,
	SLP,

	// asm8080 names: (new names, some names are already enumerated in Z80 section)
	LXI,
	STAX,
	INX,
	INR,
	DCR,
	MVI,
	DAD,
	LDAX,
	DCX,
	RAL,
	RAR,
	SHLD,
	LHLD,
	CMA,
	STA,
	STC,
	LDA,
	CMC,
	RNZ,
	JNZ,
	JMP,
	CNZ,
	ADI,
	RZ,
	JZ,
	CZ,
	ACI,
	RNC,
	JNC,
	CNC,
	SUI,
	RC,
	JC,
	CC,
	SBI,
	RPO,
	JPO,
	XTHL,
	CPO,
	ANI,
	RPE,
	PCHL,
	JPE,
	XCHG,
	CPE,
	XRI,
	RP,
	ORI,
	RM,
	SPHL,
	JM,
	CM,
	HLT,
	MOV,
	SBB,
	ANA,
	XRA,
	ORA,
	CMP,

	// Register / Argument:
	BC,
	DE,
	HL,
	SP,
	AF,
	AF2,
	B,
	C,
	D,
	E,
	H,
	L,
	XHL,
	A, // <-- DO NOT REORDER!
	XBC,
	XDE,
	R,
	I,
	XC,
	XSP,
	PC,
	F,
	N0,
	N1,
	N2,
	N3,
	N4,
	N5,
	N6,
	N7, // <-- DO NOT REORDER!
	N00,
	N08,
	N10,
	N18,
	N20,
	N28,
	N30,
	N38, // <-- DO NOT REORDER!
	Z,
	NZ,
	NC,
	PO,
	PE,
	M,
	P,
	PSW,
	N,
	NN,
	XNN,
	XN,
	DIS,
	CB,
	ED,
	IX,
	IY,
	XH,
	XL,
	YH,
	YL,
	XIX,
	XIY, // <-- keep at end

	NUM_WORD_DEFS
};

static const char word[][9] = {
	"",		"nop",	  "ld",	  "inc",   "dec",  "rlca", "ex",   "add",	   "adc",	   "sub",  "sbc",  "and",  "xor",
	"or",	"cp",	  "rlc",  "rrc",   "rl",   "rr",   "sla",  "sra",	   "sll",	   "srl",  "rrca", "djnz", "rla",
	"jr",	"rra",	  "daa",  "cpl",   "halt", "scf",  "ccf",  "in",	   "out",	   "neg",  "retn", "im",   "reti",
	"rrd",	"rld",	  "ldi",  "cpi",   "ini",  "bit",  "res",  "set",	   "outi",	   "ldd",  "cpd",  "ind",  "outd",
	"ldir", "cpir",	  "inir", "otir",  "lddr", "cpdr", "indr", "otdr",	   "ret",	   "pop",  "jp",   "call", "push",
	"rst",	"prefix", "exx",  "di",	   "ei",

	"in0",	"out0",	  "tst",  "tstio", "mlt",  "otim", "otdm", "otimr",	   "otdmr",	   "slp",

	"lxi",	"stax",	  "inx",  "inr",   "dcr",  "mvi",  "dad",  "ldax",	   "dcx",	   "ral",  "rar",  "shld", "lhld",
	"cma",	"sta",	  "stc",  "lda",   "cmc",  "rnz",  "jnz",  "jmp",	   "cnz",	   "adi",  "rz",   "jz",   "cz",
	"aci",	"rnc",	  "jnc",  "cnc",   "sui",  "rc",   "jc",   "cc",	   "sbi",	   "rpo",  "jpo",  "xthl", "cpo",
	"ani",	"rpe",	  "pchl", "jpe",   "xchg", "cpe",  "xri",  "rp",	   "ori",	   "rm",   "sphl", "jm",   "cm",
	"hlt",	"mov",	  "sbb",  "ana",   "xra",  "ora",  "cmp",

	"bc",	"de",	  "hl",	  "sp",	   "af",   "af'",  "b",	   "c",		   "d",		   "e",	   "h",	   "l",	   "(hl)",
	"a",	"(bc)",	  "(de)", "r",	   "i",	   "(c)",  "(sp)", "pc",	   "f",		   "0",	   "1",	   "2",	   "3",
	"4",	"5",	  "6",	  "7",	   "0",	   "8",	   "16",   "24",	   "32",	   "40",   "48",   "56",   "z",
	"nz",	"nc",	  "po",	  "pe",	   "m",	   "p",	   "psw",  "N",		   "NN",	   "(NN)", "(N)",  "dis",  "cb",
	"ed",	"ix",	  "iy",	  "xh",	   "xl",   "yh",   "yl",   "(ix+dis)", "(iy+dis)",
};

static_assert(NELEM(word) == NUM_WORD_DEFS, "");

static const Mnemo i8080_cmd_00[64] = {
	{NOP}, {LXI, B, NN},  {STAX, B},  {INX, B},	 {INR, B}, {DCR, B}, {MVI, B, N}, {RLC},
	{NOP}, {DAD, B},	  {LDAX, B},  {DCX, B},	 {INR, C}, {DCR, C}, {MVI, C, N}, {RRC},
	{NOP}, {LXI, D, NN},  {STAX, D},  {INX, D},	 {INR, D}, {DCR, D}, {MVI, D, N}, {RAL},
	{NOP}, {DAD, D},	  {LDAX, D},  {DCX, D},	 {INR, E}, {DCR, E}, {MVI, E, N}, {RAR},
	{NOP}, {LXI, H, NN},  {SHLD, NN}, {INX, H},	 {INR, H}, {DCR, H}, {MVI, H, N}, {DAA},
	{NOP}, {DAD, H},	  {LHLD, NN}, {DCX, H},	 {INR, L}, {DCR, L}, {MVI, L, N}, {CMA},
	{NOP}, {LXI, SP, NN}, {STA, NN},  {INX, SP}, {INR, M}, {DCR, M}, {MVI, M, N}, {STC},
	{NOP}, {DAD, SP},	  {LDA, NN},  {DCX, SP}, {INR, A}, {DCR, A}, {MVI, A, N}, {CMC},
};

static const Mnemo i8080_cmd_C0[64] = {
	{RNZ},	   {POP, B},   {JNZ, NN}, {JMP, NN},   {CNZ, NN}, {PUSH, B},  {ADI, N},	 {RST, N0}, {RZ},	   {RET},
	{JZ, NN},  {JMP, NN},  {CZ, NN},  {CALL, NN},  {ACI, N},  {RST, N1},  {RNC},	 {POP, D},	{JNC, NN}, {OUT, N},
	{CNC, NN}, {PUSH, D},  {SUI, N},  {RST, N2},   {RC},	  {RET},	  {JC, NN},	 {IN, N},	{CC, NN},  {CALL, NN},
	{SBI, N},  {RST, N3},  {RPO},	  {POP, H},	   {JPO, NN}, {XTHL},	  {CPO, NN}, {PUSH, H}, {ANI, N},  {RST, N4},
	{RPE},	   {PCHL},	   {JPE, NN}, {XCHG},	   {CPE, NN}, {CALL, NN}, {XRI, N},	 {RST, N5}, {RP},	   {POP, PSW},
	{JP, NN},  {DI},	   {CP, NN},  {PUSH, PSW}, {ORI, N},  {RST, N6},  {RM},		 {SPHL},	{JM, NN},  {EI},
	{CM, NN},  {CALL, NN}, {CPI, N},  {RST, N7},
};

static const Mnemo z80_cmd_00[64] = {
	{NOP},		   {LD, BC, NN},  {LD, XBC, A},	 {INC, BC}, {INC, B},	{DEC, B},	{LD, B, N},	  {RLCA},
	{EX, AF, AF2}, {ADD, HL, BC}, {LD, A, XBC},	 {DEC, BC}, {INC, C},	{DEC, C},	{LD, C, N},	  {RRCA},
	{DJNZ, DIS},   {LD, DE, NN},  {LD, XDE, A},	 {INC, DE}, {INC, D},	{DEC, D},	{LD, D, N},	  {RLA},
	{JR, DIS},	   {ADD, HL, DE}, {LD, A, XDE},	 {DEC, DE}, {INC, E},	{DEC, E},	{LD, E, N},	  {RRA},
	{JR, NZ, DIS}, {LD, HL, NN},  {LD, XNN, HL}, {INC, HL}, {INC, H},	{DEC, H},	{LD, H, N},	  {DAA},
	{JR, Z, DIS},  {ADD, HL, HL}, {LD, HL, XNN}, {DEC, HL}, {INC, L},	{DEC, L},	{LD, L, N},	  {CPL},
	{JR, NC, DIS}, {LD, SP, NN},  {LD, XNN, A},	 {INC, SP}, {INC, XHL}, {DEC, XHL}, {LD, XHL, N}, {SCF},
	{JR, C, DIS},  {ADD, HL, SP}, {LD, A, XNN},	 {DEC, SP}, {INC, A},	{DEC, A},	{LD, A, N},	  {CCF},
};

static const Mnemo z80_cmd_C0[64] = {
	{RET, NZ}, {POP, BC},	 {JP, NZ, NN}, {JP, NN},	  {CALL, NZ, NN}, {PUSH, BC}, {ADD, A, N}, {RST, N00},
	{RET, Z},  {RET},		 {JP, Z, NN},  {PFX, CB},	  {CALL, Z, NN},  {CALL, NN}, {ADC, A, N}, {RST, N08},
	{RET, NC}, {POP, DE},	 {JP, NC, NN}, {OUT, XN, A},  {CALL, NC, NN}, {PUSH, DE}, {SUB, A, N}, {RST, N10},
	{RET, C},  {EXX},		 {JP, C, NN},  {IN, A, XN},	  {CALL, C, NN},  {PFX, IX},  {SBC, A, N}, {RST, N18},
	{RET, PO}, {POP, HL},	 {JP, PO, NN}, {EX, HL, XSP}, {CALL, PO, NN}, {PUSH, HL}, {AND, A, N}, {RST, N20},
	{RET, PE}, {JP, HL},	 {JP, PE, NN}, {EX, DE, HL},  {CALL, PE, NN}, {PFX, ED},  {XOR, A, N}, {RST, N28},
	{RET, P},  {POP, AF},	 {JP, P, NN},  {DI},		  {CALL, P, NN},  {PUSH, AF}, {OR, A, N},  {RST, N30},
	{RET, M},  {LD, SP, HL}, {JP, M, NN},  {EI},		  {CALL, M, NN},  {PFX, IY},  {CP, A, N},  {RST, N38},
};

static const Mnemo z80_cmd_ED40[64] = {
	{IN, B, XC}, {OUT, XC, B},	{SBC, HL, BC}, {LD, XNN, BC}, {NEG}, {RETN}, {IM, N0}, {LD, I, A},
	{IN, C, XC}, {OUT, XC, C},	{ADC, HL, BC}, {LD, BC, XNN}, {NEG}, {RETI}, {IM, N0}, {LD, R, A},
	{IN, D, XC}, {OUT, XC, D},	{SBC, HL, DE}, {LD, XNN, DE}, {NEG}, {RETI}, {IM, N1}, {LD, A, I},
	{IN, E, XC}, {OUT, XC, E},	{ADC, HL, DE}, {LD, DE, XNN}, {NEG}, {RETI}, {IM, N2}, {LD, A, R},
	{IN, H, XC}, {OUT, XC, H},	{SBC, HL, HL}, {LD, XNN, HL}, {NEG}, {RETI}, {IM, N0}, {RRD},
	{IN, L, XC}, {OUT, XC, L},	{ADC, HL, HL}, {LD, HL, XNN}, {NEG}, {RETI}, {IM, N0}, {RLD},
	{IN, F, XC}, {OUT, XC, N0}, {SBC, HL, SP}, {LD, XNN, SP}, {NEG}, {RETI}, {IM, N1}, {NOP},
	{IN, A, XC}, {OUT, XC, A},	{ADC, HL, SP}, {LD, SP, XNN}, {NEG}, {RETI}, {IM, N2}, {NOP},
};

static const Mnemo z180_cmd_ED[3 * 64] = {
	{IN0, B, XN}, {OUT0, XN, B}, {NOP},			{NOP},		   {TST, B},   {NOP},  {NOP},	 {NOP},
	{IN0, C, XN}, {OUT0, XN, C}, {NOP},			{NOP},		   {TST, C},   {NOP},  {NOP},	 {NOP},
	{IN0, D, XN}, {OUT0, XN, D}, {NOP},			{NOP},		   {TST, D},   {NOP},  {NOP},	 {NOP},
	{IN0, E, XN}, {OUT0, XN, E}, {NOP},			{NOP},		   {TST, E},   {NOP},  {NOP},	 {NOP},
	{IN0, H, XN}, {OUT0, XN, H}, {NOP},			{NOP},		   {TST, H},   {NOP},  {NOP},	 {NOP},
	{IN0, L, XN}, {OUT0, XN, L}, {NOP},			{NOP},		   {TST, L},   {NOP},  {NOP},	 {NOP},
	{IN0, F, XN}, {NOP},		 {NOP},			{NOP},		   {TST, XHL}, {NOP},  {NOP},	 {NOP},
	{IN0, A, XN}, {OUT0, XN, A}, {NOP},			{NOP},		   {TST, A},   {NOP},  {NOP},	 {NOP},

	{IN, B, XC},  {OUT, XC, B},	 {SBC, HL, BC}, {LD, XNN, BC}, {NEG},	   {RETN}, {IM, N0}, {LD, I, A},
	{IN, C, XC},  {OUT, XC, C},	 {ADC, HL, BC}, {LD, BC, XNN}, {MLT, BC},  {RETI}, {NOP},	 {LD, R, A},
	{IN, D, XC},  {OUT, XC, D},	 {SBC, HL, DE}, {LD, XNN, DE}, {NOP},	   {NOP},  {IM, N1}, {LD, A, I},
	{IN, E, XC},  {OUT, XC, E},	 {ADC, HL, DE}, {LD, DE, XNN}, {MLT, DE},  {NOP},  {IM, N2}, {LD, A, R},
	{IN, H, XC},  {OUT, XC, H},	 {SBC, HL, HL}, {LD, XNN, HL}, {TST, N},   {NOP},  {NOP},	 {RRD},
	{IN, L, XC},  {OUT, XC, L},	 {ADC, HL, HL}, {LD, HL, XNN}, {MLT, HL},  {NOP},  {NOP},	 {RLD},
	{IN, F, XC},  {NOP},		 {SBC, HL, SP}, {LD, XNN, SP}, {TSTIO, N}, {NOP},  {SLP},	 {NOP},
	{IN, A, XC},  {OUT, XC, A},	 {ADC, HL, SP}, {LD, SP, XNN}, {MLT, SP},  {NOP},  {NOP},	 {NOP},

	{NOP},		  {NOP},		 {NOP},			{OTIM},		   {NOP},	   {NOP},  {NOP},	 {NOP},
	{NOP},		  {NOP},		 {NOP},			{OTDM},		   {NOP},	   {NOP},  {NOP},	 {NOP},
	{NOP},		  {NOP},		 {NOP},			{OTIMR},	   {NOP},	   {NOP},  {NOP},	 {NOP},
	{NOP},		  {NOP},		 {NOP},			{OTDMR},	   {NOP},	   {NOP},  {NOP},	 {NOP},
	{LDI},		  {CPI},		 {INI},			{OUTI},		   {NOP},	   {NOP},  {NOP},	 {NOP},
	{LDD},		  {CPD},		 {IND},			{OUTD},		   {NOP},	   {NOP},  {NOP},	 {NOP},
	{LDIR},		  {CPIR},		 {INIR},		{OTIR},		   {NOP},	   {NOP},  {NOP},	 {NOP},
	{LDDR},		  {CPDR},		 {INDR},		{OTDR},		   {NOP},	   {NOP},  {NOP},	 {NOP},
};


// ============================================================================================
// Mnemonic:

static inline void copy3(uchar* z, const uchar* q)
{
	*z++ = *q++;
	*z++ = *q++;
	*z++ = *q++;
}

static const uchar* z80_mnemo(Byte op)
{
	// return m[3] mnenonic descriptor for normal instructions
	// tempmem or const

	static const uchar cmd_ari[] = {ADD, ADC, SUB, SBC, AND, XOR, OR, CP};
	static const Mnemo cmd_halt	 = {HALT, 0, 0};

	uchar* s;
	switch (op >> 6)
	{
	case 0: return z80_cmd_00[op];
	case 1:
		if (op == 0x76) return cmd_halt;
		s	 = temp<uchar>(3);
		s[0] = LD;
		s[1] = B + ((op >> 3) & 0x07);
		s[2] = B + (op & 0x07);
		return s;
	case 2:
		s	 = temp<uchar>(3);
		s[0] = cmd_ari[(op >> 3) & 0x07];
		s[1] = A;
		s[2] = B + (op & 0x07);
		return s;
	//case 3:
	default: return z80_cmd_C0[op & 0x3f];
	}
}

static const uchar* i8080_mnemo(Byte op)
{
	// return m[3] mnenonic descriptor for normal instructions
	// tempmem or const

	static const Mnemo cmd_hlt = {HLT, 0, 0};
	static const uchar ari[]   = {ADD, ADC, SUB, SBB, ANA, XRA, ORA, CMP};
	static const uchar reg[]   = {B, C, D, E, H, L, M, A};

	uchar* s;
	switch (op >> 6)
	{
	case 0: return i8080_cmd_00[op];
	case 1:
		if (op == 0x76) return cmd_hlt;
		s	 = temp<uchar>(3);
		s[0] = LD;
		s[1] = reg[(op >> 3) & 0x07];
		s[2] = reg[op & 0x07];
		return s;
	case 2:
		s	 = temp<uchar>(3);
		s[0] = ari[(op >> 3) & 0x07];
		s[1] = reg[op & 0x07];
		s[2] = 0;
		return s;
	//case 3:
	default: return i8080_cmd_C0[op & 0x3f];
	}
}

static const uchar cmd_sh[] = {RLC, RRC, RL, RR, SLA, SRA, SLL, SRL};
static const uint8 reg_cb[] = {B, C, D, E, H, L, XHL, A};

static const uchar* z80_mnemoCB(Byte op2, const uint8* reg = reg_cb)
{
	// return m[4] mnenonic descriptor for CB instructions

	uint8 r = op2 & 0x07;
	uint8 n = (op2 >> 3) & 0x07;

	uchar* s = temp<uchar>(4);
	switch (op2 >> 6)
	{
	case 0:
		s[0] = cmd_sh[n];
		s[1] = reg[r];
		s[2] = 0;
		s[3] = 0;
		return s;
	case 1: s[0] = BIT; break;
	case 2: s[0] = RES; break;
	case 3: s[0] = SET; break;
	}
	s[1] = N0 + n;
	s[2] = reg[r];
	s[3] = 0;
	return s;
}

static const uchar* z80_mnemoXYCB(CpuID cpuid, Byte pfx, Byte op4)
{
	// return m[4] mnenonic descriptor for IXCB instruction
	// legal opcode:     "set 0,(ix+dis)"
	// illegal --ixbcxh: "set 0,xh", "set 0,b"
	// illegal --ixbcr2: "set 0,(ix+dis),h", "set 0,(ix+dis),b"
	// illegal others:   same as --ixbcr2

	static const uint8 yreg[] = {B, C, D, E, YH, YL, XIY, A};
	static const uint8 xreg[] = {B, C, D, E, XH, XL, XIX, A};

	// --ixcbxh mode:
	if (cpuid == CpuZ80_ixcbxh) return z80_mnemoCB(op4, pfx == 0xdd ? xreg : yreg);

	// --ixbcr2 and all other modes:

	uint8 r = op4 & 0x07;
	uint8 n = (op4 >> 3) & 0x07;

	uchar* s = temp<uchar>(4);
	switch (op4 >> 6)
	{
	case 0:
		s[0] = cmd_sh[n];
		s[1] = pfx == 0xdd ? XIX : XIY;
		s[2] = r == 6 ? 0 : B + r;
		s[3] = 0;
		return s;
	case 1: s[0] = BIT; break;
	case 2: s[0] = RES; break;
	case 3: s[0] = SET; break;
	}
	s[1] = N0 + n;
	s[2] = pfx == 0xdd ? XIX : XIY;
	s[3] = r == 6 ? 0 : B + r;
	return s;
}

static const uchar* z80_mnemoED(CpuID cpuid, Byte op2)
{
	// return Mnemo for ED instructions

	if (cpuid == CpuZ180)
	{
		if (op2 < 0xC0) return z180_cmd_ED[op2];
	}
	else // Z80
	{
		if (op2 >= 0x40 && op2 < 0x80) return z80_cmd_ED40[op2 - 0x40];
		if (op2 >= 0xA0 && op2 < 0xC0) return z180_cmd_ED[op2];
	}

	return z80_cmd_00[Z80::NOP];
}

static const uchar* z80_mnemoXY(Byte pfx, Byte op2)
{
	// return m[3] mnenonic descriptor for IX instructions
	// tempmem or const

	uchar* s = temp<uchar>(3);
	copy3(s, z80_mnemo(op2));

	bool ix = pfx == Z80::PFX_IX;

	if (s[2] == HL)
	{
		if (op2 != Z80::EX_DE_HL) s[2] = ix ? IX : IY;
	} // don't return wg. add ix,ix
	if (s[1] == HL)
	{
		if (op2 != Z80::EX_DE_HL) s[1] = ix ? IX : IY;
		return s;
	}

	if (s[1] == XHL)
	{
		s[1] = ix ? XIX : XIY;
		return s;
	}
	if (s[2] == XHL)
	{
		s[2] = ix ? XIX : XIY;
		return s;
	}

	if (s[1] == H) s[1] = ix ? XH : YH;
	if (s[1] == L) s[1] = ix ? XL : YL;
	if (s[2] == H) s[2] = ix ? XH : YH;
	if (s[2] == L) s[2] = ix ? XL : YL;
	return s;
}

// deprecated
cstr opcode_mnemo(CpuID cpuid, const Byte* core, Address addr)
{
	// return mnenonic with symbolic arguments for instructions
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))
	// returns tempstr or const string

	const uchar* m;
	const uint8	 op1 = peek(core, addr);

	if (cpuid == Cpu8080) { m = i8080_mnemo(op1); }
	else
		switch (op1)
		{
		case 0xCB: m = z80_mnemoCB(peek(core, addr + 1)); break;
		case 0xED: m = z80_mnemoED(cpuid, peek(core, addr + 1)); break;
		case 0xDD:
		case 0xFD:
			m = peek(core, addr + 1) == 0xCB ? z80_mnemoXYCB(cpuid, op1, peek(core, addr + 3)) :
											   z80_mnemoXY(op1, peek(core, addr + 1));
			break;
		default: m = z80_mnemo(op1); break;
		}

	cstr s1 = word[m[0]];
	if (m[1] == 0) return s1;
	cstr s2 = word[m[1]];
	cstr s3 = word[m[2]];

	str s = tempstr(5 + strlen(s2) + (*s3 ? 1 + strlen(s3) : 0));

	strcpy(s, s1);
	strcat(s, "    ");
	strcpy(s + 5, s2);

	if (*s3)
	{
		strcat(s, ",");
		strcat(s, s3);
	}
	return s;
}


// ================================================================================
// Opcode Legal State:

static inline OpcodeValidity z80_illegalCB(CpuID cpuid, Byte op)
{
	// get legal state of 0xCB instruction
	// all instructions LEGAL except SLL is UNDOCUMENTED

	return op >= 0x30 && op < 0x38 ? cpuid == CpuZ180 ? ILLEGAL_OPCODE : UNDOCUMENTED_OPCODE : LEGAL_OPCODE;
}

static inline OpcodeValidity z80_illegalED(CpuID cpuid, Byte op)
{
	// get legal state of ED instruction
	// LEGAL:        all documented opcodes
	// UNDOCUMENTED: out (c),0
	// ILLEGAL:      all others: ill. NOP or ill. aliases

	static constexpr OpcodeValidity L = LEGAL_OPCODE;
	static constexpr OpcodeValidity U = UNDOCUMENTED_OPCODE;
	static constexpr OpcodeValidity i = ILLEGAL_OPCODE;

	static const OpcodeValidity state_40[64] = {
		L, L, L, L, L, L, L, L, // IN_B_xC,	OUT_xC_B, SBC_HL_BC, LD_xNN_BC,   NEG,  RETN, IM_0, LD_I_A,
		L, L, L, L, i, L, i, L, // IN_C_xC,	OUT_xC_C, ADC_HL_BC, LD_BC_xNN,   ED4C, RETI, ED4E, LD_R_A,
		L, L, L, L, i, i, L, L, // IN_D_xC,	OUT_xC_D, SBC_HL_DE, LD_xNN_DE,   ED54, ED55, IM_1, LD_A_I,
		L, L, L, L, i, i, L, L, // IN_E_xC,	OUT_xC_E, ADC_HL_DE, LD_DE_xNN,   ED5C, ED5D, IM_2, LD_A_R,
		L, L, L, L, i, i, i, L, // IN_H_xC,	OUT_xC_H, SBC_HL_HL, ED_xNN_HL,   ED64, ED65, ED66, RRD,
		L, L, L, L, i, i, i, L, // IN_L_xC,	OUT_xC_L, ADC_HL_HL, ED_HL_xNN,   ED6C, ED6D, ED6E, RLD,
		L, U, L, L, i, i, i, i, // IN_F_xC,	OUT_xC_0, SBC_HL_SP, LD_xNN_SP,   ED74, ED75, ED76, ED77,
		L, L, L, L, i, i, i, i, // IN_A_xC,	OUT_xC_A, ADC_HL_SP, LD_SP_xNN,   ED7C, ED7D, ED7E, ED7F,
	};

	static const OpcodeValidity state_140[64] = {
		L, L, L, L, L, L, L, L, // IN_B_xC,	OUT_xC_B, SBC_HL_BC, LD_xNN_BC,   NEG,  RETN, IM_0, LD_I_A,
		L, L, L, L, L, L, i, L, // IN_C_xC,	OUT_xC_C, ADC_HL_BC, LD_BC_xNN,   *MLT,  RETI, ED4E, LD_R_A,
		L, L, L, L, i, i, L, L, // IN_D_xC,	OUT_xC_D, SBC_HL_DE, LD_xNN_DE,   ED54, ED55, IM_1, LD_A_I,
		L, L, L, L, L, i, L, L, // IN_E_xC,	OUT_xC_E, ADC_HL_DE, LD_DE_xNN,   *MLT, ED5D, IM_2, LD_A_R,
		L, L, L, L, L, i, i, L, // IN_H_xC,	OUT_xC_H, SBC_HL_HL, ED_xNN_HL,   *TST, ED65, ED66, RRD,
		L, L, L, L, L, i, i, L, // IN_L_xC,	OUT_xC_L, ADC_HL_HL, ED_HL_xNN,   *MLT, ED6D, ED6E, RLD,
		L, i, L, L, L, i, L, i, // IN_F_xC,	OUT_xC_0, SBC_HL_SP, LD_xNN_SP,   *TST, ED75, *SLP, ED77,
		L, L, L, L, L, i, i, i, // IN_A_xC,	OUT_xC_A, ADC_HL_SP, LD_SP_xNN,   *MLT, ED7D, ED7E, ED7F,
	};

	if (cpuid == CpuZ180)
	{
		if (op < 0x40) return (op & 3) == 0 || ((op & 7) == 1 && op != 0x31) ? LEGAL_OPCODE : ILLEGAL_OPCODE;
		if (op < 0x80) return state_140[op & 0x3F];	  // misc.
		if ((op & 0344) == 0xA0) return LEGAL_OPCODE; // z80 block opcodes
		if ((op & 0xE7) == 0x83) return LEGAL_OPCODE; // z180 block io opcodes
		return ILLEGAL_OPCODE;						  // all others
	}
	else // Z80
	{
		if (op < 0x40) return ILLEGAL_OPCODE;		  // all NOPs
		if (op < 0x80) return state_40[op & 0x3F];	  // misc.
		if ((op & 0344) == 0xA0) return LEGAL_OPCODE; // block opcodes
		return ILLEGAL_OPCODE;						  // all NOPs
	}
}

static inline OpcodeValidity z80_illegalXY(CpuID cpuid, Byte op)
{
	// get legal state of IX/IY instruction
	// LEGAL:		 instructions which use (hl)
	// UNDOCUMENTED: instructions, which use XH or XL are UNDOCUMENTED
	// ILLEGAL:		 all others: prefix no effect!
	//				 note: prefix 0xCB is also reported as illegal

	static constexpr OpcodeValidity L = LEGAL_OPCODE;
	static constexpr OpcodeValidity U = UNDOCUMENTED_OPCODE;
	static constexpr OpcodeValidity i = ILLEGAL_OPCODE;

	static const OpcodeValidity state_00[64] = {
		i, i, i, i, i, i, i, i, //	NOP,	 LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,	DEC_B,	LD_B_N,	 RLCA,
		i, L, i, i, i, i, i, i, //	EX_AF_AF ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,	DEC_C,	LD_C_N,	 RRCA,
		i, i, i, i, i, i, i, i, //	DJNZ,	 LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,	DEC_D,	LD_D_N,	 RLA,
		i, L, i, i, i, i, i, i, //	JR, 	 ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,	DEC_E,	LD_E_N,	 RRA,
		i, L, L, L, U, U, U, i, //	JR_NZ,	 LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,	DEC_H,	LD_H_N,	 DAA,
		i, L, L, L, U, U, U, i, //	JR_Z,	 ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,	DEC_L,	LD_L_N,	 CPL,
		i, i, i, i, L, L, L, i, //	JR_NC,	 LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,DEC_xHL LD_xHL_N SCF,
		i, L, i, i, i, i, i, i, //	JR_C,	 ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,	DEC_A,	LD_A_N,	 CCF,
	};

	if (op < 0x40)
	{
		OpcodeValidity rval = state_00[op];
		return rval == LEGAL_OPCODE || cpuid != CpuZ180 ? rval : ILLEGAL_OPCODE;
	}

	if (op < 0xC0) // LD r,r and ARI a,r:
	{
		if (((op & 7) == 6) != ((op & 0xF8) == 0x70)) return LEGAL_OPCODE; // either source or dest is (hl)
		if (cpuid == CpuZ180) return ILLEGAL_OPCODE;
		if ((op & 006) == 004) return UNDOCUMENTED_OPCODE; // LD or ARI uses H or L
		if ((op & 0360) == 0140)
			return UNDOCUMENTED_OPCODE; // LD uses H or L as dest
		else
			return ILLEGAL_OPCODE; // IX/IY prefix has no effect
	}

	using namespace Z80;
	if (op == PUSH_HL || op == POP_HL || op == JP_HL || op == LD_SP_HL || op == EX_HL_xSP)
		return LEGAL_OPCODE;
	else
		return ILLEGAL_OPCODE;
}

static inline OpcodeValidity z80_illegalXYCB(CpuID cpuid, Byte op)
{
	// get legal state of IXCB/IYCB instruction
	// all instructions which do not use IX are weird
	// instructions using IX are legal except: sll is illegal

	if ((op & 7) == 6)
		return op != Z80::SLL_xHL ? LEGAL_OPCODE : // uses hl
				   cpuid == CpuZ180 ? ILLEGAL_OPCODE :
									  UNDOCUMENTED_OPCODE;
	if (cpuid == CpuZ80_ixcbr2) return UNDOCUMENTED_OPCODE;					 // all registers: known effect
	if (cpuid == CpuZ80_ixcbxh && (op & 6) == 4) return UNDOCUMENTED_OPCODE; // uses H or L
	return ILLEGAL_OPCODE;													 // no effect or unknown effect
}

OpcodeValidity opcode_validity(CpuID cpuid, const Byte* core, Address addr)
{
	// get legal state of instruction
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))

	uint8 op = peek(core, addr);
	switch (op)
	{
	case 0xCB:
		if (cpuid == Cpu8080) return DEPRECATED_OPCODE;
		return z80_illegalCB(cpuid, peek(core, addr + 1));
	case 0xED:
		if (cpuid == Cpu8080) return DEPRECATED_OPCODE;
		return z80_illegalED(cpuid, peek(core, addr + 1));
	case 0xDD:
	case 0xFD:
		if (cpuid == Cpu8080) return DEPRECATED_OPCODE;
		return peek(core, addr + 1) != 0xCB ? z80_illegalXY(cpuid, peek(core, addr + 1)) :
											  z80_illegalXYCB(cpuid, peek(core, addr + 3));
	default:
		if (cpuid != Cpu8080) return LEGAL_OPCODE;
		if (op < 0xC0)
			return (op & 0xC7) == 0 && op != 0 ? DEPRECATED_OPCODE : LEGAL_OPCODE;
		else
			return op == 0xD9 ? DEPRECATED_OPCODE : LEGAL_OPCODE;
	}
}


// ===================================================================================
// Disassemble:

#define NEXTBYTE peek(core, addr++)

static cstr expand_word(const Byte* core, Address& addr, uint8 token)
{
	// expand token n, reading opcode parameters via *ip++ if needed
	// tempstr or const string

	uint n, nn;
	str	 s;

	switch (token)
	{
	case DIS:
		n  = NEXTBYTE;
		nn = uint16(addr + int8(n));
		s  = tempstr(14);							 // "$+123 ; $FFFF"
		sprintf(s, "$%+i ; $%04X", int8(n) + 2, nn); // "$+dis ; $dest"
		return s;
	case N:
		s = tempstr(3); // "255"
		sprintf(s, "%u", NEXTBYTE);
		return s;
	case NN:
		nn = NEXTBYTE;
		nn = nn + 256 * NEXTBYTE;
		s  = tempstr(5); // "$FFFF"
		sprintf(s, "$%04X", nn);
		return s;
	case XNN:
		nn = NEXTBYTE;
		nn = nn + 256 * NEXTBYTE;
		s  = tempstr(7); // "($FFFF)"
		sprintf(s, "($%04X)", nn);
		return s;
	case XN:
		s = tempstr(5); // "($FF)"
		sprintf(s, "($%02X)", NEXTBYTE);
		return s;
	case XIX:
		s = tempstr(8); // "(ix+123)"
		sprintf(s, "(ix%+i)", int8(NEXTBYTE));
		return s;
	case XIY:
		s = tempstr(8); // "(iy+123)"
		sprintf(s, "(iy%+i)", int8(NEXTBYTE));
		return s;
	default: return word[token];
	}
}

static cstr expand_mnemo(const Byte* core, Address& addr, const Mnemo m)
{
	cstr cmd = word[m[0]];
	if (m[1] == 0) return cmd;

	cstr s2 = expand_word(core, addr, m[1]);
	if (m[2] == 0) return catstr(cmd, " ", s2);

	cstr s3 = expand_word(core, addr, m[2]);
	return catstr(cmd, " ", s2, ",", s3);
}

static cstr expand_mnemoXYCB(const Byte* core, Address& addr, const IXCBMnemo m)
{
	cstr s = expand_mnemo(core, addr, m);
	if (m[3]) s = catstr(s, ",", expand_word(core, addr, m[3]));
	return s;
}

cstr disassemble_8080(const Byte* core, Address& addr, bool asm8080)
{
	// disassemble 8080 opcode using z80 or asm8080 syntax

	Byte op = NEXTBYTE;
	cstr s;

	// test for deprecated opcode:
	// these are all NOP, CALL, JMP or RET
	// which happen to be the same in asm8080 and Z80 syntax:

	switch (op)
	{
	case 8: return "nop ; $08: ***deprecated*** Z80: EX AF,AF'";
	case 16: return "nop ; $10: ***deprecated*** Z80: DJNZ dest";
	case 24: return "nop ; $18: ***deprecated*** Z80: JR dest";
	case 32: return "nop ; $20: ***deprecated*** Z80: JR nz,dest";
	case 40: return "nop ; $28: ***deprecated*** Z80: JR z,dest";
	case 48: return "nop ; $30: ***deprecated*** Z80: JR nc,dest";
	case 56: return "nop ; $38: ***deprecated*** Z80: JR c,dest";
	case 0xD9: return "ret ; $D9: ***deprecated*** Z80: EXX";

	case 0xCB:
		s = expand_word(core, addr, uchar(NN));
		return usingstr("%s %s ; $CB: ***deprecated*** Z80: prefix CB", (asm8080 ? "jmp" : "jp"), s);

	case 0xDD:
	case 0xED:
	case 0xFD:
		s = expand_word(core, addr, uchar(NN));
		return usingstr("call %s ; $%02X: ***deprecated*** Z80: prefix $%02X", s, op, op);
	}

	// legal opcode: return asm8080 or Z80 systax:

	const uchar* mnemo = (asm8080 ? i8080_mnemo : z80_mnemo)(op);
	return expand_mnemo(core, addr, mnemo);
}

cstr disassemble(CpuID cpuid, const Byte* core, Address& addr)
{
	// disassemble one instruction and increment ip
	// tempstr or const

	if (cpuid == Cpu8080) return disassemble_8080(core, addr, no);

	Byte		 op = NEXTBYTE;
	const uchar* m;
	cstr		 s;

	switch (op)
	{
	default: m = z80_mnemo(op); return expand_mnemo(core, addr, m);

	case 0xcb:
		op = NEXTBYTE;
		m  = z80_mnemoCB(op);
		s  = expand_mnemo(core, addr, m);
		if (cpuid != CpuZ180 || m[0] != SLL)
			return s;
		else
			return catstr(s, " ; ***illegal opcode***");

	case 0xed:
		op = NEXTBYTE;
		m  = z80_mnemoED(cpuid, op);
		s  = expand_mnemo(core, addr, m);
		if (z80_illegalED(cpuid, op) != ILLEGAL_OPCODE)
			return s;
		else
			return catstr(s, " ; ***illegal opcode***");

	case 0xdd:
	case 0xfd:
		Byte op2 = NEXTBYTE;
		if (op2 == 0xcb)
		{
			Address end = addr + 2;
			Byte	op4 = peek(core, addr + 1);
			m			= z80_mnemoXYCB(cpuid, op, op4);
			s			= expand_mnemoXYCB(core, addr, m);
			addr		= end;
			if (z80_illegalXYCB(cpuid, op4) != ILLEGAL_OPCODE)
				return s;
			else
				return catstr(s, " ; ***illegal opcode***");
		}
		m = z80_mnemoXY(op, op2);
		s = expand_mnemo(core, addr, m);
		if (z80_illegalXY(cpuid, op2) != ILLEGAL_OPCODE) return s;
		if (op2 == 0xdd || op2 == 0xed || op2 == 0xfd)
		{
			addr--;
			return usingstr("db $%02X ; ***prefix no effect***", op);
		}
		else
			return catstr(s, " ; ***illegal opcode***");
	}
}


// ===================================================================================
// Major Opcode:

static inline void skip_space(cptr& q)
{
	while (is_space(*q)) { q++; }
}
static inline void skip_word(cptr& q)
{
	while (*q > ' ' && *q < 0x7F && *q != ',') { q++; }
}

static uint8 i8080_find_cmd(cstr cmd)
{
	uint8 a = NIX, e = BC;
	while (a < e)
	{
		if (eq(cmd, word[--e])) return e;
	}
	return NIX; // not found
}

static uint8 i8080_find_arg(cstr arg)
{
	uint8 a = BC, e = NUM_WORD_DEFS;
	while (a < e)
	{
		if (eq(arg, word[a++])) return a - 1;
	}

	// N and NN can't be found because they are uppercase in words[]:
	if (eq(arg, "n")) return N;
	if (eq(arg, "nn")) return NN;

	return NIX; // not found
}

static uint8 z80_find_cmd(cstr cmd)
{
	uint8 a = NIX, e = LXI;
	while (a < e)
	{
		if (eq(cmd, word[a++])) return a - 1;
	}
	return NIX; // not found
}

static uint8 z80_find_arg(cstr arg)
{
	uint8 a = BC, e = IX;
	while (a < e)
	{
		if (eq(arg, word[a++])) return a - 1;
	}

	static struct
	{
		uint8 arg;
		char  idf[10];
	} alias[] = {
		{N, "n"},		   {NN, "nn"},	  {XN, "(n)"},		 {XNN, "(nn)"}, {DIS, "offs"}, // in JR
		{XHL, "(ix+dis)"}, {HL, "ix"},	  {XHL, "(iy+dis)"}, {HL, "iy"},

		{N00, "0x00"},	   {N00, "$00"},  {N00, "&00"},		 {N00, "00h"},	{N00, "0h"},   {N08, "0x08"}, {N08, "$08"},
		{N08, "&08"},	   {N08, "08h"},  {N08, "8h"},		 {N10, "0x10"}, {N10, "$10"},  {N10, "&10"},  {N10, "10h"},
		{N18, "0x18"},	   {N18, "$18"},  {N18, "&18"},		 {N18, "18h"},	{N20, "0x20"}, {N20, "$20"},  {N20, "&20"},
		{N20, "20h"},	   {N28, "0x28"}, {N28, "$28"},		 {N28, "&28"},	{N28, "28h"},  {N30, "0x30"}, {N30, "$30"},
		{N30, "&30"},	   {N30, "30h"},  {N38, "0x38"},	 {N38, "$38"},	{N38, "&38"},  {N38, "38h"},

		// these may lead to wrong detections, e.g. impossible "ld xh,(iy+dis)" == "ld h,(hl)":
		// {H,"xh"}, {L,"xl"},
		// {H,"yh"}, {L,"yl"},
	};

	a = 0;
	e = NELEM(alias);
	while (a < e)
	{
		if (eq(arg, alias[a++].idf)) return alias[a - 1].arg;
	}

	return NIX; // not found
}

static inline uint8 find(uint8 c, const uint8 list[8])
{
	for (uint8 i = 0; i < 8; i++)
	{
		if (list[i] == c) return i;
	}
	return 8;
}

uint8 major_opcode(cstr q) throws
{
	// calculate major opcode of instruction
	// -> this is the opcode byte for opcodes without prefix
	// -> the 2nd byte after CB, ED, IX and IY instructions
	// -> or the 4th byte after IXCB or IYCB instructions.

	// input: "ld a,N", "jr dis", "bit 0,(hl)", "ld (N),hl", etc.

	// illegals and index registers:
	// "ld a,(ix+dis)" etc. is recognized but deprecated: better use "ld a,(hl)" etc. instead.
	// "ld a,xh" etc. is not recognized: use "ld a,h" etc. instead.
	// "bit 0,(ix+dis),r" etc. is not recognized: use "bit 0,r" etc. instead.

	q = lowerstr(q);

	uint8 cmd  = NIX;
	uint8 arg1 = NIX;
	uint8 arg2 = NIX;

	skip_space(q);
	cptr q0 = q;
	skip_word(q);
	cmd = z80_find_cmd(*q ? substr(q0, q) : q0);
	if (cmd == NIX) throw DataError("unknown opcode");
	skip_space(q);

	if (*q) // arg1
	{
		q0 = q;
		skip_word(q);
		arg1 = z80_find_arg(*q ? substr(q0, q) : q0);
		if (arg1 == NIX) throw DataError("unknown argument #1");
		skip_space(q);

		if (*q == ',') // arg2
		{
			q++;
			skip_space(q);
			q0 = q;
			skip_word(q);
			arg2 = z80_find_arg(*q ? substr(q0, q) : q0);
			if (arg2 == NIX) throw DataError("unknown argument #2");
			skip_space(q);
		}

		if (*q) throw DataError("end of opcode expected");
	}

	// Search for Instruction:

	// 0x40++
	if (cmd == LD && arg1 >= B && arg1 <= A && arg2 >= B && arg2 <= A)
	{
		if (arg1 != XHL || arg2 != XHL) return Z80::LD_B_B + (arg1 - B) * 8 + (arg2 - B);
	}
	// 0x80++
	if (cmd >= ADD && cmd <= CP && arg1 == A && arg2 >= B && arg2 <= A)
	{
		return Z80::ADD_B + (cmd - ADD) * 8 + (arg2 - B);
	}
	if (cmd >= ADD && cmd <= CP && arg1 >= B && arg1 <= A && arg2 == NIX) // alternate syntax
	{
		return Z80::ADD_B + (cmd - ADD) * 8 + (arg1 - B);
	}

	// 0x00++
	for (uint i = 0; i < 0x40; i++)
	{
		const uchar* spec = z80_cmd_00[i];
		if (cmd != spec[0]) continue;
		if (arg1 != spec[1]) continue;
		if (arg2 != spec[2]) continue;
		return uint8(i);
	}

	// 0xC0++
	for (uint8 i = 0; i < 0x40; i++)
	{
		const uchar* spec = z80_cmd_C0[i];
		if (cmd != spec[0]) continue;
		if (arg1 != spec[1]) continue;
		if (arg2 != spec[2]) continue;
		return 0xc0 + i;
	}

	if (cmd == RST && arg1 >= N0 && arg1 <= N7 && arg2 == NIX) return Z80::RST00 + (arg1 - N0) * 8;
	if (cmd == HALT && arg1 == NIX) return Z80::HALT;
	if (cmd == JP && arg1 == XHL && arg2 == NIX) return Z80::JP_HL;

	// 0xCB00++
	if (cmd >= RLC && cmd <= SRL && arg1 >= B && arg1 <= A && arg2 == NIX)
	{
		return Z80::RLC_B + (cmd - RLC) * 8 + (arg1 - B);
	}

	// 0xCB40++
	if (arg1 >= N0 && arg1 <= N7 && arg2 >= B && arg2 <= A)
	{
		if (cmd == BIT) return Z80::BIT0_B + (arg1 - N0) * 8 + (arg2 - B);
		if (cmd == SET) return Z80::SET0_B + (arg1 - N0) * 8 + (arg2 - B);
		if (cmd == RES) return Z80::RES0_B + (arg1 - N0) * 8 + (arg2 - B);
	}

	// 0xED:
	for (uint8 i = 0; i < 0xC0; i++)
	{
		const uchar* spec = z180_cmd_ED[i]; // <-- include the new Z180 opcodes!
		if (cmd != spec[0]) continue;
		if (arg1 != spec[1]) continue;
		if (arg2 != spec[2]) continue;
		return i;
	}

	// error:
	throw DataError("unsuitable argument for opcode");
}

uint8 major_opcode_8080(cstr q) throws
{
	// calculate major opcode of instruction.
	// -> this is the first byte of the instruction.

	q = lowerstr(q);

	uint8 cmd  = NIX;
	uint8 arg1 = NIX;
	uint8 arg2 = NIX;

	skip_space(q);
	cptr q0 = q;
	skip_word(q);
	cmd = i8080_find_cmd(*q ? substr(q0, q) : q0);
	if (cmd == NIX) throw DataError("unknown opcode");
	skip_space(q);

	if (*q) // arg1:
	{
		q0 = q;
		skip_word(q);
		arg1 = i8080_find_arg(q ? substr(q0, q) : q0);
		if (arg1 == NIX) throw DataError("unknown argument #1");
		skip_space(q);

		if (*q == ',') // arg2:
		{
			q++;
			skip_space(q);
			q0 = q;
			skip_word(q);
			arg2 = i8080_find_arg(q ? substr(q0, q) : q0);
			if (arg2 == NIX) throw DataError("unknown argument #2");
			skip_space(q);
		}

		if (*q) throw DataError("end of opcode expected");
	}

	// Search for Instruction:

	static const uint8 regs[8] = {B, C, D, E, H, L, M, A};
	static const uint8 cmds[8] = {ADD, ADC, SUB, SBB, ANA, XRA, ORA, CMP};

	// 0x40++
	if (cmd == MOV)
	{
		uint8 z = find(arg1, regs);
		uint8 q = find(arg2, regs);
		if (z < 8 && q < 8 && (z != 6 || q != 6)) return Z80::LD_B_B + z * 8 + q;
	}

	// 0x80++
	uint8 c = find(cmd, cmds);
	if (c < 8 && (arg1 == A || arg2 == NIX))
	{
		uint8 q = find(arg2 == NIX ? arg1 : arg2, regs);
		if (q < 8) { return Z80::ADD_B + c * 8 + q; }
	}

	// 0x00++
	for (uint i = 0; i < 0x40; i++)
	{
		const uchar* spec = i8080_cmd_00[i];
		if (cmd != spec[0]) continue;
		if (arg1 != spec[1]) continue;
		if (arg2 != spec[2]) continue;
		return uint8(i);
	}

	// 0xC0++
	for (uint8 i = 0; i < 0x40; i++)
	{
		const uchar* spec = i8080_cmd_C0[i];
		if (cmd != spec[0]) continue;
		if (arg1 != spec[1]) continue;
		if (arg2 != spec[2]) continue;
		return 0xc0 + i;
	}

	if (cmd == HLT && arg1 == NIX) return Z80::HALT;

	// error:
	throw DataError("unsuitable argument for opcode");
}
