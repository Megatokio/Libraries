// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#undef NDEBUG
#define SAFETY	 2
#define LOGLEVEL 1
#include "Templates/Array.h"
#include "cstrings/cstrings.h"
#include "kio/kio.h"
#include "kio/util/defines.h"
#include "main.h"
#include "unix/FD.h"
#include "z80_goodies.h"
#include "z80_opcodes.h"


struct TestSet
{
	uint8 code[4];		// code to disassemble + 1 byte 0x00
	char  expected[17]; // expected disassembly
};


static TestSet common_tests[] = {
	// tests for i8080, Z80 and Z180

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
};

static TestSet asm8080_tests[] = {
	// tests for i8080
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

static TestSet i8080_tests[] = {
	// tests for i8080
	// Z80 syntax

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

static TestSet z80_z180_tests[] = {
	// tests for Z80 and Z180:
	// legal CB, ED and IX opcodes

	// non-prefix opcodes not present in i8080:

	{{NOP},		"nop"},
	{{DJNZ,33},	"djnz $+35 ;"},
	{{JR,128},	"jr $-126 ;"},
	{{JR_Z,127},"jr z,$+129 ;"},
	{{JR_NZ,20},"jr nz,$+22 ;"},
	{{JR_C,255},"jr c,$+1 ;"},
	{{JR_NC,0},	"jr nc,$+2 ;"},
	{{EX_AF_AF},"ex af,af'"},
	{{EXX},     "exx"},

	// prefix IX/IY opcodes:

	{{PFX_IX,LD_HL_NN, 0x43,0x21}, "ld ix,$2143"},
	{{PFX_IY,LD_HL_xNN,0x43,0x21}, "ld iy,($2143)"},
	{{PFX_IX,LD_xNN_HL,0x43,0x21}, "ld ($2143),ix"},
	{{PFX_IY,ADD_HL_BC}, "add iy,bc"},
	{{PFX_IX,ADD_HL_DE}, "add ix,de"},
	{{PFX_IY,ADD_HL_HL}, "add iy,iy"},
	{{PFX_IX,ADD_HL_HL}, "add ix,ix"},
	{{PFX_IX,ADD_HL_SP}, "add ix,sp"},
	{{PFX_IY,PUSH_HL},   "push iy"},
	{{PFX_IX,POP_HL},    "pop ix"},
	{{PFX_IY,JP_HL},     "jp iy"},
	{{PFX_IX,LD_SP_HL},  "ld sp,ix"},
	{{PFX_IY,INC_HL },   "inc iy"},
	{{PFX_IX,DEC_HL },   "dec ix"},

	{{PFX_IX,INC_xHL,66},  "inc (ix+66)"},
	{{PFX_IY,DEC_xHL,200}, "dec (iy-56)"},
	{{PFX_IX,LD_xHL_N,66,200},"ld (ix+66),200"},

	{{PFX_IX,LD_xHL_B,66}, "ld (ix+66),b"},
	{{PFX_IY,LD_xHL_H,66}, "ld (iy+66),h"},
	{{PFX_IX,LD_xHL_A,66}, "ld (ix+66),a"},
	{{PFX_IY,LD_B_xHL,66}, "ld b,(iy+66)"},
	{{PFX_IX,LD_L_xHL,66}, "ld l,(ix+66)"},
	{{PFX_IY,LD_A_xHL,66}, "ld a,(iy+66)"},

	{{PFX_IX,ADD_xHL,66}, "add a,(ix+66)"},
	{{PFX_IY,ADC_xHL,66}, "adc a,(iy+66)"},
	{{PFX_IX,SUB_xHL,66}, "sub a,(ix+66)"},
	{{PFX_IY,SBC_xHL,66}, "sbc a,(iy+66)"},
	{{PFX_IY,AND_xHL,66}, "and a,(iy+66)"},
	{{PFX_IY,XOR_xHL,66}, "xor a,(iy+66)"},
	{{PFX_IX, OR_xHL,66},  "or a,(ix+66)"},
	{{PFX_IX, CP_xHL,66},  "cp a,(ix+66)"},

	// prefix CB opcodes:

	{{PFX_CB,RLC_B},  "rlc b"}, {{PFX_CB,RLC_H}, "rlc h"}, {{PFX_CB,RLC_C}, "rlc c"}, {{PFX_CB,RLC_L},   "rlc l"},
	{{PFX_CB,RLC_D},  "rlc d"}, {{PFX_CB,RLC_A}, "rlc a"}, {{PFX_CB,RLC_E}, "rlc e"}, {{PFX_CB,RLC_xHL}, "rlc (hl)"},
	{{PFX_CB,RRC_B},  "rrc b"}, {{PFX_CB,RRC_C}, "rrc c"}, {{PFX_CB,RRC_D}, "rrc d"}, {{PFX_CB,RRC_E},   "rrc e"},
	{{PFX_CB,RRC_H},  "rrc h"}, {{PFX_CB,RRC_L}, "rrc l"}, {{PFX_CB,RRC_A}, "rrc a"}, {{PFX_CB,RRC_xHL}, "rrc (hl)"},
	{{PFX_CB,RL_B},   "rl b"},  {{PFX_CB,RL_C},  "rl c"},  {{PFX_CB,RL_D},  "rl d"},  {{PFX_CB,RL_E},    "rl e"},
	{{PFX_CB,RL_H},   "rl h"},  {{PFX_CB,RL_L},  "rl l"},  {{PFX_CB,RL_A},  "rl a"},  {{PFX_CB,RL_xHL},  "rl (hl)"},
	{{PFX_CB,RR_B},   "rr b"},  {{PFX_CB,RR_H},  "rr h"},  {{PFX_CB,RR_A},  "rr a"},  {{PFX_CB,RR_xHL},  "rr (hl)"},
	{{PFX_CB,SLA_C},  "sla c"}, {{PFX_CB,SLA_L}, "sla l"}, {{PFX_CB,SLA_A}, "sla a"}, {{PFX_CB,SLA_xHL}, "sla (hl)"},
	{{PFX_CB,SRA_B},  "sra b"}, {{PFX_CB,SRA_H}, "sra h"}, {{PFX_CB,SRA_A}, "sra a"}, {{PFX_CB,SRA_xHL}, "sra (hl)"},
	{{PFX_CB,SRL_C},  "srl c"}, {{PFX_CB,SRL_L}, "srl l"}, {{PFX_CB,SRL_A}, "srl a"}, {{PFX_CB,SRL_xHL}, "srl (hl)"},

	{{PFX_CB,BIT0_B}, "bit 0,b"}, {{PFX_CB,BIT0_C}, "bit 0,c"}, {{PFX_CB,BIT0_D}, "bit 0,d"}, {{PFX_CB,BIT0_E},  "bit 0,e"},
	{{PFX_CB,BIT0_H}, "bit 0,h"}, {{PFX_CB,BIT0_L}, "bit 0,l"}, {{PFX_CB,BIT0_A}, "bit 0,a"}, {{PFX_CB,BIT0_xHL},"bit 0,(hl)"},
	{{PFX_CB,BIT1_B}, "bit 1,b"}, {{PFX_CB,BIT1_C}, "bit 1,c"}, {{PFX_CB,BIT2_D}, "bit 2,d"}, {{PFX_CB,BIT3_E},  "bit 3,e"},
	{{PFX_CB,BIT3_H}, "bit 3,h"}, {{PFX_CB,BIT4_L}, "bit 4,l"}, {{PFX_CB,BIT5_A}, "bit 5,a"}, {{PFX_CB,BIT6_xHL},"bit 6,(hl)"},
	{{PFX_CB,BIT7_B}, "bit 7,b"}, {{PFX_CB,BIT7_C}, "bit 7,c"}, {{PFX_CB,BIT7_D}, "bit 7,d"}, {{PFX_CB,BIT7_E},  "bit 7,e"},
	{{PFX_CB,BIT7_H}, "bit 7,h"}, {{PFX_CB,BIT7_L}, "bit 7,l"}, {{PFX_CB,BIT7_A}, "bit 7,a"}, {{PFX_CB,BIT7_xHL},"bit 7,(hl)"},

	{{PFX_CB,SET0_B}, "set 0,b"}, {{PFX_CB,SET0_C}, "set 0,c"}, {{PFX_CB,SET0_D}, "set 0,d"}, {{PFX_CB,SET0_E},  "set 0,e"},
	{{PFX_CB,SET0_H}, "set 0,h"}, {{PFX_CB,SET0_L}, "set 0,l"}, {{PFX_CB,SET0_A}, "set 0,a"}, {{PFX_CB,SET0_xHL},"set 0,(hl)"},
	{{PFX_CB,SET1_B}, "set 1,b"}, {{PFX_CB,SET1_C}, "set 1,c"}, {{PFX_CB,SET2_D}, "set 2,d"}, {{PFX_CB,SET3_E},  "set 3,e"},
	{{PFX_CB,SET3_H}, "set 3,h"}, {{PFX_CB,SET4_L}, "set 4,l"}, {{PFX_CB,SET5_A}, "set 5,a"}, {{PFX_CB,SET6_xHL},"set 6,(hl)"},
	{{PFX_CB,SET7_B}, "set 7,b"}, {{PFX_CB,SET7_C}, "set 7,c"}, {{PFX_CB,SET7_D}, "set 7,d"}, {{PFX_CB,SET7_E},  "set 7,e"},
	{{PFX_CB,SET7_H}, "set 7,h"}, {{PFX_CB,SET7_L}, "set 7,l"}, {{PFX_CB,SET7_A}, "set 7,a"}, {{PFX_CB,SET7_xHL},"set 7,(hl)"},

	{{PFX_CB,RES0_B}, "res 0,b"}, {{PFX_CB,RES0_C}, "res 0,c"}, {{PFX_CB,RES0_D}, "res 0,d"}, {{PFX_CB,RES0_E},  "res 0,e"},
	{{PFX_CB,RES0_H}, "res 0,h"}, {{PFX_CB,RES0_L}, "res 0,l"}, {{PFX_CB,RES0_A}, "res 0,a"}, {{PFX_CB,RES0_xHL},"res 0,(hl)"},
	{{PFX_CB,RES1_B}, "res 1,b"}, {{PFX_CB,RES1_C}, "res 1,c"}, {{PFX_CB,RES2_D}, "res 2,d"}, {{PFX_CB,RES3_E},  "res 3,e"},
	{{PFX_CB,RES3_H}, "res 3,h"}, {{PFX_CB,RES4_L}, "res 4,l"}, {{PFX_CB,RES5_A}, "res 5,a"}, {{PFX_CB,RES6_xHL},"res 6,(hl)"},
	{{PFX_CB,RES7_B}, "res 7,b"}, {{PFX_CB,RES7_C}, "res 7,c"}, {{PFX_CB,RES7_D}, "res 7,d"}, {{PFX_CB,RES7_E},  "res 7,e"},
	{{PFX_CB,RES7_H}, "res 7,h"}, {{PFX_CB,RES7_L}, "res 7,l"}, {{PFX_CB,RES7_A}, "res 7,a"}, {{PFX_CB,RES7_xHL},"res 7,(hl)"},

	// prefix IXCB opcodes:

	{{PFX_IX,PFX_CB, 67,RLC_xHL},"rlc (ix+67)"},    {{PFX_IX,PFX_CB,127,RRC_xHL},"rrc (ix+127)"},   {{PFX_IY,PFX_CB,0,RL_xHL}, "rl (iy+0)"},
	{{PFX_IY,PFX_CB,255,RR_xHL}, "rr  (iy-1)"},     {{PFX_IY,PFX_CB,200,SLA_xHL},"sla (iy-56)"},
	{{PFX_IX,PFX_CB,128,SRA_xHL},"sra (ix-128)"},   {{PFX_IX,PFX_CB, 67,SRL_xHL},"srl (ix+67)"},
	{{PFX_IX,PFX_CB, 67,BIT0_xHL},"bit 0,(ix+67)"}, {{PFX_IY,PFX_CB,199,BIT1_xHL},"bit 1,(iy-57)"}, {{PFX_IX,PFX_CB,128,BIT2_xHL},"bit 2,(ix-128)"},
	{{PFX_IY,PFX_CB,127,BIT3_xHL},"bit 3,(iy+127)"},{{PFX_IY,PFX_CB,  0,BIT4_xHL},"bit 4,(iy+0)"},  {{PFX_IX,PFX_CB,  0,BIT5_xHL},"bit 5,(ix+0)"},
	{{PFX_IX,PFX_CB, 67,BIT6_xHL},"bit 6,(ix+67)"}, {{PFX_IY,PFX_CB, 67,BIT7_xHL},"bit 7,(iy+67)"}, {{PFX_IY,PFX_CB, 67,SET0_xHL},"set 0,(iy+67)"},
	{{PFX_IY,PFX_CB, 67,SET3_xHL},"set 3,(iy+67)"}, {{PFX_IX,PFX_CB, 67,SET6_xHL},"set 6,(ix+67)"}, {{PFX_IX,PFX_CB, 67,SET7_xHL},"set 7,(ix+67)"},
	{{PFX_IX,PFX_CB, 67,RES0_xHL},"res 0,(ix+67)"}, {{PFX_IY,PFX_CB, 67,RES5_xHL},"res 5,(iy+67)"},
	{{PFX_IX,PFX_CB, 67,RES6_xHL},"res 6,(ix+67)"}, {{PFX_IY,PFX_CB, 67,RES7_xHL},"res 7,(iy+67)"},

	// prefix ED opcodes:

	{{PFX_ED,IN_B_xC},"in b,(c)"}, {{PFX_ED,IN_C_xC},"in c,(c)"}, {{PFX_ED,IN_D_xC},"in d,(c)"}, {{PFX_ED,IN_E_xC},"in e,(c)"},
	{{PFX_ED,IN_H_xC},"in h,(c)"}, {{PFX_ED,IN_L_xC},"in l,(c)"}, {{PFX_ED,IN_F_xC},"in f,(c)"}, {{PFX_ED,IN_A_xC},"in a,(c)"},

	{{PFX_ED,OUT_xC_B},"out (c),b"}, {{PFX_ED,OUT_xC_C},"out (c),c"}, {{PFX_ED,OUT_xC_D},"out (c),d"}, {{PFX_ED,OUT_xC_E},"out (c),e"},
	{{PFX_ED,OUT_xC_H},"out (c),h"}, {{PFX_ED,OUT_xC_L},"out (c),l"}, {{PFX_ED,OUT_xC_A},"out (c),a"},

	{{PFX_ED,SBC_HL_BC},"sbc hl,bc"}, {{PFX_ED,ADC_HL_BC},"adc hl,bc"}, {{PFX_ED,SBC_HL_DE},"sbc hl,de"}, {{PFX_ED,ADC_HL_DE},"adc hl,de"},
	{{PFX_ED,SBC_HL_HL},"sbc hl,hl"}, {{PFX_ED,ADC_HL_HL},"adc hl,hl"}, {{PFX_ED,SBC_HL_SP},"sbc hl,sp"}, {{PFX_ED,ADC_HL_SP},"adc hl,sp"},

	{{PFX_ED,LD_xNN_BC,0x34,0x12},"ld ($1234),bc"}, {{PFX_ED,LD_BC_xNN,0x34,0x12},"ld bc,($1234)"},
	{{PFX_ED,LD_xNN_DE,0x34,0x12},"ld ($1234),de"}, {{PFX_ED,LD_DE_xNN,0x34,0x12},"ld de,($1234)"},
	{{PFX_ED,ED_xNN_HL,0x34,0x12},"ld ($1234),hl"}, {{PFX_ED,ED_HL_xNN,0x34,0x12},"ld hl,($1234)"},
	{{PFX_ED,LD_xNN_SP,0x34,0x12},"ld ($1234),sp"}, {{PFX_ED,LD_SP_xNN,0x34,0x12},"ld sp,($1234)"},

	{{PFX_ED,NEG}, "neg"},  {{PFX_ED,RETN},"retn"}, {{PFX_ED,RETI},"reti"},
	{{PFX_ED,IM_0},"im 0"}, {{PFX_ED,IM_1},"im 1"}, {{PFX_ED,IM_2},"im 2"},
	{{PFX_ED,LD_A_I},"ld a,i"}, {{PFX_ED,LD_I_A},"ld i,a"}, {{PFX_ED,LD_R_A},"ld r,a"}, {{PFX_ED,LD_A_R},"ld a,r"},
	{{PFX_ED,RRD},"rrd"},  {{PFX_ED,RLD},"rld"},
	{{PFX_ED,LDI}, "ldi"}, {{PFX_ED,LDIR},"ldir"}, {{PFX_ED,LDD}, "ldd"}, {{PFX_ED,LDDR},"lddr"},
	{{PFX_ED,CPI}, "cpi"}, {{PFX_ED,CPIR},"cpir"}, {{PFX_ED,CPD}, "cpd"}, {{PFX_ED,CPDR},"cpdr"},
	{{PFX_ED,INI}, "ini"}, {{PFX_ED,INIR},"inir"}, {{PFX_ED,IND}, "ind"}, {{PFX_ED,INDR},"indr"},
	{{PFX_ED,OUTI},"outi"},{{PFX_ED,OTDR},"otdr"}, {{PFX_ED,OUTD},"outd"},{{PFX_ED,OTDR},"otdr"},
};

static TestSet z180_tests[] =
{
	// tests only run for Z180
	// Z180 additional opcodes and illegal opcodes

	// SLL is illegal:

	{{PFX_CB,SLL_B},  "sll b ;"},
	{{PFX_CB,SLL_C},  "sll c ;"},
	{{PFX_CB,SLL_D},  "sll d ;"},
	{{PFX_CB,SLL_E},  "sll e ;"},
	{{PFX_CB,SLL_H},  "sll h ;"},
	{{PFX_CB,SLL_L},  "sll l ;"},
	{{PFX_CB,SLL_xHL},"sll (hl) ;"},
	{{PFX_CB,SLL_A},  "sll a ;"},

	{{PFX_IX,PFX_CB, 67,SLL_B},  "sll (ix+67),b ;"},
	{{PFX_IY,PFX_CB, 67,SLL_L},  "sll (iy+67),l ;"},
	{{PFX_IY,PFX_CB,199,SLL_xHL},"sll (iy-57) ;"},
	{{PFX_IX,PFX_CB,199,SLL_A},  "sll (ix-57),a ;"},

	// prefix IX/IY opcodes which don't use the HL register are illegal:

	{{PFX_IX,HALT},  "halt ;"},
	{{PFX_IX,LD_B_D},"ld b,d ;"},	{{PFX_IY,LD_B_C},"ld b,c ;"},
	{{PFX_IX,LD_C_E},"ld c,e ;"},	{{PFX_IY,LD_C_A},"ld c,a ;"},
	{{PFX_IY,LD_D_A},"ld d,a ;"},	{{PFX_IX,LD_D_C},"ld d,c ;"},
	{{PFX_IX,LD_E_B},"ld e,b ;"},	{{PFX_IY,LD_E_D},"ld e,d ;"},

	{{PFX_IY,ADD_B },"add a,b ;"},	{{PFX_IX,ADC_D },"adc a,d ;"},
	{{PFX_IY,SUB_C },"sub a,c ;"},	{{PFX_IX,SBC_E },"sbc a,e ;"},
	{{PFX_IX,AND_D },"and a,d ;"},	{{PFX_IY,XOR_A },"xor a,a ;"},
	{{PFX_IY,OR_E  },"or  a,e ;"},	{{PFX_IX,CP_B  },"cp  a,b ;"},
	{{PFX_IX,INC_A },"inc a ;"}, 	{{PFX_IY,DEC_C },"dec c ;"},
	{{PFX_IY,LD_B_N,5},"ld b,5 ;"}, {{PFX_IX,LD_D_N,223},"ld d,223 ;"},

	{{PFX_IX,LD_B_H},"ld b,xh ;"},	{{PFX_IY,LD_B_L},"ld b,yl ;"},
	{{PFX_IX,LD_C_H},"ld c,xh ;"},	{{PFX_IY,LD_C_L},"ld c,yl ;"},
	{{PFX_IY,LD_D_H},"ld d,yh ;"},	{{PFX_IX,LD_D_L},"ld d,xl ;"},
	{{PFX_IX,LD_E_H},"ld e,xh ;"},	{{PFX_IY,LD_E_L},"ld e,yl ;"},
	{{PFX_IY,LD_H_H},"ld yh,yh ;"},	{{PFX_IY,LD_H_L},"ld yh,yl ;"},
	{{PFX_IX,LD_L_H},"ld xl,xh ;"},	{{PFX_IX,LD_L_L},"ld xl,xl ;"},
	{{PFX_IX,LD_A_H},"ld a,xh ;"},	{{PFX_IY,LD_A_L},"ld a,yl ;"},
	{{PFX_IX,LD_H_B},"ld xh,b ;"},	{{PFX_IY,LD_L_B},"ld yl,b ;"},
	{{PFX_IX,LD_H_C},"ld xh,c ;"},	{{PFX_IY,LD_L_C},"ld yl,c ;"},
	{{PFX_IY,LD_H_D},"ld yh,d ;"},	{{PFX_IX,LD_L_D},"ld xl,d ;"},
	{{PFX_IX,LD_H_E},"ld xh,e ;"},	{{PFX_IX,LD_L_E},"ld xl,e ;"},
	{{PFX_IY,LD_H_A},"ld yh,a ;"},	{{PFX_IY,LD_L_A},"ld yl,a ;"},
	{{PFX_IY,ADD_H },"add a,yh ;"},	{{PFX_IX,ADC_H },"adc a,xh ;"},
	{{PFX_IY,SUB_H },"sub a,yh ;"},	{{PFX_IX,SBC_H },"sbc a,xh ;"},
	{{PFX_IX,AND_H },"and a,xh ;"},	{{PFX_IY,XOR_H },"xor a,yh ;"},
	{{PFX_IY,OR_H  },"or  a,yh ;"},	{{PFX_IX,CP_H  },"cp  a,xh ;"},
	{{PFX_IX,INC_H },"inc xh ;"}, 	{{PFX_IY,DEC_L },"dec yl ;"},
	{{PFX_IY,LD_H_N,5},"ld yh,5 ;"},{{PFX_IX,LD_L_N,223},"ld xl,223 ;"},

	{{PFX_IX,PFX_CB, 67,RLC_B}, "rlc (ix+67),b ;"},
	{{PFX_IX,PFX_CB,127,RRC_D}, "rrc (ix+127),d ;"},
	{{PFX_IY,PFX_CB,  0, RL_H}, "rl  (iy+0),h ;"},
	{{PFX_IY,PFX_CB,255, RR_C}, "rr  (iy-1),c ;"},
	{{PFX_IY,PFX_CB,200,SLA_E}, "sla (iy-56),e ;"},
	{{PFX_IX,PFX_CB,  1, RL_L}, "rl  (ix+1),l ;"},
	{{PFX_IX,PFX_CB,128,SRA_D}, "sra (ix-128),d ;"},
	{{PFX_IX,PFX_CB, 67,SRL_H}, "srl (ix+67),h ;"},
	{{PFX_IY,PFX_CB,  5, RL_A}, "rl  (iy+5),a ;"},
	{{PFX_IX,PFX_CB,  6,BIT0_E},"bit 0,(ix+6),e ;"},
	{{PFX_IY,PFX_CB,249,BIT1_L},"bit 1,(iy-7),l ;"},
	{{PFX_IX,PFX_CB,  8,BIT2_B},"bit 2,(ix+8),b ;"},
	{{PFX_IY,PFX_CB,  7,BIT3_H},"bit 3,(iy+7),h ;"},
	{{PFX_IY,PFX_CB,  6,BIT4_A},"bit 4,(iy+6),a ;"},
	{{PFX_IX,PFX_CB,  5,BIT5_C},"bit 5,(ix+5),c ;"},
	{{PFX_IX,PFX_CB,  4,BIT6_L},"bit 6,(ix+4),l ;"},
	{{PFX_IY,PFX_CB,  3,BIT7_B},"bit 7,(iy+3),b ;"},
	{{PFX_IY,PFX_CB,  2,SET0_D},"set 0,(iy+2),d ;"},
	{{PFX_IY,PFX_CB,  1,SET3_A},"set 3,(iy+1),a ;"},
	{{PFX_IX,PFX_CB,254,SET6_C},"set 6,(ix-2),c ;"},
	{{PFX_IX,PFX_CB,253,SET7_E},"set 7,(ix-3),e ;"},
	{{PFX_IX,PFX_CB,252,RES0_B},"res 0,(ix-4),b ;"},
	{{PFX_IY,PFX_CB,251,RES5_D},"res 5,(iy-5),d ;"},
	{{PFX_IX,PFX_CB,250,RES6_C},"res 6,(ix-6),c ;"},
	{{PFX_IY,PFX_CB,249,RES7_E},"res 7,(iy-7),e ;"},

	// prefix ED opcodes
	// all new opcodes here.
	// undocumented opcodes are illegal:

	{{PFX_ED,OUT_xC_0}, "nop ;"},

	{{PFX_ED,ED00,0x56},"in0 b,($56)"},	{{PFX_ED,ED01,0x56},"out0 ($56),b"},
	{{PFX_ED,ED08,0x56},"in0 c,($56)"},	{{PFX_ED,ED09,0x56},"out0 ($56),c"},
	{{PFX_ED,ED10,0x56},"in0 d,($56)"},	{{PFX_ED,ED11,0x56},"out0 ($56),d"},
	{{PFX_ED,ED18,0x56},"in0 e,($56)"},	{{PFX_ED,ED19,0x56},"out0 ($56),e"},
	{{PFX_ED,ED20,0x56},"in0 h,($56)"},	{{PFX_ED,ED21,0x56},"out0 ($56),h"},
	{{PFX_ED,ED28,0x56},"in0 l,($56)"},	{{PFX_ED,ED29,0x56},"out0 ($56),l"},
	{{PFX_ED,ED38,0x56},"in0 a,($56)"},	{{PFX_ED,ED39,0x56},"out0 ($56),a"},
	{{PFX_ED,ED30,0x56},"in0 f,($56)"},	{{PFX_ED,ED31,0x00},"nop ;"},	// <-- disass doesn't skip N; Z180 unknown, irrelevant!

	{{PFX_ED,ED02},"nop ;"},{{PFX_ED,ED03},"nop ;"},{{PFX_ED,ED04},"tst b"},{{PFX_ED,ED05},"nop ;"},
	{{PFX_ED,ED06},"nop ;"},{{PFX_ED,ED07},"nop ;"},{{PFX_ED,ED0A},"nop ;"},{{PFX_ED,ED0B},"nop ;"},
	{{PFX_ED,ED0C},"tst c"},{{PFX_ED,ED0D},"nop ;"},{{PFX_ED,ED0E},"nop ;"},{{PFX_ED,ED0F},"nop ;"},
	{{PFX_ED,ED12},"nop ;"},{{PFX_ED,ED13},"nop ;"},{{PFX_ED,ED14},"tst d"},{{PFX_ED,ED15},"nop ;"},
	{{PFX_ED,ED16},"nop ;"},{{PFX_ED,ED17},"nop ;"},{{PFX_ED,ED1A},"nop ;"},{{PFX_ED,ED1B},"nop ;"},
	{{PFX_ED,ED1C},"tst e"},{{PFX_ED,ED1D},"nop ;"},{{PFX_ED,ED1E},"nop ;"},{{PFX_ED,ED1F},"nop ;"},
	{{PFX_ED,ED22},"nop ;"},{{PFX_ED,ED23},"nop ;"},{{PFX_ED,ED24},"tst h"},{{PFX_ED,ED25},"nop ;"},
	{{PFX_ED,ED26},"nop ;"},{{PFX_ED,ED27},"nop ;"},{{PFX_ED,ED2A},"nop ;"},{{PFX_ED,ED2B},"nop ;"},
	{{PFX_ED,ED2C},"tst l"},{{PFX_ED,ED2D},"nop ;"},{{PFX_ED,ED2E},"nop ;"},{{PFX_ED,ED2F},"nop ;"},
	{{PFX_ED,ED32},"nop ;"},{{PFX_ED,ED33},"nop ;"},{{PFX_ED,ED3C},"tst a"},{{PFX_ED,ED34},"tst (hl)"},
	{{PFX_ED,ED35},"nop ;"},{{PFX_ED,ED36},"nop ;"},{{PFX_ED,ED37},"nop ;"},{{PFX_ED,ED3A},"nop ;"},
	{{PFX_ED,ED3B},"nop ;"},{{PFX_ED,ED3D},"nop ;"},{{PFX_ED,ED3E},"nop ;"},{{PFX_ED,ED3F},"nop ;"},

	{{PFX_ED,ED4C},"mlt bc"},{{PFX_ED,ED54},"nop ;"},{{PFX_ED,ED5C},"mlt de"},{{PFX_ED,ED64,89},"tst 89"},
	{{PFX_ED,ED6C},"mlt hl"},{{PFX_ED,ED74,89},"tstio 89"},{{PFX_ED,ED7C},"mlt sp"},{{PFX_ED,ED55},"nop ;"},
	{{PFX_ED,ED5D},"nop ;"},{{PFX_ED,ED65},"nop ;"},{{PFX_ED,ED6D},"nop ;"},{{PFX_ED,ED75},"nop ;"},
	{{PFX_ED,ED7D},"nop ;"},{{PFX_ED,ED4E},"nop ;"},{{PFX_ED,ED66},"nop ;"},{{PFX_ED,ED6E},"nop ;"},
	{{PFX_ED,ED76},"slp"  },{{PFX_ED,ED7E},"nop ;"},{{PFX_ED,ED77},"nop ;"},{{PFX_ED,ED7F},"nop ;"},

	{{PFX_ED,ED80},"nop ;"},{{PFX_ED,ED81},"nop ;"},{{PFX_ED,ED82},"nop ;"},{{PFX_ED,ED83},"otim"},
	{{PFX_ED,ED84},"nop ;"},{{PFX_ED,ED85},"nop ;"},{{PFX_ED,ED86},"nop ;"},{{PFX_ED,ED87},"nop ;"},
	{{PFX_ED,ED88},"nop ;"},{{PFX_ED,ED89},"nop ;"},{{PFX_ED,ED8A},"nop ;"},{{PFX_ED,ED8B},"otdm"},
	{{PFX_ED,ED8C},"nop ;"},{{PFX_ED,ED8D},"nop ;"},{{PFX_ED,ED8E},"nop ;"},{{PFX_ED,ED8F},"nop ;"},

	{{PFX_ED,ED90},"nop ;"},{{PFX_ED,ED91},"nop ;"},{{PFX_ED,ED92},"nop ;"},{{PFX_ED,ED93},"otimr"},
	{{PFX_ED,ED94},"nop ;"},{{PFX_ED,ED95},"nop ;"},{{PFX_ED,ED96},"nop ;"},{{PFX_ED,ED97},"nop ;"},
	{{PFX_ED,ED98},"nop ;"},{{PFX_ED,ED99},"nop ;"},{{PFX_ED,ED9A},"nop ;"},{{PFX_ED,ED9B},"otdmr"},
	{{PFX_ED,ED9C},"nop ;"},{{PFX_ED,ED9D},"nop ;"},{{PFX_ED,ED9E},"nop ;"},{{PFX_ED,ED9F},"nop ;"},

	{{PFX_ED,EDA4},"nop ;"},{{PFX_ED,EDA5},"nop ;"},{{PFX_ED,EDA6},"nop ;"},{{PFX_ED,EDA7},"nop ;"},
	{{PFX_ED,EDAC},"nop ;"},{{PFX_ED,EDAD},"nop ;"},{{PFX_ED,EDAE},"nop ;"},{{PFX_ED,EDAF},"nop ;"},
	{{PFX_ED,EDB4},"nop ;"},{{PFX_ED,EDB5},"nop ;"},{{PFX_ED,EDB6},"nop ;"},{{PFX_ED,EDB7},"nop ;"},
	{{PFX_ED,EDBC},"nop ;"},{{PFX_ED,EDBD},"nop ;"},{{PFX_ED,EDBE},"nop ;"},{{PFX_ED,EDBF},"nop ;"},

	{{PFX_ED,EDC0},"nop ;"},{{PFX_ED,EDC1},"nop ;"},{{PFX_ED,EDC2},"nop ;"},{{PFX_ED,EDC3},"nop ;"},
	{{PFX_ED,EDC4},"nop ;"},{{PFX_ED,EDC5},"nop ;"},{{PFX_ED,EDC6},"nop ;"},{{PFX_ED,EDC7},"nop ;"},
	{{PFX_ED,EDC8},"nop ;"},{{PFX_ED,EDC9},"nop ;"},{{PFX_ED,EDCA},"nop ;"},{{PFX_ED,EDCB},"nop ;"},
	{{PFX_ED,EDCC},"nop ;"},{{PFX_ED,EDCD},"nop ;"},{{PFX_ED,EDCE},"nop ;"},{{PFX_ED,EDCF},"nop ;"},
	{{PFX_ED,EDD0},"nop ;"},{{PFX_ED,EDD1},"nop ;"},{{PFX_ED,EDD2},"nop ;"},{{PFX_ED,EDD3},"nop ;"},
	{{PFX_ED,EDD4},"nop ;"},{{PFX_ED,EDD5},"nop ;"},{{PFX_ED,EDD6},"nop ;"},{{PFX_ED,EDD7},"nop ;"},
	{{PFX_ED,EDD8},"nop ;"},{{PFX_ED,EDD9},"nop ;"},{{PFX_ED,EDDA},"nop ;"},{{PFX_ED,EDDB},"nop ;"},
	{{PFX_ED,EDDC},"nop ;"},{{PFX_ED,EDDD},"nop ;"},{{PFX_ED,EDDE},"nop ;"},{{PFX_ED,EDDF},"nop ;"},
	{{PFX_ED,EDE0},"nop ;"},{{PFX_ED,EDE1},"nop ;"},{{PFX_ED,EDE2},"nop ;"},{{PFX_ED,EDE3},"nop ;"},
	{{PFX_ED,EDE4},"nop ;"},{{PFX_ED,EDE5},"nop ;"},{{PFX_ED,EDE6},"nop ;"},{{PFX_ED,EDE7},"nop ;"},
	{{PFX_ED,EDE8},"nop ;"},{{PFX_ED,EDE9},"nop ;"},{{PFX_ED,EDEA},"nop ;"},{{PFX_ED,EDEB},"nop ;"},
	{{PFX_ED,EDEC},"nop ;"},{{PFX_ED,EDED},"nop ;"},{{PFX_ED,EDEE},"nop ;"},{{PFX_ED,EDEF},"nop ;"},
	{{PFX_ED,EDF0},"nop ;"},{{PFX_ED,EDF1},"nop ;"},{{PFX_ED,EDF2},"nop ;"},{{PFX_ED,EDF3},"nop ;"},
	{{PFX_ED,EDF4},"nop ;"},{{PFX_ED,EDF5},"nop ;"},{{PFX_ED,EDF6},"nop ;"},{{PFX_ED,EDF7},"nop ;"},
	{{PFX_ED,EDF8},"nop ;"},{{PFX_ED,EDF9},"nop ;"},{{PFX_ED,EDFA},"nop ;"},{{PFX_ED,EDFB},"nop ;"},
	{{PFX_ED,EDFC},"nop ;"},{{PFX_ED,EDFD},"nop ;"},{{PFX_ED,EDFE},"nop ;"},{{PFX_ED,EDFF},"nop ;"},
};

static TestSet z80_tests[] =
{
	// tests only run for Z80
	// illegal CB and IXCB and ED opcodes
	// excluding ixcbr2 and ixcbxh opcodes

	// "valid": SLL
	// "valid": out (c),0

	{{PFX_CB,SLL_B},  "sll b"},	{{PFX_CB,SLL_C},  "sll c"},
	{{PFX_CB,SLL_D},  "sll d"},	{{PFX_CB,SLL_E},  "sll e"},
	{{PFX_CB,SLL_H},  "sll h"},	{{PFX_CB,SLL_L},  "sll l"},
	{{PFX_CB,SLL_A},  "sll a"},	{{PFX_CB,SLL_xHL},"sll (hl)"},

	{{PFX_IX,PFX_CB, 67,SLL_xHL},"sll (ix+67)"},
	{{PFX_IY,PFX_CB,189,SLL_xHL},"sll (iy-67)"},

	{{PFX_ED,OUT_xC_0},"out (c),0"},
	{{PFX_IX,HALT},  "halt ;"},

	// prefix IX/IY + byte register opcode

	{{PFX_IX,LD_B_H},"ld b,xh"},	{{PFX_IY,LD_B_L},"ld b,yl"},
	{{PFX_IX,LD_C_H},"ld c,xh"},	{{PFX_IY,LD_C_L},"ld c,yl"},
	{{PFX_IY,LD_D_H},"ld d,yh"},	{{PFX_IX,LD_D_L},"ld d,xl"},
	{{PFX_IX,LD_E_H},"ld e,xh"},	{{PFX_IY,LD_E_L},"ld e,yl"},
	{{PFX_IY,LD_H_H},"ld yh,yh"},	{{PFX_IY,LD_H_L},"ld yh,yl"},
	{{PFX_IX,LD_L_H},"ld xl,xh"},	{{PFX_IX,LD_L_L},"ld xl,xl"},
	{{PFX_IX,LD_A_H},"ld a,xh"},	{{PFX_IY,LD_A_L},"ld a,yl"},
	{{PFX_IX,LD_H_B},"ld xh,b"},	{{PFX_IY,LD_L_B},"ld yl,b"},
	{{PFX_IX,LD_H_C},"ld xh,c"},	{{PFX_IY,LD_L_C},"ld yl,c"},
	{{PFX_IY,LD_H_D},"ld yh,d"},	{{PFX_IX,LD_L_D},"ld xl,d"},
	{{PFX_IX,LD_H_E},"ld xh,e"},	{{PFX_IX,LD_L_E},"ld xl,e"},
	{{PFX_IY,LD_H_A},"ld yh,a"},	{{PFX_IY,LD_L_A},"ld yl,a"},
	{{PFX_IY,ADD_H },"add a,yh"},	{{PFX_IX,ADC_H },"adc a,xh"},
	{{PFX_IY,SUB_H },"sub a,yh"},	{{PFX_IX,SBC_H },"sbc a,xh"},
	{{PFX_IX,AND_H },"and a,xh"},	{{PFX_IY,XOR_H },"xor a,yh"},
	{{PFX_IY,OR_H  },"or  a,yh"},	{{PFX_IX,CP_H  },"cp  a,xh"},
	{{PFX_IX,INC_H },"inc xh"}, 	{{PFX_IY,DEC_L },"dec yl"},
	{{PFX_IY,LD_H_N,54},"ld yh,54"},{{PFX_IX,LD_L_N,223},"ld xl,223"},

	// undocumented prefix ED opcodes which do nothing:

	{{PFX_ED,ED00},"nop ;"},{{PFX_ED,ED01},"nop ;"},{{PFX_ED,ED02},"nop ;"},{{PFX_ED,ED03},"nop ;"},
	{{PFX_ED,ED04},"nop ;"},{{PFX_ED,ED05},"nop ;"},{{PFX_ED,ED06},"nop ;"},{{PFX_ED,ED07},"nop ;"},
	{{PFX_ED,ED08},"nop ;"},{{PFX_ED,ED09},"nop ;"},{{PFX_ED,ED0A},"nop ;"},{{PFX_ED,ED0B},"nop ;"},
	{{PFX_ED,ED0C},"nop ;"},{{PFX_ED,ED0D},"nop ;"},{{PFX_ED,ED0E},"nop ;"},{{PFX_ED,ED0F},"nop ;"},
	{{PFX_ED,ED10},"nop ;"},{{PFX_ED,ED11},"nop ;"},{{PFX_ED,ED12},"nop ;"},{{PFX_ED,ED13},"nop ;"},
	{{PFX_ED,ED14},"nop ;"},{{PFX_ED,ED15},"nop ;"},{{PFX_ED,ED16},"nop ;"},{{PFX_ED,ED17},"nop ;"},
	{{PFX_ED,ED18},"nop ;"},{{PFX_ED,ED19},"nop ;"},{{PFX_ED,ED1A},"nop ;"},{{PFX_ED,ED1B},"nop ;"},
	{{PFX_ED,ED1C},"nop ;"},{{PFX_ED,ED1D},"nop ;"},{{PFX_ED,ED1E},"nop ;"},{{PFX_ED,ED1F},"nop ;"},
	{{PFX_ED,ED20},"nop ;"},{{PFX_ED,ED21},"nop ;"},{{PFX_ED,ED22},"nop ;"},{{PFX_ED,ED23},"nop ;"},
	{{PFX_ED,ED24},"nop ;"},{{PFX_ED,ED25},"nop ;"},{{PFX_ED,ED26},"nop ;"},{{PFX_ED,ED27},"nop ;"},
	{{PFX_ED,ED28},"nop ;"},{{PFX_ED,ED29},"nop ;"},{{PFX_ED,ED2A},"nop ;"},{{PFX_ED,ED2B},"nop ;"},
	{{PFX_ED,ED2C},"nop ;"},{{PFX_ED,ED2D},"nop ;"},{{PFX_ED,ED2E},"nop ;"},{{PFX_ED,ED2F},"nop ;"},
	{{PFX_ED,ED30},"nop ;"},{{PFX_ED,ED31},"nop ;"},{{PFX_ED,ED32},"nop ;"},{{PFX_ED,ED33},"nop ;"},
	{{PFX_ED,ED34},"nop ;"},{{PFX_ED,ED35},"nop ;"},{{PFX_ED,ED36},"nop ;"},{{PFX_ED,ED37},"nop ;"},
	{{PFX_ED,ED38},"nop ;"},{{PFX_ED,ED39},"nop ;"},{{PFX_ED,ED3A},"nop ;"},{{PFX_ED,ED3B},"nop ;"},
	{{PFX_ED,ED3C},"nop ;"},{{PFX_ED,ED3D},"nop ;"},{{PFX_ED,ED3E},"nop ;"},{{PFX_ED,ED3F},"nop ;"},

	{{PFX_ED,ED80},"nop ;"},{{PFX_ED,ED81},"nop ;"},{{PFX_ED,ED82},"nop ;"},{{PFX_ED,ED83},"nop ;"},
	{{PFX_ED,ED84},"nop ;"},{{PFX_ED,ED85},"nop ;"},{{PFX_ED,ED86},"nop ;"},{{PFX_ED,ED87},"nop ;"},
	{{PFX_ED,ED88},"nop ;"},{{PFX_ED,ED89},"nop ;"},{{PFX_ED,ED8A},"nop ;"},{{PFX_ED,ED8B},"nop ;"},
	{{PFX_ED,ED8C},"nop ;"},{{PFX_ED,ED8D},"nop ;"},{{PFX_ED,ED8E},"nop ;"},{{PFX_ED,ED8F},"nop ;"},
	{{PFX_ED,ED90},"nop ;"},{{PFX_ED,ED91},"nop ;"},{{PFX_ED,ED92},"nop ;"},{{PFX_ED,ED93},"nop ;"},
	{{PFX_ED,ED94},"nop ;"},{{PFX_ED,ED95},"nop ;"},{{PFX_ED,ED96},"nop ;"},{{PFX_ED,ED97},"nop ;"},
	{{PFX_ED,ED98},"nop ;"},{{PFX_ED,ED99},"nop ;"},{{PFX_ED,ED9A},"nop ;"},{{PFX_ED,ED9B},"nop ;"},
	{{PFX_ED,ED9C},"nop ;"},{{PFX_ED,ED9D},"nop ;"},{{PFX_ED,ED9E},"nop ;"},{{PFX_ED,ED9F},"nop ;"},
	{{PFX_ED,EDA4},"nop ;"},{{PFX_ED,EDA5},"nop ;"},{{PFX_ED,EDA6},"nop ;"},{{PFX_ED,EDA7},"nop ;"},
	{{PFX_ED,EDAC},"nop ;"},{{PFX_ED,EDAD},"nop ;"},{{PFX_ED,EDAE},"nop ;"},{{PFX_ED,EDAF},"nop ;"},
	{{PFX_ED,EDB4},"nop ;"},{{PFX_ED,EDB5},"nop ;"},{{PFX_ED,EDB6},"nop ;"},{{PFX_ED,EDB7},"nop ;"},
	{{PFX_ED,EDBC},"nop ;"},{{PFX_ED,EDBD},"nop ;"},{{PFX_ED,EDBE},"nop ;"},{{PFX_ED,EDBF},"nop ;"},

	{{PFX_ED,EDC0},"nop ;"},{{PFX_ED,EDC1},"nop ;"},{{PFX_ED,EDC2},"nop ;"},{{PFX_ED,EDC3},"nop ;"},
	{{PFX_ED,EDC4},"nop ;"},{{PFX_ED,EDC5},"nop ;"},{{PFX_ED,EDC6},"nop ;"},{{PFX_ED,EDC7},"nop ;"},
	{{PFX_ED,EDC8},"nop ;"},{{PFX_ED,EDC9},"nop ;"},{{PFX_ED,EDCA},"nop ;"},{{PFX_ED,EDCB},"nop ;"},
	{{PFX_ED,EDCC},"nop ;"},{{PFX_ED,EDCD},"nop ;"},{{PFX_ED,EDCE},"nop ;"},{{PFX_ED,EDCF},"nop ;"},
	{{PFX_ED,EDD0},"nop ;"},{{PFX_ED,EDD1},"nop ;"},{{PFX_ED,EDD2},"nop ;"},{{PFX_ED,EDD3},"nop ;"},
	{{PFX_ED,EDD4},"nop ;"},{{PFX_ED,EDD5},"nop ;"},{{PFX_ED,EDD6},"nop ;"},{{PFX_ED,EDD7},"nop ;"},
	{{PFX_ED,EDD8},"nop ;"},{{PFX_ED,EDD9},"nop ;"},{{PFX_ED,EDDA},"nop ;"},{{PFX_ED,EDDB},"nop ;"},
	{{PFX_ED,EDDC},"nop ;"},{{PFX_ED,EDDD},"nop ;"},{{PFX_ED,EDDE},"nop ;"},{{PFX_ED,EDDF},"nop ;"},
	{{PFX_ED,EDE0},"nop ;"},{{PFX_ED,EDE1},"nop ;"},{{PFX_ED,EDE2},"nop ;"},{{PFX_ED,EDE3},"nop ;"},
	{{PFX_ED,EDE4},"nop ;"},{{PFX_ED,EDE5},"nop ;"},{{PFX_ED,EDE6},"nop ;"},{{PFX_ED,EDE7},"nop ;"},
	{{PFX_ED,EDE8},"nop ;"},{{PFX_ED,EDE9},"nop ;"},{{PFX_ED,EDEA},"nop ;"},{{PFX_ED,EDEB},"nop ;"},
	{{PFX_ED,EDEC},"nop ;"},{{PFX_ED,EDED},"nop ;"},{{PFX_ED,EDEE},"nop ;"},{{PFX_ED,EDEF},"nop ;"},
	{{PFX_ED,EDF0},"nop ;"},{{PFX_ED,EDF1},"nop ;"},{{PFX_ED,EDF2},"nop ;"},{{PFX_ED,EDF3},"nop ;"},
	{{PFX_ED,EDF4},"nop ;"},{{PFX_ED,EDF5},"nop ;"},{{PFX_ED,EDF6},"nop ;"},{{PFX_ED,EDF7},"nop ;"},
	{{PFX_ED,EDF8},"nop ;"},{{PFX_ED,EDF9},"nop ;"},{{PFX_ED,EDFA},"nop ;"},{{PFX_ED,EDFB},"nop ;"},
	{{PFX_ED,EDFC},"nop ;"},{{PFX_ED,EDFD},"nop ;"},{{PFX_ED,EDFE},"nop ;"},{{PFX_ED,EDFF},"nop ;"},

	// not fully decoded prefix ED opcodes:

	{{PFX_ED,NEG },"neg" },  {{PFX_ED,ED4C},"neg ;" },{{PFX_ED,ED54},"neg ;" },{{PFX_ED,ED5C},"neg ;"},
	{{PFX_ED,ED64},"neg ;"}, {{PFX_ED,ED6C},"neg ;" },{{PFX_ED,ED74},"neg ;" },{{PFX_ED,ED7C},"neg ;"},
	{{PFX_ED,RETN},"retn"},  {{PFX_ED,RETI},"reti"},  {{PFX_ED,ED55},"reti ;"},{{PFX_ED,ED5D},"reti ;"},
	{{PFX_ED,ED65},"reti ;"},{{PFX_ED,ED6D},"reti ;"},{{PFX_ED,ED75},"reti ;"},{{PFX_ED,ED7D},"reti ;"},
	{{PFX_ED,IM_0},"im 0"},  {{PFX_ED,ED4E},"im 0 ;"},{{PFX_ED,IM_1},"im 1"},  {{PFX_ED,IM_2},"im 2"},
	{{PFX_ED,ED66},"im 0 ;"},{{PFX_ED,ED6E},"im 0 ;"},{{PFX_ED,ED76},"im 1 ;"},{{PFX_ED,ED7E},"im 2 ;"},
	{{PFX_ED,ED77},"nop ;" },{{PFX_ED,ED7F},"nop ;"},
};

static TestSet z80_ixcbr2_tests[] =
{
	// tests for Z80 --ixcbr2 opcodes
	// testing the ixcbr2 opcodes

	{{PFX_IX,PFX_CB,55,RLC_B}, "rlc (ix+55),b"},{{PFX_IX,PFX_CB,55,RLC_H}, "rlc (ix+55),h"},
	{{PFX_IY,PFX_CB,55,RLC_C}, "rlc (iy+55),c"},{{PFX_IY,PFX_CB,55,RLC_L}, "rlc (iy+55),l"},
	{{PFX_IY,PFX_CB,55,RLC_D}, "rlc (iy+55),d"},{{PFX_IX,PFX_CB,55,RLC_A}, "rlc (ix+55),a"},
	{{PFX_IX,PFX_CB,55,RLC_E}, "rlc (ix+55),e"},{{PFX_IX,PFX_CB,55,RRC_B}, "rrc (ix+55),b"},
	{{PFX_IX,PFX_CB,55,RRC_C}, "rrc (ix+55),c"},{{PFX_IX,PFX_CB,55,RRC_D}, "rrc (ix+55),d"},
	{{PFX_IY,PFX_CB,55,RRC_E}, "rrc (iy+55),e"},{{PFX_IY,PFX_CB,55,RRC_H}, "rrc (iy+55),h"},
	{{PFX_IY,PFX_CB,55,RRC_L}, "rrc (iy+55),l"},{{PFX_IX,PFX_CB,55,RRC_A}, "rrc (ix+55),a"},
	{{PFX_IX,PFX_CB,55,RL_B},  "rl  (ix+55),b"},{{PFX_IX,PFX_CB,55,RL_C},  "rl  (ix+55),c"},
	{{PFX_IX,PFX_CB,55,RL_D},  "rl  (ix+55),d"},{{PFX_IX,PFX_CB,55,RL_E},  "rl  (ix+55),e"},
	{{PFX_IX,PFX_CB,55,RL_H},  "rl  (ix+55),h"},{{PFX_IY,PFX_CB,55,RL_L},  "rl  (iy+55),l"},
	{{PFX_IY,PFX_CB,55,RL_A},  "rl  (iy+55),a"},{{PFX_IY,PFX_CB,55,RR_B},  "rr  (iy+55),b"},
	{{PFX_IX,PFX_CB,55,RR_H},  "rr  (ix+55),h"},{{PFX_IX,PFX_CB,55,RR_A},  "rr  (ix+55),a"},
	{{PFX_IX,PFX_CB,55,RR_L},  "rr  (ix+55),l"},{{PFX_IX,PFX_CB,55,SLA_C}, "sla (ix+55),c"},
	{{PFX_IX,PFX_CB,55,SLA_L}, "sla (ix+55),l"},{{PFX_IX,PFX_CB,55,SLA_A}, "sla (ix+55),a"},
	{{PFX_IY,PFX_CB,55,SLA_H}, "sla (iy+55),h"},{{PFX_IY,PFX_CB,55,SRA_B}, "sra (iy+55),b"},
	{{PFX_IY,PFX_CB,55,SRA_H}, "sra (iy+55),h"},{{PFX_IX,PFX_CB,55,SRA_A}, "sra (ix+55),a"},
	{{PFX_IX,PFX_CB,55,SRA_L}, "sra (ix+55),l"},{{PFX_IX,PFX_CB,55,SLL_C}, "sll (ix+55),c"},
	{{PFX_IX,PFX_CB,55,SLL_L}, "sll (ix+55),l"},{{PFX_IX,PFX_CB,55,SLL_A}, "sll (ix+55),a"},
	{{PFX_IX,PFX_CB,55,SLL_H}, "sll (ix+55),h"},{{PFX_IX,PFX_CB,55,SRL_C}, "srl (ix+55),c"},
	{{PFX_IY,PFX_CB,55,SRL_L}, "srl (iy+55),l"},{{PFX_IY,PFX_CB,55,SRL_A}, "srl (iy+55),a"},
	{{PFX_IY,PFX_CB,55,SRL_H}, "srl (iy+55),h"},

	{{PFX_IX,PFX_CB,3,BIT0_B}, "bit 0,(ix+3),b"}, {{PFX_IX,PFX_CB,3,BIT0_C}, "bit 0,(ix+3),c"},
	{{PFX_IY,PFX_CB,3,BIT0_D}, "bit 0,(iy+3),d"}, {{PFX_IY,PFX_CB,3,BIT0_E}, "bit 0,(iy+3),e"},
	{{PFX_IY,PFX_CB,3,BIT0_H}, "bit 0,(iy+3),h"}, {{PFX_IX,PFX_CB,3,BIT0_L}, "bit 0,(ix+3),l"},
	{{PFX_IX,PFX_CB,3,BIT0_A}, "bit 0,(ix+3),a"}, {{PFX_IX,PFX_CB,3,BIT1_B}, "bit 1,(ix+3),b"},
	{{PFX_IX,PFX_CB,3,BIT1_C}, "bit 1,(ix+3),c"}, {{PFX_IX,PFX_CB,3,BIT2_D}, "bit 2,(ix+3),d"},
	{{PFX_IX,PFX_CB,3,BIT3_E}, "bit 3,(ix+3),e"}, {{PFX_IY,PFX_CB,3,BIT3_H}, "bit 3,(iy+3),h"},
	{{PFX_IY,PFX_CB,3,BIT4_L}, "bit 4,(iy+3),l"}, {{PFX_IY,PFX_CB,3,BIT5_A}, "bit 5,(iy+3),a"},
	{{PFX_IX,PFX_CB,3,BIT7_B}, "bit 7,(ix+3),b"}, {{PFX_IX,PFX_CB,3,BIT7_C}, "bit 7,(ix+3),c"},
	{{PFX_IX,PFX_CB,3,BIT7_D}, "bit 7,(ix+3),d"}, {{PFX_IX,PFX_CB,3,BIT7_E}, "bit 7,(ix+3),e"},
	{{PFX_IY,PFX_CB,3,BIT7_H}, "bit 7,(iy+3),h"}, {{PFX_IY,PFX_CB,3,BIT7_L}, "bit 7,(iy+3),l"},
	{{PFX_IY,PFX_CB,3,BIT7_A}, "bit 7,(iy+3),a"},

	{{PFX_IX,PFX_CB,3,SET0_B}, "set 0,(ix+3),b"}, {{PFX_IX,PFX_CB,3,SET0_C}, "set 0,(ix+3),c"},
	{{PFX_IY,PFX_CB,3,SET0_D}, "set 0,(iy+3),d"}, {{PFX_IY,PFX_CB,3,SET0_E}, "set 0,(iy+3),e"},
	{{PFX_IY,PFX_CB,3,SET0_H}, "set 0,(iy+3),h"}, {{PFX_IX,PFX_CB,3,SET0_L}, "set 0,(ix+3),l"},
	{{PFX_IX,PFX_CB,3,SET0_A}, "set 0,(ix+3),a"}, {{PFX_IX,PFX_CB,3,SET1_B}, "set 1,(ix+3),b"},
	{{PFX_IX,PFX_CB,3,SET1_C}, "set 1,(ix+3),c"}, {{PFX_IX,PFX_CB,3,SET2_D}, "set 2,(ix+3),d"},
	{{PFX_IX,PFX_CB,3,SET3_E}, "set 3,(ix+3),e"}, {{PFX_IY,PFX_CB,3,SET3_H}, "set 3,(iy+3),h"},
	{{PFX_IY,PFX_CB,3,SET4_L}, "set 4,(iy+3),l"}, {{PFX_IY,PFX_CB,3,SET5_A}, "set 5,(iy+3),a"},
	{{PFX_IX,PFX_CB,3,SET7_B}, "set 7,(ix+3),b"}, {{PFX_IX,PFX_CB,3,SET7_C}, "set 7,(ix+3),c"},
	{{PFX_IX,PFX_CB,3,SET7_D}, "set 7,(ix+3),d"}, {{PFX_IX,PFX_CB,3,SET7_E}, "set 7,(ix+3),e"},
	{{PFX_IY,PFX_CB,3,SET7_H}, "set 7,(iy+3),h"}, {{PFX_IY,PFX_CB,3,SET7_L}, "set 7,(iy+3),l"},
	{{PFX_IY,PFX_CB,3,SET7_A}, "set 7,(iy+3),a"},

	{{PFX_IX,PFX_CB,4,RES0_B}, "res 0,(ix+4),b"}, {{PFX_IX,PFX_CB,250,RES0_C}, "res 0,(ix-6),c"},
	{{PFX_IY,PFX_CB,4,RES0_D}, "res 0,(iy+4),d"}, {{PFX_IY,PFX_CB,250,RES0_E}, "res 0,(iy-6),e"},
	{{PFX_IY,PFX_CB,4,RES0_H}, "res 0,(iy+4),h"}, {{PFX_IX,PFX_CB,250,RES0_L}, "res 0,(ix-6),l"},
	{{PFX_IX,PFX_CB,4,RES0_A}, "res 0,(ix+4),a"}, {{PFX_IX,PFX_CB,250,RES1_B}, "res 1,(ix-6),b"},
	{{PFX_IX,PFX_CB,4,RES1_C}, "res 1,(ix+4),c"}, {{PFX_IX,PFX_CB,250,RES2_D}, "res 2,(ix-6),d"},
	{{PFX_IX,PFX_CB,4,RES3_E}, "res 3,(ix+4),e"}, {{PFX_IY,PFX_CB,250,RES3_H}, "res 3,(iy-6),h"},
	{{PFX_IY,PFX_CB,4,RES4_L}, "res 4,(iy+4),l"}, {{PFX_IY,PFX_CB,250,RES5_A}, "res 5,(iy-6),a"},
	{{PFX_IX,PFX_CB,4,RES7_B}, "res 7,(ix+4),b"}, {{PFX_IX,PFX_CB,250,RES7_C}, "res 7,(ix-6),c"},
	{{PFX_IX,PFX_CB,4,RES7_D}, "res 7,(ix+4),d"}, {{PFX_IX,PFX_CB,250,RES7_E}, "res 7,(ix-6),e"},
	{{PFX_IY,PFX_CB,4,RES7_H}, "res 7,(iy+4),h"}, {{PFX_IY,PFX_CB,250,RES7_L}, "res 7,(iy-6),l"},
	{{PFX_IY,PFX_CB,4,RES7_A}, "res 7,(iy+4),a"},
};

static TestSet z80_noopt_tests[] =
{
	// tests for Z80 without --ixcbr2 or --ixcbxh

	// IXCB opcodes are disassembled as for --ixcbr2
	// but a warning comment is appended

	{{PFX_IX,PFX_CB,55,RLC_B}, "rlc (ix+55),b ;"},{{PFX_IX,PFX_CB,55,RLC_H}, "rlc (ix+55),h ;"},
	{{PFX_IY,PFX_CB,55,RLC_C}, "rlc (iy+55),c ;"},{{PFX_IY,PFX_CB,55,RLC_L}, "rlc (iy+55),l ;"},
	{{PFX_IY,PFX_CB,55,RLC_D}, "rlc (iy+55),d ;"},{{PFX_IX,PFX_CB,55,RLC_A}, "rlc (ix+55),a ;"},
	{{PFX_IX,PFX_CB,55,RLC_E}, "rlc (ix+55),e ;"},{{PFX_IX,PFX_CB,55,RRC_B}, "rrc (ix+55),b ;"},
	{{PFX_IX,PFX_CB,55,RRC_C}, "rrc (ix+55),c ;"},{{PFX_IX,PFX_CB,55,RRC_D}, "rrc (ix+55),d ;"},
	{{PFX_IY,PFX_CB,55,RRC_E}, "rrc (iy+55),e ;"},{{PFX_IY,PFX_CB,55,RRC_H}, "rrc (iy+55),h ;"},
	{{PFX_IY,PFX_CB,55,RRC_L}, "rrc (iy+55),l ;"},{{PFX_IX,PFX_CB,55,RRC_A}, "rrc (ix+55),a ;"},
	{{PFX_IX,PFX_CB,55,RL_B},  "rl  (ix+55),b ;"},{{PFX_IX,PFX_CB,55,RL_C},  "rl  (ix+55),c ;"},
	{{PFX_IX,PFX_CB,55,RL_D},  "rl  (ix+55),d ;"},{{PFX_IX,PFX_CB,55,RL_E},  "rl  (ix+55),e ;"},
	{{PFX_IX,PFX_CB,55,RL_H},  "rl  (ix+55),h ;"},{{PFX_IY,PFX_CB,55,RL_L},  "rl  (iy+55),l ;"},
	{{PFX_IY,PFX_CB,55,RL_A},  "rl  (iy+55),a ;"},{{PFX_IY,PFX_CB,55,RR_B},  "rr  (iy+55),b ;"},
	{{PFX_IX,PFX_CB,55,RR_H},  "rr  (ix+55),h ;"},{{PFX_IX,PFX_CB,55,RR_A},  "rr  (ix+55),a ;"},
	{{PFX_IX,PFX_CB,55,RR_L},  "rr  (ix+55),l ;"},{{PFX_IX,PFX_CB,55,SLA_C}, "sla (ix+55),c ;"},
	{{PFX_IX,PFX_CB,55,SLA_L}, "sla (ix+55),l ;"},{{PFX_IX,PFX_CB,55,SLA_A}, "sla (ix+55),a ;"},
	{{PFX_IY,PFX_CB,55,SLA_H}, "sla (iy+55),h ;"},{{PFX_IY,PFX_CB,55,SRA_B}, "sra (iy+55),b ;"},
	{{PFX_IY,PFX_CB,55,SRA_H}, "sra (iy+55),h ;"},{{PFX_IX,PFX_CB,55,SRA_A}, "sra (ix+55),a ;"},
	{{PFX_IX,PFX_CB,55,SRA_L}, "sra (ix+55),l ;"},{{PFX_IX,PFX_CB,55,SLL_C}, "sll (ix+55),c ;"},
	{{PFX_IX,PFX_CB,55,SLL_L}, "sll (ix+55),l ;"},{{PFX_IX,PFX_CB,55,SLL_A}, "sll (ix+55),a ;"},
	{{PFX_IX,PFX_CB,55,SLL_H}, "sll (ix+55),h ;"},{{PFX_IX,PFX_CB,55,SRL_C}, "srl (ix+55),c ;"},
	{{PFX_IY,PFX_CB,55,SRL_L}, "srl (iy+55),l ;"},{{PFX_IY,PFX_CB,55,SRL_A}, "srl (iy+55),a ;"},
	{{PFX_IY,PFX_CB,55,SRL_H}, "srl (iy+55),h ;"},

	{{PFX_IX,PFX_CB,3,BIT0_B}, "bit 0,(ix+3),b ;"}, {{PFX_IX,PFX_CB,3,BIT0_C}, "bit 0,(ix+3),c ;"},
	{{PFX_IY,PFX_CB,3,BIT0_D}, "bit 0,(iy+3),d ;"}, {{PFX_IY,PFX_CB,3,BIT0_E}, "bit 0,(iy+3),e ;"},
	{{PFX_IY,PFX_CB,3,BIT0_H}, "bit 0,(iy+3),h ;"}, {{PFX_IX,PFX_CB,3,BIT0_L}, "bit 0,(ix+3),l ;"},
	{{PFX_IX,PFX_CB,3,BIT0_A}, "bit 0,(ix+3),a ;"}, {{PFX_IX,PFX_CB,3,BIT1_B}, "bit 1,(ix+3),b ;"},
	{{PFX_IX,PFX_CB,3,BIT1_C}, "bit 1,(ix+3),c ;"}, {{PFX_IX,PFX_CB,3,BIT2_D}, "bit 2,(ix+3),d ;"},
	{{PFX_IX,PFX_CB,3,BIT3_E}, "bit 3,(ix+3),e ;"}, {{PFX_IY,PFX_CB,3,BIT3_H}, "bit 3,(iy+3),h ;"},
	{{PFX_IY,PFX_CB,3,BIT4_L}, "bit 4,(iy+3),l ;"}, {{PFX_IY,PFX_CB,3,BIT5_A}, "bit 5,(iy+3),a ;"},
	{{PFX_IX,PFX_CB,3,BIT7_B}, "bit 7,(ix+3),b ;"}, {{PFX_IX,PFX_CB,3,BIT7_C}, "bit 7,(ix+3),c ;"},
	{{PFX_IX,PFX_CB,3,BIT7_D}, "bit 7,(ix+3),d ;"}, {{PFX_IX,PFX_CB,3,BIT7_E}, "bit 7,(ix+3),e ;"},
	{{PFX_IY,PFX_CB,3,BIT7_H}, "bit 7,(iy+3),h ;"}, {{PFX_IY,PFX_CB,3,BIT7_L}, "bit 7,(iy+3),l ;"},
	{{PFX_IY,PFX_CB,3,BIT7_A}, "bit 7,(iy+3),a ;"},

	{{PFX_IX,PFX_CB,3,SET0_B}, "set 0,(ix+3),b ;"}, {{PFX_IX,PFX_CB,3,SET0_C}, "set 0,(ix+3),c ;"},
	{{PFX_IY,PFX_CB,3,SET0_D}, "set 0,(iy+3),d ;"}, {{PFX_IY,PFX_CB,3,SET0_E}, "set 0,(iy+3),e ;"},
	{{PFX_IY,PFX_CB,3,SET0_H}, "set 0,(iy+3),h ;"}, {{PFX_IX,PFX_CB,3,SET0_L}, "set 0,(ix+3),l ;"},
	{{PFX_IX,PFX_CB,3,SET0_A}, "set 0,(ix+3),a ;"}, {{PFX_IX,PFX_CB,3,SET1_B}, "set 1,(ix+3),b ;"},
	{{PFX_IX,PFX_CB,3,SET1_C}, "set 1,(ix+3),c ;"}, {{PFX_IX,PFX_CB,3,SET2_D}, "set 2,(ix+3),d ;"},
	{{PFX_IX,PFX_CB,3,SET3_E}, "set 3,(ix+3),e ;"}, {{PFX_IY,PFX_CB,3,SET3_H}, "set 3,(iy+3),h ;"},
	{{PFX_IY,PFX_CB,3,SET4_L}, "set 4,(iy+3),l ;"}, {{PFX_IY,PFX_CB,3,SET5_A}, "set 5,(iy+3),a ;"},
	{{PFX_IX,PFX_CB,3,SET7_B}, "set 7,(ix+3),b ;"}, {{PFX_IX,PFX_CB,3,SET7_C}, "set 7,(ix+3),c ;"},
	{{PFX_IX,PFX_CB,3,SET7_D}, "set 7,(ix+3),d ;"}, {{PFX_IX,PFX_CB,3,SET7_E}, "set 7,(ix+3),e ;"},
	{{PFX_IY,PFX_CB,3,SET7_H}, "set 7,(iy+3),h ;"}, {{PFX_IY,PFX_CB,3,SET7_L}, "set 7,(iy+3),l ;"},
	{{PFX_IY,PFX_CB,3,SET7_A}, "set 7,(iy+3),a ;"},

	{{PFX_IX,PFX_CB,4,RES0_B}, "res 0,(ix+4),b ;"}, {{PFX_IX,PFX_CB,250,RES0_C}, "res 0,(ix-6),c ;"},
	{{PFX_IY,PFX_CB,4,RES0_D}, "res 0,(iy+4),d ;"}, {{PFX_IY,PFX_CB,250,RES0_E}, "res 0,(iy-6),e ;"},
	{{PFX_IY,PFX_CB,4,RES0_H}, "res 0,(iy+4),h ;"}, {{PFX_IX,PFX_CB,250,RES0_L}, "res 0,(ix-6),l ;"},
	{{PFX_IX,PFX_CB,4,RES0_A}, "res 0,(ix+4),a ;"}, {{PFX_IX,PFX_CB,250,RES1_B}, "res 1,(ix-6),b ;"},
	{{PFX_IX,PFX_CB,4,RES1_C}, "res 1,(ix+4),c ;"}, {{PFX_IX,PFX_CB,250,RES2_D}, "res 2,(ix-6),d ;"},
	{{PFX_IX,PFX_CB,4,RES3_E}, "res 3,(ix+4),e ;"}, {{PFX_IY,PFX_CB,250,RES3_H}, "res 3,(iy-6),h ;"},
	{{PFX_IY,PFX_CB,4,RES4_L}, "res 4,(iy+4),l ;"}, {{PFX_IY,PFX_CB,250,RES5_A}, "res 5,(iy-6),a ;"},
	{{PFX_IX,PFX_CB,4,RES7_B}, "res 7,(ix+4),b ;"}, {{PFX_IX,PFX_CB,250,RES7_C}, "res 7,(ix-6),c ;"},
	{{PFX_IX,PFX_CB,4,RES7_D}, "res 7,(ix+4),d ;"}, {{PFX_IX,PFX_CB,250,RES7_E}, "res 7,(ix-6),e ;"},
	{{PFX_IY,PFX_CB,4,RES7_H}, "res 7,(iy+4),h ;"}, {{PFX_IY,PFX_CB,250,RES7_L}, "res 7,(iy-6),l ;"},
	{{PFX_IY,PFX_CB,4,RES7_A}, "res 7,(iy+4),a ;"},
};

static TestSet z80_ixcbxh_tests[] =
{
	// tests for Z80 --ixcbxh opcodes
	// testing the ixcbxh opcodes

{{PFX_IX,PFX_CB,55,RLC_B}, "rlc b ;"}, {{PFX_IX,PFX_CB,55,RLC_H}, "rlc xh"},
{{PFX_IY,PFX_CB,55,RLC_C}, "rlc c ;"}, {{PFX_IY,PFX_CB,55,RLC_L}, "rlc yl"},
{{PFX_IY,PFX_CB,55,RLC_D}, "rlc d ;"}, {{PFX_IX,PFX_CB,55,RLC_A}, "rlc a ;"},
{{PFX_IX,PFX_CB,55,RLC_E}, "rlc e ;"}, {{PFX_IY,PFX_CB,55,RLC_H}, "rlc yh"},
{{PFX_IX,PFX_CB,55,RRC_B}, "rrc b ;"}, {{PFX_IX,PFX_CB,55,RRC_H}, "rrc xh"},
{{PFX_IY,PFX_CB,55,RRC_C}, "rrc c ;"}, {{PFX_IY,PFX_CB,55,RRC_L}, "rrc yl"},
{{PFX_IY,PFX_CB,55,RRC_D}, "rrc d ;"}, {{PFX_IX,PFX_CB,55,RRC_A}, "rrc a ;"},
{{PFX_IX,PFX_CB,55,RRC_E}, "rrc e ;"}, {{PFX_IX,PFX_CB,55,RRC_L}, "rrc xl"},
{{PFX_IX,PFX_CB,55,RL_B},  "rl  b ;"}, {{PFX_IX,PFX_CB,55,RL_H }, "rl  xh"},
{{PFX_IX,PFX_CB,55,RL_C},  "rl  c ;"}, {{PFX_IX,PFX_CB,55,RL_L }, "rl  xl"},
{{PFX_IX,PFX_CB,55,RL_D},  "rl  d ;"}, {{PFX_IY,PFX_CB,55,RL_A }, "rl  a ;"},
{{PFX_IY,PFX_CB,55,RL_E},  "rl  e ;"}, {{PFX_IY,PFX_CB,55,RL_L }, "rl  yl"},
{{PFX_IX,PFX_CB,55,RR_B},  "rr  b ;"}, {{PFX_IX,PFX_CB,55,RR_H }, "rr  xh"},
{{PFX_IX,PFX_CB,55,RR_D},  "rr  d ;"}, {{PFX_IY,PFX_CB,55,RR_L }, "rr  yl"},
{{PFX_IX,PFX_CB,55,SLA_B}, "sla b ;"}, {{PFX_IY,PFX_CB,55,SLA_H}, "sla yh"},
{{PFX_IX,PFX_CB,55,SLA_D}, "sla d ;"}, {{PFX_IX,PFX_CB,55,SLA_L}, "sla xl"},
{{PFX_IX,PFX_CB,55,SRA_C}, "sra c ;"}, {{PFX_IY,PFX_CB,55,SRA_H}, "sra yh"},
{{PFX_IX,PFX_CB,55,SRA_E}, "sra e ;"}, {{PFX_IX,PFX_CB,55,SRA_L}, "sra xl"},
{{PFX_IX,PFX_CB,55,SLL_B}, "sll b ;"}, {{PFX_IY,PFX_CB,55,SLL_H}, "sll yh"},
{{PFX_IX,PFX_CB,55,SLL_D}, "sll d ;"}, {{PFX_IX,PFX_CB,55,SLL_L}, "sll xl"},
{{PFX_IX,PFX_CB,55,SRL_C}, "srl c ;"}, {{PFX_IY,PFX_CB,55,SRL_H}, "srl yh"},
{{PFX_IX,PFX_CB,55,SRL_E}, "srl e ;"}, {{PFX_IX,PFX_CB,55,SRL_L}, "srl xl"},

{{PFX_IX,PFX_CB,3,BIT0_B}, "bit 0,b ;"}, {{PFX_IX,PFX_CB,3,BIT0_H}, "bit 0,xh"},
{{PFX_IY,PFX_CB,3,BIT0_C}, "bit 0,c ;"}, {{PFX_IY,PFX_CB,3,BIT0_L}, "bit 0,yl"},
{{PFX_IX,PFX_CB,3,BIT1_D}, "bit 1,d ;"}, {{PFX_IX,PFX_CB,3,BIT1_H}, "bit 1,xh"},
{{PFX_IY,PFX_CB,3,BIT1_E}, "bit 1,e ;"}, {{PFX_IY,PFX_CB,3,BIT1_L}, "bit 1,yl"},
{{PFX_IX,PFX_CB,3,BIT2_B}, "bit 2,b ;"}, {{PFX_IY,PFX_CB,3,BIT2_H}, "bit 2,yh"},
{{PFX_IY,PFX_CB,3,BIT3_C}, "bit 3,c ;"}, {{PFX_IX,PFX_CB,3,BIT3_H}, "bit 3,xh"},
{{PFX_IX,PFX_CB,3,BIT4_D}, "bit 4,d ;"}, {{PFX_IX,PFX_CB,3,BIT4_H}, "bit 4,xh"},
{{PFX_IY,PFX_CB,3,BIT5_E}, "bit 5,e ;"}, {{PFX_IX,PFX_CB,3,BIT5_L}, "bit 5,xl"},
{{PFX_IX,PFX_CB,3,BIT6_A}, "bit 6,a ;"}, {{PFX_IY,PFX_CB,3,BIT6_L}, "bit 6,yl"},
{{PFX_IY,PFX_CB,3,BIT7_B}, "bit 7,b ;"}, {{PFX_IY,PFX_CB,3,BIT7_L}, "bit 7,yl"},
{{PFX_IX,PFX_CB,3,SET0_B}, "set 0,b ;"}, {{PFX_IX,PFX_CB,3,SET0_H}, "set 0,xh"},
{{PFX_IY,PFX_CB,3,SET0_C}, "set 0,c ;"}, {{PFX_IY,PFX_CB,3,SET0_L}, "set 0,yl"},
{{PFX_IX,PFX_CB,3,SET1_D}, "set 1,d ;"}, {{PFX_IX,PFX_CB,3,SET1_H}, "set 1,xh"},
{{PFX_IY,PFX_CB,3,SET1_E}, "set 1,e ;"}, {{PFX_IY,PFX_CB,3,SET1_L}, "set 1,yl"},
{{PFX_IX,PFX_CB,3,SET2_B}, "set 2,b ;"}, {{PFX_IY,PFX_CB,3,SET2_H}, "set 2,yh"},
{{PFX_IY,PFX_CB,3,SET3_C}, "set 3,c ;"}, {{PFX_IX,PFX_CB,3,SET3_H}, "set 3,xh"},
{{PFX_IX,PFX_CB,3,SET4_D}, "set 4,d ;"}, {{PFX_IX,PFX_CB,3,SET4_H}, "set 4,xh"},
{{PFX_IY,PFX_CB,3,SET5_E}, "set 5,e ;"}, {{PFX_IX,PFX_CB,3,SET5_L}, "set 5,xl"},
{{PFX_IX,PFX_CB,3,SET6_A}, "set 6,a ;"}, {{PFX_IY,PFX_CB,3,SET6_L}, "set 6,yl"},
{{PFX_IY,PFX_CB,3,SET7_B}, "set 7,b ;"}, {{PFX_IY,PFX_CB,3,SET7_L}, "set 7,yl"},
{{PFX_IX,PFX_CB,3,RES0_B}, "res 0,b ;"}, {{PFX_IX,PFX_CB,3,RES0_H}, "res 0,xh"},
{{PFX_IY,PFX_CB,3,RES0_C}, "res 0,c ;"}, {{PFX_IY,PFX_CB,3,RES0_L}, "res 0,yl"},
{{PFX_IX,PFX_CB,3,RES1_D}, "res 1,d ;"}, {{PFX_IX,PFX_CB,3,RES1_H}, "res 1,xh"},
{{PFX_IY,PFX_CB,3,RES1_E}, "res 1,e ;"}, {{PFX_IY,PFX_CB,3,RES1_L}, "res 1,yl"},
{{PFX_IX,PFX_CB,3,RES2_B}, "res 2,b ;"}, {{PFX_IY,PFX_CB,3,RES2_H}, "res 2,yh"},
{{PFX_IY,PFX_CB,3,RES3_C}, "res 3,c ;"}, {{PFX_IX,PFX_CB,3,RES3_H}, "res 3,xh"},
{{PFX_IX,PFX_CB,3,RES4_D}, "res 4,d ;"}, {{PFX_IX,PFX_CB,3,RES4_H}, "res 4,xh"},
{{PFX_IY,PFX_CB,3,RES5_E}, "res 5,e ;"}, {{PFX_IX,PFX_CB,3,RES5_L}, "res 5,xl"},
{{PFX_IX,PFX_CB,3,RES6_A}, "res 6,a ;"}, {{PFX_IY,PFX_CB,3,RES6_L}, "res 6,yl"},
{{PFX_IY,PFX_CB,3,RES7_B}, "res 7,b ;"}, {{PFX_IY,PFX_CB,3,RES7_L}, "res 7,yl"},
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


static void run_tests (uint& num_tests, uint& num_errors, CpuID cpu_id, TestSet* tests, uint nelem)
{
	TRY
		for (uint i=0; i < nelem; )
		{
			TestSet& test = tests[i++];
			log("%02x%02x%02x%02x ", test.code[0],test.code[1],test.code[2],test.code[3]);
			if ((i&15)==0) logNl();

			uint16 addr = 0;
			cstr disassembly = disassemble(cpu_id, test.code, addr);
			assert_same(test.expected,disassembly);
			assert(addr>0 && addr<=4);

			//assert(!!strchr(disassembly,'*') == (opcode_legal_state(cpu_id,test.code,0x0000) > UNDOCUMENTED_OPCODE));
			if (!!strchr(disassembly,'*') != (opcode_validity(cpu_id,test.code,0x0000) > UNDOCUMENTED_OPCODE))
			{
				static cstr name[]={"LEGAL","UNDOCUMENTED","DEPRECATED","ILLEGAL"};
				log("\n\"%s\": wrong opcode legal state: he: %s ", disassembly, name[opcode_validity(cpu_id, test.code, 0)]);
				num_errors++;
			}

			//assert(addr == opcode_length(cpu_id, test.code));
			if (addr != opcode_length(cpu_id, test.code))
			{
				log("\nwrong opcode length: me: %i, he: %i ", addr,opcode_length(cpu_id, test.code));
				num_errors++;
			}
		}
	END
	logline("##");
}

static void test_disass_asm8080 (uint& num_tests, uint& num_errors)
{
	logIn("test disass --asm8080");

	for (uint i=0; i < NELEM(asm8080_tests); )
	{
		TestSet& test = asm8080_tests[i++];
		log("%02x ", test.code[0]);
		if ((i&31)==0) logNl();

		TRY
			uint16 addr = 0;
			cstr disassembly = disassemble_8080(test.code, addr);
			assert_same(test.expected,disassembly);
			assert(addr>0 && addr<=4);
			assert(addr==opcode_length(Cpu8080, test.code));
			assert(!!strchr(disassembly,';') == !!opcode_validity(Cpu8080,test.code,0x0000));
		END
	}
	logline("##");
}

static void test_disass_z80 (uint& num_tests, uint& num_errors, CpuID cpu_id)
{
	cstr cpu_str = cpu_id==Cpu8080?"8080":cpu_id==CpuZ180?"Z180":"Z80";
	cstr opt_str = cpu_id==CpuZ80_ixcbr2 ? " --ixcbr2" : cpu_id == CpuZ80_ixcbxh ? " --ixcbxh" : "";
	logIn("test disass --cpu=%s%s",cpu_str,opt_str);

	logline("%s: common tests",cpu_str);
	run_tests(num_tests,num_errors,cpu_id,common_tests,NELEM(common_tests));

	if (cpu_id == Cpu8080)
	{
		logline("i8080 tests");
		run_tests(num_tests,num_errors,cpu_id,i8080_tests,NELEM(i8080_tests));
	}
	else if (cpu_id == CpuZ180)
	{
		logline("z180 tests");
		run_tests(num_tests,num_errors,cpu_id,z180_tests,NELEM(z180_tests));
		logline("z180: z80/z180 tests");
		run_tests(num_tests,num_errors,cpu_id,z80_z180_tests,NELEM(z80_z180_tests));
	}
	else // if (cpu_id == CpuZ80)
	{
		logline("z80 tests");
		run_tests(num_tests,num_errors,cpu_id,z80_tests,NELEM(z80_tests));
		logline("z80: z80/z180 tests");
		run_tests(num_tests,num_errors,cpu_id,z80_z180_tests,NELEM(z80_z180_tests));

		if (cpu_id == CpuZ80_ixcbr2)
		{
			logline("z80 ixcbr2 tests");
			run_tests(num_tests,num_errors,cpu_id,z80_ixcbr2_tests,NELEM(z80_ixcbr2_tests));
		}
		else if (cpu_id == CpuZ80_ixcbxh)
		{
			logline("z80 ixcbxh tests");
			run_tests(num_tests,num_errors,cpu_id,z80_ixcbxh_tests,NELEM(z80_ixcbxh_tests));
		}
		else
		{
			logline("z80 no-opt tests");
			run_tests(num_tests,num_errors,cpu_id,z80_noopt_tests,NELEM(z80_noopt_tests));
		}
	}

}


void test_z80_disass (uint& num_tests, uint& num_errors)
{
	logIn("test z80_disass");

	test_disass_z80(num_tests,num_errors,CpuZ80);
	test_disass_z80(num_tests,num_errors,CpuZ80_ixcbr2);
	test_disass_z80(num_tests,num_errors,CpuZ80_ixcbxh);
	test_disass_z80(num_tests,num_errors,CpuZ180);
	test_disass_z80(num_tests,num_errors,Cpu8080);
	test_disass_asm8080(num_tests,num_errors);
}







