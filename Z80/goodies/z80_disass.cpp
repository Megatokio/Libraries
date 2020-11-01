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
	XH,		XL,		YH,		YL,		XIX,	XIY
};

static cstr word[] =
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
	"$00",	"$08",	"$10",	"$18",	"$20",	"$28",	"$30",	"$38",
	"z",	"nz",	"nc",	"po",	"pe",	"m",	"p",
	"N",	"NN",	"(NN)",	"(N)",	"dis",	"cb",	"ed",
	"xh",	"xl",	"yh",	"yl",	"(ix+dis)","(iy+dis)"
};

static char cmd_00[64][3] =
{	{NOP,0,0},	{LD,BC,NN},	{LD,XBC,A},	{INC,BC,0},	{INC,B,0},	{DEC,B,0},	{LD,B,N},	{RLCA,0,0},
	{EX,AF,AF2},{ADD,HL,BC},{LD,A,XBC},	{DEC,BC,0},	{INC,C,0},	{DEC,C,0},	{LD,C,N},	{RRCA,0,0},
	{DJNZ,DIS,0},{LD,DE,NN},{LD,XDE,A},	{INC,DE,0},	{INC,D,0},	{DEC,D,0},	{LD,D,N},	{RLA,0,0},
	{JR,DIS,0},	{ADD,HL,DE},{LD,A,XDE},	{DEC,DE,0},	{INC,E,0},	{DEC,E,0},	{LD,E,N},	{RRA,0,0},
	{JR,NZ,DIS},{LD,HL,NN},	{LD,XNN,HL},{INC,HL,0},	{INC,H,0},	{DEC,H,0},	{LD,H,N},	{DAA,0,0},
	{JR,Z,DIS},	{ADD,HL,HL},{LD,HL,XNN},{DEC,HL,0},	{INC,L,0}, 	{DEC,L,0}, 	{LD,L,N},	{CPL,0,0},
	{JR,NC,DIS},{LD,SP,NN},	{LD,XNN,A},	{INC,SP,0},	{INC,XHL,0},{DEC,XHL,0},{LD,XHL,N},	{SCF,0,0},
	{JR,C,DIS},	{ADD,HL,SP},{LD,A,XNN},	{DEC,SP,0},	{INC,A,0},	{DEC,A,0},	{LD,A,N},	{CCF,0,0}
};

static char cmd_C0[64][3] =
{	{RET,NZ,0},	{POP,BC,0},	{JP,NZ,NN},	{JP,NN,0},	{CALL,NZ,NN},	{PUSH,BC,0},	{ADD,A,N},	{RST,N00,0},
	{RET,Z,0},	{RET,0,0},	{JP,Z,NN},	{PFX,CB,0},	{CALL,Z,NN},	{CALL,NN,0},	{ADC,A,N},	{RST,N08,0},
	{RET,NC,0},	{POP,DE,0},	{JP,NC,NN},	{OUT,XN,A},	{CALL,NC,NN},	{PUSH,DE,0},	{SUB,A,N},	{RST,N10,0},
	{RET,C,0},	{EXX,0,0},	{JP,C,NN},	{IN,A,XN},	{CALL,C,NN},	{PFX,IX,0},		{SBC,A,N},	{RST,N18,0},
	{RET,PO,0},	{POP,HL,0},	{JP,PO,NN},	{EX,HL,XSP},{CALL,PO,NN},	{PUSH,HL,0},	{AND,A,N},	{RST,N20,0},
	{RET,PE,0},	{JP,HL,0},	{JP,PE,NN},	{EX,DE,HL},	{CALL,PE,NN},	{PFX,ED,0},		{XOR,A,N},	{RST,N28,0},  // ld pc,hl statt jp(hl) da: (hl) wird zu (ix+dis)
	{RET,P,0},	{POP,AF,0},	{JP,P,NN},	{DI,0,0},	{CALL,P,NN},	{PUSH,AF,0},	{OR,A,N},	{RST,N30,0},
	{RET,M,0},	{LD,SP,HL},	{JP,M,NN},	{EI,0,0},	{CALL,M,NN},	{PFX,IY,0},		{CP,A,N},	{RST,N38,0}
};

