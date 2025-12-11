// Copyright (c) 2020 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "CpuID.h"
#include "kio/kio.h"


namespace z80
{

typedef uint8 Byte;
typedef int	  Address;


class DisAss
{
public:
	const Byte* core  = nullptr;
	CpuID		cpuid = CpuZ80_ixcbr2;

public:
	explicit DisAss(CpuID id, const Byte* core = nullptr) noexcept : core(core), cpuid(id) {}
	explicit DisAss(const Byte* core = nullptr) noexcept : core(core) {}
	virtual ~DisAss() {}

	// subclasses need to overload this:
	virtual Byte peek(Address a) const { return core[uint16(a)]; }

	cstr disassemble(Address address_in, Address& address_out, bool asm8080 = false) const;
	cstr disassemble(Address, bool asm8080 = false) const;

	int	 opcodeLength(Address) const;
	bool opcodeIsLegal(Address) const;
	bool opcodeCanBranch(Address) const;
	int	 clockCycles(Address) const;
	int	 clockCyclesOnBranch(Address) const;

private:
	cstr tostr(uint8 idf, Address&) const;
	cstr tostr(const struct Meno&, Address&) const;
};


// simple interface I:
inline int	opcode_length(CpuID, Byte op1, Byte op2) noexcept;			// op2 only used if op1==0xED
inline bool opcode_can_branch(CpuID, Byte op1, Byte op2) noexcept;		// op2 only used if op1==0xED
inline int	clock_cycles_on_branch(CpuID, Byte op1, Byte op2) noexcept; // op2 only used if op1==0xED
inline int	clock_cycles(CpuID, Byte op1, Byte op2, Byte op4) noexcept; // dito, op4 only for IXCB/IYCB

// simple interface II:
inline cstr disassemble(CpuID, const Byte* core, Address, bool asm8080 = false) noexcept;
inline cstr disassemble(CpuID, const Byte* core, Address, Address&, bool asm8080 = false) noexcept;
inline bool opcode_is_legal(CpuID, const Byte*, Address = 0) noexcept;
inline int	opcode_length(CpuID, const Byte* core, Address = 0) noexcept;
inline bool opcode_can_branch(CpuID, const Byte* core, Address = 0) noexcept;
inline int	clock_cycles(CpuID, const Byte* core, Address = 0) noexcept;
inline int	clock_cycles_on_branch(CpuID, const Byte* core, Address = 0) noexcept;
extern cstr opcode_mnemo(CpuID, const Byte*, Address = 0, bool asm8080 = false) noexcept;
extern Byte major_opcode(CpuID, cstr q, bool asm8080 = false) throws;


// _____________________________________________
// implementations:

inline cstr disassemble(CpuID cpuid, const Byte* core, Address a, bool asm8080) noexcept
{
	return DisAss(cpuid, core).disassemble(a, asm8080);
}
inline cstr disassemble(CpuID cpuid, const Byte* core, Address a, Address& a_out, bool asm8080) noexcept
{
	return DisAss(cpuid, core).disassemble(a, a_out, asm8080);
}
inline bool opcode_is_legal(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return DisAss(cpuid, core).opcodeIsLegal(a);
}
inline int opcode_length(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return DisAss(cpuid, core).opcodeLength(a);
}
inline bool opcode_can_branch(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return DisAss(cpuid, core).opcodeCanBranch(a);
}
inline int clock_cycles(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return DisAss(cpuid, core).clockCycles(a);
}
inline int clock_cycles_on_branch(CpuID cpuid, const Byte* core, Address a) noexcept
{
	return DisAss(cpuid, core).clockCyclesOnBranch(a);
}
inline int opcode_length(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return DisAss(cpuid, core).opcodeLength(0);
}
inline bool opcode_can_branch(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return DisAss(cpuid, core).opcodeCanBranch(0);
}
inline int clock_cycles_on_branch(CpuID cpuid, Byte op1, Byte op2) noexcept
{
	Byte core[] = {op1, op2};
	return DisAss(cpuid, core).clockCyclesOnBranch(0);
}
inline int clock_cycles(CpuID cpuid, Byte op1, Byte op2, Byte op4) noexcept
{
	Byte core[] = {op1, op2, 0, op4};
	return DisAss(cpuid, core).clockCycles(0);
}


} // namespace z80


/*































*/
