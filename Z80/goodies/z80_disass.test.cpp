/*	Copyright  (c)	Günter Woigk 2020 - 2020
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

#undef NDEBUG
#define SAFETY 2
#define LOGLEVEL 1
#include "kio/kio.h"
#include "unix/FD.h"
#include "cstrings/cstrings.h"
#include "Templates/Array.h"
#include "kio/util/defines.h"
#include "main.h"
#include "z80_disass.h"
#include "z80_goodies.h"
#include "z80_opcodes.h"


// text used in deprecation warning in 8080 code:
//static char deprecated[] = "***deprecated***";


struct TestSet
{
	uint8 code[5];
	char expected[15];
};

#define NN(nn) (nn&0xff),(nn>>8)


static TestSet i8080_tests[] =
{
	// tests only for i8080
	// using Z80 syntax

	{{NOP},		"nop"},
	{{EX_AF_AF},"nop ;"},
	{{DJNZ},	"nop ;"},
	{{JR},		"nop ;"},
	{{JR_Z},	"nop ;"},
	{{JR_NZ},	"nop ;"},
	{{JR_C},	"nop ;"},
	{{JR_NC},	"nop ;"},
	{{EXX},     "ret ;"},
	{{PFX_CB,1,2}, "jp $0201 ;"},
	{{PFX_IX,1,2}, "call $0201 ;"},
	{{PFX_ED,1,2}, "call $0201 ;"},
	{{PFX_IY,1,2}, "call $0201 ;"},
};

static TestSet z80_tests[] =
{
	// tests only run for Z80

	{{EX_AF_AF},	"ex af,af'"   },
	{{DJNZ,66},		"djnz $+68 ;" },		// "djnz $+dis ; $dest" --> "djnz $+68 ; $0044"
	{{JR,66},		"jr $+68 ;"   },
	{{JR_Z,66},		"jr z,$+68 ;" },
	{{JR_NZ,256-66},"jr nz,$-64 ;"},
	{{JR_C,256-66},	"jr c,$-64 ;" },
	{{JR_NC,66},	"jr nc,$+68 ;"},
	{{EXX},			"exx"         },
};

static TestSet common_tests[] =
{
	// tests run for i8080, Z80 and Z180

	{{NOP}, "nop"},

	{{LD_BC_NN,0x44,0x55},	"ld bc,$5544"},
	{{ADD_HL_BC},			"add hl,bc"},
	{{LD_DE_NN,0x44,0x55},	"ld de,$5544"},
	{{ADD_HL_DE},			"add hl,de"},
	{{LD_HL_NN,0x44,0x55},	"ld hl,$5544"},
	{{ADD_HL_HL},			"add hl,hl"},
	{{LD_SP_NN,0x44,0x55},	"ld sp,$5544"},
	{{ADD_HL_SP},			"add hl,sp"},

	{{LD_xBC_A,}, "ld (bc),a"},
	{{LD_A_xBC,}, "ld a,(bc)"},
	{{LD_xDE_A,}, "ld (de),a"},
	{{LD_A_xDE,}, "ld a,(de)"},
	{{LD_xNN_HL,0x34,0x12}, "ld ($1234),hl"},
	{{LD_HL_xNN,0x34,0x12}, "ld hl,($1234)"},
	{{LD_xNN_A,0x34,0x12},  "ld ($1234),a"},
	{{LD_A_xNN,0x34,0x12},  "ld a,($1234)"},

	{{INC_BC}, "inc bc"}, {{INC_B}, "inc b"},     {{DEC_B}, "dec b"},     {{LD_B_N,77}, "ld b,77"},
	{{DEC_BC}, "dec bc"}, {{INC_C}, "inc c"},     {{DEC_C}, "dec c"},     {{LD_C_N,77}, "ld c,77"},
	{{INC_DE}, "inc de"}, {{INC_D}, "inc d"},     {{DEC_D}, "dec d"},     {{LD_D_N,77}, "ld d,77"},
	{{DEC_DE}, "dec de"}, {{INC_E}, "inc e"},     {{DEC_E}, "dec e"},     {{LD_E_N,77}, "ld e,77"},
	{{INC_HL}, "inc hl"}, {{INC_H}, "inc h"},     {{DEC_H}, "dec h"},     {{LD_H_N,77}, "ld h,77"},
	{{DEC_HL}, "dec hl"}, {{INC_L}, "inc l"},     {{DEC_L}, "dec l"},     {{LD_L_N,77}, "ld l,77"},
	{{INC_SP}, "inc sp"}, {{INC_xHL},"inc (hl)"}, {{DEC_xHL},"dec (hl)"}, {{LD_xHL_N,7}, "ld (hl),7"},
	{{DEC_SP}, "dec sp"}, {{INC_A}, "inc a"},     {{DEC_A}, "dec a"},     {{LD_A_N,177}, "ld a,177"},

	{{RLCA}, "rlca"},	{{RRCA}, "rrca"},	{{RLA}, "rla"},		{{RRA}, "rra"},
	{{DAA}, "daa"},		{{CPL}, "cpl"},		{{SCF}, "scf"},		{{CCF}, "ccf"},

	{{LD_B_B},  "ld b,b"},	 {{LD_B_C},  "ld b,c"},	  {{LD_B_D},  "ld b,d"},	 {{LD_B_E},  "ld b,e"},
	{{LD_C_B},  "ld c,b"},	 {{LD_C_C},  "ld c,c"},	  {{LD_C_D},  "ld c,d"},	 {{LD_C_E},  "ld c,e"},
	{{LD_D_B},  "ld d,b"},	 {{LD_D_C},  "ld d,c"},	  {{LD_D_D},  "ld d,d"},	 {{LD_D_E},  "ld d,e"},
	{{LD_E_B},  "ld e,b"},	 {{LD_E_C},  "ld e,c"},	  {{LD_E_D},  "ld e,d"},	 {{LD_E_E},  "ld e,e"},
	{{LD_H_B},  "ld h,b"},	 {{LD_H_C},  "ld h,c"},	  {{LD_H_D},  "ld h,d"},	 {{LD_H_E},  "ld h,e"},
	{{LD_L_B},  "ld l,b"},	 {{LD_L_C},  "ld l,c"},	  {{LD_L_D},  "ld l,d"},	 {{LD_L_E},  "ld l,e"},
	{{LD_A_B},  "ld a,b"},	 {{LD_A_C},  "ld a,c"},	  {{LD_A_D},  "ld a,d"},	 {{LD_A_E},  "ld a,e"},
	{{LD_xHL_B},"ld (hl),b"},{{LD_xHL_C},"ld (hl),c"},{{LD_xHL_D},"ld (hl),d"},  {{LD_xHL_E},"ld (hl),e"},

	{{LD_B_H},  "ld b,h"},	 {{LD_B_L},  "ld b,l"},	  {{LD_B_A},  "ld b,a"},	 {{LD_B_xHL}, "ld b,(hl)"},
	{{LD_C_H},  "ld c,h"},	 {{LD_C_L},  "ld c,l"},	  {{LD_C_A},  "ld c,a"},	 {{LD_C_xHL}, "ld c,(hl)"},
	{{LD_D_H},  "ld d,h"},	 {{LD_D_L},  "ld d,l"},	  {{LD_D_A},  "ld d,a"},	 {{LD_D_xHL}, "ld d,(hl)"},
	{{LD_E_H},  "ld e,h"},	 {{LD_E_L},  "ld e,l"},	  {{LD_E_A},  "ld e,a"},	 {{LD_E_xHL}, "ld e,(hl)"},
	{{LD_H_H},  "ld h,h"},	 {{LD_H_L},  "ld h,l"},	  {{LD_H_A},  "ld h,a"},	 {{LD_H_xHL}, "ld h,(hl)"},
	{{LD_L_H},  "ld l,h"},	 {{LD_L_L},  "ld l,l"},	  {{LD_L_A},  "ld l,a"},	 {{LD_L_xHL}, "ld l,(hl)"},
	{{LD_A_H},  "ld a,h"},	 {{LD_A_L},  "ld a,l"},	  {{LD_A_A},  "ld a,a"},	 {{LD_A_xHL}, "ld a,(hl)"},
	{{LD_xHL_H},"ld (hl),h"},{{LD_xHL_L},"ld (hl),l"},{{LD_xHL_A},"ld (hl),a"},  {{HALT},	  "halt"},

	{{ADD_B}, "add a,b"},  {{ADD_C}, "add a,c"},  {{ADD_D}, "add a,d"},  {{ADD_E}, "add a,e"},
	{{ADC_B}, "adc a,b"},  {{ADC_C}, "adc a,c"},  {{ADC_D}, "adc a,d"},  {{ADC_E}, "adc a,e"},
	{{SUB_B}, "sub a,b"},  {{SUB_C}, "sub a,c"},  {{SUB_D}, "sub a,d"},  {{SUB_E}, "sub a,e"},
	{{SBC_B}, "sbc a,b"},  {{SBC_C}, "sbc a,c"},  {{SBC_D}, "sbc a,d"},  {{SBC_E}, "sbc a,e"},
	{{AND_B}, "and a,b"},  {{AND_C}, "and a,c"},  {{AND_D}, "and a,d"},  {{AND_E}, "and a,e"},
	{{XOR_B}, "xor a,b"},  {{XOR_C}, "xor a,c"},  {{XOR_D}, "xor a,d"},  {{XOR_E}, "xor a,e"},
	{{ OR_B},  "or a,b"},  {{ OR_C},  "or a,c"},  {{ OR_D},  "or a,d"},  {{ OR_E},  "or a,e"},
	{{ CP_B},  "cp a,b"},  {{ CP_C},  "cp a,c"},  {{ CP_D},  "cp a,d"},  {{ CP_E},  "cp a,e"},

	{{ADD_H}, "add a,h"},  {{ADD_L}, "add a,l"},  {{ADD_A}, "add a,a"},  {{ADD_xHL}, "add a,(hl)"},
	{{ADC_H}, "adc a,h"},  {{ADC_L}, "adc a,l"},  {{ADC_A}, "adc a,a"},  {{ADC_xHL}, "adc a,(hl)"},
	{{SUB_H}, "sub a,h"},  {{SUB_L}, "sub a,l"},  {{SUB_A}, "sub a,a"},  {{SUB_xHL}, "sub a,(hl)"},
	{{SBC_H}, "sbc a,h"},  {{SBC_L}, "sbc a,l"},  {{SBC_A}, "sbc a,a"},  {{SBC_xHL}, "sbc a,(hl)"},
	{{AND_H}, "and a,h"},  {{AND_L}, "and a,l"},  {{AND_A}, "and a,a"},  {{AND_xHL}, "and a,(hl)"},
	{{XOR_H}, "xor a,h"},  {{XOR_L}, "xor a,l"},  {{XOR_A}, "xor a,a"},  {{XOR_xHL}, "xor a,(hl)"},
	{{ OR_H},  "or a,h"},  {{ OR_L},  "or a,l"},  {{ OR_A},  "or a,a"},  {{ OR_xHL},  "or a,(hl)"},
	{{ CP_H},  "cp a,h"},  {{ CP_L},  "cp a,l"},  {{ CP_A},  "cp a,a"},  {{ CP_xHL},  "cp a,(hl)"},

	{{RET_NZ}, "ret nz"}, {{POP_BC}, "pop bc"},   {{JP_NZ,0x45,0x23}, "jp nz,$2345"},{{RST00}, "rst 0"},
	{{RET_Z }, "ret z"},  {{RET   }, "ret"},      {{JP_Z ,0x45,0x23}, "jp z,$2345"}, {{RST08}, "rst 8"},
	{{RET_NC}, "ret nc"}, {{POP_DE}, "pop de"},   {{JP_NC,0x45,0x23}, "jp nc,$2345"},{{RST10}, "rst 16"},
	{{RET_C }, "ret c"},                          {{JP_C ,0x45,0x23}, "jp c,$2345"}, {{RST18}, "rst 24"},
	{{RET_PO}, "ret po"}, {{POP_HL}, "pop hl"},   {{JP_PO,0x45,0x23}, "jp po,$2345"},{{RST20}, "rst 32"},
	{{RET_PE}, "ret pe"}, {{JP_HL }, "jp hl"},    {{JP_PE,0x45,0x23}, "jp pe,$2345"},{{RST28}, "rst 40"},
	{{RET_P }, "ret p"},  {{POP_AF}, "pop af"},   {{JP_P ,0x45,0x23}, "jp p,$2345"}, {{RST30}, "rst 48"},
	{{RET_M }, "ret m"},  {{LD_SP_HL},"ld sp,hl"},{{JP_M ,0x45,0x23}, "jp m,$2345"}, {{RST38}, "rst 56"},

	{{CALL_NZ,0x45,0x23}, "call nz,$2345"}, {{ADD_N,123}, "add a,123"}, {{JP,0,24  }, "jp $1800"},   {{PUSH_BC}, "push bc"},
	{{CALL_Z ,0x45,0x23}, "call z,$2345"},  {{ADC_N,123}, "adc a,123"}, {{OUTA,0x88}, "out ($88),a"},{{CALL,1,8},"call $0801"},
	{{CALL_NC,0x45,0x23}, "call nc,$2345"}, {{SUB_N,123}, "sub a,123"}, {{INA,0x88 }, "in a,($88)"}, {{PUSH_DE}, "push de"},
	{{CALL_C ,0x45,0x23}, "call c,$2345"},  {{SBC_N,123}, "sbc a,123"}, {{EX_HL_xSP}, "ex hl,(sp)"}, {{PUSH_HL}, "push hl"},
	{{CALL_PO,0x45,0x23}, "call po,$2345"}, {{AND_N,123}, "and a,123"}, {{EX_DE_HL }, "ex de,hl"},   {{PUSH_AF}, "push af"},
	{{CALL_PE,0x45,0x23}, "call pe,$2345"}, {{XOR_N,123}, "xor a,123"}, {{DI,	   }, "di"},
	{{CALL_P ,0x45,0x23}, "call p,$2345"},  {{OR_N ,123},  "or a,123"}, {{EI,	   }, "ei"},
	{{CALL_M ,0x45,0x23}, "call m,$2345"},  {{CP_N ,123},  "cp a,123"},

/*
	{{PFX_CB,	 }, ""},
	{{PFX_IX, }, ""},
	{{PFX_ED, }, ""},
	{{PFX_IY, }, ""},
*/
};

