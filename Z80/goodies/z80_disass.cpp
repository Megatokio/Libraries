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

#include "z80_disass.h"
#include "z80_goodies.h"


typedef uchar Mnemo[3];


// ----	Z80 opcode definitions ------------------------------------------------------

enum
{
	NIX,	NOP,	LD,		INC,	DEC,	RLCA,	EX,		ADD,
	RRCA,	DJNZ,	RLA,	JR,		RRA,	DAA,	CPL,	HALT,
	SCF,	CCF,	RLC,	RRC,	RL,		RR,		SLA,	SRA,
	SLL,	SRL,	IN,		OUT,	SBC,	NEG,	RETN,	IM,
	ADC,	RETI,	RRD,	RLD,	SUB,	AND,	XOR,
	OR,		CP,		BIT,	RES,	SET,	LDI,	CPI,	INI,
	OUTI,	LDD,	CPD,	IND,	OUTD,	LDIR,	CPIR,	INIR,
	OTIR,	LDDR,	CPDR,	INDR,	OTDR,	RET,	POP,	JP,
	CALL,	PUSH,	RST,	PFX,	EXX,	DI,		EI,
	BC,		DE,		HL,		IX,		IY,		SP,		AF,		AF2,
	B,		C,		D,		E,		H,		L,		XHL,	A,		// <- KEEP THIS ORDER!
	XBC,	XDE,	R,		I,		XC,		XSP,	PC,		F,
	N0,		N1,		N2,		N3,		N4,		N5,		N6,		N7,
	N00,	N08,	N10,	N18,	N20,	N28,	N30,	N38,
	Z,		NZ,		NC,		PO,		PE,		M,		P,
	N,		NN,		XNN,	XN,		DIS,	CB,		ED,
	XH,		XL,		YH,		YL,		XIX,	XIY,
	IN0,	OUT0,	TST,	TSTIO,	MLT,	OTIM,	OTDM,	OTIMR,	OTDMR, SLP,
	LXI,STAX,INX,INR,DCR,MVI,DAD,LDAX,DCX,
	RAL,RAR,SHLD,LHLD,CMA,STA,STC,LDA,CMC,
	RNZ,JNZ,JMP,CNZ,ADI,RZ,JZ,CZ,ACI,RNC,JNC,CNC,
	SUI,RC,JC,CC,SBI,RPO,JPO,XTHL,CPO,ANI,RPE,PCHL,
	JPE,XCHG,CPE,XRI,RP,PSW,ORI,RM,SPHL,JM,CM,HLT,
	SBB,ANA,XRA,ORA,CMP,
	NUM_WORD_DEFS
};

static const char word[][9] =
{
	"",		"nop",	"ld",	"inc",	"dec",	"rlca",	"ex",	"add",
	"rrca",	"djnz",	"rla",	"jr",	"rra",	"daa",	"cpl",	"halt",
	"scf",	"ccf",	"rlc",	"rrc",	"rl",	"rr",	"sla",	"sra",
	"sll",	"srl",	"in",	"out",	"sbc",	"neg",	"retn",	"im",
	"adc",	"reti",	"rrd",	"rld",	"sub",	"and",	"xor",
	"or",	"cp",	"bit",	"res",	"set",	"ldi",	"cpi",	"ini",
	"outi",	"ldd",	"cpd",	"ind",	"outd",	"ldir",	"cpir",	"inir",
	"otir",	"lddr",	"cpdr",	"indr",	"otdr",	"ret",	"pop",	"jp",
	"call",	"push",	"rst",	"prefix","exx",	"di",	"ei",
	"bc",	"de",	"hl",	"ix",	"iy",	"sp",	"af",	"af'",
	"b",	"c",	"d",	"e",	"h",	"l",	"(hl)",	"a",
	"(bc)",	"(de)",	"r",	"i",	"(c)",	"(sp)",	"pc",	"f",
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",
	"0",	"8",	"16",	"24",	"32",	"40",	"48",	"56",
	"z",	"nz",	"nc",	"po",	"pe",	"m",	"p",
	"N",	"NN",	"(NN)",	"(N)",	"dis",	"cb",	"ed",
	"xh",	"xl",	"yh",	"yl",	"(ix+dis)","(iy+dis)",
	"in0",	"out0",	"tst",	"tstio","mlt",	"otim",	"otdm",	"otimr","otdmr","slp",
	"lxi",	"stax",	"inx",	"inr",	"dcr",	"mvi",	"dad",	"ldax",	"dcx",
	"ral",	"rar",	"shld",	"lhld",	"cma",	"sta",	"stc",	"lda",	"cmc",
	"rnz",	"jnz",	"jmp",	"cnz",	"adi",	"rz",	"jz",	"cz",	"aci",	"rnc",	"jnc",	"cnc",
	"sui",	"rc",	"jc",	"cc",	"sbi",	"rpo",	"jpo",	"xthl",	"cpo",	"ani",	"rpe",	"pchl",
	"jpe",	"xchg",	"cpe",	"xri",	"rp",	"psw",	"ori",	"rm",	"sphl",	"jm",	"cm",	"hlt",
	"sbb",	"ana",	"xra",	"ora",	"cmp",
};

