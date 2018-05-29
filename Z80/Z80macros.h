/*	Copyright  (c)	Günter Woigk 1996 - 2015
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


	Z80 Emulator
	originally based on fMSX; Copyright (C) Marat Fayzullin 1994,1995

	macro definitions
*/


// access z80 registers:

#define	IFF1	registers.iff1		// actually used irpt flip flop
#define	IFF2	registers.iff2		// copy of iff1 during nmi processing
#define	RR		registers.r			// 7 bit DRAM refresh counter
#define	RI		registers.i			// hi byte of interrupt vector: i register

#define	RA		registers.a
#define	RF		registers.f
#define	RB		registers.b
#define	RC		registers.c
#define	RD		registers.d
#define	RE		registers.e
#define	RH		registers.h
#define	RL		registers.l

#define	RA2		registers.a2
#define	RF2		registers.f2

#define	XH		registers.xh
#define	XL		registers.xl
#define	YH		registers.yh
#define	YL		registers.yl
#define	PCH		registers.pch
#define	PCL		registers.pcl
#define	SPH		registers.sph
#define	SPL		registers.spl

#define	BC		registers.bc
#define	DE		registers.de
#define	HL		registers.hl

#define	BC2		registers.bc2
#define	DE2		registers.de2
#define	HL2		registers.hl2

#define	IX		registers.ix
#define	IY		registers.iy
#define	PC		registers.pc
#define	SP		registers.sp


// read/write data:

#ifndef INCR_CC
#define INCR_CC(N)		cc += (N)
#endif

#ifndef INCR_R
#define INCR_R()		r += 1
#endif

#ifndef INCR_IC
#define INCR_IC()		/*nop*/
#endif

#ifndef	PEEK
#define	PEEK(R,A)		{ INCR_CC(3); R=peek(A); }
#endif
#ifndef	POKE
#define	POKE(A,R)		{ INCR_CC(3); poke(A,R); }
#endif

#ifndef	GET_INSTR
#define	GET_INSTR(R)	{ INCR_CC(4); INCR_R(); INCR_IC(); R=peek(pc++); }
#endif
#ifndef	GET_CB_OP
#define	GET_CB_OP(R)	{ INCR_CC(4); INCR_R(); R=peek(pc++); }
#endif
#ifndef	GET_ED_OP
#define	GET_ED_OP(R)	{ INCR_CC(4); INCR_R(); R=peek(pc++); }
#endif
#ifndef	GET_XY_OP
#define	GET_XY_OP(R)	{ INCR_CC(4); INCR_R(); R=peek(pc++); }
#endif
#ifndef	GET_XYCB_OP
#define	GET_XYCB_OP(R)	{ INCR_CC(5); R=peek(pc++); }
#endif
#ifndef	GET_N
#define	GET_N(R)		{ INCR_CC(3); R=peek(pc++); }
#endif
#ifndef	SKIP_N
#define	SKIP_N()		{ INCR_CC(3); peek(pc++); }
#endif

#ifndef SKIP_5X1CC
#define SKIP_5X1CC(RR)	{ INCR_CC(5); }
#endif

#ifndef SKIP_1CC
#define SKIP_1CC(RR)	{ INCR_CC(1); }
#endif

#ifndef SKIP_2X1CC
#define SKIP_2X1CC(RR)	{ INCR_CC(2); }
#endif

#ifndef SKIP_4X1CC
#define SKIP_4X1CC(RR)	{ INCR_CC(4); }
#endif

#ifndef SKIP_7X1CC
#define SKIP_7X1CC(RR)	{ INCR_CC(7); }
#endif


#ifndef OUTPUT			
#define	OUTPUT(A,B)		{ INCR_CC(4); this->handle_output(cc-2,A,B); }
#endif

#ifndef INPUT			
#define	INPUT(A,B)		{ INCR_CC(4); B = this->handle_input(cc-2,A); }
#endif

#ifndef UPDATE		
#define UPDATE()		{ cc_next_update = this->handle_update(cc, cc_exit); }
#endif


// hooks:

#ifndef Z80_INFO_CTOR
#define	Z80_INFO_CTOR			/* nop */
#endif

#ifndef Z80_INFO_DTOR
#define	Z80_INFO_DTOR			/* nop */
#endif