static TestSet asm8080_tests[] =
{
	// tests only run for i8080
	// asm8080 syntax

	{{NOP},		"nop"},
	{{EX_AF_AF},"nop ;"},
	{{DJNZ},	"nop ;"},
	{{JR},		"nop ;"},
	{{JR_Z},	"nop ;"},
	{{JR_NZ},	"nop ;"},
	{{JR_C},	"nop ;"},
	{{JR_NC},	"nop ;"},
	{{EXX},     "ret ;"},
	{{PFX_CB,1,2}, "jmp $0201 ;"},
	{{PFX_IX,1,2}, "call $0201 ;"},
	{{PFX_ED,1,2}, "call $0201 ;"},
	{{PFX_IY,1,2}, "call $0201 ;"},

	{{LD_BC_NN,0x44,0x55},	"lxi b,$5544"},
	{{ADD_HL_BC},			"dad b"},
	{{LD_DE_NN,0x44,0x55},	"lxi d,$5544"},
	{{ADD_HL_DE},			"dad d"},
	{{LD_HL_NN,0x44,0x55},	"lxi h,$5544"},
	{{ADD_HL_HL},			"dad h"},
	{{LD_SP_NN,0x44,0x55},	"lxi sp,$5544"},
	{{ADD_HL_SP},			"dad sp"},

	{{LD_xBC_A,}, "stax b"},
	{{LD_A_xBC,}, "ldax b"},
	{{LD_xDE_A,}, "stax d"},
	{{LD_A_xDE,}, "ldax d"},
	{{LD_xNN_HL,0x34,0x12}, "shld $1234"},
	{{LD_HL_xNN,0x34,0x12}, "lhld $1234"},
	{{LD_xNN_A,0x34,0x12},  "sta $1234"},
	{{LD_A_xNN,0x34,0x12},  "lda $1234"},

	{{INC_BC}, "inx b"},  {{INC_B},  "inr b"},  {{DEC_B},  "dcr b"},  {{LD_B_N,77},  "mvi b,77"},
	{{DEC_BC}, "dcx b"},  {{INC_C},  "inr c"},  {{DEC_C},  "dcr c"},  {{LD_C_N,77},  "mvi c,77"},
	{{INC_DE}, "inx d"},  {{INC_D},  "inr d"},  {{DEC_D},  "dcr d"},  {{LD_D_N,77},  "mvi d,77"},
	{{DEC_DE}, "dcx d"},  {{INC_E},  "inr e"},  {{DEC_E},  "dcr e"},  {{LD_E_N,77},  "mvi e,77"},
	{{INC_HL}, "inx h"},  {{INC_H},  "inr h"},  {{DEC_H},  "dcr h"},  {{LD_H_N,77},  "mvi h,77"},
	{{DEC_HL}, "dcx h"},  {{INC_L},  "inr l"},  {{DEC_L},  "dcr l"},  {{LD_L_N,77},  "mvi l,77"},
	{{INC_SP}, "inx sp"}, {{INC_xHL},"inr m"},  {{DEC_xHL},"dcr m"},  {{LD_xHL_N,7}, "mvi m,7"},
	{{DEC_SP}, "dcx sp"}, {{INC_A},  "inr a"},  {{DEC_A},  "dcr a"},  {{LD_A_N,177}, "mvi a,177"},

	{{RLCA}, "rlc"},	{{RRCA}, "rrc"},	{{RLA}, "ral"},		{{RRA}, "rar"},
	{{DAA},  "daa"},	{{CPL},  "cma"},	{{SCF}, "stc"},		{{CCF}, "cmc"},

	{{LD_B_B},  "ld b,b"},	 {{LD_B_C},  "ld b,c"},	  {{LD_B_D},  "ld b,d"},	 {{LD_B_E},  "ld b,e"},
	{{LD_C_B},  "ld c,b"},	 {{LD_C_C},  "ld c,c"},	  {{LD_C_D},  "ld c,d"},	 {{LD_C_E},  "ld c,e"},
	{{LD_D_B},  "ld d,b"},	 {{LD_D_C},  "ld d,c"},	  {{LD_D_D},  "ld d,d"},	 {{LD_D_E},  "ld d,e"},
	{{LD_E_B},  "ld e,b"},	 {{LD_E_C},  "ld e,c"},	  {{LD_E_D},  "ld e,d"},	 {{LD_E_E},  "ld e,e"},
	{{LD_H_B},  "ld h,b"},	 {{LD_H_C},  "ld h,c"},	  {{LD_H_D},  "ld h,d"},	 {{LD_H_E},  "ld h,e"},
	{{LD_L_B},  "ld l,b"},	 {{LD_L_C},  "ld l,c"},	  {{LD_L_D},  "ld l,d"},	 {{LD_L_E},  "ld l,e"},
	{{LD_A_B},  "ld a,b"},	 {{LD_A_C},  "ld a,c"},	  {{LD_A_D},  "ld a,d"},	 {{LD_A_E},  "ld a,e"},
	{{LD_xHL_B},"ld m,b"},   {{LD_xHL_C},"ld m,c"},   {{LD_xHL_D},"ld m,d"},     {{LD_xHL_E},"ld m,e"},

	{{LD_B_H},  "ld b,h"},	 {{LD_B_L},  "ld b,l"},	  {{LD_B_A},  "ld b,a"},	 {{LD_B_xHL}, "ld b,m"},
	{{LD_C_H},  "ld c,h"},	 {{LD_C_L},  "ld c,l"},	  {{LD_C_A},  "ld c,a"},	 {{LD_C_xHL}, "ld c,m"},
	{{LD_D_H},  "ld d,h"},	 {{LD_D_L},  "ld d,l"},	  {{LD_D_A},  "ld d,a"},	 {{LD_D_xHL}, "ld d,m"},
	{{LD_E_H},  "ld e,h"},	 {{LD_E_L},  "ld e,l"},	  {{LD_E_A},  "ld e,a"},	 {{LD_E_xHL}, "ld e,m"},
	{{LD_H_H},  "ld h,h"},	 {{LD_H_L},  "ld h,l"},	  {{LD_H_A},  "ld h,a"},	 {{LD_H_xHL}, "ld h,m"},
	{{LD_L_H},  "ld l,h"},	 {{LD_L_L},  "ld l,l"},	  {{LD_L_A},  "ld l,a"},	 {{LD_L_xHL}, "ld l,m"},
	{{LD_A_H},  "ld a,h"},	 {{LD_A_L},  "ld a,l"},	  {{LD_A_A},  "ld a,a"},	 {{LD_A_xHL}, "ld a,m"},
	{{LD_xHL_H},"ld m,h"},   {{LD_xHL_L},"ld m,l"},   {{LD_xHL_A},"ld m,a"},     {{HALT},	  "hlt"},

	{{ADD_B}, "add b"},  {{ADD_C}, "add c"},  {{ADD_D}, "add d"},  {{ADD_E}, "add e"},
	{{ADC_B}, "adc b"},  {{ADC_C}, "adc c"},  {{ADC_D}, "adc d"},  {{ADC_E}, "adc e"},
	{{SUB_B}, "sub b"},  {{SUB_C}, "sub c"},  {{SUB_D}, "sub d"},  {{SUB_E}, "sub e"},
	{{SBC_B}, "sbb b"},  {{SBC_C}, "sbb c"},  {{SBC_D}, "sbb d"},  {{SBC_E}, "sbb e"},
	{{AND_B}, "ana b"},  {{AND_C}, "ana c"},  {{AND_D}, "ana d"},  {{AND_E}, "ana e"},
	{{XOR_B}, "xra b"},  {{XOR_C}, "xra c"},  {{XOR_D}, "xra d"},  {{XOR_E}, "xra e"},
	{{ OR_B}, "ora b"},  {{ OR_C}, "ora c"},  {{ OR_D}, "ora d"},  {{ OR_E}, "ora e"},
	{{ CP_B}, "cmp b"},  {{ CP_C}, "cmp c"},  {{ CP_D}, "cmp d"},  {{ CP_E}, "cmp e"},

	{{ADD_H}, "add h"},  {{ADD_L}, "add l"},  {{ADD_A}, "add a"},  {{ADD_xHL}, "add m"},
	{{ADC_H}, "adc h"},  {{ADC_L}, "adc l"},  {{ADC_A}, "adc a"},  {{ADC_xHL}, "adc m"},
	{{SUB_H}, "sub h"},  {{SUB_L}, "sub l"},  {{SUB_A}, "sub a"},  {{SUB_xHL}, "sub m"},
	{{SBC_H}, "sbb h"},  {{SBC_L}, "sbb l"},  {{SBC_A}, "sbb a"},  {{SBC_xHL}, "sbb m"},
	{{AND_H}, "ana h"},  {{AND_L}, "ana l"},  {{AND_A}, "ana a"},  {{AND_xHL}, "ana m"},
	{{XOR_H}, "xra h"},  {{XOR_L}, "xra l"},  {{XOR_A}, "xra a"},  {{XOR_xHL}, "xra m"},
	{{ OR_H}, "ora h"},  {{ OR_L}, "ora l"},  {{ OR_A}, "ora a"},  {{ OR_xHL}, "ora m"},
	{{ CP_H}, "cmp h"},  {{ CP_L}, "cmp l"},  {{ CP_A}, "cmp a"},  {{ CP_xHL}, "cmp m"},

	{{RET_NZ}, "rnz"}, {{POP_BC}, "pop b"},   {{JP_NZ,0x45,0x23}, "jnz $2345"},{{RST00}, "rst 0"},
	{{RET_Z }, "rz"},  {{RET   }, "ret"},     {{JP_Z ,0x45,0x23}, "jz $2345"}, {{RST08}, "rst 1"},
	{{RET_NC}, "rnc"}, {{POP_DE}, "pop d"},   {{JP_NC,0x45,0x23}, "jnc $2345"},{{RST10}, "rst 2"},
	{{RET_C }, "rc"},                         {{JP_C ,0x45,0x23}, "jc $2345"}, {{RST18}, "rst 3"},
	{{RET_PO}, "rpo"}, {{POP_HL}, "pop h"},   {{JP_PO,0x45,0x23}, "jpo $2345"},{{RST20}, "rst 4"},
	{{RET_PE}, "rpe"}, {{JP_HL }, "pchl"},    {{JP_PE,0x45,0x23}, "jpe $2345"},{{RST28}, "rst 5"},
	{{RET_P }, "rp"},  {{POP_AF}, "pop psw"}, {{JP_P ,0x45,0x23}, "jp $2345"}, {{RST30}, "rst 6"},
	{{RET_M }, "rm"},  {{LD_SP_HL},"sphl"},   {{JP_M ,0x45,0x23}, "jm $2345"}, {{RST38}, "rst 7"},

	{{CALL_NZ,0x45,0x23}, "cnz $2345"}, {{ADD_N,123}, "adi 123"}, {{JP,0,24  }, "jmp $1800"},{{PUSH_BC}, "push b"},
	{{CALL_Z ,0x45,0x23}, "cz $2345"},  {{ADC_N,123}, "aci 123"}, {{OUTA,188},  "out 188"},  {{CALL,1,8},"call $0801"},
	{{CALL_NC,0x45,0x23}, "cnc $2345"}, {{SUB_N,123}, "sui 123"}, {{INA,188 },  "in 188"},   {{PUSH_DE}, "push d"},
	{{CALL_C ,0x45,0x23}, "cc $2345"},  {{SBC_N,123}, "sbi 123"}, {{EX_HL_xSP}, "xthl"},     {{PUSH_HL}, "push h"},
	{{CALL_PO,0x45,0x23}, "cpo $2345"}, {{AND_N,123}, "ani 123"}, {{EX_DE_HL }, "xchg"},     {{PUSH_AF}, "push psw"},
	{{CALL_PE,0x45,0x23}, "cpe $2345"}, {{XOR_N,123}, "xri 123"}, {{DI,	   },   "di"},
	{{CALL_P ,0x45,0x23}, "cp $2345"},  {{OR_N ,123}, "ori 123"}, {{EI,	   },   "ei"},
	{{CALL_M ,0x45,0x23}, "cm $2345"},  {{CP_N ,123}, "cpi 123"},
};