static_assert(NELEM(word) == NUM_WORD_DEFS, "");

static const Mnemo i8080_cmd_00[64] =
{
	{NOP},  {LXI,B,NN},	 {STAX,B},	{INX,B},  {INR,B},  {DCR,B},  {MVI,B,N},  {RLC},
	{NOP},  {DAD,B},	 {LDAX,B},	{DCX,B},  {INR,C},  {DCR,C},  {MVI,C,N},  {RRC},
	{NOP},  {LXI,D,NN},	 {STAX,D},	{INX,D},  {INR,D},  {DCR,D},  {MVI,D,N},  {RAL},
	{NOP},  {DAD,D},	 {LDAX,D},	{DCX,D},  {INR,E},  {DCR,E},  {MVI,E,N},  {RAR},
	{NOP},  {LXI,H,NN},	 {SHLD,NN},	{INX,H},  {INR,H},  {DCR,H},  {MVI,H,N},  {DAA},
	{NOP},  {DAD,H},	 {LHLD,NN},	{DCX,H},  {INR,L},  {DCR,L},  {MVI,L,N},  {CMA},
	{NOP},  {LXI,SP,NN}, {STA,NN},	{INX,SP}, {INR,M},  {DCR,M},  {MVI,M,N},  {STC},
	{NOP},  {DAD,SP},	 {LDA,NN},	{DCX,SP}, {INR,A},  {DCR,A},  {MVI,A,N},  {CMC},
};

static const Mnemo i8080_cmd_C0[64] =
{
	{RNZ},	{POP,B},   {JNZ,NN},  {JMP,NN}, {CNZ,NN}, {PUSH,B},	  {ADI,N},  {RST,N0},
	{RZ},	{RET},	   {JZ,NN},	  {JMP,NN}, {CZ,NN},  {CALL,NN},  {ACI,N},  {RST,N1},
	{RNC},	{POP,D},   {JNC,NN},  {OUT,N},  {CNC,NN}, {PUSH,D},	  {SUI,N},  {RST,N2},
	{RC},	{RET},	   {JC,NN},	  {IN,N},	{CC,NN},  {CALL,NN},  {SBI,N},  {RST,N3},
	{RPO},	{POP,H},   {JPO,NN},  {XTHL},	{CPO,NN}, {PUSH,H},	  {ANI,N},  {RST,N4},
	{RPE},	{PCHL},	   {JPE,NN},  {XCHG},	{CPE,NN}, {CALL,NN},  {XRI,N},  {RST,N5},
	{RP},	{POP,PSW}, {JP,NN},	  {DI},		{CP,NN},  {PUSH,PSW}, {ORI,N},  {RST,N6},
	{RM},	{SPHL},	   {JM,NN},	  {EI},		{CM,NN},  {CALL,NN},  {CPI,N},  {RST,N7},
};