static char cmd_ED40[64][3] =
{	{IN,B,XC},	{OUT,XC,B},	{SBC,HL,BC},	{LD,XNN,BC},	{NEG,0,0},	{RETN,0,0},	{IM,N0,0},	{LD,I,A},
	{IN,C,XC},	{OUT,XC,C},	{ADC,HL,BC},	{LD,BC,XNN},	{NEG,0,0},	{RETI,0,0},	{IM,N0,0},	{LD,R,A},
	{IN,D,XC},	{OUT,XC,D},	{SBC,HL,DE},	{LD,XNN,DE},	{NEG,0,0},	{RETN,0,0},	{IM,N1,0},	{LD,A,I},
	{IN,E,XC},	{OUT,XC,E},	{ADC,HL,DE},	{LD,DE,XNN},	{NEG,0,0},	{RETI,0,0},	{IM,N2,0},	{LD,A,R},
	{IN,H,XC},	{OUT,XC,H},	{SBC,HL,HL},	{LD,XNN,HL},	{NEG,0,0},	{RETN,0,0},	{IM,N0,0},	{RRD,0,0},
	{IN,L,XC},	{OUT,XC,L},	{ADC,HL,HL},	{LD,HL,XNN},	{NEG,0,0},	{RETI,0,0},	{IM,N0,0},	{RLD,0,0},
	{IN,F,XC},	{OUT,XC,N0},{SBC,HL,SP},	{LD,XNN,SP},	{NEG,0,0},	{RETN,0,0},	{IM,N1,0},	{NOP,0,0},
	{IN,A,XC},	{OUT,XC,A},	{ADC,HL,SP},	{LD,SP,XNN},	{NEG,0,0},	{RETI,0,0},	{IM,N2,0},	{NOP,0,0}
};

static char cmd_halt[]	= { HALT,0,0 };
static char cmd_nop[]	= { NOP,0,0 };
static char c_ari[]     = { ADD,ADC,SUB,SBC,AND,XOR,OR,CP };
static char c_blk[]     = { LDI,CPI,INI,OUTI,0,0,0,0,LDD,CPD,IND,OUTD,0,0,0,0, LDIR,CPIR,INIR,OTIR,0,0,0,0,LDDR,CPDR,INDR,OTDR };
static char c_sh[]  	= { RLC,RRC,RL,RR,SLA,SRA,SLL,SRL };


// ============================================================================================
// Mnemonic:

inline void copy3 (char* z, const char* q)
{
	*z++ = *q++;
	*z++ = *q++;
	*z++ = *q++;
}

static char* mnemo (Byte op)	// Z80
{
	// return m[3] mnenonic descriptor for normal instructions
	// tempmem or const

	char* s;
	switch(op>>6)
	{
	case 0:
		return cmd_00[op];
	case 1:
		if (op==0x76) return cmd_halt;
		s = tempmem(3);
		s[0] = LD;
		s[1] = B + ((op>>3)&0x07);
		s[2] = B + (op&0x07);
		return s;
	case 2:
		s = tempmem(3);
		s[0] = c_ari[(op>>3)&0x07];
		s[1] = A;
		s[2] = B + (op&0x07);
		return s;
	//case 3:
	default:
		return cmd_C0[op&0x3f];
	}
}