#ifndef Z80_INFO_IRPT			/* cpu cycle of irpt ack is cc-2 */
#define Z80_INFO_IRPT()			/* nop */
#endif

#ifndef Z80_INFO_NMI			/* cpu cycle of nmi ack is cc-2 */
#define	Z80_INFO_NMI()			/* nop */
#endif

#ifndef Z80_INFO_RETI
#define	Z80_INFO_RETI			/* nop */
#endif

#ifndef Z80_INFO_RETN
#define	Z80_INFO_RETN			/* nop */
#endif

#ifndef Z80_INFO_HALT
#define	Z80_INFO_HALT			/* nop */
#endif

#ifndef Z80_INFO_ILLEGAL
#define Z80_INFO_ILLEGAL(CC,PC)	/* nop */
#endif

#ifndef Z80_INFO_POP
#define Z80_INFO_POP			/* nop */
#endif

#ifndef Z80_INFO_RET
#define Z80_INFO_RET			/* nop */
#endif

#ifndef Z80_INFO_EX_HL_xSP
#define Z80_INFO_EX_HL_xSP		/* nop */
#endif

#ifndef Z80_INFO_RST00
#define Z80_INFO_RST00			/* nop */
#endif

#ifndef Z80_INFO_RST08
#define Z80_INFO_RST08			/* nop */
#endif

#ifndef Z80_INFO_RST10
#define Z80_INFO_RST10			/* nop */
#endif

#ifndef Z80_INFO_RST18
#define Z80_INFO_RST18			/* nop */
#endif

#ifndef Z80_INFO_RST20
#define Z80_INFO_RST20			/* nop */
#endif

#ifndef Z80_INFO_RST28
#define Z80_INFO_RST28			/* nop */
#endif

#ifndef Z80_INFO_RST30
#define Z80_INFO_RST30			/* nop */
#endif

#ifndef Z80_INFO_RST38
#define Z80_INFO_RST38			/* nop */
#endif

#ifndef Z80_INFO_EI
#define Z80_INFO_EI				/* nop */
#endif

#ifndef Z80_INFO_LD_R_A
#define Z80_INFO_LD_R_A			/* nop */
#endif

#ifndef Z80_INFO_LD_I_A
#define Z80_INFO_LD_I_A			/* nop */
#endif




// --------------------------------------------------------------------
// ----	INSTRUCTION MACROS --------------------------------------------
//		no user servicable parts inside.
// --------------------------------------------------------------------


#define	GET_NN(RR)		{ GET_N(RR); GET_N(wm); RR += 256*wm; }
#define	POP(R)			{ PEEK(R,SP); SP++; }
#define	PUSH(R)			{ --SP; POKE(SP,R); }


/*	RLC ... SRL:	set/clr C, Z, P, S;
			clear	N=0, H=0
			pres.	none
*/
#define M_RLC(R)      				\
	rf  = R>>7;						\
	R   = (R<<1)+rf; 				\
	rf |= zlog_flags[R]

#define M_RRC(R)      				\
	rf  = R&0x01;					\
	R   = (R>>1)+(rf<<7);		 	\
	rf |= zlog_flags[R]

#define M_RL(R)						\
	if (R&0x80)						\
	{	R 	= (R<<1)+(rf&0x01);		\
		rf	= zlog_flags[R]+C_FLAG;	\
	} else							\
	{	R 	= (R<<1)+(rf&0x01);		\
		rf	= zlog_flags[R];		\
	}

#define M_RR(R)						\
	if (R&0x01)						\
	{	R 	= (R>>1)+(rf<<7);		\
		rf	= zlog_flags[R]+C_FLAG;	\
	} else							\
	{	R 	= (R>>1)+(rf<<7);		\
		rf	= zlog_flags[R];		\
	}

#define M_SLA(R)					\
	rf	= R>>7;						\
	R <<= 1;						\
	rf |= zlog_flags[R]

#define M_SRA(R)					\
	rf	= R&0x01;					\
	R	= (R&0x80)+(R>>1);			\
	rf |= zlog_flags[R]

#define M_SLL(R)					\
	rf	= R>>7;						\
	R	= (R<<1)+1;					\
	rf |= zlog_flags[R]