static const Mnemo z80_cmd_00[64] =
{	{NOP},	    {LD,BC,NN},	{LD,XBC,A},	{INC,BC},	{INC,B},	{DEC,B},	{LD,B,N},	{RLCA},
	{EX,AF,AF2},{ADD,HL,BC},{LD,A,XBC},	{DEC,BC},	{INC,C},	{DEC,C},	{LD,C,N},	{RRCA},
	{DJNZ,DIS}, {LD,DE,NN}, {LD,XDE,A},	{INC,DE},	{INC,D},	{DEC,D},	{LD,D,N},	{RLA},
	{JR,DIS},	{ADD,HL,DE},{LD,A,XDE},	{DEC,DE},	{INC,E},	{DEC,E},	{LD,E,N},	{RRA},
	{JR,NZ,DIS},{LD,HL,NN},	{LD,XNN,HL},{INC,HL},	{INC,H},	{DEC,H},	{LD,H,N},	{DAA},
	{JR,Z,DIS},	{ADD,HL,HL},{LD,HL,XNN},{DEC,HL},	{INC,L}, 	{DEC,L}, 	{LD,L,N},	{CPL},
	{JR,NC,DIS},{LD,SP,NN},	{LD,XNN,A},	{INC,SP},	{INC,XHL},	{DEC,XHL},	{LD,XHL,N},	{SCF},
	{JR,C,DIS},	{ADD,HL,SP},{LD,A,XNN},	{DEC,SP},	{INC,A},	{DEC,A},	{LD,A,N},	{CCF},
};

static const Mnemo z80_cmd_C0[64] =
{	{RET,NZ}, {POP,BC},	 {JP,NZ,NN}, {JP,NN},	 {CALL,NZ,NN},	{PUSH,BC},  {ADD,A,N},  {RST,N00},
	{RET,Z},  {RET},	 {JP,Z,NN},	 {PFX,CB},	 {CALL,Z,NN},	{CALL,NN},  {ADC,A,N},  {RST,N08},
	{RET,NC}, {POP,DE},	 {JP,NC,NN}, {OUT,XN,A}, {CALL,NC,NN},	{PUSH,DE},  {SUB,A,N},  {RST,N10},
	{RET,C},  {EXX},	 {JP,C,NN},	 {IN,A,XN},	 {CALL,C,NN},	{PFX,IX},	{SBC,A,N},  {RST,N18},
	{RET,PO}, {POP,HL},	 {JP,PO,NN}, {EX,HL,XSP},{CALL,PO,NN},	{PUSH,HL},  {AND,A,N},  {RST,N20},
	{RET,PE}, {JP,HL},	 {JP,PE,NN}, {EX,DE,HL}, {CALL,PE,NN},	{PFX,ED},	{XOR,A,N},  {RST,N28},
	{RET,P},  {POP,AF},	 {JP,P,NN},	 {DI},		 {CALL,P,NN},	{PUSH,AF},  {OR,A,N},   {RST,N30},
	{RET,M},  {LD,SP,HL},{JP,M,NN},	 {EI},		 {CALL,M,NN},	{PFX,IY},	{CP,A,N},   {RST,N38},
};

static const Mnemo z80_cmd_ED40[64] =
{	{IN,B,XC},	{OUT,XC,B},	{SBC,HL,BC},  {LD,XNN,BC},  {NEG},  {RETN},  {IM,N0},  {LD,I,A},
	{IN,C,XC},	{OUT,XC,C},	{ADC,HL,BC},  {LD,BC,XNN},  {NEG},  {RETI},  {IM,N0},  {LD,R,A},
	{IN,D,XC},	{OUT,XC,D},	{SBC,HL,DE},  {LD,XNN,DE},  {NEG},  {RETN},  {IM,N1},  {LD,A,I},
	{IN,E,XC},	{OUT,XC,E},	{ADC,HL,DE},  {LD,DE,XNN},  {NEG},  {RETI},  {IM,N2},  {LD,A,R},
	{IN,H,XC},	{OUT,XC,H},	{SBC,HL,HL},  {LD,XNN,HL},  {NEG},  {RETN},  {IM,N0},  {RRD},
	{IN,L,XC},	{OUT,XC,L},	{ADC,HL,HL},  {LD,HL,XNN},  {NEG},  {RETI},  {IM,N0},  {RLD},
	{IN,F,XC},	{OUT,XC,N0},{SBC,HL,SP},  {LD,XNN,SP},  {NEG},  {RETN},  {IM,N1},  {NOP},
	{IN,A,XC},	{OUT,XC,A},	{ADC,HL,SP},  {LD,SP,XNN},  {NEG},  {RETI},  {IM,N2},  {NOP},
};

