// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "CpuID.h"
#include "kio/kio.h"


typedef uint8  Byte;
typedef uint16 Address;

inline uint8  peek(const Byte* core, Address a) { return core[a]; }
inline uint16 peek_word(const Byte* core, Address a)
{
	uint16 w = core[a++];
	return w + 256 * core[a];
}


enum OpcodeValidity {
	LEGAL_OPCODE,
	UNDOCUMENTED_OPCODE, // undocumented opcodes which have a useful new effect:
						 // z80: SLL, use of XH, XL, YH and YL, IXCBR2 or IXCBXH if option ON

	DEPRECATED_OPCODE, // opcode aliases and undocumented opcodes which have no useful new effect:
					   // 8080: opcode aliases reused by z80; z80: 0xED aliases for RETI, IM_x

	ILLEGAL_OPCODE // unhandled ops, undocumented nops, ops with uncertain effect:
				   // z180: all; z80: 0xED nops, IX/IY with no effect, IXCBR2 or IXCBXH if option OFF
};


/***** disass.cpp *****/

// get information about the legal state of an opcode:
extern OpcodeValidity opcode_validity(CpuID, const Byte* core, Address);

// disassemble 8080, Z80 or Z180 opcode using Z80 syntax:
// the address is incremented to skip the disassembled opcode
extern cstr disassemble(CpuID, const Byte* core, Address&);

// disassemble 8080 opcode using 8080 or Z80 syntax:
// the address is incremented to skip the disassembled opcode
extern cstr disassemble_8080(const Byte* core, Address&, bool asm8080 = yes);

// Calculate the "major" opcode of an instruction mnemonic.
extern uint8 major_opcode(cstr q) throws;	   // z80 syntax: 8080, Z80, Z180
extern uint8 major_opcode_8080(cstr q) throws; // asm8080 syntax: 8080 only


/***** z80_opcode_length.cpp *****/

// Calculate length of opcode
extern uint opcode_length(CpuID, const Byte* core, Address = 0) noexcept;


/***** z80_clock_cycles.cpp *****/

// Test whether this is a conditionally branching opcode (jp cc, ret cc, ldir, etc.)
extern bool opcode_can_branch(CpuID, const Byte* core, Address = 0) noexcept;

// Calculate the execution time in clock cycles for an opcode
extern uint clock_cycles(CpuID, const Byte* core, Address = 0) noexcept;

// Calculate the execution time if the opcode branches
extern uint clock_cycles_on_branch(CpuID, const Byte* core, Address = 0) noexcept;


__attribute__((deprecated)) // use function with Core* and Address
extern bool
opcode_can_branch(CpuID, uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED
__attribute__((deprecated))								 // use function with Core* and Address
extern uint
clock_cycles(CpuID, uint8 op1, uint8 op2, uint8 op4) noexcept; // dito, op4 only for IXCB/IYCB
__attribute__((deprecated))									   // use function with Core* and Address
extern uint
clock_cycles_on_branch(CpuID, uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED

__attribute__((deprecated)) // use function with Core* and Address
extern bool
z80_opcode_can_branch(uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED
__attribute__((deprecated))							  // use function with Core* and Address
extern uint
z80_clock_cycles(uint8 op1, uint8 op2, uint8 op4) noexcept; // dito, op4 only for IXCB/IYCB
__attribute__((deprecated))									// use function with Core* and Address
extern uint
z80_clock_cycles_on_branch(uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED

__attribute__((deprecated)) // use function with Core* and Address
extern bool
z180_opcode_can_branch(uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED
__attribute__((deprecated))							   // use function with Core* and Address
extern uint
z180_clock_cycles(uint8 op1, uint8 op2, uint8 op4) noexcept; // dito, op4 only for IXCB/IYCB
__attribute__((deprecated))									 // use function with Core* and Address
extern uint
z180_clock_cycles_on_branch(uint8 op1, uint8 op2) noexcept; // op2 only used if op1==0xED

__attribute__((deprecated)) // use function with Core* and Address
extern bool
i8080_opcode_can_branch(uint8 op) noexcept;
__attribute__((deprecated)) // use function with Core* and Address
extern uint
i8080_clock_cycles(uint8 op) noexcept;
__attribute__((deprecated)) // use function with Core* and Address
extern uint
i8080_clock_cycles_on_branch(uint8 op) noexcept;

enum __attribute__((deprecated)) // use enum above
{
	LegalOpcode,
	IllegalOpcode,
	WeirdOpcode
};

__attribute__((deprecated)) // use opcode_validity()
inline int
opcode_legal_state(CpuID id, const Byte* core, Address addr)
{
	return opcode_validity(id, core, addr);
}

__attribute__((deprecated)) // use real disassemble()
extern cstr
opcode_mnemo(CpuID, const Byte* core, Address addr);

__attribute__((deprecated)) // use major_opcode()
extern uint8
z80_major_opcode(cstr q) throws;
