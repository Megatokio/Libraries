#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2015
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
*/


// ----	Nice defines -------------------------------------------------------

#ifndef MIN
	#define MIN(a,b)	((a)<(b)?(a):(b))
#endif
#ifndef MAX
	#define MAX(a,b)	((a)>(b)?(a):(b))
#endif
#ifndef ABS
	#define ABS(a)   	((a)<0?-(a):(a))
#endif

//#define	LIMIT(A,N,E)	if (N<(A)) N=(A); else if (N<=(E)) {} else N=(E)	// [A … E]
//#define	SWAP(a,b)		((a)^=(b),(b)^=(a),(a)^=(b))
//#define	SORT(a,b)		if ((a)<=(b)) {} else SWAP(a,b)
#define	SIGN(a)			(int((a)>0) - int((a)<0))
#define	NELEM(feld)		(sizeof(feld)/sizeof((feld)[0]))					// UNSIGNED !!
//#define BIT(B,N)		(((N)>>(B))&1)										// get bit value at bit position B
#define	BITMASK(i,n)	((0xFFFFFFFF<<(i)) ^ (0xFFFFFFFF<<((i)+(n))))		// mask to select bits ]i+n .. i]
#define	RMASK(n)		(~(0xFFFFFFFF<<(n)))								// mask to select n bits from the right
#define LMASK(i)		(  0xFFFFFFFF<<(i) )								// mask to select all but the i bits from right



// ----	Calculate base 2 logarithm or 'position' of leftmost '1' bit -----------------
//		note: for 0x00000000 the returned value is 0 as for 0x00000001
//		note: overloaded functions for c++ below
#define LOG2NIBBLE(N)	(((N)>7)+((N)>3)+((N)>1))											/* 0 ...  3 */
#define LOG2BYTE(N)		(uint8(N) >  0x0Fu ? 4 + LOG2NIBBLE(uint8 (N)>>4 ) : LOG2NIBBLE(N))	/* 0 ...  7 */
#define LOG2SHORT(N)	(uint16(N)>0x00FFu ? 8 + LOG2BYTE  (uint16(N)>>8 ) : LOG2BYTE(N)  )	/* 0 ... 15 */
#define LOG2(N)			(uint32(N)>0xFFFFu ? 16+ LOG2SHORT (uint32(N)>>16) : LOG2SHORT(N) )	/* 0 ... 31 */