static const Mnemo z180_cmd_ED[3*64] =
{
	{IN0,B,XN}, {OUT0,XN,B},  {NOP},  {NOP},  {TST,B},	 {NOP},  {NOP},  {NOP},
	{IN0,C,XN}, {OUT0,XN,C},  {NOP},  {NOP},  {TST,C},	 {NOP},  {NOP},  {NOP},
	{IN0,D,XN}, {OUT0,XN,D},  {NOP},  {NOP},  {TST,D},	 {NOP},  {NOP},  {NOP},
	{IN0,E,XN}, {OUT0,XN,E},  {NOP},  {NOP},  {TST,E},	 {NOP},  {NOP},  {NOP},
	{IN0,H,XN}, {OUT0,XN,H},  {NOP},  {NOP},  {TST,H},	 {NOP},  {NOP},  {NOP},
	{IN0,L,XN}, {OUT0,XN,L},  {NOP},  {NOP},  {TST,L},	 {NOP},  {NOP},  {NOP},
	{IN0,F,XN}, {NOP},		  {NOP},  {NOP},  {TST,XHL}, {NOP},  {NOP},  {NOP},
	{IN0,A,XN}, {OUT0,XN,A},  {NOP},  {NOP},  {TST,A},	 {NOP},  {NOP},  {NOP},

	{IN,B,XC},  {OUT,XC,B},  {SBC,HL,BC},  {LD,XNN,BC},  {NEG},		{RETN},	 {IM,N0},	{LD,I,A},
	{IN,C,XC},  {OUT,XC,C},  {ADC,HL,BC},  {LD,BC,XNN},  {MLT,BC},	{RETI},	 {NOP},		{LD,R,A},
	{IN,D,XC},  {OUT,XC,D},  {SBC,HL,DE},  {LD,XNN,DE},  {NOP},		{NOP},	 {IM,N1},	{LD,A,I},
	{IN,E,XC},  {OUT,XC,E},  {ADC,HL,DE},  {LD,DE,XNN},  {MLT,DE},	{NOP},	 {IM,N2},	{LD,A,R},
	{IN,H,XC},  {OUT,XC,H},  {SBC,HL,HL},  {LD,XNN,HL},  {TST,N},	{NOP},	 {NOP},		{RRD},
	{IN,L,XC},  {OUT,XC,L},  {ADC,HL,HL},  {LD,HL,XNN},  {MLT,HL},	{NOP},	 {NOP},		{RLD},
	{IN,F,XC},  {NOP},		 {SBC,HL,SP},  {LD,XNN,SP},  {TSTIO,N},	{NOP},	 {SLP},		{NOP},
	{IN,A,XC},  {OUT,XC,A},  {ADC,HL,SP},  {LD,SP,XNN},  {MLT,SP},	{NOP},	 {NOP},		{NOP},

	{NOP},	{NOP},	{NOP},	{OTIM},	 {NOP},  {NOP},  {NOP},  {NOP},
	{NOP},	{NOP},	{NOP},	{OTDM},	 {NOP},  {NOP},  {NOP},  {NOP},
	{NOP},	{NOP},	{NOP},	{OTIMR}, {NOP},  {NOP},  {NOP},  {NOP},
	{NOP},	{NOP},	{NOP},	{OTDMR}, {NOP},  {NOP},  {NOP},  {NOP},
	{LDI},	{CPI},	{INI},	{OUTI},	 {NOP},  {NOP},  {NOP},  {NOP},
	{LDD},	{CPD},	{IND},	{OUTD},	 {NOP},  {NOP},  {NOP},  {NOP},
	{LDIR},	{CPIR},	{INIR},	{OTIR},	 {NOP},  {NOP},  {NOP},  {NOP},
	{LDDR},	{CPDR},	{INDR},	{OTDR},	 {NOP},  {NOP},  {NOP},  {NOP},
};


