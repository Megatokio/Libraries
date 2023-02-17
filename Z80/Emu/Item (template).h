#pragma once
// Copyright (c) 2004 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/**	Simple template for class Item to get you started.

	Class Z80 is based on class Item and file "Z80.h" #includes "Item.h".
	Your "Item.h" should be placed in your project, not here.

	There should be an instance of class Machine which owns all Items.
	The components of your Machine should be based on class Item.
	Starting in Machine, Items should be "attached" in a linked list
	and the Z80 should be the first item.

	Items should reimplement the virtual functions of the Item interface as needed.
	Class Z80 does not reimplement input(), output() and update()
	because the Z80 calls these functions for all attached Items.
*/

typedef unsigned char	   uint8;
typedef unsigned short int uint16;
typedef unsigned int	   uint;
typedef int				   int32;
static_assert(sizeof(int32) == 4, "");
static_assert(sizeof(uint16) == 2, "");

enum IsaID {
	isa_none = 0,
	isa_unknown,
	isa_Screen,
	isa_Machine,
	isa_Z80,
	isa_Sio,
	isa_SystemTimer,
	isa_Keyboard,
	isa_Joystick,
	isa_Mmu
};


//===================================================================


class Item
{
	friend class Machine;

protected:
	IsaID	 id;
	Machine* machine;
	Item *	 next, *prev;		 // linked list of Items. Z80 should be the first Item.
	uint	 in_mask, in_bits;	 // address and decoded bits mask for input
	uint	 out_mask, out_bits; // address and decoded bits mask for output

	bool  irpt;				 // Item asserts interrupt
	uint8 int_ack_byte;		 // byte read in int ack cycle (0 = no interupts)
	uint  int0_call_address; // mostly unused

	int32 cc_next_update; // next time to call update()

public:
	Item(Machine*, IsaID);
	Item(Machine*, IsaID, uint o_addr, uint o_mask, uint i_addr, uint i_mask);
	virtual ~Item();
	Item(const Item&)			 = delete;
	Item& operator=(const Item&) = delete;

	inline bool matches_in(uint addr) const { return (addr & in_mask) == in_bits; }
	inline bool matches_out(uint addr) const { return (addr & out_mask) == out_bits; }

	// ==============
	// Item interface
	// ==============

	/**	Initialize Item. Cpu cycle count cc = 0 and real-world time = 0 sec.
		Called to "power up" the machine.
	*/
	virtual void init(/*int32 cc=0*/) {}

	/**	Reset the Item.
		The reset occurs at cc and Items which interact with the outside world
		(sound, floppy, sio, etc.) may choose to update up to cc first.
		Then reset the Item.
		Do _not_ reset cpu cycle count and real-world time to 0 after reset()!
		reset() should be called after init() was called for all Items for power-up reset.
	*/
	virtual void reset(int32 cc) {}

	/**	Handle IN opcode.
		input() is called for all Items behind the Z80 for which matches_in() returns true
		by the default implementation in "Z80.cpp".
		The Item should update the inner state up to cc if needed.
		Then store the read byte by ANDing it with the current byte. This allows proper emulation
		of multiple devices responding to an IN opcode.
		Return true if the Item needs a scheduled call at cc_next_update.
	*/
	virtual bool input(int32 cc, uint addr, uint8& byte) { return false; }

	/**	Handle OUT opcode.
		output() is called for all Items behind the Z80 for which matches_out() returns true
		by the default implementation in "Z80.cpp".
		The Item should update the inner state up to cc if needed.
		Then handle the byte.
		Return true if the Item needs a scheduled call at cc_next_update.
	*/
	virtual bool output(int32 cc, uint addr, uint8 byte) { return false; }

	/**	Perform a scheduled update.
		update() is called by the Z80 when the cpu cycle counter cc reaches cc_next_update.
		The Item should update the inner state up to cc if needed.
		Then perform your asynchronous action.
		Set or clear the interrupt flag irpt.
		Update cc_next_update.
		Set cc_next_update = NEVER if no more updates are needed.
	*/
	virtual void update(int32 cc) {}

	/**	Shift the cpu cycle base.
		At 4 MHz the cpu cycle counter overflows an int32 after 20 minutes approximately.
		This is no problem but many people don't know how to properly compare cycling counters.
		Therefore the cpu cycle should be regularly shifted by calling shift_cc() for all items.
		Items which have timing requirements (in general those which use cc_next_update)
		must update their cpu cycle based time stamps.
		The passed cc_offset must be a positive value which must be subtracted.
		shift_cc() may be called for the system timer interrupt or for a monitor frame flyback.
	*/
	virtual void shift_cc(int32 cc, int32 cc_offset) {}

	enum { NEVER = 0x7fffffff };
};