static bool same(cstr a, cstr b)
{
	// compare disassembly result
	// some opcodes also have a comment after ';':
	// these are only tested to exist

	while (*a == *b && *a && *a!=';')
	{
		a++; b++;

		if (is_space(*a) && is_space(*b))
		{
			do { a++; } while(is_space(*a));
			do { b++; } while(is_space(*b));
			continue;
		}
	}

	return (*a == *b);
}

#define assert_same(A,B) if (!same(A,B)) throw InternalError(__FILE__, __LINE__, usingstr("FAILED: \"%s\" != \"%s\"",A,B))


static void run_tests (uint& num_tests, uint& num_errors, CpuID cpu_id, int option, TestSet* tests, uint nelem)
{
	log("** ");
	for (uint i=0; i < nelem; )
	{
		TestSet& test = tests[i++];
		log("%02x ", test.code[0]);
		if ((i&31)==0) logNl();

		TRY
			uint16 addr = 0;
			cstr disassembly = disassemble(cpu_id, test.code, addr, option);
			assert_same(test.expected,disassembly);
			assert(addr>0 && addr<=4);
			assert(test.code[addr] == 0);
		END

	}
	logline("##");
}

static void test_disass_asm8080 (uint& num_tests, uint& num_errors)
{
	logIn("test disass --asm8080");
	run_tests(num_tests,num_errors,Cpu8080,DISASS_ASM8080,asm8080_tests,NELEM(asm8080_tests));
}