#define M_SRL(R)					\
	rf	= R&0x01;					\
	R >>= 1;						\
	rf |= zlog_flags[R]


/*	BIT:	set/clr	Z
			clear	N=0, H=1
			pres	C
			takes other flags from corresponding bits in tested byte!
*/
#define M_BIT(N,R)								\
	rf	= (rf&C_FLAG) + 						\
		  (R&(S_FLAG+P_FLAG)) + 				\
		   H_FLAG + 							\
		  ((R&N)?0:Z_FLAG)


/*	ADD ... CP:	set/clr	Z, S, V, C, N, H
				pres	none
*/
#define M_ADD(R)								\
	wm	= ra+R;									\
	rf	= wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			 + (~(ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			 + ((ra^R^wml)&H_FLAG);				\
	ra	= wml

#define M_SUB(R)								\
	wm	= ra-R;									\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;	\
	ra	= wml

#define M_ADC(R)								\
	wm	= ra+R+(rf&C_FLAG);						\
	rf	= wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			 + (~(ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			 + ((ra^R^wml)&H_FLAG);				\
	ra	= wml

#define M_SBC(R)								\
	wm	= ra-R-(rf&C_FLAG);						\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;	\
	ra	= wml

#define M_CP(R)									\
	wm	= ra-R;									\
	rf	= -wmh + (wml?0:Z_FLAG) + (wml&S_FLAG)	\
			  + ((ra^R)&(wml^ra)&0x80?V_FLAG:0)	\
			  + ((ra^R^wml)&H_FLAG) + N_FLAG;


/*	AND ... XOR:	set/clr	Z, P, S
					clear	C=0, N=0, H=0/1 (OR,XOR/AND)
					pres	none
*/
#define M_AND(R)								\
	ra &= R;									\
	rf	= H_FLAG|zlog_flags[ra]

#define M_OR(R)									\
	ra |= R;									\
	rf	= zlog_flags[ra]

#define M_XOR(R)								\
	ra ^= R;									\
	rf	= zlog_flags[ra]


/*	INC ... DEC:	set/clr	Z,P,S,H
					clear	N=0/1 (INC/DEC)
					pres	C
*/
#define M_INC(R)								\
	R++;										\
	rf	= (rf&C_FLAG) + 						\
		  (R?0:Z_FLAG) + 						\
		  (R&S_FLAG) + 							\
		  (R==0x80?V_FLAG:0) + 					\
		  (R&0x0F?0:H_FLAG)

#define M_DEC(R)								\
	R--;										\
	rf	= (rf&C_FLAG) + 						\
		  (R?0:Z_FLAG) + 						\
		  (R&S_FLAG) + 							\
		  (R==0x7F?V_FLAG:0) + 					\
		  (((R+1)&0x0F)?0:H_FLAG) +				\
		   N_FLAG


/*	ADDW:	set/clr	C
			clear	N=0
			pres	Z, P, S
			unkn	H
*/
#define M_ADDW(R1,R2)							\
	rf &= ~(N_FLAG+C_FLAG);						\
	rf |= ((uint32)R1+(uint32)R2)>>16;			\
	R1 += R2;


/*	ADCW, SBCW:	set/clr	C,Z,V,S
			clear	N=0/1 (ADC/SBC)
			unkn	H
			pres	none
*/
#define M_ADCW(R)								\
	wm	= HL+R+(rf&C_FLAG);						\
	rf	= (((uint32)HL+(uint32)R+(rf&C_FLAG))>>16)\
			+ (wm?0:Z_FLAG) + (wmh&S_FLAG)		\
			+ (~(HL^R)&(wm^HL)&0x8000?V_FLAG:0);\
	HL	= wm

#define M_SBCW(R)								\
	wm	= HL-R-(rf&C_FLAG);						\
	rf	= (((uint32)HL-(uint32)R-(rf&C_FLAG))>>31)\
			+ (wm?0:Z_FLAG) + (wmh&S_FLAG)		\
			+ ((HL^R)&(wm^HL)&0x8000?V_FLAG:0)	\
			+ N_FLAG;							\
	HL	= wm


/*	IN	set/clr	Z, P, S, H
		clear	N=0
		pres	C
*/
#define M_IN(R)									\
	INPUT(BC,R);								\
	rf	= (rf&C_FLAG) + zlog_flags[R]










