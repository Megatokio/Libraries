#pragma once
// Copyright (c) 1996 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	Z80 cpu emulation
*/

#include "Item.h"		// note: copy template into your project
#include "Z80options.h" // note: copy template into your project


#define CPU_PAGESIZE (1 << CPU_PAGEBITS)
#define CPU_PAGEMASK (CPU_PAGESIZE - 1)
#define CPU_PAGES	 (0x10000 >> CPU_PAGEBITS)


// ----	helper ----

#include "kio/detect_configuration.h"
#ifdef __BIG_ENDIAN__ // m68k, ppc
static inline uint8& lowbyte(CoreByte& n) noexcept { return *(((uint8*)&n) + (sizeof(CoreByte) - 1)); }
#endif
#ifdef __LITTLE_ENDIAN__ // i386
static inline uint8& lowbyte(CoreByte& n) noexcept { return reinterpret_cast<uint8&>(n); }
#endif


// ----	memory pages ----

struct PageInfo
{
	CoreByte* data_r; // flags in the high bytes, data in the low byte
	CoreByte* data_w; // flags in the high bytes, data in the low byte
};


// ----	z80 registers ----

union Z80Registers
{
	uint16 nn[16];
	struct
	{
		uint16 af, bc, de, hl, af2, bc2, de2, hl2, ix, iy, pc, sp, iff, ir;
	};
#ifdef __BIG_ENDIAN__ // m68k, ppc
	struct
	{
		uint8 a, f, b, c, d, e, h, l, a2, f2, b2, c2, d2, e2, h2, l2, xh, xl, yh, yl, pch, pcl, sph, spl, iff1, iff2, i,
			r, im, xxx;
	};
#endif
#ifdef __LITTLE_ENDIAN__ // i386
	struct
	{
		uint8 f, a, c, b, e, d, l, h, f2, a2, c2, b2, e2, d2, l2, h2, xl, xh, yl, yh, pcl, pch, spl, sph, iff1, iff2, r,
			i, im, xxx;
	};
#endif
};


class Z80 : public Item
{
public:
	typedef int32  CpuCycle; // cpu clock cycle
	typedef uint16 Address;
	typedef uint8  Byte;
	typedef uint16 Word;

	// public access to internal data
	// in general FOR READING ONLY!

	CoreByte noreadpage[CPU_PAGESIZE];	// used for reading from unmapped addresses, default fill value is 0xFF
	CoreByte nowritepage[CPU_PAGESIZE]; // used for writing to unmapped addresses

	PageInfo	 page[CPU_PAGES]; // mapped memory
	Z80Registers registers;		  // z80 registers
	CpuCycle	 cc;			  // cpu clock cycle	(inside run() a local variable cc is used!)
	uint		 nmi;			  // idle=0, flipflop set=1, execution pending=2
	bool		 halt;			  // HALT instruction executed
	// these are expected to be defined in class Item and reused by the Z80 here to 'collect the state':
	// uint8	int_ack_byte;		// byte read in int ack cycle
	// uint16	int0_call_address;	// mostly unused
	// int32	cc_next_update;		// next time to call update()
	// bool		irpt;				// interrupt asserted

private:
	Z80(const Z80&)			   = delete; // prohibit
	Z80& operator=(const Z80&) = delete; // prohibit

	void	 reset_registers() noexcept;
	void	 handle_output(CpuCycle, Address, Byte);
	Byte	 handle_input(CpuCycle, Address);
	CpuCycle handle_update(CpuCycle, CpuCycle cc_next);
	void	 _init();


public:
	template<class T>
	Z80(T t) : Item(t)
	{
		_init();
	}
	template<class T, class U>
	Z80(T t, U u) : Item(t, u)
	{
		_init();
	}
	virtual ~Z80() override;

	// Item interface:
	virtual void init() override;
	virtual void reset(CpuCycle) override;
	//virtual bool	input		(CpuCycle, CpuAddress, Byte& byte) override;
	//virtual bool	output		(CpuCycle, CpuAddress, Byte) override;
	//virtual void	update		(CpuCycle) override;
	virtual void shift_cc(CpuCycle, int32) override;


	// Run the Cpu:
	int run(CpuCycle cc_exit, uint options = 0);

	void setNmi() noexcept
	{
		if (nmi == 0)
		{
			nmi			   = 3;
			cc_next_update = cc;
		}
		nmi |= 1;
	}
	void clearNmi() noexcept { nmi &= ~1u; } // clear FF.set only

	// note: there is no setInterrupt() and clearInterrupt(), because the cpu
	// collects the interrupt state of all attached items in handle_update().


	// Map/Unmap memory:
	void mapRom(Address, uint16 size, CoreByte*) noexcept;
	void mapWom(Address, uint16 size, CoreByte*) noexcept;
	void mapRam(Address, uint16 size, CoreByte*) noexcept;
	void mapRam(Address, uint16 size, CoreByte* r, CoreByte* w) noexcept;

	void unmapRom(Address, uint16 size) noexcept;
	void unmapWom(Address, uint16 size) noexcept;
	void unmapRam(Address, uint16 size) noexcept;

	void unmapAllMemory() noexcept { unmapRam(0, 0); }
	void unmapMemory(CoreByte*, uint32 size) noexcept;

	// Access memory:
	PageInfo&		getPage(Address a) noexcept { return page[a >> CPU_PAGEBITS]; }
	const PageInfo& getPage(Address a) const noexcept { return page[a >> CPU_PAGEBITS]; }

	CoreByte* rdPtr(Address a) noexcept { return getPage(a).data_r + a; }
	CoreByte* wrPtr(Address a) noexcept { return getPage(a).data_w + a; }

	Byte peek(Address a) const noexcept { return lowbyte(getPage(a).data_r[a]); }
	void poke(Address a, Byte c) noexcept { lowbyte(getPage(a).data_w[a]) = c; }
	Word peek2(Address) const noexcept;
	void poke2(Address, Word n) noexcept;
	Word pop2() noexcept;
	void push2(Word n) noexcept;

	void copyBufferToRam(const uint8* q, Address dest, uint16 cnt) noexcept;
	void copyRamToBuffer(Address src, uint8* z, uint16 cnt) noexcept;
	void copyBufferToRam(const CoreByte* q, Address dest, uint16 cnt) noexcept;
	void copyRamToBuffer(Address src, CoreByte* z, uint16 cnt) noexcept;
	void readRamFromFile(class FD& fd, Address dest, uint16 cnt);
	void writeRamToFile(class FD& fd, Address src, uint16 cnt);


	static inline void c2b(const CoreByte* q, uint8* z, uint n) noexcept
	{
		for (uint i = 0; i < n; i++) z[i] = uint8(q[i]);
	}
	static inline void b2c(const uint8* q, CoreByte* z, uint n) noexcept
	{
		for (uint i = 0; i < n; i++) lowbyte(z[i]) = q[i];
	}
	static inline void c2c(const CoreByte* q, CoreByte* z, uint n) noexcept
	{
		for (uint i = 0; i < n; i++) lowbyte(z[i]) = uint8(q[i]);
	}
};