// ============================================================================================
// Mnemonic:

static inline void copy3 (uchar* z, const uchar* q)
{
	*z++ = *q++;
	*z++ = *q++;
	*z++ = *q++;
}

static const uchar* z80_mnemo (Byte op)
{
	// return m[3] mnenonic descriptor for normal instructions
	// tempmem or const

	static const uchar cmd_ari[]  = { ADD,ADC,SUB,SBC,AND,XOR,OR,CP };
	static const Mnemo cmd_halt = { HALT,0,0 };

	uchar* s;
	switch(op>>6)
	{
	case 0:
		return z80_cmd_00[op];
	case 1:
		if (op==0x76) return cmd_halt;
		s = temp<uchar>(3);
		s[0] = LD;
		s[1] = B + ((op>>3)&0x07);
		s[2] = B + (op&0x07);
		return s;
	case 2:
		s = temp<uchar>(3);
		s[0] = cmd_ari[(op>>3)&0x07];
		s[1] = A;
		s[2] = B + (op&0x07);
		return s;
	//case 3:
	default:
		return z80_cmd_C0[op&0x3f];
	}
}

static const uchar* i8080_mnemo (Byte op)
{
	// return m[3] mnenonic descriptor for normal instructions
	// tempmem or const

	static const Mnemo cmd_hlt = { HLT,0,0 };
	static const uchar ari[]   = { ADD,ADC,SUB,SBB,ANA,XRA,ORA,CMP };
	static const uchar reg[]   = { B,C,D,E,H,L,M,A };

	uchar* s;
	switch(op>>6)
	{
	case 0:
		return i8080_cmd_00[op];
	case 1:
		if (op==0x76) return cmd_hlt;
		s = temp<uchar>(3);
		s[0] = LD;
		s[1] = reg[(op>>3)&0x07];
		s[2] = reg[op&0x07];
		return s;
	case 2:
		s = temp<uchar>(3);
		s[0] = ari[(op>>3)&0x07];
		s[1] = reg[op&0x07];
		s[2] = 0;
		return s;
	//case 3:
	default:
		return i8080_cmd_C0[op&0x3f];
	}
}

static uchar* z80_mnemoCB (Byte op2)
{
	// return m[3] mnenonic descriptor for CB instructions
	// tempmem or const

	static const uchar cmd_sh[] = { RLC,RRC,RL,RR,SLA,SRA,SLL,SRL };

	uchar* s = temp<uchar>(3);
	switch(op2>>6)
	{
	case 0:
		s[0] = cmd_sh[(op2>>3)&0x07];
		s[1] = B + (op2&0x07);
		s[2] = 0;
		return s;
	case 1:
		s[0] = BIT; break;
	case 2:
		s[0] = RES; break;
	case 3:
		s[0] = SET; break;
	}
	s[1] = N0 + ((op2>>3)&0x07);
	s[2] = B  + (op2&0x07);
	return s;
}

static const uchar* z80_mnemoIXCB (Byte op4)
{
	// return m[3] mnenonic descriptor for IXCB instructions
	// tempmem or const

	uchar* c = z80_mnemoCB(op4);
	if (c[1]==XHL) c[1] = XIX;	// this is only allowed, because mnemo() doesn't
	if (c[2]==XHL) c[2] = XIX;	// retrieve a pointer but creates mnemo descr ad hoc
	return c;
}

static const uchar* z80_mnemoIYCB (Byte op4)
{
	// return Mnemo for IYCB instructions
	// tempmem or const

	uchar* c = z80_mnemoCB(op4);
	if (c[1]==XHL) c[1] = XIY;	// this is only allowed, because mnemo() doesn't
	if (c[2]==XHL) c[2] = XIY;	// retrieve a pointer but creates mnemo descr ad hoc
	return c;
}

