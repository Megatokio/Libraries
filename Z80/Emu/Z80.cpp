// Copyright (c) 1996 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	Z80 Emulator	version
					initially based on fMSX; Copyright (C) Marat Fayzullin 1994,1995
*/

#define Z80_SOURCE		// -> include other class header files or typedefs only
#include "Z80.h"		// major header file
#include "Z80macros.h"	// required and optional macros
#include "Z80opcodes.h" // opcode enumeration
#ifndef NO_class_FD
  #include "unix/FD.h"
#endif


// ==================================================================================
//							ctor, dtor
// ==================================================================================

void Z80::_init()
{
	// common initialization for the templated constructors

	Z80_INFO_CTOR;

	// init dummy read page for non-mapped memory:	flags=0, byte=0xFF
	// init dummy write page for non-mapped memory: flags=0, byte=don't care
	// note: no flags are set: dep. on implementation of PEEK and POKE these never add waitstates etc.
	for (int i = 0; i < CPU_PAGESIZE; i++) { noreadpage[i] = 0x000000FF; } // flags=0x000000, data=0xFF
	memset(nowritepage, 0, sizeof(nowritepage));

	// init all pages with "no memory" and "no waitmap"
	unmapAllMemory();
}

Z80::~Z80() { Z80_INFO_DTOR; }

void Z80::init(/*cc=0*/)
{
	Item::init();
	cc			 = 0;
	nmi			 = 0;
	int_ack_byte = 0xff; // in case it's never set
	reset_registers();
	unmapAllMemory();
}

void Z80::reset(CpuCycle cc)
{
	assert(cc == this->cc);

	Item::reset(cc);
	this->cc = cc;
	reset_registers();
}

void Z80::reset_registers() noexcept
{
	// reset registers:
	BC = DE = HL = IX = IY = PC = SP = BC2 = DE2 = HL2 = 0;
	RA = RA2 = RF = RF2 = RR = RI = 0;

	// reset other internal state:
	IFF1 = IFF2	 = disabled; // disable interrupts
	registers.im = 0;		 // interrupt mode := 0
	nmi &= ~2u;				 // clear nmi FF
	halt = no;
}


// ======================================================================
//							helpers
// ======================================================================

Word Z80::peek2(Address addr) const noexcept { return Word(peek(addr) + (peek(addr + 1) << 8)); }

void Z80::poke2(Address addr, Word n) noexcept
{
	poke(addr, Byte(n));
	poke(addr + 1, Byte(n >> 8));
}

Word Z80::pop2() noexcept
{
	registers.sp += 2;
	return peek2(registers.sp - 2);
}

void Z80::push2(Word n) noexcept
{
	registers.sp -= 2;
	poke2(registers.sp, n);
}

