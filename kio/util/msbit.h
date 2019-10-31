#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "kio/kio.h"


/* ----	Calculate base 2 logarithm or 'position' of leftmost '1' bit -----------------
		2014-02-15: renamed log2 to msbit
		return value:
				msbit(n>0) = int(log(2,n))
		note:	msbit(n=1) = 0
		caveat:	msbit(n=0) = 0		// illegal argument!

		note: template does not work correctly because i don't know how to cast a signed type to a same-sized unsigned type
*/
//INL int msbnibble	( uint8 n )	{ return (n>7) + (n>3) + (n>1); }										// 0 ..  3
inline int	msbit ( uint8 n )	{ int b=0,i=4; do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 ..  7
inline int	msbit ( uint16 n )	{ int b=0,i=8; do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 15
inline int	msbit ( uint32 n )	{ int b=0,i=16;do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 31
inline int	msbit ( uint64 n )	{ int b=0,i=32;do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 63
inline int	msbit ( int8 n )	{ return msbit(uint8(n));  }
inline int	msbit ( int16 n )	{ return msbit(uint16(n)); }
inline int	msbit ( int32 n )	{ return msbit(uint32(n)); }
inline int	msbit ( int64 n )	{ return msbit(uint64(n)); }


/* ----	Calculate the number of digits required to print a number:
		return value:
				binaryDigits(n=0) = 1
				binaryDigits(n>0) = ceil(msbit(n+1))
*/
template <class T> inline int	binaryDigits ( T number )	 { return msbit(number)  +1; }	// result >= 1
template <class T> inline int	hexDigits	 ( T number )	 { return msbit(number)/4+1; }	// result >= 1


/* ----	Calculate the number of digits required to store a numbers of a given range:

		return value:
				reqBits(n) = ceil(msbit(n))
		note:	reqBits(1) = ceil(msbit(1)) = 0
		caveat:	reqBits(0) = ceil(msbit(0)) = 0		// illegal range!
*/
//template <class T> inline int	reqBits		( T count )	{ return count>1 ? msbit(count-1)  +1 : 0; }		TEST NEEDED!
//template <class T> inline int	reqNibbles	( T count )	{ return count>1 ? msbit(count-1)/4+1 : 0; }		TEST NEEDED!
//template <class T> inline int	reqBytes	( T count )	{ return count>1 ? msbit(count-1)/8+1 : 0; }		TEST NEEDED!









