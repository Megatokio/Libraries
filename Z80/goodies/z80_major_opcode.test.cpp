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
	uint8 major_opcode;
	char  instruction[15];
};


static TestSet asm8080_tests[] = {
	// tests for i8080
	// asm8080 syntax

	{LD_BC_NN,	"lxi b,NN"},
	{ADD_HL_BC,	"dad b"},
	{LD_DE_NN,	"lxi d,NN"},
	{ADD_HL_DE,	"dad d"},
	{LD_HL_NN,	"lxi h,NN"},
	{ADD_HL_HL,	"dad h"},
	{LD_SP_NN,	"lxi sp,NN"},
	{ADD_HL_SP,	"dad sp"},

	{LD_xBC_A, "stax b"},
	{LD_A_xBC, "ldax b"},
	{LD_xDE_A, "stax d"},
	{LD_A_xDE, "ldax d"},
	{LD_xNN_HL, "shld NN"},
	{LD_HL_xNN, "lhld NN"},
	{LD_xNN_A,  "sta NN"},
	{LD_A_xNN,  "lda NN"},

	{INC_BC, "inx b"},  {INC_B,  "inr b"},  {DEC_B,  "dcr b"},  {LD_B_N,  "mvi b,N"},
	{DEC_BC, "dcx b"},  {INC_C,  "inr c"},  {DEC_C,  "dcr c"},  {LD_C_N,  "mvi c,N"},
	{INC_DE, "inx d"},  {INC_D,  "inr d"},  {DEC_D,  "dcr d"},  {LD_D_N,  "mvi d,N"},
	{DEC_DE, "dcx d"},  {INC_E,  "inr e"},  {DEC_E,  "dcr e"},  {LD_E_N,  "mvi e,N"},
	{INC_HL, "inx h"},  {INC_H,  "inr h"},  {DEC_H,  "dcr h"},  {LD_H_N,  "mvi h,N"},
	{DEC_HL, "dcx h"},  {INC_L,  "inr l"},  {DEC_L,  "dcr l"},  {LD_L_N,  "mvi l,N"},
	{INC_SP, "inx sp"}, {INC_xHL,"inr m"},  {DEC_xHL,"dcr m"},  {LD_xHL_N,"mvi m,n"},
	{DEC_SP, "dcx sp"}, {INC_A,  "inr a"},  {DEC_A,  "dcr a"},  {LD_A_N,  "mvi a,n"},

	{RLCA, "rlc"},  {RRCA, "rrc"},  {RLA, "ral"},  {RRA, "rar"},
	{DAA,  "daa"},  {CPL,  "cma"},  {SCF, "stc"},  {CCF, "cmc"},

	{LD_B_B,  "mov b,b"},  {LD_B_C,  "mov b,c"},  {LD_B_D,  "mov b,d"},  {LD_B_E,  "mov b,e"},
	{LD_C_B,  "mov c,b"},  {LD_C_C,  "mov c,c"},  {LD_C_D,  "mov c,d"},  {LD_C_E,  "mov c,e"},
	{LD_D_B,  "mov d,b"},  {LD_D_C,  "mov d,c"},  {LD_D_D,  "mov d,d"},  {LD_D_E,  "mov d,e"},
	{LD_E_B,  "mov e,b"},  {LD_E_C,  "mov e,c"},  {LD_E_D,  "mov e,d"},  {LD_E_E,  "mov e,e"},
	{LD_H_B,  "mov h,b"},  {LD_H_C,  "mov h,c"},  {LD_H_D,  "mov h,d"},  {LD_H_E,  "mov h,e"},
	{LD_L_B,  "mov l,b"},  {LD_L_C,  "mov l,c"},  {LD_L_D,  "mov l,d"},  {LD_L_E,  "mov l,e"},
	{LD_A_B,  "mov a,b"},  {LD_A_C,  "mov a,c"},  {LD_A_D,  "mov a,d"},  {LD_A_E,  "mov a,e"},
	{LD_xHL_B,"mov m,b"},  {LD_xHL_C,"mov m,c"},  {LD_xHL_D,"mov m,d"},  {LD_xHL_E,"mov m,e"},

	{LD_B_H,  "mov b,h"},  {LD_B_L,  "mov b,l"},  {LD_B_A,  "mov b,a"},  {LD_B_xHL, "mov b,m"},
	{LD_C_H,  "mov c,h"},  {LD_C_L,  "mov c,l"},  {LD_C_A,  "mov c,a"},  {LD_C_xHL, "mov c,m"},
	{LD_D_H,  "mov d,h"},  {LD_D_L,  "mov d,l"},  {LD_D_A,  "mov d,a"},  {LD_D_xHL, "mov d,m"},
	{LD_E_H,  "mov e,h"},  {LD_E_L,  "mov e,l"},  {LD_E_A,  "mov e,a"},  {LD_E_xHL, "mov e,m"},
	{LD_H_H,  "mov h,h"},  {LD_H_L,  "mov h,l"},  {LD_H_A,  "mov h,a"},  {LD_H_xHL, "mov h,m"},
	{LD_L_H,  "mov l,h"},  {LD_L_L,  "mov l,l"},  {LD_L_A,  "mov l,a"},  {LD_L_xHL, "mov l,m"},
	{LD_A_H,  "mov a,h"},  {LD_A_L,  "mov a,l"},  {LD_A_A,  "mov a,a"},  {LD_A_xHL, "mov a,m"},
	{LD_xHL_H,"mov m,h"},  {LD_xHL_L,"mov m,l"},  {LD_xHL_A,"mov m,a"},  {HALT, "hlt"},

	{ADD_B, "add b"},  {ADD_C, "add c"},  {ADD_D, "add d"},  {ADD_E, "add e"},
	{ADC_B, "adc b"},  {ADC_C, "adc c"},  {ADC_D, "adc d"},  {ADC_E, "adc e"},
	{SUB_B, "sub b"},  {SUB_C, "sub c"},  {SUB_D, "sub d"},  {SUB_E, "sub e"},
	{SBC_B, "sbb b"},  {SBC_C, "sbb c"},  {SBC_D, "sbb d"},  {SBC_E, "sbb e"},
	{AND_B, "ana b"},  {AND_C, "ana c"},  {AND_D, "ana d"},  {AND_E, "ana e"},
	{XOR_B, "xra b"},  {XOR_C, "xra c"},  {XOR_D, "xra d"},  {XOR_E, "xra e"},
	{ OR_B, "ora b"},  { OR_C, "ora c"},  { OR_D, "ora d"},  { OR_E, "ora e"},
	{ CP_B, "cmp b"},  { CP_C, "cmp c"},  { CP_D, "cmp d"},  { CP_E, "cmp e"},

	{ADD_H, "add h"},  {ADD_L, "add l"},  {ADD_A, "add a"},  {ADD_xHL, "add m"},
	{ADC_H, "adc h"},  {ADC_L, "adc l"},  {ADC_A, "adc a"},  {ADC_xHL, "adc m"},
	{SUB_H, "sub h"},  {SUB_L, "sub l"},  {SUB_A, "sub a"},  {SUB_xHL, "sub m"},
	{SBC_H, "sbb h"},  {SBC_L, "sbb l"},  {SBC_A, "sbb a"},  {SBC_xHL, "sbb m"},
	{AND_H, "ana h"},  {AND_L, "ana l"},  {AND_A, "ana a"},  {AND_xHL, "ana m"},
	{XOR_H, "xra h"},  {XOR_L, "xra l"},  {XOR_A, "xra a"},  {XOR_xHL, "xra m"},
	{ OR_H, "ora h"},  { OR_L, "ora l"},  { OR_A, "ora a"},  { OR_xHL, "ora m"},
	{ CP_H, "cmp h"},  { CP_L, "cmp l"},  { CP_A, "cmp a"},  { CP_xHL, "cmp m"},

	{RET_NZ, "rnz"}, {POP_BC, "pop b"},   {JP_NZ, "jnz NN"}, {RST00, "rst 0"},
	{RET_Z , "rz"},  {RET   , "ret"},     {JP_Z , "jz  NN"}, {RST08, "rst 1"},
	{RET_NC, "rnc"}, {POP_DE, "pop d"},   {JP_NC, "jnc NN"}, {RST10, "rst 2"},
	{RET_C , "rc"},                       {JP_C , "jc  NN"}, {RST18, "rst 3"},
	{RET_PO, "rpo"}, {POP_HL, "pop h"},   {JP_PO, "jpo NN"}, {RST20, "rst 4"},
	{RET_PE, "rpe"}, {JP_HL , "pchl"},    {JP_PE, "jpe NN"}, {RST28, "rst 5"},
	{RET_P , "rp"},  {POP_AF, "pop psw"}, {JP_P , "jp  nn"}, {RST30, "rst 6"},
	{RET_M , "rm"},  {LD_SP_HL,"sphl"},   {JP_M , "jm  NN"}, {RST38, "rst 7"},

	{CALL_NZ, "cnz NN"},  {ADD_N, "adi n"}, {JP,   "jmp nn"},    {PUSH_BC, "push b"},
	{CALL_Z , "cz  NN"},  {ADC_N, "aci N"}, {OUTA, "out N"},     {CALL,    "call NN"},
	{CALL_NC, "cnc nn"},  {SUB_N, "sui N"}, {INA,  "in N"},      {PUSH_DE, "push d"},
	{CALL_C , "cc  NN"},  {SBC_N, "sbi N"}, {EX_HL_xSP, "xthl"}, {PUSH_HL, "push h"},
	{CALL_PO, "cpo NN"},  {AND_N, "ani n"}, {EX_DE_HL, "xchg"},  {PUSH_AF, "push psw"},
	{CALL_PE, "cpe nn"},  {XOR_N, "xri n"}, {DI,   "di"},
	{CALL_P , "cp  NN"},  {OR_N , "ori N"}, {EI,   "ei"},
	{CALL_M , "cm  NN"},  {CP_N , "cpi N"},
};

