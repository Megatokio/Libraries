#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

#include "kio/kio.h"


/* ----	Calculate base 2 logarithm or 'position' of leftmost '1' bit -----------------
		2014-02-15: renamed log2 to msbit
		return value:
				msbit(n>0) = int(log(2,n))
		note:	msbit(n=1) = 0
		caveat:	msbit(n=0) = 0		// illegal argument!

		note: template does not work correctly because i don't know how to cast a signed type to a same-sized unsigned type
*/
//INL int msbnibble	( uint8 n )	 { return (n>7) + (n>3) + (n>1); }										// 0 ..  3
INL int	msbit		( uint8 n )	 { int b=0,i=4; do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 ..  7
INL int	msbit		( uint16 n ) { int b=0,i=8; do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 15
INL int	msbit		( uint32 n ) { int b=0,i=16;do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 31
INL int	msbit		( uint64 n ) { int b=0,i=32;do{ if(n>>i){n>>=i;b+=i;} }while((i>>=1)); return b; } 	// 0 .. 63
INL int	msbit		( int8 n )	 { return msbit(uint8(n));  }
INL int	msbit		( int16 n )	 { return msbit(uint16(n)); }
INL int	msbit		( int32 n )	 { return msbit(uint32(n)); }
INL int	msbit		( int64 n )	 { return msbit(uint64(n)); }


/* ----	Calculate the number of digits required to print a number:
		return value:
				binaryDigits(n=0) = 1
				binaryDigits(n>0) = ceil(msbit(n+1))
*/
template <class T> INL int	binaryDigits ( T number )	 { return msbit(number)  +1; }	// result >= 1
template <class T> INL int	hexDigits	 ( T number )	 { return msbit(number)/4+1; }	// result >= 1


/* ----	Calculate the number of digits required to store a numbers of a given range:

		return value:
				reqBits(n) = ceil(msbit(n))
		note:	reqBits(1) = ceil(msbit(1)) = 0
		caveat:	reqBits(0) = ceil(msbit(0)) = 0		// illegal range!
*/
template <class T> INL int	reqBits		( T count )	{ return count>1 ? msbit(count-1)  +1 : 0; }
template <class T> INL int	reqNibbles	( T count )	{ return count>1 ? msbit(count-1)/4+1 : 0; }
template <class T> INL int	reqBytes	( T count )	{ return count>1 ? msbit(count-1)/8+1 : 0; }