static const uchar* z80_mnemoED (Byte op2)
{
	// return Mnemo for ED instructions

	if (op2 >= 0x40 && op2 < 0x80) return z80_cmd_ED40[op2-0x40];
	if (op2 >= 0xA0 && op2 < 0xC0) return z180_cmd_ED[op2];
	static const Mnemo cmd_nop  = { NOP,0,0 };
	return cmd_nop;
}

static const uchar* z180_mnemoED (Byte op2)
{
	// return Mnemo for ED instructions

	if (op2 < 0xC0) return z180_cmd_ED[op2];
	static const Mnemo cmd_nop  = { NOP,0,0 };
	return cmd_nop;
}

static const uchar* z80_mnemoIX (Byte op2)
{
	// return m[3] mnenonic descriptor for IX instructions
	// tempmem or const

	uchar* s = temp<uchar>(3);
	copy3(s, z80_mnemo(op2));
	if (s[1]==XHL) { s[1] = XIX; return s; }
	if (s[2]==XHL) { s[2] = XIX; return s; }
	if (s[1]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[1] = IX;  return s; }
	if (s[2]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[2] = IX;  return s; }
	if (s[1]==H) s[1] = XH;
	if (s[1]==L) s[1] = XL;
	if (s[2]==H) s[2] = XH;
	if (s[2]==L) s[2] = XL;
	return s;
}

static const uchar* z80_mnemoIY (Byte op2)
{
	// return m[3] mnenonic descriptor for IY instructions
	// tempmem or const

	uchar* s = temp<uchar>(3);
	copy3(s, z80_mnemo(op2));
	if (s[1]==XHL) { s[1] = XIY; return s; }
	if (s[2]==XHL) { s[2] = XIY; return s; }
	if (s[1]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[1] = IY;  return s; }
	if (s[2]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[2] = IY;  return s; }
	if (s[1]==H) s[1] = YH;
	if (s[1]==L) s[1] = YL;
	if (s[2]==H) s[2] = YH;
	if (s[2]==L) s[2] = YL;
	return s;
}

// deprecated
cstr opcode_mnemo (CpuID cpuid, const Byte* core, Address addr)
{
	// return mnenonic with symbolic arguments for instructions
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))
	// returns tempstr or const string

	assert(cpuid==CpuZ80 || cpuid==CpuDefault);	// others: TODO
	(void)cpuid;

	const uchar* m;
	const uint8 op1 = peek(core,addr);

	switch(op1)
	{
	case 0xCB:	m = z80_mnemoCB(peek(core,addr+1)); break;
	case 0xED:	m = z80_mnemoED(peek(core,addr+1)); break;
	case 0xDD:	m = peek(core,addr+1)==0xCB ? z80_mnemoIXCB(peek(core,addr+3)) : z80_mnemoIX(peek(core,addr+1)); break;
	case 0xFD:	m = peek(core,addr+1)==0xCB ? z80_mnemoIYCB(peek(core,addr+3)) : z80_mnemoIY(peek(core,addr+1)); break;
	default:	m = z80_mnemo(op1);                 break;
	}

	cstr s1 = word[m[0]];  if(m[1]==0) return s1;
	cstr s2 = word[m[1]];
	cstr s3 = word[m[2]];

	str s = tempstr( 5 + strlen(s2) + (*s3?1+strlen(s3):0) );

	strcpy(s,s1);
	strcat(s,"    ");
	strcpy(s+5,s2);

	if(*s3)
	{
		strcat(s,",");
		strcat(s,s3);
	}
	return s;
}


// ================================================================================
// Opcode Legal State:

static inline int z80_illegalCB (Byte op)
{
	// get legal state of CB instruction
	// all instructions legal except: sll is illegal

	return op>=0x30 && op<0x38 ? IllegalOpcode : LegalOpcode;
}

