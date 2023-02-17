#pragma once
// Copyright (c) 2015 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/**	Z80 cpu options (template)
	options and macro definitions for the Z80 cpu.
	Your "Z80options.h" should be placed in your project, not here.
*/


/**	Define the system's memory page size.
	Required even if the system does not support pageing.
	You may set a smaller page size than actually used by the emulated system
	to keep the dummy read and write pages for the Z80 small
	at the expense of some speed when the system is pageing memory.
*/
#define CPU_PAGEBITS 12 // 4 kB


/**	Define datatype used for bytes in the system's memory.
	The byte visible to the CPU is in the low byte.
	The 8 (or maybe 24) high bits can be used for flags
	which can be tested in the memory read and write macros in "Z80macros.h".
	uint16 gives you 8 bit for the CPU plus 8 bit for the flags.
*/
typedef uint16 CoreByte;


/**	anything only needed if included from Z80.cpp
	Your custom replacements for the default macros in "Z80macros.h" should go here.
*/
#ifdef Z80_SOURCE
// these are the default definitions as defined in Z80macros.h
// and could be left out here:
  #define OUTPUT(A, B)                   \
	{                                    \
	  INCR_CC(4);                        \
	  this->handle_output(cc - 2, A, B); \
	}
  #define INPUT(A, B)                    \
	{                                    \
	  INCR_CC(4);                        \
	  B = this->handle_input(cc - 2, A); \
	}
  #define UPDATE()                                       \
	{                                                    \
	  cc_next_update = this->handle_update(cc, cc_exit); \
	}
#endif
