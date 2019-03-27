#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2015
  					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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