static TestSet z80_tests[] = {
	// tests for i8080, Z80 and Z180

	{NOP, "nop"},

	{LD_BC_NN, "ld bc,NN"},
	{ADD_HL_BC, "add hl,bc"},
	{LD_DE_NN, "ld de,NN"},
	{ADD_HL_DE, "add hl,de"},
	{LD_HL_NN, "ld hl,nn"},
	{ADD_HL_HL, "add hl,hl"},
	{LD_SP_NN, "ld sp,NN"},
	{ADD_HL_SP, "add hl,sp"},

	{LD_xBC_A, "ld (bc),a"},
	{LD_A_xBC, "ld a,(bc)"},
	{LD_xDE_A, "ld (de),a"},
	{LD_A_xDE, "ld a,(de)"},
	{LD_xNN_HL, "ld (NN),hl"},
	{LD_HL_xNN, "ld hl,(nn)"},
	{LD_xNN_A,  "ld (nn),a"},
	{LD_A_xNN,  "ld a,(nn)"},

	{INC_BC, "inc bc"}, {INC_B, "inc b"},     {DEC_B, "dec b"},     {LD_B_N, "ld b,N"},
	{DEC_BC, "dec bc"}, {INC_C, "inc c"},     {DEC_C, "dec c"},     {LD_C_N, "ld c,N"},
	{INC_DE, "inc de"}, {INC_D, "inc d"},     {DEC_D, "dec d"},     {LD_D_N, "ld d,N"},
	{DEC_DE, "dec de"}, {INC_E, "inc e"},     {DEC_E, "dec e"},     {LD_E_N, "ld e,N"},
	{INC_HL, "inc hl"}, {INC_H, "inc h"},     {DEC_H, "dec h"},     {LD_H_N, "ld h,N"},
	{DEC_HL, "dec hl"}, {INC_L, "inc l"},     {DEC_L, "dec l"},     {LD_L_N, "ld l,N"},
	{INC_SP, "inc sp"}, {INC_xHL,"inc (hl)"}, {DEC_xHL,"dec (hl)"}, {LD_xHL_N, "ld (hl),N"},
	{DEC_SP, "dec sp"}, {INC_A, "inc a"},     {DEC_A, "dec a"},     {LD_A_N, "ld a,N"},

	{RLCA, "rlca"},  {RRCA, "rrca"},  {RLA, "rla"},  {RRA, "rra"},
	{DAA,  "daa"},   {CPL, "cpl"},	  {SCF, "scf"},  {CCF, "ccf"},

	{LD_B_B,  "ld b,b"},	 {LD_B_C,  "ld b,c"},	  {LD_B_D,  "ld b,d"},	 {LD_B_E,  "ld b,e"},
	{LD_C_B,  "ld c,b"},	 {LD_C_C,  "ld c,c"},	  {LD_C_D,  "ld c,d"},	 {LD_C_E,  "ld c,e"},
	{LD_D_B,  "ld d,b"},	 {LD_D_C,  "ld d,c"},	  {LD_D_D,  "ld d,d"},	 {LD_D_E,  "ld d,e"},
	{LD_E_B,  "ld e,b"},	 {LD_E_C,  "ld e,c"},	  {LD_E_D,  "ld e,d"},	 {LD_E_E,  "ld e,e"},
	{LD_H_B,  "ld h,b"},	 {LD_H_C,  "ld h,c"},	  {LD_H_D,  "ld h,d"},	 {LD_H_E,  "ld h,e"},
	{LD_L_B,  "ld l,b"},	 {LD_L_C,  "ld l,c"},	  {LD_L_D,  "ld l,d"},	 {LD_L_E,  "ld l,e"},
	{LD_A_B,  "ld a,b"},	 {LD_A_C,  "ld a,c"},	  {LD_A_D,  "ld a,d"},	 {LD_A_E,  "ld a,e"},
	{LD_xHL_B,"ld (hl),b"},  {LD_xHL_C,"ld (hl),c"},  {LD_xHL_D,"ld (hl),d"},{LD_xHL_E,"ld (hl),e"},

	{LD_B_H,  "ld b,h"},	 {LD_B_L,  "ld b,l"},	  {LD_B_A,  "ld b,a"},	 {LD_B_xHL, "ld b,(hl)"},
	{LD_C_H,  "ld c,h"},	 {LD_C_L,  "ld c,l"},	  {LD_C_A,  "ld c,a"},	 {LD_C_xHL, "ld c,(hl)"},
	{LD_D_H,  "ld d,h"},	 {LD_D_L,  "ld d,l"},	  {LD_D_A,  "ld d,a"},	 {LD_D_xHL, "ld d,(hl)"},
	{LD_E_H,  "ld e,h"},	 {LD_E_L,  "ld e,l"},	  {LD_E_A,  "ld e,a"},	 {LD_E_xHL, "ld e,(hl)"},
	{LD_H_H,  "ld h,h"},	 {LD_H_L,  "ld h,l"},	  {LD_H_A,  "ld h,a"},	 {LD_H_xHL, "ld h,(hl)"},
	{LD_L_H,  "ld l,h"},	 {LD_L_L,  "ld l,l"},	  {LD_L_A,  "ld l,a"},	 {LD_L_xHL, "ld l,(hl)"},
	{LD_A_H,  "ld a,h"},	 {LD_A_L,  "ld a,l"},	  {LD_A_A,  "ld a,a"},	 {LD_A_xHL, "ld a,(hl)"},
	{LD_xHL_H,"ld (hl),h"},  {LD_xHL_L,"ld (hl),l"},  {LD_xHL_A,"ld (hl),a"},{HALT, "halt"},

	{ADD_B, "add a,b"},  {ADD_C, "add a,c"},  {ADD_D, "add a,d"},  {ADD_E, "add a,e"},
	{ADC_B, "adc a,b"},  {ADC_C, "adc a,c"},  {ADC_D, "adc a,d"},  {ADC_E, "adc a,e"},
	{SUB_B, "sub a,b"},  {SUB_C, "sub a,c"},  {SUB_D, "sub a,d"},  {SUB_E, "sub a,e"},
	{SBC_B, "sbc a,b"},  {SBC_C, "sbc a,c"},  {SBC_D, "sbc a,d"},  {SBC_E, "sbc a,e"},
	{AND_B, "and a,b"},  {AND_C, "and a,c"},  {AND_D, "and a,d"},  {AND_E, "and a,e"},
	{XOR_B, "xor a,b"},  {XOR_C, "xor a,c"},  {XOR_D, "xor a,d"},  {XOR_E, "xor a,e"},
	{ OR_B,  "or a,b"},  { OR_C,  "or a,c"},  { OR_D,  "or a,d"},  { OR_E,  "or a,e"},
	{ CP_B,  "cp a,b"},  { CP_C,  "cp a,c"},  { CP_D,  "cp a,d"},  { CP_E,  "cp a,e"},

	{ADD_H, "add a,h"},  {ADD_L, "add a,l"},  {ADD_A, "add a,a"},  {ADD_xHL, "add a,(hl)"},
	{ADC_H, "adc a,h"},  {ADC_L, "adc a,l"},  {ADC_A, "adc a,a"},  {ADC_xHL, "adc a,(hl)"},
	{SUB_H, "sub a,h"},  {SUB_L, "sub a,l"},  {SUB_A, "sub a,a"},  {SUB_xHL, "sub a,(hl)"},
	{SBC_H, "sbc a,h"},  {SBC_L, "sbc a,l"},  {SBC_A, "sbc a,a"},  {SBC_xHL, "sbc a,(hl)"},
	{AND_H, "and a,h"},  {AND_L, "and a,l"},  {AND_A, "and a,a"},  {AND_xHL, "and a,(hl)"},
	{XOR_H, "xor a,h"},  {XOR_L, "xor a,l"},  {XOR_A, "xor a,a"},  {XOR_xHL, "xor a,(hl)"},
	{ OR_H,  "or a,h"},  { OR_L,  "or a,l"},  { OR_A,  "or a,a"},  { OR_xHL,  "or a,(hl)"},
	{ CP_H,  "cp a,h"},  { CP_L,  "cp a,l"},  { CP_A,  "cp a,a"},  { CP_xHL,  "cp a,(hl)"},

	{RET_NZ, "ret nz"}, {POP_BC,  "pop bc"},   {JP_NZ, "jp nz,NN"}, {RST00, "rst 0"},
	{RET_Z , "ret z"},  {RET   ,  "ret"},      {JP_Z , "jp z, NN"}, {RST08, "rst 0x08"},
	{RET_NC, "ret nc"}, {POP_DE,  "pop de"},   {JP_NC, "jp nc,NN"}, {RST10, "rst 16"},
	{RET_C , "ret c"},                         {JP_C , "jp c, nn"}, {RST18, "rst 18h"},
	{RET_PO, "ret po"}, {POP_HL,  "pop hl"},   {JP_PO, "jp po,NN"}, {RST20, "rst $20"},
	{RET_PE, "ret pe"}, {JP_HL ,  "jp hl"},    {JP_PE, "jp pe,NN"}, {RST28, "rst &28"},
	{RET_P , "ret p"},  {POP_AF,  "pop af"},   {JP_P , "jp p, NN"}, {RST30, "rst 6"},
	{RET_M , "ret m"},  {LD_SP_HL,"ld sp,hl"}, {JP_M , "jp m, NN"}, {RST38, "rst 56"},

	{CALL_NZ, "call nz,NN"},  {ADD_N, "add a,N"}, {JP,        "jp NN"},      {PUSH_BC, "push bc"},
	{CALL_Z , "call z, NN"},  {ADC_N, "adc a,N"}, {OUTA,      "out (N),a"},  {CALL,    "call NN"},
	{CALL_NC, "call nc,NN"},  {SUB_N, "sub a,N"}, {INA,       "in a,(n)"},   {PUSH_DE, "push de"},
	{CALL_C , "call c, NN"},  {SBC_N, "sbc a,N"}, {EX_HL_xSP, "ex hl,(sp)"}, {PUSH_HL, "push hl"},
	{CALL_PO, "call po,NN"},  {AND_N, "and a,N"}, {EX_DE_HL , "ex de,hl"},   {PUSH_AF, "push af"},
	{CALL_PE, "call pe,NN"},  {XOR_N, "xor a,N"}, {DI, "di"},
	{CALL_P , "call p, NN"},  {OR_N ,  "or a,N"}, {EI, "ei"},
	{CALL_M , "call m, NN"},  {CP_N ,  "cp a,N"},

	// non-prefix opcodes not present in i8080:

	{DJNZ, "djnz dis"},
	{JR, "jr   dis"},
	{JR_Z, "jr z, dis"},
	{JR_NZ, "jr nz,dis"},
	{JR_C, "jr c, dis"},
	{JR_NC, "jr nc,dis"},
	{EX_AF_AF, "ex af,af'"},
	{EXX, "exx"},

	// prefix IX/IY opcodes:

	{LD_HL_NN, "ld ix,NN"},
	{LD_HL_xNN, "ld iy,(NN)"},
	{LD_xNN_HL, "ld (NN),ix"},
	{ADD_HL_BC, "add iy,bc"},
	{ADD_HL_DE, "add ix,de"},
	{ADD_HL_HL, "add iy,iy"},
	{ADD_HL_HL, "add ix,ix"},
	{ADD_HL_SP, "add ix,sp"},
	{PUSH_HL, "push iy"},
	{POP_HL, "pop ix"},
	{JP_HL, "jp  iy"},
	{LD_SP_HL, "ld sp,ix"},
	{INC_HL, "inc iy"},
	{DEC_HL, "dec ix"},

	{INC_xHL, "inc (ix+dis)"},
	{DEC_xHL, "dec (iy+dis)"},
	{LD_xHL_N, "ld (ix+dis),N"},

	{LD_xHL_B, "ld (ix+dis),b"},
	{LD_xHL_H, "ld (iy+dis),h"},
	{LD_xHL_A, "ld (ix+dis),a"},
	{LD_B_xHL, "ld b,(iy+dis)"},
	{LD_L_xHL, "ld l,(ix+dis)"},
	{LD_A_xHL, "ld a,(iy+dis)"},

	{ADD_xHL, "add a,(ix+dis)"},
	{ADC_xHL, "adc a,(iy+dis)"},
	{SUB_xHL, "sub a,(ix+dis)"},
	{SBC_xHL, "sbc a,(iy+dis)"},
	{AND_xHL, "and a,(iy+dis)"},
	{XOR_xHL, "xor a,(iy+dis)"},
	{OR_xHL, "or a,(ix+dis)"},
	{CP_xHL, "cp a,(ix+dis)"},

	// prefix CB opcodes:

	{RLC_B,  "rlc b"}, {RLC_H, "rlc h"}, {RLC_C, "rlc c"}, {RLC_L,   "rlc l"},
	{RLC_D,  "rlc d"}, {RLC_A, "rlc a"}, {RLC_E, "rlc e"}, {RLC_xHL, "rlc (hl)"},
	{RRC_B,  "rrc b"}, {RRC_C, "rrc c"}, {RRC_D, "rrc d"}, {RRC_E,   "rrc e"},
	{RRC_H,  "rrc h"}, {RRC_L, "rrc l"}, {RRC_A, "rrc a"}, {RRC_xHL, "rrc (hl)"},
	{RL_B ,  "rl b"},  {RL_C , "rl c"},  {RL_D , "rl d"},  {RL_E,    "rl e"},
	{RL_H ,  "rl h"},  {RL_L , "rl l"},  {RL_A , "rl a"},  {RL_xHL,  "rl (hl)"},
	{RR_B ,  "rr b"},  {RR_H , "rr h"},  {RR_A , "rr a"},  {RR_xHL,  "rr (hl)"},
	{SLA_C,  "sla c"}, {SLA_L, "sla l"}, {SLA_A, "sla a"}, {SLA_xHL, "sla (hl)"},
	{SRA_B,  "sra b"}, {SRA_H, "sra h"}, {SRA_A, "sra a"}, {SRA_xHL, "sra (hl)"},
	{SRL_C,  "srl c"}, {SRL_L, "srl l"}, {SRL_A, "srl a"}, {SRL_xHL, "srl (hl)"},

	{BIT0_B, "bit 0,b"}, {BIT0_C, "bit 0,c"}, {BIT0_D, "bit 0,d"}, {BIT0_E,  "bit 0,e"},
	{BIT0_H, "bit 0,h"}, {BIT0_L, "bit 0,l"}, {BIT0_A, "bit 0,a"}, {BIT0_xHL,"bit 0,(hl)"},
	{BIT1_B, "bit 1,b"}, {BIT1_C, "bit 1,c"}, {BIT2_D, "bit 2,d"}, {BIT3_E,  "bit 3,e"},
	{BIT3_H, "bit 3,h"}, {BIT4_L, "bit 4,l"}, {BIT5_A, "bit 5,a"}, {BIT6_xHL,"bit 6,(hl)"},
	{BIT7_B, "bit 7,b"}, {BIT7_C, "bit 7,c"}, {BIT7_D, "bit 7,d"}, {BIT7_E,  "bit 7,e"},
	{BIT7_H, "bit 7,h"}, {BIT7_L, "bit 7,l"}, {BIT7_A, "bit 7,a"}, {BIT7_xHL,"bit 7,(hl)"},

	{SET0_B, "set 0,b"}, {SET0_C, "set 0,c"}, {SET0_D, "set 0,d"}, {SET0_E,  "set 0,e"},
	{SET0_H, "set 0,h"}, {SET0_L, "set 0,l"}, {SET0_A, "set 0,a"}, {SET0_xHL,"set 0,(hl)"},
	{SET1_B, "set 1,b"}, {SET1_C, "set 1,c"}, {SET2_D, "set 2,d"}, {SET3_E , "set 3,e"},
	{SET3_H, "set 3,h"}, {SET4_L, "set 4,l"}, {SET5_A, "set 5,a"}, {SET6_xHL,"set 6,(hl)"},
	{SET7_B, "set 7,b"}, {SET7_C, "set 7,c"}, {SET7_D, "set 7,d"}, {SET7_E , "set 7,e"},
	{SET7_H, "set 7,h"}, {SET7_L, "set 7,l"}, {SET7_A, "set 7,a"}, {SET7_xHL,"set 7,(hl)"},

	{RES0_B, "res 0,b"}, {RES0_C, "res 0,c"}, {RES0_D, "res 0,d"}, {RES0_E , "res 0,e"},
	{RES0_H, "res 0,h"}, {RES0_L, "res 0,l"}, {RES0_A, "res 0,a"}, {RES0_xHL,"res 0,(hl)"},
	{RES1_B, "res 1,b"}, {RES1_C, "res 1,c"}, {RES2_D, "res 2,d"}, {RES3_E , "res 3,e"},
	{RES3_H, "res 3,h"}, {RES4_L, "res 4,l"}, {RES5_A, "res 5,a"}, {RES6_xHL,"res 6,(hl)"},
	{RES7_B, "res 7,b"}, {RES7_C, "res 7,c"}, {RES7_D, "res 7,d"}, {RES7_E , "res 7,e"},
	{RES7_H, "res 7,h"}, {RES7_L, "res 7,l"}, {RES7_A, "res 7,a"}, {RES7_xHL,"res 7,(hl)"},

	// prefix IXCB opcodes:

	{RLC_xHL, "rlc (ix+dis)"},   {RRC_xHL , "rrc (ix+dis)"},   {RL_xHL, "rl (iy+dis)"},
	{RR_xHL,  "rr  (iy+dis)"},   {SLA_xHL , "sla (iy+dis)"},
	{SRA_xHL, "sra (ix+dis)"},   {SRL_xHL , "srl (ix+dis)"},
	{BIT0_xHL,"bit 0,(ix+dis)"}, {BIT1_xHL, "bit 1,(iy+dis)"}, {BIT2_xHL, "bit 2,(ix+dis)"},
	{BIT3_xHL,"bit 3,(iy+dis)"}, {BIT4_xHL, "bit 4,(iy+dis)"}, {BIT5_xHL, "bit 5,(ix+dis)"},
	{BIT6_xHL,"bit 6,(ix+dis)"}, {BIT7_xHL, "bit 7,(iy+dis)"}, {SET0_xHL, "set 0,(iy+dis)"},
	{SET3_xHL,"set 3,(iy+dis)"}, {SET6_xHL, "set 6,(ix+dis)"}, {SET7_xHL, "set 7,(ix+dis)"},
	{RES0_xHL,"res 0,(ix+dis)"}, {RES5_xHL, "res 5,(iy+dis)"},
	{RES6_xHL,"res 6,(ix+dis)"}, {RES7_xHL, "res 7,(iy+dis)"},

	// prefix ED opcodes:

	{IN_B_xC,"in b,(c)"}, {IN_C_xC,"in c,(c)"}, {IN_D_xC,"in d,(c)"}, {IN_E_xC,"in e,(c)"},
	{IN_H_xC,"in h,(c)"}, {IN_L_xC,"in l,(c)"}, {IN_F_xC,"in f,(c)"}, {IN_A_xC,"in a,(c)"},

	{OUT_xC_B,"out (c),b"}, {OUT_xC_C,"out (c),c"}, {OUT_xC_D,"out (c),d"}, {OUT_xC_E,"out (c),e"},
	{OUT_xC_H,"out (c),h"}, {OUT_xC_L,"out (c),l"}, {OUT_xC_A,"out (c),a"},

	{SBC_HL_BC,"sbc hl,bc"}, {ADC_HL_BC,"adc hl,bc"}, {SBC_HL_DE,"sbc hl,de"}, {ADC_HL_DE,"adc hl,de"},
	{SBC_HL_HL,"sbc hl,hl"}, {ADC_HL_HL,"adc hl,hl"}, {SBC_HL_SP,"sbc hl,sp"}, {ADC_HL_SP,"adc hl,sp"},

	{LD_xNN_BC, "ld (NN),bc"}, {LD_BC_xNN, "ld bc,(NN)"},
	{LD_xNN_DE, "ld (NN),de"}, {LD_DE_xNN, "ld de,(NN)"},
	{LD_xNN_SP, "ld (NN),sp"}, {LD_SP_xNN, "ld sp,(NN)"},

	{NEG,  "neg"},   {RETN, "retn"}, {RETI, "reti"},
	{IM_0, "im 0"},  {IM_1, "im 1"}, {IM_2, "im 2"},
	{LD_A_I,"ld a,i"}, {LD_I_A,"ld i,a"}, {LD_R_A,"ld r,a"}, {LD_A_R,"ld a,r"},
	{RRD,  "rrd"},   {RLD,  "rld"},
	{LDI,  "ldi"},   {LDIR, "ldir"}, {LDD, "ldd"}, {LDDR,"lddr"},
	{CPI,  "cpi"},   {CPIR, "cpir"}, {CPD, "cpd"}, {CPDR,"cpdr"},
	{INI,  "ini"},   {INIR, "inir"}, {IND, "ind"}, {INDR,"indr"},
	{OUTI, "outi"},  {OTDR, "otdr"}, {OUTD,"outd"},{OTDR,"otdr"},

	// illegal CB and IXCB and ED opcodes

	{SLL_B, "sll b"},
	{SLL_C, "sll c"},
	{SLL_D, "sll d"},
	{SLL_E, "sll e"},
	{SLL_H, "sll h"},
	{SLL_L, "sll l"},
	{SLL_A, "sll a"},
	{SLL_xHL, "sll (hl)"},
	{SLL_xHL, "sll (ix+dis)"},
	{SLL_xHL, "sll (iy+dis)"},
	//{OUT_xC_0, "out (c),0"},

	// Z180 additional opcodes

	{IN0_B_xN, "in0 b,(N)"},	{OUT0_xN_B, "out0 (N),b"},
	{IN0_C_xN, "in0 c,(N)"},	{OUT0_xN_C, "out0 (N),c"},
	{IN0_D_xN, "in0 d,(N)"},	{OUT0_xN_D, "out0 (N),d"},
	{IN0_E_xN, "in0 e,(N)"},	{OUT0_xN_E, "out0 (N),e"},
	{IN0_H_xN, "in0 h,(N)"},	{OUT0_xN_H, "out0 (N),h"},
	{IN0_L_xN, "in0 l,(N)"},	{OUT0_xN_L, "out0 (N),l"},
	{IN0_A_xN, "in0 a,(N)"},	{OUT0_xN_A, "out0 (N),a"},
	{IN0_F_xN, "in0 f,(N)"},

	{TST_B, "tst b"},
	{TST_C, "tst c"},
	{TST_D, "tst d"},
	{TST_E, "tst e"},
	{TST_H, "tst h"},
	{TST_L, "tst l"},
	{TST_A, "tst a"},
	{TST_xHL, "tst (hl)"},

	{MLT_BC, "mlt bc"},
	{MLT_DE, "mlt de"},
	{TST_N, "tst N"},
	{MLT_HL, "mlt hl"},
	{TSTIO, "tstio N"},
	{MLT_SP, "mlt sp"},
	{SLP, "slp"},

	{OTIM, "otim"},
	{OTDM, "otdm"},
	{OTIMR, "otimr"},
	{OTDMR, "otdmr"},
};


#undef END
#define END                                                                                                            \
  }                                                                                                                    \
  catch (std::exception & e)                                                                                           \
  {                                                                                                                    \
	num_errors++;                                                                                                      \
	logline("%s: %s", test->instruction, e.what());                                                                    \
  }


static void test_disass_asm8080(uint& num_tests, uint& num_errors)
{
	logIn("test major_opcode -- asm8080 syntax");

	TestSet* test;
	TRY for (uint i = 0; i < NELEM(asm8080_tests);)
	{
		test = &asm8080_tests[i++];
		assert(test->major_opcode == major_opcode_8080(test->instruction));
	}
	END
}

static void test_disass_z80(uint& num_tests, uint& num_errors)
{
	logIn("test major_opcode -- Z80 syntax");

	TestSet* test;
	TRY for (uint i = 0; i < NELEM(z80_tests);)
	{
		test = &z80_tests[i++];
		assert(test->major_opcode == major_opcode(test->instruction));
	}
	END
}

void test_z80_major_opcode(uint& num_tests, uint& num_errors)
{
	logIn("test z80_major_opcode");

	test_disass_asm8080(num_tests, num_errors);
	test_disass_z80(num_tests, num_errors);
}