void Z80::copyRamToBuffer(Address q, uint8* z, uint16 cnt) noexcept
{
	uint16 n = CPU_PAGESIZE - (q & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2b(rdPtr(q), z, n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::copyBufferToRam(const uint8* q, Address z, uint16 cnt) noexcept
{
	uint16 n = CPU_PAGESIZE - (z & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		b2c(q, wrPtr(z), n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::copyRamToBuffer(Address q, CoreByte* z, uint16 cnt) noexcept
{
	uint16 n = CPU_PAGESIZE - (q & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2c(rdPtr(q), z, n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::copyBufferToRam(const CoreByte* q, Address z, uint16 cnt) noexcept
{
	uint16 n = CPU_PAGESIZE - (z & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2c(q, wrPtr(z), n);
		q += n;
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

#ifndef NO_class_FD
void Z80::readRamFromFile(class FD& fd, Address z, uint16 cnt)
{
	uint8  bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE - (z & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		fd.read_bytes(bu, n);
		b2c(bu, wrPtr(z), n);
		z += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}

void Z80::writeRamToFile(class FD& fd, Address q, uint16 cnt)
{
	uint8  bu[CPU_PAGESIZE];
	uint16 n = CPU_PAGESIZE - (q & CPU_PAGEMASK);
	do {
		if (n > cnt && cnt) n = cnt;
		c2b(rdPtr(q), bu, n);
		fd.write_bytes(bu, n);
		q += n;
		cnt -= n;
		n = CPU_PAGESIZE;
	}
	while (cnt);
}
#endif


/*	Attach ram/rom to address space
	Attach real memory to cpu address space

	For best performance set CPU_PAGESIZE to the page size of the emulated machine.
	If you emulate multiple machines, set it to the lowest page size of any machine.

	addr	= cpu start address of page
	size	= size of page
				0x0000 is assumed to mean 0x10000 (entire address range)
				addr and size must be multiples of CPU_PAGESIZE
				addr+size must not extend beyond address space end 0x10000

	r_data,
	w_data	= pointer to page data

	waitmap	= wait cycles map for this page
	wmsize	= size of waitmap
				NULL  => no waitstates map
				access to waitmap: cc += waitmap[cc%wmsize]

	data pointers in PgInfo struct point to the virtual location of address $0000
	so that you can access data using page.data_r[addr>>CPU_PAGEBITS][addr]
*/

void Z80::mapRam(Address addr, uint16 size, CoreByte* r_data, CoreByte* w_data) noexcept
{
	// map Core page(s) for reading and
	// map Core page(s) for writing

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert(r_data);
	assert(w_data);
	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfass wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);

	r_data -= addr;
	w_data -= addr;

	for (; a <= e; a++)
	{
		a->data_r = r_data;
		a->data_w = w_data;
	}
}

void Z80::mapRam(Address addr, uint16 size, CoreByte* data) noexcept
{
	// map Core page(s) for reading and writing

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert(data);
	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++) { a->data_w = a->data_r = data; }
}

void Z80::mapWom(Address addr, uint16 size, CoreByte* data) noexcept
{
	// map write-only memory
	// mapping for reading is not changed

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert(data);
	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++) { a->data_w = data; }
}

void Z80::mapRom(Address addr, uint16 size, CoreByte* data) noexcept
{
	// map Read-only memory
	// mapping for writing is not changed

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert(data);
	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);

	data -= addr;

	for (; a <= e; a++) { a->data_r = data; }
}

void Z80::unmapWom(Address addr, uint16 size) noexcept
{
	// unmap write pages

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);
	CoreByte* w = nowritepage - addr;
	assert(size_t(w) + addr == size_t(nowritepage));

	for (; a <= e; a++)
	{
		a->data_w = w;
		w -= CPU_PAGESIZE;
	}
}

void Z80::unmapRom(Address addr, uint16 size) noexcept
{
	// unmap read pages

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);
	CoreByte* r = noreadpage - addr;
	assert(size_t(r) + addr == size_t(noreadpage));

	for (; a <= e; a++)
	{
		a->data_r = r;
		r -= CPU_PAGESIZE;
	}
}

void Z80::unmapRam(Address addr, uint16 size) noexcept
{
	// unmap read and write pages

	size -= CPU_PAGESIZE; // note: size=0  =>  size=0x10000

	assert((addr & CPU_PAGEMASK) == 0); // addr muss in's 'CPU_PAGESIZE' raster passen
	assert((size & CPU_PAGEMASK) == 0); // addr+size ebenfalls wieder
	assert(size <= 0xffff - addr);		// wrap around address space end

	PageInfo* a = page + (addr >> CPU_PAGEBITS);
	PageInfo* e = a + (size >> CPU_PAGEBITS);
	CoreByte* r = noreadpage - addr;
	assert(size_t(r) + addr == size_t(noreadpage));
	CoreByte* w = nowritepage - addr;
	assert(size_t(w) + addr == size_t(nowritepage));

	for (; a <= e; a++)
	{
		a->data_r = r;
		r -= CPU_PAGESIZE;
		a->data_w = w;
		w -= CPU_PAGESIZE;
	}
}

void Z80::unmapMemory(CoreByte* a, uint32 size) noexcept
{
	// unmap memory a[size] whereever it is currently mapped.
	// use this if the exact paging location can not be determined easily.
	// this function should be avoided.

	CoreByte* e = a + size;
	CoreByte* p;
	Address	  i = 0;

	do {
		p = rdPtr(i);
		if (p >= a && p < e)
		{
			PageInfo& p = page[i >> CPU_PAGEBITS];
			p.data_r	= noreadpage - i;
		}

		p = wrPtr(i);
		if (p >= a && p < e)
		{
			PageInfo& p = page[i >> CPU_PAGEBITS];
			p.data_w	= nowritepage - i;
		}
	}
	while ((i += CPU_PAGESIZE) != 0);
}


#ifndef Z80_NO_handle_output
void Z80::handle_output(CpuCycle cc, Address a, Byte b)
{
	// default implementation for an OUT handler:
	// used in default implementation of macro OUTPUT() in Z80macros.h

	for (Item* item = next; item; item = item->next) // assumes that the CPU is 1st in list
	{
		if (item->matches_out(a))
		{
			if (item->output(cc, a, b)) cc_next_update = cc;
			break; // shortcut: assumes there is at most 1 item responding to this address
		}
	}
}
#endif

#ifndef Z80_NO_handle_input
Z80::Byte Z80::handle_input(CpuCycle cc, Address a)
{
	// default implementation for an IN handler:
	// used in default implementation of macro INPUT() in Z80macros.h

	Byte b = 0xff;

	for (Item* item = next; item; item = item->next) // assumes that the CPU is 1st in list
	{
		if (item->matches_in(a))
		{
			if (item->input(cc, a, b)) cc_next_update = cc;
			return b; // shortcut: assumes there is at most 1 item responding to this address
		}
	}
	return b;
}
#endif

#ifndef Z80_NO_handle_update
inline Z80::CpuCycle Z80::handle_update(CpuCycle cc, CpuCycle cc_next_update)
{
	// template to update items' state machines and for interrupt detection:
	// used in default implementation of macro UPDATE() in Z80macros.h

	irpt = off;

	for (Item* item = next; item; item = item->next) // assumes that the CPU is 1st in list
	{
		if (cc >= item->cc_next_update) item->update(cc);

		cc_next_update = min(cc_next_update, item->cc_next_update);

		if (item->irpt)
		{
			irpt			  = on;
			int_ack_byte	  = item->int_ack_byte;
			int0_call_address = item->int0_call_address;
		}
	}
	return cc_next_update;
}
#endif

void Z80::shift_cc(CpuCycle, int32 dis)
{
	// shift cpu clock cycle time base
	// called when video frame completed or
	// when major system timer interrupt activated.

	cc -= dis;
	//cc_next_update = cc;
}


// ===========================================================================
// ====	The Z80 Engine =======================================================
// ===========================================================================

int Z80::run(CpuCycle cc_exit, uint options)
{
	CpuCycle cc_next_update;
	CpuCycle cc; // cpu cycle counter

	int result = 0; // return 0: T cycle count expired

	uint16 pc; // z80 program counter
	uint8  ra; // z80 a register
	uint8  rf; // z80 flags
	uint8  r;  // z80 r register bit 0...6

	(void)options; // silence warnings if not used

	// looping & jumping:
#define LOOP goto nxtcmnd // LOOP to next instruction
#define POKE_AND_LOOP(W, C) \
  {                         \
	w = W;                  \
	c = C;                  \
	goto poke_and_nxtcmd;   \
  }					// POKE(w,c) and goto next instr.
#define EXIT goto x // exit from cpu

	// load local variables from data members:
	LOAD_REGISTERS;


slow_loop:

	// ----	Update all Items and Poll Interrupts ----

	//	we come here
	//	- because run() was just entered
	//	- because cc >= cc_next_update --> an item requires an internal update, irpt may toggle
	//	- EI was executed and we need to re-check interrupts

	UPDATE();


	// ---- NMI TEST ---------------

	// test non-maskable interrupt:
	// the NMI is edge-triggered and automatically cleared

	if (nmi & 2 /*pending*/)
	{				// 11 T cycles, probably: M1:5T, M2:3T, M3:3T
		nmi &= ~2u; // processing no longer pending
		if (halt)
		{
			halt = no;
			pc++;
		}
		//IFF2 = IFF1;				// save current irpt enable bit
		IFF1 = disabled; // disable irpt, preserve IFF2
		INCR_R();
		INCR_CC(5); // M1: 5 T: interrupt acknowledge cycle
		Z80_INFO_NMI();
		PUSH(pc >> 8); // M2: 3 T: push pch
		PUSH(pc);	   // M3: 3 T: push pcl
		pc = 0x0066;   // jump to 0x0066
					   //LOOP;
	}


	// ---- INTERRUPT TEST -----------------

	// test maskable interrupt:
	// note: the /INT signal is not cleared by int ack
	//		 the /INT signal is sampled once per instruction at the end of instruction, during refresh cycle
	//		 if the interrupt is not started until the /INT signal goes away then it is lost!

	if (!irpt) LOOP;			// no interrupt asserted
	if (IFF1 == disabled) LOOP; // irpt disabled?

	if (halt)
	{
		halt = no;
		pc++;
	}
	IFF1 = IFF2 = disabled; // disable interrupt
	INCR_R();				// M1: 2 cc + standard opcode timing (min. 4 cc)
	INCR_CC(6);				// /HALT test and busbyte read in cc+4
	Z80_INFO_IRPT();

	switch (registers.im)
	{
		uint16 w;
	case 0:
		/*	mode 0: read instruction from bus
		NOTE:	docs say that M1 is always 6 cc in mode 0, but that is not conclusive:
				the 2 additional T cycles are before opcode fetch, and thus they must always add to instruction execution
				the timing from the moment of instruction fetch (e.g. cc +2 in a normal M1 cycle, cc +4 in int ack M1 cycle)
				and can't be shortended.
				to be tested somehow.	kio 2004-11-12
	*/
		switch (int_ack_byte)
		{
		case RST00:
		case RST08: //	timing: M1: 2+5 cc: opcode RSTxx  ((RST is 5 cc))
		case RST10:
		case RST18: //			M2:	3 cc:   push pch
		case RST20:
		case RST28: //			M3: 3 cc:   push pcl
		case RST30:
		case RST38:
			INCR_CC(1);
			w = int_ack_byte - RST00; // w = address to call
			goto irpt_xw;

		case CALL:				   //	timing:	M1:   2+4 cc: opcode CALL
			INCR_CC(7);			   //			M2+3: 3+4 cc: get NN
			w = int0_call_address; //  		M4+5: 3+3 cc: push pch, pcl		w = address to call
			goto irpt_xw;

		default:	//	only RSTxx and CALL NN are supported
			TODO(); //  any other opcode is of no real use.		((throws InternalError))
		}

	case 1:
		/*	Mode 1:	RST38
		NOTE:	docs say, timing is 13 cc for implicit RST38 in mode 1 and 12 cc for fetched RST38 in mode 0.
				maybe it is just vice versa? in mode 1 the implicitely known RST38 can be executed with the start of the M1 cycle
				and finish after 5 cc, prolonged to the irpt ackn M1 cycle min. length of 6 cc. Currently i'll stick to 13 cc as doc'd.
				to be tested somehow.	kio 2004-11-12
		TODO:	does the cpu a dummy read in M1?
				at least ZX Spectrum videoram waitcycles is no issue because irpt is off-screen.
	*/
		INCR_CC(1);	  //	timing:	M1:	7 cc: int ack cycle (as doc'd)
		w = 0x0038;	  //			M2: 3 cc: push pch					w = address to call
		goto irpt_xw; //			M3: 3 cc: push pcl

	irpt_xw:
		PUSH(pc >> 8); //	M2: 3 cc: push pch
		PUSH(pc);	   //	M3: 3 cc: push pcl and load pc
		pc = w;		   // w = address to call
		LOOP;

	case 2:
		/*	Mode 2:	jump via table
	*/
		INCR_CC(1);	   //	timing:	M1: 7 cc: int ack  and  read interrupt vector from bus
		PUSH(pc >> 8); //			M2: 3 cc: push pch
		PUSH(pc);	   //			M3: 3 cc: push pcl
		w = registers.i * 256 + int_ack_byte;
		PEEK(PCL, w);	  //			M4: 3 cc: read low byte from table
		PEEK(PCH, w + 1); //			M5: 3 cc: read high byte and jump
		pc = PC;
		LOOP;

	default: IERR(); // bogus irpt mode
	}

	IERR();


	// ==========================================================================
	// MAIN INSTRUCTION DISPATCHER
	// ==========================================================================


	uint8  c;		// general purpose byte register
	uint16 w;		// general purpose word register
#define wl uint8(w) // access low byte of w
#define wh (w >> 8) // access high byte of w


poke_and_nxtcmd:
	POKE(w, c); // --> CPU_WAITCYCLES, CPU_READSCREEN, cc+=3, Poke(w,c)

nxtcmnd:
	if (cc < cc_next_update) // fast loop exit test
	{
	loop_ei:
		GET_INSTR(c);
#include "Z80core.h"
		IERR(); // all opcodes decoded!
	}

	// ---- EXIT TEST ----

	if (result == 0 && cc < cc_exit) goto slow_loop;
	goto x;
x:
	SAVE_REGISTERS;
	return result;
}
