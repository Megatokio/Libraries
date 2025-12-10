// Copyright (c) 1996 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "z80_goodies.h"
//#include "z80_opcodes.h"


namespace z80
{

// clang-format off

// Opcode names in "z80_opcodes.h" partially conflict with enum Token names.
// => define the required opcodes here instead of include "z80_opcodes.h"
//	  to avoid danger of accidentially using a value from the wrong enum.
enum : Byte {
	EX_DE_HL = 0xEB,
	PFX_CB	 = 0xCB,
	PFX_IX	 = 0xDD,
	PFX_ED	 = 0xED,
	PFX_IY	 = 0xFD,
	SLL_xHL	 = 0x36, // after CD
};

// ----	Z80 opcode definitions ------------------------------------------------------

enum Token : uint8 { // Instruction:

	// Z80:
	NIX,	NOP,	LD,		INC,	DEC,	RLCA,	EX,
	ADD,	ADC,	SUB,	SBC,	AND,	XOR,	OR,		CP,  
	RLC,	RRC,	RL,		RR,		SLA,	SRA,	SLL,	SRL, 
	RRCA,	DJNZ,	RLA,	JR,		RRA,	DAA,	CPL,	HALT,
	SCF,	CCF,	IN,		OUT,	NEG,	RETN,	IM,		RETI,
	RRD,	RLD,	LDI,	CPI,	INI,	BIT,	RES,	SET,
	OUTI,	LDD,	CPD,	IND,	OUTD,	LDIR,	CPIR,	INIR,	OTIR,	LDDR,	CPDR,	INDR,	OTDR,
	RET,	POP,	JP,		CALL,	PUSH,	RST,	PFX,	EXX,
	DI,		EI,

	// Z180 new opcodes:
	IN0,	OUT0,	TST,	TSTIO,	MLT,	OTIM,	OTDM,	OTIMR,	OTDMR,	SLP,
	
	// Z80n new opcodes:
	SWAPNIB,MIRROR, TEST,	SETAE,
	BSLA,	BSRA,	BSRL,	BSRF,	BRLC,
	MUL,	OUTINB,	NEXTREG,PIXELDN,PIXELAD, 
	LDIX,	LDWS,	LDDX,	LDIRX,	LDPIRX, LDDRX,
	
	// asm8080 names: (new names, some names are already enumerated in Z80 section)
	LXI,	STAX,	INX,	INR,	DCR,	MVI,	DAD,	LDAX,
	DCX,	RAL,	RAR,	SHLD,	LHLD,	CMA,	STA,	STC,
	LDA,	CMC,	RNZ,	JNZ,	JMP,	CNZ,	ADI,	RZ,
	JZ,		CZ,		ACI,	RNC,	JNC,	CNC,	SUI,	RC,
	JC,		CC,		SBI,	RPO,	JPO,	XTHL,	CPO,	ANI,
	RPE,	PCHL,	JPE,	XCHG,	CPE,	XRI,	RP,		ORI,
	RM,		SPHL,	JM,		CM,		HLT,	MOV,	SBB,	ANA,
	XRA,	ORA,	CMP,

	// Register / Argument:
	BC,		DE,		HL,		SP,		AF,		AF2,
	B,		C,		D,		E,		H,		L,		XHL,	A,	 
	XBC,	XDE,	R,		I,		XC,		XSP,	PC,		F,
	N0,		N1,		N2,		N3,		N4,		N5,		N6,		N7,  N08,	N10,	N18,	N20,	N28,	N30,	N38, 
	Z,		NZ,				NC,		PO,		PE,		M,		P,
	PSW,	N,		NN,		NNr,	XNN,	XN,		DIS,	CB,		ED,
	IX,		IY,		XH,		XL,		YH,		YL,		XIX,	XIY,

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
	
	"swapnib",	"mirror",	"test",		"setae",
	"bsla",		"bsra",		"bsrl",		"bsrf",		"brlc",
	"mul",		"outinb",	"nextreg",	"pixeldn",	"pixelad",
	"ldix",		"ldws",		"lddx",		"ldirx",	"ldpirx",	"lddrx",
	
	"lxi",	"stax",	  "inx",  "inr",   "dcr",  "mvi",  "dad",  "ldax",	   "dcx",	   "ral",  "rar",  "shld", "lhld",
	"cma",	"sta",	  "stc",  "lda",   "cmc",  "rnz",  "jnz",  "jmp",	   "cnz",	   "adi",  "rz",   "jz",   "cz",
	"aci",	"rnc",	  "jnc",  "cnc",   "sui",  "rc",   "jc",   "cc",	   "sbi",	   "rpo",  "jpo",  "xthl", "cpo",
	"ani",	"rpe",	  "pchl", "jpe",   "xchg", "cpe",  "xri",  "rp",	   "ori",	   "rm",   "sphl", "jm",   "cm",
	"hlt",	"mov",	  "sbb",  "ana",   "xra",  "ora",  "cmp",