static inline int z80_illegalED (Byte op)
{
	// get legal state of ED instruction
	// 0x00-0x3F and 0x80-0xFF weird except block instructions
	// 0x40-0x7F legal or weird
	// in f,(c) is legal; out (c),0 is weird

	const static char il[] = "1111111111110101111100111111001111110001111100011011000011110000";

	if ((op>>6)==1)	return il[op-0x40]-'0' ? WeirdOpcode : LegalOpcode;
	return *z80_mnemoED(op)==NOP ? WeirdOpcode : LegalOpcode;
}

static inline int z80_illegalXY (Byte op)
{
	// get legal state of IX/IY instruction
	// all illegal instructions, which use XH or XL are illegal
	// all illegal instructions, which don't use XH or XL are weird
	// prefixes are legal

	const uchar* c = z80_mnemo(op);

	if (c[0]==PFX || c[1]==XHL || c[2]==XHL) return LegalOpcode;
	if (c[1]==H||c[1]==L||c[2]==H||c[2]==L) return IllegalOpcode;
	return WeirdOpcode;
}

static inline int z80_illegalXYCB (Byte op)
{
	// get legal state of IXCB/IYCB instruction
	// all instructions which do not use IX are weird
	// instructions using IX are legal except: sll is illegal

	if ((op&0x07)!=6) return WeirdOpcode;
	return op>=0x30 && op<0x38 ? IllegalOpcode : LegalOpcode;
}

int opcode_legal_state (CpuID cpuid, const Byte* core, Address addr)
{
	// get legal state of instruction
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))

	assert(cpuid==CpuZ80 || cpuid==CpuDefault);	// others: TODO
	(void)cpuid;

	switch (peek(core,addr))
	{
	case 0xCB: return z80_illegalCB(peek(core,addr+1));
	case 0xED: return z80_illegalED(peek(core,addr+1));
	case 0xDD:
	case 0xFD: return peek(core,addr+1)==0xCB ? z80_illegalXYCB(peek(core,addr+3)) : z80_illegalXY(peek(core,addr+1));
	default:   return LegalOpcode;
	}
}


// ===================================================================================
// Disassemble:

#define NEXTBYTE    peek(core,addr++)

static cstr expand_word (const Byte* core, Address& addr, uint8 token)
{
	// expand token n, reading opcode parameters via *ip++ if needed
	// tempstr or const string

	uint n,nn;
	str s;

	switch (token)
	{
	case DIS:
		n  = NEXTBYTE;
		nn = Word(addr + int8(n));
		s = tempstr(14);							// "$+123 ; $FFFF"
		sprintf(s, "$%+i ; $%04X", int8(n+2), nn);	// "$+dis ; $dest"
		return s;
	case N:
		s = tempstr(3);    // "255"
		sprintf(s, "%u", NEXTBYTE);
		return s;
	case NN:
		nn = NEXTBYTE;
		nn = nn + 256 * NEXTBYTE;
		s = tempstr(5);    // "$FFFF"
		sprintf(s, "$%04X", nn);
		return s;
	case XNN:
		nn = NEXTBYTE;
		nn = nn + 256 * NEXTBYTE;
		s = tempstr(7);    // "($FFFF)"
		sprintf(s, "($%04X)", nn);
		return s;
	case XN:
		s = tempstr(5);    // "($FF)"
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
	default:
		return word[token];
	}
}

static cstr expand_mnemo (const Byte* core, Address& addr, const Mnemo m)
{
	cstr cmd = word[m[0]];
	if (m[1]==0) return cmd;

	cstr s2 = expand_word(core, addr, m[1]);
	if (m[2]==0) return catstr(cmd," ",s2);

	cstr s3 = expand_word(core, addr, m[2]);
	return catstr(cmd, " ", s2, ",", s3);
}

