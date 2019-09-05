#pragma once
/*	Copyright  (c)	Günter Woigk 2014 - 2019
					mailto:kio@little-bat.de

	This file is free software.

 	Permission to use, copy, modify, distribute, and sell this software
 	and its documentation for any purpose is hereby granted without fee,
 	provided that the above copyright notice appears in all copies and
 	that both that copyright notice, this permission notice and the
 	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.


	z80 opcode clock cycles
	-----------------------
*/

#include "kio/kio.h"

#ifndef DEFINE_CB_TABLES
#define DEFINE_CB_TABLES 0
#endif

/*	the tables
	easy computable CB tables only on request
*/
extern uint8 cc_normal[256];
extern uint8 cc_ED[256];
extern uint8 cc_XY[256];
#if DEFINE_CB_TABLES
extern uint8 cc_CB[256];
extern uint8 cc_XYCB[256];
#endif


/*	the functions
	op2 and op4 evaluated only if required
*/
extern bool z80_opcode_can_branch(uint8 op1, uint8 op2);			// op2 only used if op1==0xED
extern uint z80_clock_cycles(uint8 op1, uint8 op2, uint8 op4);		// dito, op4 only for IXCB/IYCB
extern uint z80_clock_cycles_on_branch(uint8 op1, uint8 op2);		// op2 only used if op1==0xED