	"bc",	"de",	"hl",	"sp",	"af",	"af'",
	"b",	"c",	"d",	"e",	"h",	"l",	"(hl)",	"a",
	"(bc)",	"(de)", "r",	"i",	"(c)",	"(sp)", "pc",	"f",
	"0",	"1",	"2",	"3",	"4",	"5",	"6",	"7",	"8",	"16",   "24",	"32",	"40",   "48",   "56",
	"z",	"nz",	"nc",	"po",	"pe",	"m",	"p",	"psw",
	"N",	"NN",	"NN",	"(NN)",	"(N)",  "dis",
	"cb",	"ed",	"ix",	"iy",	"xh",	"xl",	"yh",	"yl",	"(ix+dis)", "(iy+dis)",
};

static_assert(NELEM(word) == NUM_WORD_DEFS, "");


struct alignas(4) Meno // Mnemonic
{
	Token a,b=NIX,c=NIX,d=NIX;
};

using MenoRow = Meno[32];
using MenoPage=const MenoRow*[8];


static constexpr MenoRow asm8080_cmd_00 = {
	{NOP},		{LXI,B,NN},	{STAX,B},	{INX,B},	{INR,B},	{DCR,B},	{MVI,B,N},	{RLC},
	{NOP},		{DAD,B},	{LDAX,B},	{DCX,B},	{INR,C},	{DCR,C},	{MVI,C,N},	{RRC},
	{NOP},		{LXI,D,NN},	{STAX,D},	{INX,D},	{INR,D},	{DCR,D},	{MVI,D,N},	{RAL},
	{NOP},		{DAD,D},	{LDAX,D},	{DCX,D},	{INR,E},	{DCR,E},	{MVI,E,N},	{RAR}};
static constexpr MenoRow asm8080_cmd_20 = {
	{NOP},		{LXI,H,NN},	{SHLD,NN},	{INX,H},	{INR,H},	{DCR,H},	{MVI,H,N},	{DAA},
	{NOP},		{DAD,H},	{LHLD,NN},	{DCX,H},	{INR,L},	{DCR,L},	{MVI,L,N},	{CMA},
	{NOP},		{LXI,SP,NN},{STA,NN},	{INX,SP},	{INR,M},	{DCR,M},	{MVI,M,N},	{STC},
	{NOP},		{DAD,SP},	{LDA,NN},	{DCX,SP},	{INR,A},	{DCR,A},	{MVI,A,N},	{CMC}};
static constexpr MenoRow asm8080_cmd_40 = {
	{MOV,B,B},	{MOV,B,C},	{MOV,B,D},	{MOV,B,E},	{MOV,B,H},	{MOV,B,L},	{MOV,B,M},	{MOV,B,A},
	{MOV,C,B},	{MOV,C,C},	{MOV,C,D},	{MOV,C,E},	{MOV,C,H},	{MOV,C,L},	{MOV,C,M},	{MOV,C,A},
	{MOV,D,B},	{MOV,D,C},	{MOV,D,D},	{MOV,D,E},	{MOV,D,H},	{MOV,D,L},	{MOV,D,M},	{MOV,D,A},
	{MOV,E,B},	{MOV,E,C},	{MOV,E,D},	{MOV,E,E},	{MOV,E,H},	{MOV,E,L},	{MOV,E,M},	{MOV,E,A}};
static constexpr MenoRow asm8080_cmd_60 = {
	{MOV,H,B},	{MOV,H,C},	{MOV,H,D},	{MOV,H,E},	{MOV,H,H},	{MOV,H,L},	{MOV,H,M},	{MOV,H,A},
	{MOV,L,B},	{MOV,L,C},	{MOV,L,D},	{MOV,L,E},	{MOV,L,H},	{MOV,L,L},	{MOV,L,M},	{MOV,L,A},
	{MOV,M,B},	{MOV,M,C},	{MOV,M,D},	{MOV,M,E},	{MOV,M,H},	{MOV,M,L},	{HLT},		{MOV,M,A},
	{MOV,A,B},	{MOV,A,C},	{MOV,A,D},	{MOV,A,E},	{MOV,A,H},	{MOV,A,L},	{MOV,A,M},	{MOV,A,A}};
static constexpr MenoRow asm8080_cmd_80 = {
	{ADD,B},	{ADD,C},	{ADD,D},	{ADD,E},	{ADD,H},	{ADD,L},	{ADD,M},	{ADD,A},  
	{ADC,B},	{ADC,C},	{ADC,D},	{ADC,E},	{ADC,H},	{ADC,L},	{ADC,M},	{ADC,A},  
	{SUB,B},	{SUB,C},	{SUB,D},	{SUB,E},	{SUB,H},	{SUB,L},	{SUB,M},	{SUB,A},  
	{SBB,B},	{SBB,C},	{SBB,D},	{SBB,E},	{SBB,H},	{SBB,L},	{SBB,M},	{SBB,A}}; 
static constexpr MenoRow asm8080_cmd_A0 = {
	{ANA,B},	{ANA,C},	{ANA,D},	{ANA,E},	{ANA,H},	{ANA,L},	{ANA,M},	{ANA,A},  
	{XRA,B},	{XRA,C},	{XRA,D},	{XRA,E},	{XRA,H},	{XRA,L},	{XRA,M},	{XRA,A},  
	{ORA,B},	{ORA,C},	{ORA,D},	{ORA,E},	{ORA,H},	{ORA,L},	{ORA, M},	{ORA,A},  
	{CMP,B},	{CMP,C},	{CMP,D},	{CMP,E},	{CMP,H},	{CMP,L},	{CMP, M},	{CMP,A}}; 
static constexpr MenoRow asm8080_cmd_C0 = {
	{RNZ},		{POP, B},	{JNZ, NN},	{JMP, NN},	{CNZ, NN},	{PUSH, B},	{ADI, N},	{RST, N0},
	{RZ},		{RET},		{JZ, NN},	{JMP, NN},  {CZ, NN},	{CALL, NN},	{ACI, N},	{RST, N1},
	{RNC},		{POP, D},	{JNC, NN},	{OUT, N},	{CNC, NN},	{PUSH, D},	{SUI, N},	{RST, N2},
	{RC},		{RET},		{JC, NN},	{IN, N},	{CC, NN},	{CALL, NN},	{SBI, N},	{RST, N3}};
static constexpr MenoRow asm8080_cmd_E0 = {
	{RPO},		{POP, H},	{JPO, NN},	{XTHL}, 	{CPO, NN},	{PUSH, H},	{ANI, N},	{RST, N4},
	{RPE},		{PCHL},		{JPE, NN},	{XCHG}, 	{CPE, NN},	{CALL, NN},	{XRI, N},	{RST, N5},
	{RP},		{POP, PSW},	{JP, NN},	{DI},		{CP, NN},	{PUSH, PSW},{ORI, N},	{RST, N6},
	{RM},		{SPHL},		{JM, NN},	{EI},		{CM, NN},	{CALL, NN},	{CPI, N},	{RST, N7}};

static constexpr MenoRow z80_cmd_00 = {
	{NOP},		{LD,BC,NN}, {LD,XBC,A},	{INC,BC},	{INC,B},	{DEC,B},	{LD,B,N},	{RLCA},
	{EX,AF,AF2},{ADD,HL,BC},{LD,A,XBC},	{DEC,BC},	{INC,C},	{DEC,C},	{LD,C,N},	{RRCA},
	{DJNZ,DIS},	{LD,DE,NN}, {LD,XDE,A},	{INC,DE},	{INC,D},	{DEC,D},	{LD,D,N},	{RLA},
	{JR,DIS},	{ADD,HL,DE},{LD,A,XDE},	{DEC,DE},	{INC,E},	{DEC,E},	{LD,E,N},	{RRA}};
static constexpr MenoRow z80_cmd_20 = {
	{JR,NZ,DIS},{LD,HL,NN}, {LD,XNN,HL},{INC,HL},	{INC,H},	{DEC,H},	{LD,H,N},	{DAA},
	{JR,Z,DIS}, {ADD,HL,HL},{LD,HL,XNN},{DEC,HL},	{INC,L},	{DEC,L},	{LD,L,N},	{CPL},
	{JR,NC,DIS},{LD,SP,NN}, {LD,XNN,A},	{INC,SP},	{INC,XHL},	{DEC,XHL},	{LD,XHL,N},	{SCF},
	{JR,C,DIS}, {ADD,HL,SP},{LD,A,XNN},	{DEC,SP},	{INC,A},	{DEC,A},	{LD,A,N},	{CCF}};
static constexpr MenoRow z80_cmd_40 = {
	{LD,B,B},	{LD,B,C},	{LD,B,D},	{LD,B,E},	{LD,B,H},	{LD,B,L},	{LD,B,XHL},	{LD,B,A},
	{LD,C,B},	{LD,C,C},	{LD,C,D},	{LD,C,E},	{LD,C,H},	{LD,C,L},	{LD,C,XHL},	{LD,C,A},
	{LD,D,B},	{LD,D,C},	{LD,D,D},	{LD,D,E},	{LD,D,H},	{LD,D,L},	{LD,D,XHL},	{LD,D,A},
	{LD,E,B},	{LD,E,C},	{LD,E,D},	{LD,E,E},	{LD,E,H},	{LD,E,L},	{LD,E,XHL},	{LD,E,A}};
static constexpr MenoRow z80_cmd_60 = {
	{LD,H,B},	{LD,H,C},	{LD,H,D},	{LD,H,E},	{LD,H,H},	{LD,H,L},	{LD,H,XHL},	{LD,H,A},
	{LD,L,B},	{LD,L,C},	{LD,L,D},	{LD,L,E},	{LD,L,H},	{LD,L,L},	{LD,L,XHL},	{LD,L,A},
	{LD,XHL,B},	{LD,XHL,C},	{LD,XHL,D},	{LD,XHL,E},	{LD,XHL,H},	{LD,XHL,L},	{HALT},		{LD,XHL,A},
	{LD,A,B},	{LD,A,C},	{LD,A,D},	{LD,A,E},	{LD,A,H},	{LD,A,L},	{LD,A,XHL},	{LD,A,A}};
static constexpr MenoRow z80_cmd_80 = {
	{ADD,A,B},	{ADD,A,C},	{ADD,A,D},	{ADD,A,E},	{ADD,A,H},	{ADD,A,L},	{ADD,A,XHL}, {ADD,A,A},
	{ADC,A,B},	{ADC,A,C},	{ADC,A,D},	{ADC,A,E},	{ADC,A,H},	{ADC,A,L},	{ADC,A,XHL}, {ADC,A,A},
	{SUB,A,B},	{SUB,A,C},	{SUB,A,D},	{SUB,A,E},	{SUB,A,H},	{SUB,A,L},	{SUB,A,XHL}, {SUB,A,A},
	{SBC,A,B},	{SBC,A,C},	{SBC,A,D},	{SBC,A,E},	{SBC,A,H},	{SBC,A,L},	{SBC,A,XHL}, {SBC,A,A}};
static constexpr MenoRow z80_cmd_80_alt = {
	{ADD,B},	{ADD,C},	{ADD,D},	{ADD,E},	{ADD,H},	{ADD,L},	{ADD,XHL},	{ADD,A},
	{ADC,B},	{ADC,C},	{ADC,D},	{ADC,E},	{ADC,H},	{ADC,L},	{ADC,XHL},	{ADC,A},
	{SUB,B},	{SUB,C},	{SUB,D},	{SUB,E},	{SUB,H},	{SUB,L},	{SUB,XHL},	{SUB,A},
	{SBC,B},	{SBC,C},	{SBC,D},	{SBC,E},	{SBC,H},	{SBC,L},	{SBC,XHL},	{SBC,A}};
static constexpr MenoRow z80_cmd_A0 = {
	{AND,A,B},	{AND,A,C},	{AND,A,D},	{AND,A,E},	{AND,A,H},	{AND,A,L},	{AND,A,XHL}, {AND,A,A},
	{XOR,A,B},	{XOR,A,C},	{XOR,A,D},	{XOR,A,E},	{XOR,A,H},	{XOR,A,L},	{XOR,A,XHL}, {XOR,A,A},
	{OR,A,B},	{OR,A,C},	{OR,A,D},	{OR,A,E},	{OR,A,H},	{OR,A,L},	{OR,A,XHL},	 {OR,A,A},
	{CP,A,B},	{CP,A,C},	{CP,A,D},	{CP,A,E},	{CP,A,H},	{CP,A,L},	{CP,A,XHL},	 {CP,A,A}};
static constexpr MenoRow z80_cmd_A0_alt = {
	{AND,B},	{AND,C},	{AND,D},	{AND,E},	{AND,H},	{AND,L},	{AND,XHL},	{AND,A},
	{XOR,B},	{XOR,C},	{XOR,D},	{XOR,E},	{XOR,H},	{XOR,L},	{XOR,XHL},	{XOR,A},
	{OR,B},		{OR,C},		{OR,D},		{OR,E},		{OR,H},		{OR,L},		{OR,XHL},	{OR,A},
	{CP,B},		{CP,C},		{CP,D},		{CP,E},		{CP,H},		{CP,L},		{CP,XHL},	{CP,A}};
static constexpr MenoRow z80_cmd_C0 = {
	{RET,NZ},	{POP,BC},	{JP,NZ,NN}, {JP,NN},	{CALL,NZ,NN}, {PUSH,BC}, {ADD,A,N}, {RST,N0},
	{RET,Z},	{RET},		{JP,Z,NN},  {PFX,CB},	{CALL,Z,NN},  {CALL,NN}, {ADC,A,N}, {RST,N08},
	{RET,NC},	{POP,DE},	{JP,NC,NN}, {OUT,XN,A},	{CALL,NC,NN}, {PUSH,DE}, {SUB,A,N}, {RST,N10},
	{RET,C},	{EXX},		{JP,C,NN},  {IN,A,XN},	{CALL,C,NN},  {PFX,IX},  {SBC,A,N}, {RST,N18}};
static constexpr MenoRow z80_cmd_C0_alt = {
	{RET,NZ},	{POP,BC},	{JP,NZ,NN}, {JP,NN},	{CALL,NZ,NN}, {PUSH,BC}, {ADD,N}, {RST,N0},
	{RET,Z},	{RET},		{JP,Z,NN},  {PFX,CB},	{CALL,Z,NN},  {CALL,NN}, {ADC,N}, {RST,N1},
	{RET,NC},	{POP,DE},	{JP,NC,NN}, {OUT,XN,A},	{CALL,NC,NN}, {PUSH,DE}, {SUB,N}, {RST,N2},
	{RET,C},	{EXX},		{JP,C,NN},  {IN,A,XN},	{CALL,C,NN},  {PFX,IX},  {SBC,N}, {RST,N3}};
static constexpr MenoRow z80_cmd_E0 = {
	{RET,PO},	{POP,HL},	{JP,PO,NN}, {EX,HL,XSP},{CALL,PO,NN}, {PUSH,HL}, {AND,A,N}, {RST,N20},
	{RET,PE},	{JP,HL},	{JP,PE,NN}, {EX,DE,HL}, {CALL,PE,NN}, {PFX,ED},  {XOR,A,N}, {RST,N28},
	{RET,P},	{POP,AF},	{JP,P,NN},  {DI},		{CALL,P,NN},  {PUSH,AF}, {OR,A,N},  {RST,N30},
	{RET,M},	{LD,SP,HL},	{JP,M,NN},  {EI},		{CALL,M,NN},  {PFX,IY},  {CP,A,N},  {RST,N38}};
static constexpr MenoRow z80_cmd_E0_alt = {
	{RET,PO},	{POP,HL},	{JP,PO,NN}, {EX,HL,XSP},{CALL,PO,NN}, {PUSH,HL}, {AND,N}, {RST,N4},
	{RET,PE},	{JP,HL},	{JP,PE,NN}, {EX,DE,HL}, {CALL,PE,NN}, {PFX,ED},  {XOR,N}, {RST,N5},
	{RET,P},	{POP,AF},	{JP,P,NN},  {DI},		{CALL,P,NN},  {PUSH,AF}, {OR,N},  {RST,N6},
	{RET,M},	{LD,SP,HL},	{JP,M,NN},  {EI},		{CALL,M,NN},  {PFX,IY},  {CP,N},  {RST,N7}};

static constexpr MenoRow i8080_cmd_00 = {
	{NOP},		{LD,BC,NN}, {LD,XBC,A},	{INC,BC},	{INC,B},	{DEC,B},	{LD,B,N},	{RLCA},
	{NOP},		{ADD,HL,BC},{LD,A,XBC},	{DEC,BC},	{INC,C},	{DEC,C},	{LD,C,N},	{RRCA},
	{NOP},		{LD,DE,NN}, {LD,XDE,A},	{INC,DE},	{INC,D},	{DEC,D},	{LD,D,N},	{RLA},
	{NOP},		{ADD,HL,DE},{LD,A,XDE},	{DEC,DE},	{INC,E},	{DEC,E},	{LD,E,N},	{RRA}};
static constexpr MenoRow i8080_cmd_20 = {
	{NOP},		{LD,HL,NN}, {LD,XNN,HL},{INC,HL},	{INC,H},	{DEC,H},	{LD,H,N},	{DAA},
	{NOP},		{ADD,HL,HL},{LD,HL,XNN},{DEC,HL},	{INC,L},	{DEC,L},	{LD,L,N},	{CPL},
	{NOP},		{LD,SP,NN}, {LD,XNN,A},	{INC,SP},	{INC,XHL},	{DEC,XHL},	{LD,XHL,N},	{SCF},
	{NOP},		{ADD,HL,SP},{LD,A,XNN},	{DEC,SP},	{INC,A},	{DEC,A},	{LD,A,N},	{CCF}};
static constexpr MenoRow i8080_cmd_C0 = {
	{RET,NZ},	{POP,BC},	{JP,NZ,NN}, {JP,NN},	{CALL,NZ,NN}, {PUSH,BC}, {ADD,A,N}, {RST,N0},
	{RET,Z},	{RET},		{JP,Z,NN},  {JP,NN},	{CALL,Z,NN},  {CALL,NN}, {ADC,A,N}, {RST,N08},
	{RET,NC},	{POP,DE},	{JP,NC,NN}, {OUT,XN,A},	{CALL,NC,NN}, {PUSH,DE}, {SUB,A,N}, {RST,N10},
	{RET,C},	{RET},		{JP,C,NN},  {IN,A,XN},	{CALL,C,NN},  {CALL,NN}, {SBC,A,N}, {RST,N18}};
static constexpr MenoRow i8080_cmd_E0 = {
	{RET,PO},	{POP,HL},	{JP,PO,NN}, {EX,HL,XSP},{CALL,PO,NN}, {PUSH,HL}, {AND,A,N}, {RST,N20},
	{RET,PE},	{JP,HL},	{JP,PE,NN}, {EX,DE,HL}, {CALL,PE,NN}, {CALL,NN}, {XOR,A,N}, {RST,N28},
	{RET,P},	{POP,AF},	{JP,P,NN},  {DI},		{CALL,P,NN},  {PUSH,AF}, {OR,A,N},  {RST,N30},
	{RET,M},	{LD,SP,HL},	{JP,M,NN},  {EI},		{CALL,M,NN},  {CALL,NN}, {CP,A,N},  {RST,N38}};

#define i8080_cmd_40 z80_cmd_40
#define i8080_cmd_60 z80_cmd_60
#define i8080_cmd_80 z80_cmd_80
#define i8080_cmd_A0 z80_cmd_A0

static constexpr MenoRow only_nops = {
	{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP}};
static constexpr MenoRow z80_cmd_ED40 = {
	{IN, B, XC}, {OUT, XC, B},	{SBC, HL, BC}, {LD, XNN, BC}, {NEG}, {RETN}, {IM, N0}, {LD, I, A},
	{IN, C, XC}, {OUT, XC, C},	{ADC, HL, BC}, {LD, BC, XNN}, {NEG}, {RETI}, {IM, N0}, {LD, R, A},
	{IN, D, XC}, {OUT, XC, D},	{SBC, HL, DE}, {LD, XNN, DE}, {NEG}, {RETI}, {IM, N1}, {LD, A, I},
	{IN, E, XC}, {OUT, XC, E},	{ADC, HL, DE}, {LD, DE, XNN}, {NEG}, {RETI}, {IM, N2}, {LD, A, R}};
static constexpr MenoRow z80_cmd_ED60 = {
	{IN, H, XC}, {OUT, XC, H},	{SBC, HL, HL}, {LD, XNN, HL}, {NEG}, {RETI}, {IM, N0}, {RRD},
	{IN, L, XC}, {OUT, XC, L},	{ADC, HL, HL}, {LD, HL, XNN}, {NEG}, {RETI}, {IM, N0}, {RLD},
	{IN, F, XC}, {OUT, XC, N0}, {SBC, HL, SP}, {LD, XNN, SP}, {NEG}, {RETI}, {IM, N1}, {NOP},
	{IN, A, XC}, {OUT, XC, A},	{ADC, HL, SP}, {LD, SP, XNN}, {NEG}, {RETI}, {IM, N2}, {NOP}};
static constexpr MenoRow z80_cmd_EDA0 = {
	{LDI},		 {CPI},		{INI},		{OUTI},		{NOP},	{NOP},	{NOP},	{NOP},
	{LDD},		 {CPD},		{IND},		{OUTD},		{NOP},	{NOP},	{NOP},	{NOP},
	{LDIR},		 {CPIR},	{INIR},		{OTIR},		{NOP},	{NOP},	{NOP},	{NOP},
	{LDDR},		 {CPDR},	{INDR},		{OTDR},		{NOP},	{NOP},	{NOP},	{NOP}};

#define z80_cmd_ED00 only_nops
#define z80_cmd_ED20 only_nops
#define z80_cmd_ED80 only_nops
#define z80_cmd_EDC0 only_nops
#define z80_cmd_EDE0 only_nops

static constexpr MenoRow z180_cmd_ED00 = {
	{IN0,B,XN},	{OUT0,XN,B},{NOP},		  {NOP},		{TST,B},	{NOP},	{NOP},	{NOP},
	{IN0,C,XN},	{OUT0,XN,C},{NOP},		  {NOP},		{TST,C},	{NOP},	{NOP},	{NOP},
	{IN0,D,XN},	{OUT0,XN,D},{NOP},		  {NOP},		{TST,D},	{NOP},	{NOP},	{NOP},
	{IN0,E,XN},	{OUT0,XN,E},{NOP},		  {NOP},		{TST,E},	{NOP},	{NOP},	{NOP}};
static constexpr MenoRow z180_cmd_ED20 = {
	{IN0,H,XN},	{OUT0,XN,H},{NOP},		  {NOP},		{TST,H},	{NOP},	{NOP},	{NOP},
	{IN0,L,XN},	{OUT0,XN,L},{NOP},		  {NOP},		{TST,L},	{NOP},	{NOP},	{NOP},
	{IN0,F,XN},	{NOP},		{NOP},		  {NOP},		{TST,XHL},	{NOP},	{NOP},	{NOP},
	{IN0,A,XN},	{OUT0,XN,A},{NOP},		  {NOP},		{TST,A},	{NOP},	{NOP},	{NOP}};
static constexpr MenoRow z180_cmd_ED40 = {
	{IN,B,XC},	{OUT,XC,B},	{SBC,HL,BC},  {LD,XNN,BC},	{NEG},		{RETN},	{IM,N0},{LD,I,A},
	{IN,C,XC},	{OUT,XC,C},	{ADC,HL,BC},  {LD,BC,XNN},	{MLT,BC},	{RETI},	{NOP},	{LD,R,A},
	{IN,D,XC},	{OUT,XC,D},	{SBC,HL,DE},  {LD,XNN,DE},	{NOP},		{NOP},	{IM,N1},{LD,A,I},
	{IN,E,XC},	{OUT,XC,E},	{ADC,HL,DE},  {LD,DE,XNN},	{MLT,DE},	{NOP},	{IM,N2},{LD,A,R}};
static constexpr MenoRow z180_cmd_ED60 = {
	{IN,H,XC},	{OUT,XC,H},	{SBC,HL,HL},  {LD,XNN,HL},	{TST,N},	{NOP},	{NOP},	{RRD},
	{IN,L,XC},	{OUT,XC,L},	{ADC,HL,HL},  {LD,HL,XNN},	{MLT,HL},	{NOP},	{NOP},	{RLD},
	{IN,F,XC},	{NOP},		{SBC,HL,SP},  {LD,XNN,SP},	{TSTIO,N},	{NOP},	{SLP},	{NOP},
	{IN,A,XC},	{OUT,XC,A},	{ADC,HL,SP},  {LD,SP,XNN},	{MLT,SP},	{NOP},	{NOP},	{NOP}};
static constexpr MenoRow z180_cmd_ED80 = {
	{NOP},		{NOP},		{NOP},		{OTIM},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{OTDM},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{OTIMR}, 	{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		{NOP},		{OTDMR}, 	{NOP},		{NOP},		{NOP},		{NOP}};

#define z180_cmd_EDA0 z80_cmd_EDA0
#define z180_cmd_EDC0 z80_cmd_EDC0
#define z180_cmd_EDE0 z80_cmd_EDE0

static constexpr MenoRow z80n_cmd_ED20 = {
	{NOP},		{NOP},		  {NOP},		{SWAPNIB},	{MIRROR,A}, {NOP},		{NOP},		{TEST,N},
	{BSLA,DE,B},{BSRA,DE,B},  {BSRL,DE,B},	{BSRF,DE,B},{BRLC,DE,B},{NOP},		{NOP},		{NOP},
	{MUL,D,E},	{ADD,HL,A},	  {ADD,DE,A},	{ADD,BC,A},	{ADD,HL,NN},{ADD,DE,NN},{ADD,BC,NN},{NOP},
	{NOP},		{NOP},		  {NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP}};
static constexpr MenoRow z80n_cmd_ED80 = {
	{NOP},		{NOP},		  {NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP},
	{NOP},		{NOP},		  {PUSH,NN},	{NOP},		{NOP},		{NOP},		{NOP},		{NOP},
	{OUTINB},	{NEXTREG,N,N},{NEXTREG,N,A},{PIXELDN},	{PIXELAD},	{SETAE},	{NOP},		{NOP},
	{JP,XC},	{NOP},		  {NOP},		{NOP},		{NOP},		{NOP},		{NOP},		{NOP}};
static constexpr MenoRow z80n_cmd_EDA0 = {
	{LDI},		{CPI},		  {INI},		{OUTI},		{LDIX}, 	{LDWS},		{NOP},		{NOP},
	{LDD},		{CPD},		  {IND},		{OUTD},		{LDDX}, 	{NOP},		{NOP},		{NOP},
	{LDIR},		{CPIR},		  {INIR},		{OTIR},		{LDIRX},	{NOP},		{NOP},		{LDPIRX},
	{LDDR},		{CPDR},		  {INDR},		{OTDR},		{LDDRX},	{NOP},		{NOP},		{NOP}};

#define z80n_cmd_ED00 z80_cmd_ED00
#define z80n_cmd_ED40 z80_cmd_ED40
#define z80n_cmd_ED60 z80_cmd_ED60
#define z80n_cmd_EDC0 z80_cmd_EDC0
#define z80n_cmd_EDE0 z80_cmd_EDE0

static constexpr MenoPage asm8080_cmd = {
	&asm8080_cmd_00, &asm8080_cmd_20, &asm8080_cmd_40, &asm8080_cmd_60,
	&asm8080_cmd_80, &asm8080_cmd_A0, &asm8080_cmd_C0, &asm8080_cmd_E0};
static constexpr MenoPage i8080_cmd = {
	&i8080_cmd_00,	&i8080_cmd_20,	&i8080_cmd_40,	&i8080_cmd_60,
	&i8080_cmd_80,	&i8080_cmd_A0,	&i8080_cmd_C0,	&i8080_cmd_E0};
static constexpr MenoPage z80_cmd = {
	&z80_cmd_00,	&z80_cmd_20,	&z80_cmd_40,	&z80_cmd_60,
	&z80_cmd_80,	&z80_cmd_A0,	&z80_cmd_C0,	&z80_cmd_E0};
static constexpr MenoPage z80_cmd_alt = {
	&z80_cmd_00,	&z80_cmd_20,	&z80_cmd_40,	&z80_cmd_60,
	&z80_cmd_80_alt,&z80_cmd_A0_alt,&z80_cmd_C0_alt,&z80_cmd_E0_alt};
static constexpr MenoPage z80_cmd_ED = {
	&z80_cmd_ED00,	&z80_cmd_ED20,	&z80_cmd_ED40,	&z80_cmd_ED60,
	&z80_cmd_ED80,	&z80_cmd_EDA0,	&z80_cmd_EDC0,	&z80_cmd_EDE0};
static constexpr MenoPage z180_cmd_ED = {
	&z180_cmd_ED00, &z180_cmd_ED20,	&z180_cmd_ED40, &z180_cmd_ED60,
	&z180_cmd_ED80,	&z180_cmd_EDA0,	&z180_cmd_EDC0,	&z180_cmd_EDE0};
static constexpr MenoPage z80n_cmd_ED = {
	&z80n_cmd_ED00,	&z80n_cmd_ED20,	&z80n_cmd_ED40,	&z80n_cmd_ED60,
	&z80n_cmd_ED80,	&z80n_cmd_EDA0,	&z80n_cmd_EDC0,	&z80n_cmd_EDE0};


// ----	Z80 Opcode Validity ------------------------------------------------------

static constexpr uint8 i8080_legal[32] = {
	// deprecated opcode aliases are flagged as "illegal":
	0xff,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f,0x7f, // NOPs
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0b11111111,0b11101111,	// JP
	0b11111111,0b10111011,	// RET, CALL
	0b11111111,0b11111011,	// CALL
	0b11111111,0b11111011};	// CALL

static constexpr uint8 z80n_legal_ED[32] = {
	// all supported opcodes, incl. 'OUT (c),0" are reported as "legal":
	0b00000000, // ED00,		ED01,		ED02,		ED03,		ED04,		ED05,		ED06,		ED07,
	0b00000000, // ED08,		ED09,		ED0A,		ED0B,		ED0C,		ED0D,		ED0E,		ED0F,
	0b00000000, // ED10,		ED11,		ED12,		ED13,		ED14,		ED15,		ED16,		ED17,
	0b00000000, // ED18,		ED19,		ED1A,		ED1B,		ED1C,		ED1D,		ED1E,		ED1F,
	0b00011001, // ED20,		ED21,		ED22,		SWAPNIB,	MIRROR_A,	ED25,		ED26,		TEST_N,
	0b11111000, // BSLA_DE_B,	BSRA_DE_B,	BSRL_DE_B,	BSRF_DE_B,	BRLC_DE_B,	ED2D,		ED2E,		ED2F,
	0b11111110, // MUL_D_E,		ADD_HL_A,	ADD_DE_A,	ADD_BC_A,	ADD_HL_NN,	ADD_DE_NN,	ADD_BC_NN,	ED37,
	0b00000000, // ED38,		ED39,		ED3A,		ED3B,		ED3C,		ED3D,		ED3E,		ED3F,

	0b11111111, // IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	0b11110101, // IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	NEG**,		RETI,		IM0**,		LD_R_A,
	0b11110011, // IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	NEG**,		RETI**,		IM_1,		LD_A_I,
	0b11110011, // IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	NEG**,		RETI**,		IM_2,		LD_A_R,
	0b11110001, // IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	NEG**,		RETI**,		IM0**,		RRD,
	0b11110001, // IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	NEG**,		RETI**,		IM0**,		RLD,
	0b11110000, // IN_F_xC, 	OUT_xC_0,	SBC_HL_SP,	LD_xNN_SP,	NEG**,		RETI**,		IM1**,		ED77,
	0b11110000, // IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	NEG**,		RETI**,		IM2**,		ED7F,

	0b00000000, // ED80,		ED81,		ED82,		ED83,		ED84,		ED85,		ED86,		ED87,
	0b00100000, // ED88,		ED89,		PUSH_NN,	ED8B,		ED8C,		ED8D,		ED8E,		ED8F,
	0b11111100, // OUTINB,		NEXTREG_N_N,NEXTREG_N_A,PIXELDN,	PIXELAD,	SETAE,		ED96,		ED97,
	0b10000000, // JP_xC,		ED99,		ED9A,		ED9B,		ED9C,		ED9D,		ED9E,		ED9F,
	0b11111100, // LDI, 		CPI,		INI,		OUTI,		LDIX,		LDWS,		EDA6,		EDA7,
	0b11111000, // LDD, 		CPD,		IND,		OUTD,		LDDX,		EDAD,		EDAE,		EDAF,
	0b11111001, // LDIR,		CPIR,		INIR,		OTIR,		LDIRX,		EDB5,		EDB6,		LDPIRX,
	0b11111000, // LDDR,		CPDR,		INDR,		OTDR,		LDDRX,		EDBD,		EDBE,		EDBF,
	0,0,0,0,0,0,0,0,
};

static constexpr uint8 z180_legal_ED[32] = {
	// all executable opcodes are flagged as "legal":
	0b11001000, // IN0_B_xN, 	OUT0_xN_B, 	ED02,		ED03,		TST_B,		ED05,		ED06,		ED07,
	0b11001000, // IN0_C_xN, 	OUT0_xN_C, 	ED0A,		ED0B,		TST_C,		ED0D,		ED0E,		ED0F,
	0b11001000, // IN0_D_xN, 	OUT0_xN_D, 	ED12,		ED13,		TST_D,		ED15,		ED16,		ED17,
	0b11001000, // IN0_E_xN, 	OUT0_xN_E, 	ED1A,		ED1B,		TST_E,		ED1D,		ED1E,		ED1F,
	0b11001000, // IN0_H_xN, 	OUT0_xN_H, 	ED22,		ED23,		TST_H,		ED25,		ED26,		ED27,
	0b11001000, // IN0_L_xN, 	OUT0_xN_L, 	ED2A,		ED2B,		TST_L,		ED2D,		ED2E,		ED2F,
	0b10001000, // IN0_F_xN, 	ED31,		ED32,		ED33,		TST_xHL,	ED35,		ED36,		ED37,
	0b11001000, // IN0_A_xN, 	OUT0_xN_A,	ED3A,		ED3B,		TST_A,		ED3D,		ED3E,		ED3F,
	
	0b11111111, // IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	0b11111101, // IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	MLT_BC,		RETI,		ED4E,		LD_R_A,
	0b11110011, // IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	ED54,		ED55,		IM_1,		LD_A_I,
	0b11111011, // IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	MLT_DE,		ED5D,		IM_2,		LD_A_R,
	0b11111001, // IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	TST_N,		ED65,		ED66,		RRD,
	0b11111001, // IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	MLT_HL,		ED6D,		ED6E,		RLD,
	0b10111010, // IN_F_xC, 	ED71,		SBC_HL_SP,	LD_xNN_SP,	TSTIO,		ED75,		SLP,		ED77,
	0b11111000, // IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	MLT_SP,		ED7D,		ED7E,		ED7F,

	0b00010000, // ED80,		ED81,		ED82,		OTIM,		ED84,		ED85,		ED86,		ED87,
	0b00010000, // ED88,		ED89,		ED8A,		OTDM,		ED8C,		ED8D,		ED8E,		ED8F,
	0b00010000, // ED90,		ED91,		ED92,		OTIMR,		ED94,		ED95,		ED96,		ED97,
	0b00010000, // ED98,		ED99,		ED9A,		OTDMR,		ED9C,		ED9D,		ED9E,		ED9F,
	0b11110000, // LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	0b11110000, // LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	0b11110000, // LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	0b11110000, // LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
	0,0,0,0,0,0,0,0,
};

static constexpr uint8 z80_legal_ED[32] = {
	// undocumented opcode aliases are flagged as "illegal", all others, incl. 'out (c),0' are 'legal':
	0,0,0,0,0,0,0,0,
	0b11111111, // IN_B_xC, 	OUT_xC_B,	SBC_HL_BC,	LD_xNN_BC,	NEG,		RETN,		IM_0,		LD_I_A,
	0b11110101, // IN_C_xC, 	OUT_xC_C,	ADC_HL_BC,	LD_BC_xNN,	NEG**,		RETI,		IM0**,		LD_R_A,
	0b11110011, // IN_D_xC, 	OUT_xC_D,	SBC_HL_DE,	LD_xNN_DE,	NEG**,		RETI**,		IM_1,		LD_A_I,
	0b11110011, // IN_E_xC, 	OUT_xC_E,	ADC_HL_DE,	LD_DE_xNN,	NEG**,		RETI**,		IM_2,		LD_A_R,
	0b11110001, // IN_H_xC, 	OUT_xC_H,	SBC_HL_HL,	LD_xNN_HL,	NEG**,		RETI**,		IM0**,		RRD,
	0b11110001, // IN_L_xC, 	OUT_xC_L,	ADC_HL_HL,	LD_HL_xNN,	NEG**,		RETI**,		IM0**,		RLD,
	0b11110000, // IN_F_xC, 	OUT_xC_0,	SBC_HL_SP,	LD_xNN_SP,	NEG**,		RETI**,		IM1**,		ED77,
	0b11110000, // IN_A_xC, 	OUT_xC_A,	ADC_HL_SP,	LD_SP_xNN,	NEG**,		RETI**,		IM2**,		ED7F,

	0b00000000,	// ED80,		ED81,		ED82,		ED83,		ED84,		ED85,		ED86,		ED87,
	0b00000000,	// ED88,		ED89,		ED8A,		ED8B,		ED8C,		ED8D,		ED8E,		ED8F,
	0b00000000,	// ED90,		ED91,		ED92,		ED93,		ED94,		ED95,		ED96,		ED97,
	0b00000000,	// ED98,		ED99,		ED9A,		ED9B,		ED9C,		ED9D,		ED9E,		ED9F,
	0b11110000,	// LDI, 		CPI,		INI,		OUTI,		EDA4,		EDA5,		EDA6,		EDA7,
	0b11110000,	// LDD, 		CPD,		IND,		OUTD,		EDAC,		EDAD,		EDAE,		EDAF,
	0b11110000,	// LDIR,		CPIR,		INIR,		OTIR,		EDB4,		EDB5,		EDB6,		EDB7,
	0b11110000,	// LDDR,		CPDR,		INDR,		OTDR,		EDBC,		EDBD,		EDBE,		EDBF,
	0,0,0,0,0,0,0,0,
};

static constexpr uint8 z80_legalIX[32] = {
	// legal:	instructions which use hl, (hl), xh or xl 
	// illegal:	all others incl. prefix 0xCB:
	0b00000000, // NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	0b01000000, // EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	0b00000000, // DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	0b01000000, // JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	0b01111110, // JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	0b01111110, // JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	0b00001110, // JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	0b01000000, // JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,

	0b00001110, // LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	0b00001110, // LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	0b00001110, // LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	0b00001110, // LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	0b11111111, // LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	0b11111111, // LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	0b11111101, // LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	0b00001110, // LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,

	0b00001110, // ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	0b00001110, // ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	0b00001110, // SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	0b00001110, // SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	0b00001110, // AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	0b00001110, // XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	0b00001110, // OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	0b00001110, // CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,

	0b00000000, // RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	0b00000000, // RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	0b00000000, // RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	0b00000000, // RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	0b01010100, // RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	0b01000000, // RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	0b00000000, // RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	0b01000000, // RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
};

static constexpr uint8 z180_legalIX[32] = {
	// legal:	instructions which use hl or (hl)
	// illegal:	all others incl. prefix 0xCB:
	0b00000000, // NOP, 		LD_BC_NN,	LD_xBC_A,	INC_BC,		INC_B,		DEC_B,		LD_B_N,		RLCA,
	0b01000000, // EX_AF_AF,	ADD_HL_BC,	LD_A_xBC,	DEC_BC,		INC_C,		DEC_C,		LD_C_N,		RRCA,
	0b00000000, // DJNZ,		LD_DE_NN,	LD_xDE_A,	INC_DE,		INC_D,		DEC_D,		LD_D_N,		RLA,
	0b01000000, // JR,  		ADD_HL_DE,	LD_A_xDE,	DEC_DE,		INC_E,		DEC_E,		LD_E_N,		RRA,
	0b01110000, // JR_NZ,		LD_HL_NN,	LD_xNN_HL,	INC_HL,		INC_H,		DEC_H,		LD_H_N,		DAA,
	0b01110000, // JR_Z,		ADD_HL_HL,	LD_HL_xNN,	DEC_HL,		INC_L,		DEC_L,		LD_L_N,		CPL,
	0b00001110, // JR_NC,		LD_SP_NN,	LD_xNN_A,	INC_SP,		INC_xHL,	DEC_xHL,	LD_xHL_N,	SCF,
	0b01000000, // JR_C,		ADD_HL_SP,	LD_A_xNN,	DEC_SP,		INC_A,		DEC_A,		LD_A_N,		CCF,

	0b00000010, // LD_B_B,		LD_B_C,		LD_B_D,		LD_B_E,		LD_B_H,		LD_B_L,		LD_B_xHL,	LD_B_A,
	0b00000010, // LD_C_B,		LD_C_C,		LD_C_D,		LD_C_E,		LD_C_H,		LD_C_L,		LD_C_xHL,	LD_C_A,
	0b00000010, // LD_D_B,		LD_D_C,		LD_D_D,		LD_D_E,		LD_D_H,		LD_D_L,		LD_D_xHL,	LD_D_A,
	0b00000010, // LD_E_B,		LD_E_C,		LD_E_D,		LD_E_E,		LD_E_H,		LD_E_L,		LD_E_xHL,	LD_E_A,
	0b00000010, // LD_H_B,		LD_H_C,		LD_H_D,		LD_H_E,		LD_H_H,		LD_H_L,		LD_H_xHL,	LD_H_A,
	0b00000010, // LD_L_B,		LD_L_C,		LD_L_D,		LD_L_E,		LD_L_H,		LD_L_L,		LD_L_xHL,	LD_L_A,
	0b11111101, // LD_xHL_B,	LD_xHL_C,	LD_xHL_D,	LD_xHL_E,	LD_xHL_H,	LD_xHL_L,	HALT,		LD_xHL_A,
	0b00000010, // LD_A_B,		LD_A_C,		LD_A_D,		LD_A_E,		LD_A_H,		LD_A_L,		LD_A_xHL,	LD_A_A,

	0b00000010, // ADD_B,		ADD_C,		ADD_D,		ADD_E,		ADD_H,		ADD_L,		ADD_xHL,	ADD_A,
	0b00000010, // ADC_B,		ADC_C,		ADC_D,		ADC_E,		ADC_H,		ADC_L,		ADC_xHL,	ADC_A,
	0b00000010, // SUB_B,		SUB_C,		SUB_D,		SUB_E,		SUB_H,		SUB_L,		SUB_xHL,	SUB_A,
	0b00000010, // SBC_B,		SBC_C,		SBC_D,		SBC_E,		SBC_H,		SBC_L,		SBC_xHL,	SBC_A,
	0b00000010, // AND_B,		AND_C,		AND_D,		AND_E,		AND_H,		AND_L,		AND_xHL,	AND_A,
	0b00000010, // XOR_B,		XOR_C,		XOR_D,		XOR_E,		XOR_H,		XOR_L,		XOR_xHL,	XOR_A,
	0b00000010, // OR_B,		OR_C,		OR_D,		OR_E,		OR_H,		OR_L,		OR_xHL,		OR_A,
	0b00000010, // CP_B,		CP_C,		CP_D,		CP_E,		CP_H,		CP_L,		CP_xHL,		CP_A,

	0b00000000, // RET_NZ,		POP_BC,		JP_NZ,		JP,			CALL_NZ,	PUSH_BC,	ADD_N,		RST00,
	0b00000000, // RET_Z,		RET,		JP_Z,		PFX_CB,		CALL_Z,		CALL,		ADC_N,		RST08,
	0b00000000, // RET_NC,		POP_DE,		JP_NC,		OUTA,		CALL_NC,	PUSH_DE,	SUB_N,		RST10,
	0b00000000, // RET_C,		EXX,		JP_C,		INA,		CALL_C,		PFX_IX,		SBC_N,		RST18,
	0b01010100, // RET_PO,		POP_HL,		JP_PO,		EX_HL_xSP,	CALL_PO,	PUSH_HL,	AND_N,		RST20,
	0b01000000, // RET_PE,		JP_HL,		JP_PE,		EX_DE_HL,	CALL_PE,	PFX_ED,		XOR_N,		RST28,
	0b00000000, // RET_P,		POP_AF,		JP_P,		DI,			CALL_P,		PUSH_AF,	OR_N,		RST30,
	0b01000000, // RET_M,		LD_SP_HL,	JP_M,		EI,			CALL_M,		PFX_IY,		CP_N,		RST38
};


//	==========================================================================================================
//	=	
//	=
//	=
//	==========================================================================================================


static const Meno& asm8080_meno(Byte op) { return (*asm8080_cmd[op>>5])[op&31]; }
static const Meno& i8080_meno(Byte op)   { return (*i8080_cmd[op>>5])[op&31]; }
static const Meno& z80_meno(Byte op)     { return (*z80_cmd[op>>5])[op&31]; }
static const Meno& z80_meno_alt(Byte op) { return (*z80_cmd_alt[op>>5])[op&31]; }

static const Meno& z80_menoED(CpuID cpuid, Byte op)
{
	if (cpuid == CpuZ180) return (*z180_cmd_ED[op>>5])[op&31];
	if (cpuid == CpuZ80n) return (*z80n_cmd_ED[op>>5])[op&31];
	else				  return (*z80_cmd_ED[op>>5])[op&31];
}

static constexpr Token regs[]  = {B, C, D, E,  H,  L, XHL, A, HL};
static constexpr Token xregs[] = {B, C, D, E, XH, XL, XIX, A, IX};
static constexpr Token yregs[] = {B, C, D, E, YH, YL, XIY, A, IY};

static Meno z80_menoCB(Byte op, const Token* reg = regs)
{
	static constexpr Token cmds[] = {RLC, RRC, RL, RR, SLA, SRA, SLL, SRL};
	static constexpr Token bits[] = {N0,N1,N2,N3,N4,N5,N6,N7};
	uint8 r = op & 0x07;
	uint8 n = (op >> 3) & 0x07;

	switch (op >> 6)
	{
	default: 
	case 0: return Meno{cmds[n],reg[r]};
	case 1: return Meno{BIT,bits[n],reg[r]};
	case 2: return Meno{RES,bits[n],reg[r]};
	case 3: return Meno{SET,bits[n],reg[r]};
	}
}

static Meno z80_menoXY(Byte op1, Byte op2)
{
	Meno m = z80_meno(op2);
	if (op2 == EX_DE_HL) return m;  // 'ex hl,de' always uses HL

	const Token* xyregs = op1 == PFX_IX ? xregs : yregs;
	
	if (m.b == HL) { m.b = xyregs[8]; if (m.c != HL) return m; }	// IX or IY
	if (m.c == HL) { m.c = xyregs[8]; return m; }					// IX or IY 

	if (m.b == XHL) { m.b = xyregs[6]; return m; } // XIX or XIY 
	if (m.c == XHL) { m.c = xyregs[6]; return m; } // XIX or XIY

	if (m.b == H) m.b = xyregs[4];	// XH or YH
	if (m.b == L) m.b = xyregs[5];	// XL or YL
	if (m.c == H) m.c = xyregs[4];	// XH or YH
	if (m.c == L) m.c = xyregs[5];	// XL or YL
	return m;
}

static const Meno z80_menoXYCB(CpuID cpuid, Byte op1, Byte op4) 
{
	// return mnenonic descriptor for IXCB / IYCB instruction
	//
	// illegals: most cpuids use ixbcr2 style:
	//		--ixbcr2:  "set 0,(ix+dis),b" .. "set 0,(ix+dis),h"	 set (ix+i) plus b,c,d,e,h,l,a
	//		--ixbcxh:  "set 0,b" .. "set 0,xh"					 set b,c,d,e,xh,xl,(ix+i),a

	const Token* xyregs = op1 == PFX_IX ? xregs : yregs;
	Meno m = z80_menoCB(op4,xyregs);
	uint8 r = op4 & 0x07;

	if (r == 6 || cpuid == CpuZ80_ixcbxh) return m;			 // legal opcode or --ixcbxh mode	
	else if (op4 < 0x40) return Meno{m.a,xyregs[6],regs[r]}; // --ixbcr2 and all other modes (shift ops)
	else return Meno{m.a,m.b,xyregs[6],regs[r]};			 // --ixbcr2 and all other modes (bit ops)
}

static cstr tostr(uint8 idf, FnPeek peek, Address& addr)
{
	// expand token, reading opcode parameters via *ip++ if needed

	switch (idf)
	{
	uint n, nn;
	case DIS: n  = peek(addr++); nn = uint16(addr + int8(n));
			  if (addr == 2) return usingstr("$%+i", int8(n) + 2);   // "$+123"
			  else return usingstr("$%+i ; $%04X", int8(n) + 2, nn); // "$+123 ; $FFFF"
	case NN:  n = peek(addr++); nn = n + 256 * peek(addr++); return usingstr("$%04X", nn);	 // "$FFFF"
	case NNr: n = peek(addr++); nn = 256 * n + peek(addr++); return usingstr("$%04X", nn);	 // "$FFFF"
	case XNN: n = peek(addr++); nn = n + 256 * peek(addr++); return usingstr("($%04X)", nn); // "($FFFF)"
	case N:   return usingstr("%u", peek(addr++));			  // "255"
	case XN:  return usingstr("($%02X)", peek(addr++));		  // "($FF)"
	case XIX: return usingstr("(ix%+i)", int8(peek(addr++))); // "(ix+123)"
	case XIY: return usingstr("(iy%+i)", int8(peek(addr++))); // "(iy+123)"
	default:  return word[idf];
	}
}

cstr tostr(const Meno& m, FnPeek peek, Address& addr)
{
	if (m.b == NIX) return word[m.a];
	cstr b = tostr(m.b,peek,addr);
	if (m.c == NIX) return catstr(word[m.a], " ", b);
	cstr c = tostr(m.c,peek,addr);
	if (m.d == NIX) return catstr(word[m.a]," ", b,",", c); 
	return catstr(word[m.a]," ", b,",", c,",", word[m.d]); 
}
 
cstr disassemble(CpuID cpuid, FnPeek peek, Address& addr, bool asm8080) 
{
	// disassemble one instruction and increment addr

	Byte op = peek(addr++);
	if (cpuid == Cpu8080) return tostr(asm8080?asm8080_meno(op):i8080_meno(op), peek, addr);

	switch (op)
	{
	default:	return tostr(z80_meno(op), peek, addr);
	case 0xcb:	op = peek(addr++);
				return tostr(z80_menoCB(op), peek, addr);
	case 0xed:	op = peek(addr++);
				return tostr(z80_menoED(cpuid, op), peek, addr);
	case 0xdd:
	case 0xfd:	Byte op2 = peek(addr++);
				if (op2 != 0xcb) return tostr(z80_menoXY(op, op2), peek, addr);
				Address end = addr+2;
				Byte	op4 = peek(addr + 1);
				cstr    s   = tostr(z80_menoXYCB(cpuid, op, op4), peek, addr);
				addr		= end;
				return s;
	}
}


static inline bool peek_bit(uint8 op, const uint8 legal[32]) { return legal[op >> 3] & (0x80 >> (op & 7)); }
static inline Byte peek(const Byte* core, Address a) { return core[a]; }

bool opcode_is_legal(CpuID cpuid, const Byte* core, Address a) noexcept
{
	Byte op = peek(core,a);
	if(cpuid == Cpu8080) return peek_bit(op,i8080_legal);
	switch(op)
	{
	case PFX_CB:
		if (cpuid != CpuZ180) return true;	// all opcodes, incl. SLL are reported as "legal"
		else return (op >> 3) != 6;			// Z180: SLL is reported as "illegal"
	case PFX_ED:
		if(cpuid==CpuZ80n) return peek_bit(op,z80n_legal_ED); 
		if(cpuid==CpuZ180) return peek_bit(op,z180_legal_ED); 
		else			   return peek_bit(op,z80_legal_ED); 
	case PFX_IX:
	case PFX_IY:
		if(peek(core,a+1) != PFX_CB)
			return peek_bit(op, (cpuid==CpuZ180 ? z180_legalIX : z80_legalIX));
		op = peek(core,a+3);
		switch(cpuid)
		{
		case CpuZ180:		return (op & 7) == 6 && op != SLL_xHL;	// only opcodes using (ix+dis) are legal
		case CpuZ80_ixcbxh: return (op & 7) >= 4 && (op & 7) <= 6;	// opcodes using xh, xl and (ix+dis)
		case CpuZ80_ixcbr2: return true;							// whatever use there is
		default: return true;
		}
	default: return true;
	}
}

cstr tostr(const Meno& m)
{
	return	m.b == NIX ? word[m.a] :
			m.c == NIX ? catstr(word[m.a],"    ",word[m.b]) :
			m.d == NIX ? catstr(word[m.a],"    ",word[m.b],",",word[m.c])  :
						 catstr(word[m.a],"    ",word[m.b],",",word[m.c],",",word[m.d]);
}

cstr opcode_mnemo(CpuID cpuid, const Byte* core, Address a, bool asm8080) noexcept
{
	// return mnenonic with symbolic arguments for instructions
	// op2 and op4 are only used if required (( op2: op1=XY/CB/ED; op4: op1,2=XY,CB ))

	const uint8 op = peek(core,a);
	if (cpuid == Cpu8080) return tostr(asm8080?asm8080_meno(op):i8080_meno(op));

	switch (op)
	{
	default:   return tostr(z80_meno(op));
	case 0xCB: return tostr(z80_menoCB(peek(core,a+1)));
	case 0xED: return tostr(z80_menoED(cpuid, peek(core,a+1)));
	case 0xDD: 
	case 0xFD: if(peek(core,a+1) == 0xCB)
					return tostr(z80_menoXYCB(cpuid, op, peek(core,a+3)));
			   else	return tostr(z80_menoXY(op, peek(core,a+1)));
	}
}

// clang-format on


// ===================================================================================
// ###################################################################################
// ===================================================================================
//
// Major Opcode:

constexpr bool operator==(const Meno& m1, const Meno& m2) { return *intptr(&m1) == *intptr(&m2); }

static cstr next_word(cptr& q)
{
	while (is_space(*q)) { q++; }
	cptr q0 = q;
	while (*q > ' ' && *q < 0x7F && *q != ',') { q++; }
	return substr(q0, q);
}

static Token find_cmd(CpuID, cstr cmd)
{
	for (uint i = NOP; i < BC; i++)
	{
		if (lceq(cmd, word[i])) return Token(i);
	}
	return NIX; // not found
}

static Token find_arg(CpuID, cstr arg)
{
	static constexpr struct
	{
		Token arg;
		char  idf[9];
	} alias[] = {
		{DIS, "offs"}, {XHL, "(ix+dis)"}, {HL, "ix"},	 {XHL, "(iy+dis)"}, {HL, "iy"},	  {N0, "0x00"},	 {N0, "$00"},
		{N0, "&00"},   {N0, "00h"},		  {N0, "0h"},	 {N08, "0x08"},		{N08, "$08"}, {N08, "&08"},	 {N08, "08h"},
		{N08, "8h"},   {N10, "0x10"},	  {N10, "$10"},	 {N10, "&10"},		{N10, "10h"}, {N18, "0x18"}, {N18, "$18"},
		{N18, "&18"},  {N18, "18h"},	  {N20, "0x20"}, {N20, "$20"},		{N20, "&20"}, {N20, "20h"},	 {N28, "0x28"},
		{N28, "$28"},  {N28, "&28"},	  {N28, "28h"},	 {N30, "0x30"},		{N30, "$30"}, {N30, "&30"},	 {N30, "30h"},
		{N38, "0x38"}, {N38, "$38"},	  {N38, "&38"},	 {N38, "38h"},

		// these may lead to wrong detections, e.g. impossible "ld xh,(iy+dis)" == "ld h,(hl)":
		// {H,"xh"}, {L,"xl"},
		// {H,"yh"}, {L,"yl"},
	};

	for (uint i = 0; i < NELEM(alias); i++)
	{
		if (lceq(arg, alias[i].idf)) return alias[i].arg;
	}

	for (uint i = BC; i < NUM_WORD_DEFS; i++)
	{
		if (lceq(arg, word[i])) return Token(i);
	}

	return NIX; // not found
}


Byte major_opcode(CpuID cpuid, cstr q, bool asm8080) throws
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

	Meno   m {NIX, NIX, NIX, NIX};
	Token& cmd	= m.a;
	Token& arg1 = m.b;
	Token& arg2 = m.c;

	cmd = find_cmd(cpuid, next_word(q));
	if (cmd == NIX) throw DataError("unknown opcode");

	while (is_space(*q)) { q++; }
	if (*q) // arg1
	{
		arg1 = find_arg(cpuid, next_word(q));
		if (arg1 == NIX) throw DataError("unknown argument #1");

		while (is_space(*q)) { q++; }
		if (*q == ',') // arg2
		{
			arg2 = find_arg(cpuid, next_word(++q));
			if (arg2 == NIX) throw DataError("unknown argument #2");
		}

		if (*q) throw DataError("format error in opcode spec");
	}

	// Search for Instruction:

	if (asm8080)
	{
		for (uint i = 0; i < 0x100; i++)
		{
			if (m == asm8080_meno(Byte(i))) return uint8(i);
		}
	}
	else
	{
		for (uint i = 0; i < 0x100; i++)
		{
			if (m == z80_meno(Byte(i))) return uint8(i);
		}
		for (uint i = 0x80; i < 0x100; i++)
		{
			if (m == z80_meno_alt(Byte(i))) return uint8(i);
		}
		if (cpuid != Cpu8080)
		{
			for (uint i = 0; i < 0x100; i++)
			{
				if (m == z80_menoCB(Byte(i))) return uint8(i);
			}
			for (uint i = 0; i < 0xC0; i++)
			{
				if (m == z80_menoED(cpuid, Byte(i))) return uint8(i);
			}
		}
	}

	throw DataError("opcode not found");
}


} // namespace z80


/*









































*/