static char* mnemoCB (Byte op2)	// Z80
{
	// return m[3] mnenonic descriptor for CB instructions
	// tempmem or const

	char* s = tempmem(3);
	switch(op2>>6)
	{
	case 0:
		s[0] = c_sh[(op2>>3)&0x07];
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

static char* mnemoIXCB (Byte op4)	// Z80
{
	// return m[3] mnenonic descriptor for IXCB instructions
	// tempmem or const

	char* c = mnemoCB(op4);
	if (c[1]==XHL) c[1] = char(XIX);	// this is only allowed, because mnemo() doesn't
	if (c[2]==XHL) c[2] = char(XIX);	// retrieve a pointer but creates mnemo descr ad hoc
	return c;
}

static char* mnemoIYCB (Byte op4)	// Z80
{
	// return m[3] mnenonic descriptor for IYCB instructions
	// tempmem or const

	char* c = mnemoCB(op4);
	if (c[1]==XHL) c[1] = char(XIY);	// this is only allowed, because mnemo() doesn't
	if (c[2]==XHL) c[2] = char(XIY);	// retrieve a pointer but creates mnemo descr ad hoc
	return c;
}

static char* mnemoED (Byte op2)	// Z80
{
	// return m[3] mnenonic descriptor for ED instructions
	// tempmem or const

	if(op2<0x40) return cmd_nop;

	if(op2>=0x080)
	{
		if( (op2&0xE4)!=0xA0 ) return cmd_nop;

		char* s = tempmem(3);
		s[0]=c_blk[op2&0x1B];
		s[1]=0;
		s[2]=0;
		return s;
	}

	return cmd_ED40[op2-0x40];
}

static char* mnemoIX (Byte op2)	// Z80
{
	// return m[3] mnenonic descriptor for IX instructions
	// tempmem or const

	char* s = tempmem(3);
	copy3(s, mnemo(op2));
	if (s[1]==XHL) { s[1] = char(XIX); return s; }
	if (s[2]==XHL) { s[2] = char(XIX); return s; }
	if (s[1]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[1] = IX;  return s; }
	if (s[2]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[2] = IX;  return s; }
	if (s[1]==H) s[1] = XH;
	if (s[1]==L) s[1] = XL;
	if (s[2]==H) s[2] = XH;
	if (s[2]==L) s[2] = XL;
	return s;
}

static char* mnemoIY (Byte op2)	// Z80
{
	// return m[3] mnenonic descriptor for IY instructions
	// tempmem or const

	char* s = tempmem(3);
	copy3(s, mnemo(op2));
	if (s[1]==XHL) { s[1] = char(XIY); return s; }
	if (s[2]==XHL) { s[2] = char(XIY); return s; }
	if (s[1]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[1] = IY;  return s; }
	if (s[2]==HL)  { if (op2!=0xEB/*EX_DE_HL*/) s[2] = IY;  return s; }
	if (s[1]==H) s[1] = YH;
	if (s[1]==L) s[1] = YL;
	if (s[2]==H) s[2] = YH;
	if (s[2]==L) s[2] = YL;
	return s;
}

cstr opcode_mnemo (CpuID cpuid, const Byte* core, Address addr)
{
	// return mnenonic with symbolic arguments for instructions
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))
	// returns tempstr or const string

	assert(cpuid==CpuZ80 || cpuid==CpuDefault);	// others: TODO
	(void)cpuid;

	char* m;
	const uint8 op1 = peek(core,addr);

	switch(op1)
	{
	case 0xCB:	m = mnemoCB(peek(core,addr+1)); break;
	case 0xED:	m = mnemoED(peek(core,addr+1)); break;
	case 0xDD:	m = peek(core,addr+1)==0xCB ? mnemoIXCB(peek(core,addr+3)) : mnemoIX(peek(core,addr+1)); break;
	case 0xFD:	m = peek(core,addr+1)==0xCB ? mnemoIYCB(peek(core,addr+3)) : mnemoIY(peek(core,addr+1)); break;
	default:	m = mnemo(op1);                 break;
	}

	cstr s1 = word[uchar(m[0])];  if(m[1]==0) return s1;
	cstr s2 = word[uchar(m[1])];
	cstr s3 = word[uchar(m[2])];

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

inline int illegalCB (Byte op)	// Z80
{
	// get legal state of CB instruction
	// all instructions legal except: sll is illegal

	return op>=0x30 && op<0x38 ? IllegalOpcode : LegalOpcode;
}

inline int illegalED (Byte op)	// Z80
{
	// get legal state of ED instruction
	// 0x00-0x3F and 0x80-0xFF weird except block instructions
	// 0x40-0x7F legal or weird
	// in f,(c) is legal; out (c),0 is weird

	const static char il[] = "1111111111110101111100111111001111110001111100011011000011110000";

	if ((op>>6)==1)	return il[op-0x40]-'0' ? WeirdOpcode : LegalOpcode;
	return *mnemoED(op)==NOP ? WeirdOpcode : LegalOpcode;
}

inline int illegalXY (Byte op)	// Z80
{
	// get legal state of IX/IY instruction
	// all illegal instructions, which use XH or XL are illegal
	// all illegal instructions, which don't use XH or XL are weird
	// prefixes are legal

	cstr c = mnemo(op);

	if (c[0]==PFX || c[1]==XHL || c[2]==XHL) return LegalOpcode;
	if (c[1]==H||c[1]==L||c[2]==H||c[2]==L) return IllegalOpcode;
	return WeirdOpcode;
}

inline int illegalXYCB (Byte op)	// Z80
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
	case 0xCB: return illegalCB(peek(core,addr+1));
	case 0xED: return illegalED(peek(core,addr+1));
	case 0xDD:
	case 0xFD: return peek(core,addr+1)==0xCB ? illegalXYCB(peek(core,addr+3)) : illegalXY(peek(core,addr+1));
	default:   return LegalOpcode;
	}
}