static void test_disass_z80 (uint& num_tests, uint& num_errors, CpuID cpu_id, int option)
{
	cstr cpu_str = cpu_id==Cpu8080?"8080":cpu_id==CpuZ180?"Z180":"Z80";
	cstr opt_str = option==DISASS_IXCBR2?" --ixcbr2":option==DISASS_IXCBXH?" --ixcbxh":"";
	logIn("test disass --cpu=%s%s",cpu_str,opt_str);

	run_tests(num_tests,num_errors,cpu_id,option,common_tests,NELEM(common_tests));

	if (cpu_id == Cpu8080)
		run_tests(num_tests,num_errors,cpu_id,option,i8080_tests,NELEM(i8080_tests));

	if (cpu_id == CpuZ80)
		run_tests(num_tests,num_errors,cpu_id,option,z80_tests,NELEM(z80_tests));
}


void test_z80_disass (uint& num_tests, uint& num_errors)
{
	logIn("test z80_disass");

	test_disass_z80(num_tests,num_errors,CpuZ80,DISASS_STD);
	test_disass_z80(num_tests,num_errors,CpuZ80,DISASS_IXCBR2);
	test_disass_z80(num_tests,num_errors,CpuZ80,DISASS_IXCBXH);
	test_disass_z80(num_tests,num_errors,CpuZ180,DISASS_STD);
	test_disass_z80(num_tests,num_errors,Cpu8080,DISASS_STD);
	test_disass_asm8080(num_tests,num_errors);
}