cstr disassemble_z80 (const Byte* core, Address& addr, int option)
{
	// disassemble one instruction and increment ip
	// tempstr or const string

	// TODO: option DISASS_IXCBR2 and DISASS_IXCBXH
	(void)option;

	const uchar* m;
	Byte op = NEXTBYTE;
	bool ixcb = 0;

	switch(op)
	{
	case 0xcb:
		m = z80_mnemoCB(NEXTBYTE);
		break;
	case 0xed:
		m = z80_mnemoED(NEXTBYTE);
		break;
	case 0xdd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? z80_mnemoIXCB(peek(core,addr+1)) : z80_mnemoIX(op);
		break;
	case 0xfd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? z80_mnemoIYCB(peek(core,addr+1)) : z80_mnemoIY(op);
		break;
	default:
		m = z80_mnemo(op);
		break;
	}

	cstr s = expand_mnemo(core,addr,m);
	addr += ixcb;
	return s;
}

cstr disassemble_z180 (const Byte* core, Address& addr)
{
	const uchar* m;
	Byte op = NEXTBYTE;
	bool ixcb = 0;
	bool legal = yes;
	cstr s;

	switch(op)
	{
	case 0xcb:
		m = z80_mnemoCB(NEXTBYTE);
		legal = m[0] != SLL;
		break;
	case 0xed:
		m = z180_mnemoED(NEXTBYTE);		// TODO
		legal = m[0] != NOP;
		break;
	case 0xdd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? z80_mnemoIXCB(peek(core,addr+1)) : z80_mnemoIX(op);
		legal = m[0]!=SLL && (m[1]==IX || m[1]==XIX || m[2]==IX || m[2]==XIX);
		break;
	case 0xfd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? z80_mnemoIYCB(peek(core,addr+1)) : z80_mnemoIY(op);
		legal = m[0]!=SLL && (m[1]==IY || m[1]==XIY || m[2]==IY || m[2]==XIY);
		break;
	default:
		m = z80_mnemo(op);		// this should be ok for all other opcodes
		break;
	}

	s = expand_mnemo(core,addr,m);
	addr += ixcb;
	return legal ? s : catstr(s, " ; ***illegal opcode***");
}

cstr disassemble_8080 (const Byte* core, Address& addr, int syntax)
{
	// disassemble 8080 opcode using z80 or asm8080 syntax

	Byte op = NEXTBYTE;
	cstr s;

	// test for deprecated opcode:
	// these are all NOP, CALL, JP or RET
	// which happen to be the same in asm8080 and Z80 syntax:

	switch(op)
	{
	case 8:		return "nop ; $08: ***deprecated*** Z80: EX AF,AF'";
	case 16:	return "nop ; $10: ***deprecated*** Z80: DJNZ dest";
	case 24:	return "nop ; $18: ***deprecated*** Z80: JR dest";
	case 32:	return "nop ; $20: ***deprecated*** Z80: JR nz,dest";
	case 40:	return "nop ; $28: ***deprecated*** Z80: JR z,dest";
	case 48:	return "nop ; $30: ***deprecated*** Z80: JR nc,dest";
	case 56:	return "nop ; $38: ***deprecated*** Z80: JR c,dest";
	case 0xD9:	return "ret ; $D9: ***deprecated*** Z80: EXX";

	case 0xCB:
		s = expand_word(core,addr,uchar(NN));
		return usingstr("%s %s ; $CB: ***deprecated*** Z80: prefix CB", (syntax==DISASS_ASM8080?"jmp":"jp"), s);

	case 0xDD:
	case 0xED:
	case 0xFD:
		s = expand_word(core,addr,uchar(NN));
		return usingstr("call %s ; $%02X: ***deprecated*** Z80: prefix $%02X",s,op,op);
	}

	// legal opcode: return asm8080 or Z80 systax:

	const uchar* mnemo = (syntax == DISASS_ASM8080 ? i8080_mnemo : z80_mnemo)(op);
	return expand_mnemo(core,addr,mnemo);
}

cstr disassemble (CpuID cpuid, const Byte* core, Address& addr, int option)
{
	// disassemble one instruction and increment addr
	// tempstr or const string

	switch(cpuid)
	{
	default:
	case CpuZ80:	return disassemble_z80(core,addr,option);
	case CpuZ180:	return disassemble_z180(core,addr);
	case Cpu8080:	return disassemble_8080(core,addr,option);
	}
}





























