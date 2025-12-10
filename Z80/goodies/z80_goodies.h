// Copyright (c) 2020 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "CpuID.h"
#include "kio/kio.h"
#include <functional>


namespace z80
{

typedef uint8  Byte;
typedef uint16 Address;

using FnPeek = std::function<Byte(Address)>;


// disassemble opcode using Z80 or asm8080 syntax:
// the address is incremented to skip the opcode
extern cstr disassemble(CpuID, FnPeek, Address&, bool asm8080 = false);
inline cstr disassemble(CpuID, const Byte* core, Address&, bool asm8080 = false) noexcept;

// Calculate length of opcode
extern uint opcode_length(CpuID, FnPeek, Address);
inline uint opcode_length(CpuID, const Byte* core, Address = 0) noexcept;

// Test whether this is a conditionally branching opcode (jp cc, ret cc, ldir, etc.)
extern bool opcode_can_branch(CpuID, FnPeek, Address);
inline bool opcode_can_branch(CpuID, const Byte* core, Address = 0) noexcept;

// Calculate the execution time in clock cycles for an opcode
extern uint clock_cycles(CpuID, FnPeek, Address);
inline uint clock_cycles(CpuID, const Byte* core, Address = 0) noexcept;

// Calculate the execution time if the opcode branches
extern uint clock_cycles_on_branch(CpuID, FnPeek, Address);
inline uint clock_cycles_on_branch(CpuID, const Byte* core, Address = 0) noexcept;


// utils:
extern bool opcode_is_legal(CpuID, const Byte*, Address = 0) noexcept;
inline cstr opcode_mnemo(CpuID, const Byte*, Address = 0, bool asm8080 = false) noexcept;
extern Byte major_opcode(CpuID, cstr q, bool asm8080 = false) throws;


// simple interface:
inline uint opcode_length(CpuID, Byte op1, Byte op2) noexcept;			// op2 only used if op1==0xED
inline bool opcode_can_branch(CpuID, Byte op1, Byte op2) noexcept;		// op2 only used if op1==0xED
inline uint clock_cycles_on_branch(CpuID, Byte op1, Byte op2) noexcept; // op2 only used if op1==0xED
inline uint clock_cycles(CpuID, Byte op1, Byte op2, Byte op4) noexcept; // dito, op4 only for IXCB/IYCB


// _____________________________________________
// implementations:

inline cstr disassemble(CpuID cpuid, const Byte* core, Address& a, bool asm8080) noexcept
{
	return disassemble(cpuid, [core](Address a) { return core[a]; }, a, asm8080);
}
inline uint opcode_length(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return opcode_length(cpuid, [core](Address a) { return core[a]; }, a);
}
inline bool opcode_can_branch(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return opcode_can_branch(cpuid, [core](Address a) { return core[a]; }, a);
}
inline uint clock_cycles(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return clock_cycles(cpuid, [core](Address a) { return core[a]; }, a);
}
inline uint clock_cycles_on_branch(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return clock_cycles_on_branch(cpuid, [core](Address a) { return core[a]; }, a);
}
inline uint opcode_length(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return opcode_length(cpuid, [core](Address a) { return core[a]; }, 0);
}
inline bool opcode_can_branch(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return opcode_can_branch(cpuid, [core](Address a) { return core[a]; }, 0);
}
inline uint clock_cycles_on_branch(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return clock_cycles_on_branch(cpuid, [core](Address a) { return core[a]; }, 0);
}
inline uint clock_cycles(CpuID cpuid, Byte op1, Byte op2, Byte op4) noexcept
{
	Byte core[] = {op1, op2, 0, op4};
	return clock_cycles(cpuid, [core](Address a) { return core[a]; }, 0);
}


} // namespace z80


/*































*/