// ===================================================================================
// Disassemble:

#define NEXTBYTE    peek(core,addr++)

static cstr expand_word (uint8 token, const Byte* core, Address& addr)
{
	// expand token n, reading opcode parameters via *ip++ if needed
	// tempstr or const string

	uint nn;
	str s;

	switch (token)
	{
	case DIS:
		nn = NEXTBYTE;
		nn = Word(addr + int8(nn));
		s = tempstr(5);    // "$FFFF"
		sprintf(s, "$%04X", nn);		// branch destination
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
		sprintf(s, "(ix%+i)", NEXTBYTE);
		return s;
	case XIY:
		s = tempstr(8); // "(iy+123)"
		sprintf(s, "(iy%+i)", NEXTBYTE);
		return s;
	default:
		return word[token];
	}
}

cstr disassemble_z80 (const Byte* core, Address& addr)
{
	// disassemble one instruction and increment ip
	// tempstr or const string

	cstr m;
	Byte op = NEXTBYTE;
	bool ixcb = 0;

	switch(op)
	{
	case 0xcb:
		m = mnemoCB(NEXTBYTE);
		break;
	case 0xed:
		m = mnemoED(NEXTBYTE);
		break;
	case 0xdd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? mnemoIXCB(peek(core,addr+1)) : mnemoIX(op);
		break;
	case 0xfd:
		op = NEXTBYTE;
		ixcb = op == 0xCB;
		m = ixcb ? mnemoIYCB(peek(core,addr+1)) : mnemoIY(op);
		break;
	default:
		m = mnemo(op);
		break;
	}

	cstr s1 = word[uchar(m[0])]; if(m[1]==0) return s1;
	cstr s2 = expand_word(uchar(m[1]),core,addr);
	cstr s3 = expand_word(uchar(m[2]),core,addr);
	addr += ixcb;

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

cstr disassemble_i8080 (const Byte* core, Address& addr)
{
	// disassemble 8080 opcode using z80 syntax

	Byte op = NEXTBYTE;
	cstr m;

	static const char cmd_call[] = { CALL,NN,0 };
	static const char cmd_jp[]	 = { JP,NN,0 };

	switch(op)
	{
	case 8:		// ex af,af'
	case 16:	// djnz
	case 24:	// jr
	case 32:	// jr nz
	case 40:	// jr z
	case 48:	// jr nc
	case 56:	// jr c
		return "nop";

	case 0xD9:	// exx
		return "ret";

	case 0xCB:
		m = cmd_jp; break;

	case 0xDD:
	case 0xED:
	case 0xFD:
		m = cmd_call; break;

	default:
		m = mnemo(op);
	}

	cstr s1 = word[uchar(m[0])]; if(m[1]==0) return s1;
	cstr s2 = expand_word(uchar(m[1]),core,addr);
	cstr s3 = expand_word(uchar(m[2]),core,addr);

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

cstr disassemble_z180 (const Byte* core, Address& addr)
{
	(void)core;
	(void)addr;
	TODO();
}

cstr disassemble (CpuID cpuid, const Byte* core, Address& addr, bool alternate_syntax)
{
	// disassemble one instruction and increment ip
	// tempstr or const string

	switch(cpuid)
	{
	case CpuDefault:
	case CpuZ80:	return disassemble_z80(core,addr);
	case CpuZ180:	return disassemble_z180(core,addr);
	case Cpu8080:	return (alternate_syntax?disassemble_asm8080:disassemble_i8080)(core,addr);
	}
	IERR();
}


static const char op8080[256][9] =
{
	"nop",		//	0x00 	1
	"2lxi b,",	//	0x01 	3	B <- byte 3, C <- byte 2
	"stax b",	//	0x02 	1	(BC) <- A
	"inx b",	//	0x03 	1	BC <- BC+1
	"inr b",	//	0x04 	1	B <- B+1
	"dcr b",	//	0x05 	1	B <- B-1
	"1mvi b,",	//	0x06 	2	B <- byte 2
	"rlc",		//	0x07 	1	A = A << 1; bit 0 = prev bit 7; CY = prev bit 7
	"db 08h",	//	0x08	1	nop (deprecated)
	"dad b",	//	0x09 	1	HL = HL + BC
	"ldax b",	//	0x0a 	1	A <- (BC)
	"dcx b",	//	0x0b 	1	BC = BC-1
	"inr c",	//	0x0c 	1	C <- C+1
	"dcr c",	//	0x0d 	1	C <-C-1
	"1mvi c,",	//	0x0e 	2	C <- byte 2
	"rrc",		//	0x0f 	1	A = A >> 1; bit 7 = prev bit 0; CY = prev bit 0
	"db 10h",	//	0x10	1	nop (deprecated)
	"2lxi d,",	//	0x11 	3	D <- byte 3, E <- byte 2
	"stax d",	//	0x12 	1	(DE) <- A
	"inx d",	//	0x13 	1	DE <- DE + 1
	"inr d",	//	0x14 	1	D <- D+1
	"dcr d",	//	0x15 	1	D <- D-1
	"1mvi d,",	//	0x16 	2	D <- byte 2
	"ral",		//	0x17 	1	A = A << 1; bit 0 = prev CY; CY = prev bit 7
	"db 18h",	//	0x18	1	nop (deprecated)
	"dad d",	//	0x19 	1	HL = HL + DE
	"ldax d",	//	0x1a 	1	A <- (DE)
	"dcx d",	//	0x1b 	1	DE = DE-1
	"inr e",	//	0x1c 	1	E <-E+1
	"dcr e",	//	0x1d 	1	E <- E-1
	"1mvi e,",	//	0x1e 	2	E <- byte 2
	"rar",		//	0x1f 	1	A = A >> 1; bit 7 = prev bit 7; CY = prev bit 0
	"db 20h",	//	0x20	1	nop (deprecated)
	"2lxi h,",	//	0x21 	3	H <- byte 3, L <- byte 2
	"2shld ",	//	0x22 	3	(NN) <-L; (NN+1)<-H
	"inx h",	//	0x23 	1	HL <- HL + 1
	"inr h",	//	0x24 	1	H <- H+1
	"dcr h",	//	0x25 	1	H <- H-1
	"1mvi h,",	//	0x26 	2	L <- byte 2
	"daa",		//	0x27 	1	decimal adjust prev. arith. op.
	"db 28h",	//	0x28	1	nop (deprecated)
	"dad h",	//	0x29 	1	HL = HL + HI
	"2lhld ",	//	0x2a 	3	L <- (NN); H<-(NN+1)
	"dcx h",	//	0x2b 	1	HL = HL-1
	"inr l",	//	0x2c 	1	L <- L+1
	"dcr l",	//	0x2d 	1	L <- L-1
	"1mvi l,",	//	0x2e 	2	L <- byte 2
	"cma",		//	0x2f 	1	A <- ~A
	"db 30h",	//	0x30	1	nop (deprecated)
	"2lxi sp,",	//	0x31 	3	SP.hi <- byte 3, SP.lo <- byte 2
	"2sta ",	//	0x32 	3	(NN) <- A
	"inx sp",	//	0x33 	1	SP = SP + 1
	"inr m",	//	0x34 	1	(HL) <- (HL)+1
	"dcr m",	//	0x35 	1	(HL) <- (HL)-1
	"1mvi m,",	//	0x36 	2	(HL) <- byte 2
	"stc",		//	0x37 	1	CY = 1
	"db 38h",	//	0x38	1	nop (deprecated)
	"dad sp",	//	0x39 	1	HL = HL + SP
	"2lda ",	//	0x3a 	3	A <- (NN)
	"dcx sp",	//	0x3b 	1	SP = SP-1
	"inr a",	//	0x3c 	1	A <- A+1
	"dcr a",	//	0x3d 	1	A <- A-1
	"1mvi a,",	//	0x3e 	2	A <- byte 2
	"cmc",		//	0x3f 	1	CY=!CY
	"mov b,b",	//	0x40 	1	B <- B
	"mov b,c",	//	0x41 	1	B <- C
	"mov b,d",	//	0x42 	1	B <- D
	"mov b,e",	//	0x43 	1	B <- E
	"mov b,h",	//	0x44 	1	B <- H
	"mov b,l",	//	0x45 	1	B <- L
	"mov b,m",	//	0x46 	1	B <- (HL)
	"mov b,a",	//	0x47 	1	B <- A
	"mov c,b",	//	0x48 	1	C <- B
	"mov c,c",	//	0x49 	1	C <- C
	"mov c,d",	//	0x4a 	1	C <- D
	"mov c,e",	//	0x4b 	1	C <- E
	"mov c,h",	//	0x4c 	1	C <- H
	"mov c,l",	//	0x4d 	1	C <- L
	"mov c,m",	//	0x4e 	1	C <- (HL)
	"mov c,a",	//	0x4f 	1	C <- A
	"mov d,b",	//	0x50 	1	D <- B
	"mov d,c",	//	0x51 	1	D <- C
	"mov d,d",	//	0x52 	1	D <- D
	"mov d,e",	//	0x53 	1	D <- E
	"mov d,h",	//	0x54 	1	D <- H
	"mov d,l",	//	0x55 	1	D <- L
	"mov d,m",	//	0x56 	1	D <- (HL)
	"mov d,a",	//	0x57 	1	D <- A
	"mov e,b",	//	0x58 	1	E <- B
	"mov e,c",	//	0x59 	1	E <- C
	"mov e,d",	//	0x5a 	1	E <- D
	"mov e,e",	//	0x5b 	1	E <- E
	"mov e,h",	//	0x5c 	1	E <- H
	"mov e,l",	//	0x5d 	1	E <- L
	"mov e,m",	//	0x5e 	1	E <- (HL)
	"mov e,a",	//	0x5f 	1	E <- A
	"mov h,b",	//	0x60 	1	H <- B
	"mov h,c",	//	0x61 	1	H <- C
	"mov h,d",	//	0x62 	1	H <- D
	"mov h,e",	//	0x63 	1	H <- E
	"mov h,h",	//	0x64 	1	H <- H
	"mov h,l",	//	0x65 	1	H <- L
	"mov h,m",	//	0x66 	1	H <- (HL)
	"mov h,a",	//	0x67 	1	H <- A
	"mov l,b",	//	0x68 	1	L <- B
	"mov l,c",	//	0x69 	1	L <- C
	"mov l,d",	//	0x6a 	1	L <- D
	"mov l,e",	//	0x6b 	1	L <- E
	"mov l,h",	//	0x6c 	1	L <- H
	"mov l,l",	//	0x6d 	1	L <- L
	"mov l,m",	//	0x6e 	1	L <- (HL)
	"mov l,a",	//	0x6f 	1	L <- A
	"mov m,b",	//	0x70 	1	(HL) <- B
	"mov m,c",	//	0x71 	1	(HL) <- C
	"mov m,d",	//	0x72 	1	(HL) <- D
	"mov m,e",	//	0x73 	1	(HL) <- E
	"mov m,h",	//	0x74 	1	(HL) <- H
	"mov m,l",	//	0x75 	1	(HL) <- L
	"hlt",		//	0x76 	1	halt cpu and wait for interrupt
	"mov m,a",	//	0x77 	1	(HL) <- C
	"mov a,b",	//	0x78 	1	A <- B
	"mov a,c",	//	0x79 	1	A <- C
	"mov a,d",	//	0x7a 	1	A <- D
	"mov a,e",	//	0x7b 	1	A <- E
	"mov a,h",	//	0x7c 	1	A <- H
	"mov a,l",	//	0x7d 	1	A <- L
	"mov a,m",	//	0x7e 	1	A <- (HL)
	"mov a,a",	//	0x7f 	1	A <- A
	"add b",	//	0x80 	1	A <- A + B
	"add c",	//	0x81 	1	A <- A + C
	"add d",	//	0x82 	1	A <- A + D
	"add e",	//	0x83 	1	A <- A + E
	"add h",	//	0x84 	1	A <- A + H
	"add l",	//	0x85 	1	A <- A + L
	"add m",	//	0x86 	1	A <- A + (HL)
	"add a",	//	0x87 	1	A <- A + A
	"adc b",	//	0x88 	1	A <- A + B + CY
	"adc c",	//	0x89 	1	A <- A + C + CY
	"adc d",	//	0x8a 	1	A <- A + D + CY
	"adc e",	//	0x8b 	1	A <- A + E + CY
	"adc h",	//	0x8c 	1	A <- A + H + CY
	"adc l",	//	0x8d 	1	A <- A + L + CY
	"adc m",	//	0x8e 	1	A <- A + (HL) + CY
	"adc a",	//	0x8f 	1	A <- A + A + CY
	"sub b",	//	0x90 	1	A <- A - B
	"sub c",	//	0x91 	1	A <- A - C
	"sub d",	//	0x92 	1	A <- A + D
	"sub e",	//	0x93 	1	A <- A - E
	"sub h",	//	0x94 	1	A <- A + H
	"sub l",	//	0x95 	1	A <- A - L
	"sub m",	//	0x96 	1	A <- A + (HL)
	"sub a",	//	0x97 	1	A <- A - A
	"sbb b",	//	0x98 	1	A <- A - B - CY
	"sbb c",	//	0x99 	1	A <- A - C - CY
	"sbb d",	//	0x9a 	1	A <- A - D - CY
	"sbb e",	//	0x9b 	1	A <- A - E - CY
	"sbb h",	//	0x9c 	1	A <- A - H - CY
	"sbb l",	//	0x9d 	1	A <- A - L - CY
	"sbb m",	//	0x9e 	1	A <- A - (HL) - CY
	"sbb a",	//	0x9f 	1	A <- A - A - CY
	"ana b",	//	0xa0 	1	A <- A & B
	"ana c",	//	0xa1 	1	A <- A & C
	"ana d",	//	0xa2 	1	A <- A & D
	"ana e",	//	0xa3 	1	A <- A & E
	"ana h",	//	0xa4 	1	A <- A & H
	"ana l",	//	0xa5 	1	A <- A & L
	"ana m",	//	0xa6 	1	A <- A & (HL)
	"ana a",	//	0xa7 	1	A <- A & A
	"xra b",	//	0xa8 	1	A <- A ^ B
	"xra c",	//	0xa9 	1	A <- A ^ C
	"xra d",	//	0xaa 	1	A <- A ^ D
	"xra e",	//	0xab 	1	A <- A ^ E
	"xra h",	//	0xac 	1	A <- A ^ H
	"xra l",	//	0xad 	1	A <- A ^ L
	"xra m",	//	0xae 	1	A <- A ^ (HL)
	"xra a",	//	0xaf 	1	A <- A ^ A
	"ora b",	//	0xb0 	1	A <- A | B
	"ora c",	//	0xb1 	1	A <- A | C
	"ora d",	//	0xb2 	1	A <- A | D
	"ora e",	//	0xb3 	1	A <- A | E
	"ora h",	//	0xb4 	1	A <- A | H
	"ora l",	//	0xb5 	1	A <- A | L
	"ora m",	//	0xb6 	1	A <- A | (HL)
	"ora a",	//	0xb7 	1	A <- A | A
	"cmp b",	//	0xb8 	1	A - B
	"cmp c",	//	0xb9 	1	A - C
	"cmp d",	//	0xba 	1	A - D
	"cmp e",	//	0xbb 	1	A - E
	"cmp h",	//	0xbc 	1	A - H
	"cmp l",	//	0xbd 	1	A - L
	"cmp m",	//	0xbe 	1	A - (HL)
	"cmp a",	//	0xbf 	1	A - A
	"rnz",		//	0xc0 	1	if NZ, RET
	"pop b",	//	0xc1 	1	C <- (sp); B <- (sp+1); sp <- sp+2
	"2jnz ",	//	0xc2 	3	if NZ, PC <- NN
	"2jmp ",	//	0xc3 	3	PC <= NN
	"2cnz ",	//	0xc4 	3	if NZ, CALL NN
	"push b",	//	0xc5 	1	(sp-2)<-C; (sp-1)<-B; sp <- sp - 2
	"1adi ",	//	0xc6 	2	A <- A + byte
	"rst 0",	//	0xc7 	1	CALL $0
	"rz",		//	0xc8 	1	if Z, RET
	"ret",		//	0xc9 	1	PC.lo <- (sp); PC.hi<-(sp+1); SP <- SP+2
	"2jz ",		//	0xca 	3	if Z, PC <- NN
	"2jp ",		//	0xcb	3	jp NN (deprecated, Z80: PFX)
	"2cz ",		//	0xcc 	3	if Z, CALL NN
	"2call ",	//	0xcd 	3	(SP-1)<-PC.hi;(SP-2)<-PC.lo;SP<-SP+2;PC=NN
	"1aci ",	//	0xce 	2	A <- A + data + CY
	"rst 1",	//	0xcf 	1	CALL $8
	"rnc",		//	0xd0 	1	if NCY, RET
	"pop d",	//	0xd1 	1	E <- (sp); D <- (sp+1); sp <- sp+2
	"2jnc ",	//	0xd2 	3	if NCY, PC<-NN
	"1out ",	//	0xd3 	2	output byte to peripheral ic
	"2cnc ",	//	0xd4 	3	if NCY, CALL NN
	"push d",	//	0xd5 	1	(sp-2)<-E; (sp-1)<-D; sp <- sp - 2
	"1sui ",	//	0xd6 	2	A <- A - data
	"rst 2",	//	0xd7 	1	CALL $10
	"rc",		//	0xd8 	1	if CY, RET
	"ret",		//	0xd9	1	RET (deprecated, Z80: EXX)
	"2jc ",		//	0xda 	3	if CY, PC<-NN
	"1in ",		//	0xdb 	2	input byte from peripheral ic
	"2cc ",		//	0xdc 	3	if CY, CALL NN
	"2call ",	//	0xdd	3	call NN (deprecated, Z80: PFX)
	"1sbi ",	//	0xde 	2	A <- A - data - CY
	"rst 3",	//	0xdf 	1	CALL $18
	"rpo",		//	0xe0 	1	if PO, RET
	"pop h",	//	0xe1 	1	L <- (sp); H <- (sp+1); sp <- sp+2
	"2jpo ",	//	0xe2 	3	if PO, PC <- NN
	"xthl",		//	0xe3 	1	L <-> (SP); H <-> (SP+1)
	"2cpo ",	//	0xe4 	3	if PO, CALL NN
	"push h",	//	0xe5 	1	(sp-2)<-L; (sp-1)<-H; sp <- sp - 2
	"1ani ",	//	0xe6 	2	A <- A & data
	"rst 4",	//	0xe7 	1	CALL $20
	"rpe",		//	0xe8 	1	if PE, RET
	"pchl",		//	0xe9 	1	PC.hi <- H; PC.lo <- L
	"2jpe ",	//	0xea 	3	if PE, PC <- NN
	"xchg",		//	0xeb 	1	H <-> D; L <-> E
	"2cpe ",	//	0xec 	3	if PE, CALL NN
	"2call ",	//	0xed	3	call NN (deprecated, Z80: PFX)
	"1xri ",	//	0xee 	2	A <- A ^ data
	"rst 5",	//	0xef 	1	CALL $28
	"rp",		//	0xf0 	1	if P, RET
	"pop psw",	//	0xf1 	1	flags <- (sp); A <- (sp+1); sp <- sp+2
	"2jp ",		//	0xf2 	3	if P=1 PC <- NN
	"di",		//	0xf3 	1	disable interrupts
	"2cp ",		//	0xf4 	3	if P, PC <- NN
	"push psw",	//	0xf5 	1	(sp-2)<-flags; (sp-1)<-A; sp <- sp - 2
	"1ori ",	//	0xf6 	2	A <- A | data
	"rst 6",	//	0xf7 	1	CALL $30
	"rm",		//	0xf8 	1	if M, RET
	"sphl",		//	0xf9 	1	SP=HL
	"2jm ",		//	0xfa 	3	if M, PC <- NN
	"ei",		//	0xfb 	1	enable interrupts
	"2cm ",		//	0xfc 	3	if M, CALL NN
	"2call ",	//	0xfd	3	call NN (deprecated, Z80: PFX)
	"1cpi ",	//	0xfe 	2	A - data
	"rst 7",	//	0xff 	1	CALL $38
};

cstr disassemble_asm8080 (const Byte* core, Address& addr)
{
	// i8080 with asm8080 syntax

	Byte op = peek(core,addr);

	cstr m = op8080[op];
	if (*m > '2') return m;		// no prefix '1' for N or '2' for NN

	uint nn = NEXTBYTE;
	if (*m=='1') return catstr(m+1, tostr(nn));

	nn += 256 * NEXTBYTE;
	return catstr(m+1, hexstr(nn, nn<0xA000?4:5), "h");
}





























