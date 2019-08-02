#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.
*/

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
#define CPU_PAGEBITS 12  // 4 kB


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
#define	OUTPUT(A,B)		{ INCR_CC(4); this->handle_output(cc-2,A,B); }
#define	INPUT(A,B)		{ INCR_CC(4); B = this->handle_input(cc-2,A); }
#define UPDATE()		{ cc_next_update = this->handle_update(cc, cc_exit); }
#endif







































