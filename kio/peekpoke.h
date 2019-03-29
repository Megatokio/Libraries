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


---	byte order independent and unaligned peek & poke ---

	PeekX => 'abcd' == *(uint32*)"abcd"	--> access data written by m68k, ppc, ...	<-- this is the internet order
	PeekZ => 'abcd' == *(uint32*)"dcba"	--> access data written by i386, z80, ...	<-- avoid if possible
	peek  =>  machine order, unaligned. --> glue for machines which don't support unaligned memory access.
*/

#include "kio/kio.h"
#include <utility>


#define P(i)	(u8ptr(p))[i]


// peek / poke in Big Endian or Network Byte Order: High Byte first:
// this is also the native byte order of 68000 and PPC CPUs.
//
#if defined(_BIG_ENDIAN) && !_ALIGNMENT_REQUIRED
	inline uint8  peek1X (void const* p)		noexcept { return *u8ptr(p);  }
	inline uint16 peek2X (void const* p)		noexcept { return *u16ptr(p); }
	inline uint32 peek4X (void const* p)		noexcept { return *u32ptr(p); }
	inline uint64 peek8X (void const* p)		noexcept { return *u64ptr(p); }
	inline void   poke1X (void* p, uint8 n) 	noexcept { *u8ptr(p)  = n; }
	inline void   poke2X (void* p, uint16 n)	noexcept { *u16ptr(p) = n; }
	inline void   poke4X (void* p, uint32 n)	noexcept { *u32ptr(p) = n; }
	inline void   poke8X (void* p, uint64 n)	noexcept { *u64ptr(p) = n; }
#else
	inline uint8  peek1X (void const* p)		noexcept { return P(0); }
	inline uint16 peek2X (void const* p)		noexcept { return uint16(P(0)<<8  | P(1)); }
	inline uint32 peek4X (void const* p)		noexcept { return uint32(P(0)<<24 | P(1)<<16 | P(2)<<8 | P(3)); }
	inline uint64 peek8X (void const* p)		noexcept { return uint64(peek4X(p))<<32 | peek4X(ptr(p)+4); }
	inline void   poke1X (void* p, uint8 n) 	noexcept { P(0) = n; }
	inline void   poke2X (void* p, uint16 n)	noexcept { P(0) = n>>8;  P(1) = uint8(n); }
	inline void   poke4X (void* p, uint32 n)	noexcept { for (int i=4; i--;) { P(i) = uint8(n); n >>= 8; } }
	inline void   poke8X (void* p, uint64 n)	noexcept { for (int i=8; i--;) { P(i) = uint8(n); n >>= 8; } }
#endif
	inline void   poke3X (void* p, uint32 n)	noexcept { P(0) = uint8(n>>16); poke2X(ptr(p)+1,uint16(n)); }
	inline uint32 peek3X (void const* p)		noexcept { return uint32(P(0)<<16 | peek2X(ptr(p)+1)); }


// peek / poke in Little Endian Byte Order: Low Byte first:
// this is the native byte order of Z80 and i386 CPUs.
//
#if defined(_LITTLE_ENDIAN) && !_ALIGNMENT_REQUIRED
	inline uint8  peek1Z (void const* p)		noexcept { return *u8ptr(p);  }
	inline uint16 peek2Z (void const* p)		noexcept { return *u16ptr(p); }
	inline uint32 peek4Z (void const* p)		noexcept { return *u32ptr(p); }
	inline uint64 peek8Z (void const* p)		noexcept { return *u64ptr(p); }
	inline void   poke1Z (void* p, uint8 n) 	noexcept { *u8ptr(p)  = n; }
	inline void   poke2Z (void* p, uint16 n)	noexcept { *u16ptr(p) = n; }
	inline void   poke4Z (void* p, uint32 n)	noexcept { *u32ptr(p) = n; }
	inline void   poke8Z (void* p, uint64 n)	noexcept { *u64ptr(p) = n; }
#else
	inline uint8  peek1Z (void const* p)		noexcept { return								 P(0); }
	inline uint16 peek2Z (void const* p)		noexcept { return					   P(1)<<8 | P(0); }
	inline uint32 peek4Z (void const* p)		noexcept { return P(3)<<24 | P(2)<<16 | P(1)<<8 | P(0); }
	inline uint64 peek8Z (void const* p)		noexcept { return ((uint64)peek4X((ptr)p+4)<<32) + peek4X(p); }
	inline void   poke1Z (void* p, uint8 n)		noexcept {								     P(0)=n; }
	inline void   poke2Z (void* p, uint16 n)	noexcept {					      P(1)=n>>8; P(0)=n; }
	inline void   poke4Z (void* p, uint32 n)	noexcept { P(3)=n>>24; P(2)=n>>16; P(1)=n>>8; P(0)=n; }
	inline void   poke8Z (void* p, uint64 n)	noexcept { for(uint i=8;i--;) P(i) = n>>(i<<3); }
#endif
	inline void   poke3Z (void* p, uint32 n)	noexcept { P(2) = uint8(n>>16); poke2Z(p,uint16(n)); }
	inline uint32 peek3Z (void const* p)		noexcept { return uint32(P(2)<<16 | peek2Z(p)); }


#undef P


/* ----	unaligned peek & poke -------------------------------
		note: on PDP machines data is stored in little endian order.
		unaligned peek and Poke on PDP machines is braindead anyway.
*/
#ifdef _BIG_ENDIAN
	inline uint8  peek1 (void const* p)		noexcept { return Peek1X(p); }
	inline uint16 peek2 (void const* p)		noexcept { return peek2X(p); }
	inline uint32 peek3 (void const* p)		noexcept { return Peek3X(p); }
	inline uint32 peek4 (void const* p)		noexcept { return peek4X(p); }
	inline uint64 peek8 (void const* p)		noexcept { return peek8X(p); }

	inline void   poke1 (void* p, int8 n)	noexcept { Poke1X(p,n); }
	inline void   poke2 (void* p, int16 n)	noexcept { poke2X(p,n); }
	inline void   poke3 (void* p, int32 n)	noexcept { Poke3X(p,n); }
	inline void   poke4 (void* p, int32 n)	noexcept { poke4X(p,n); }
	inline void   poke8 (void* p, int64 n)	noexcept { poke8X(p,n); }

	inline void   poke1 (void* p, uint8 n)  noexcept { Poke1X(p,n); }
	inline void   poke2 (void* p, uint16 n) noexcept { poke2X(p,n); }
	inline void   poke3 (void* p, uint32 n) noexcept { Poke3X(p,n); }
	inline void   poke4 (void* p, uint32 n) noexcept { poke4X(p,n); }
	inline void   poke8 (void* p, uint64 n) noexcept { poke8X(p,n); }
#else
	inline uint8  peek1 (void const* p)		noexcept { return peek1Z(p); }
	inline uint16 peek2 (void const* p)		noexcept { return peek2Z(p); }
	inline uint32 peek3 (void const* p)		noexcept { return peek3Z(p); }
	inline uint32 peek4 (void const* p)		noexcept { return peek4Z(p); }
	inline uint64 peek8 (void const* p)		noexcept { return peek8Z(p); }

	inline void   poke1 (void* p, int   n)	noexcept { poke1Z(p,uint8(n)); }
	inline void   poke2 (void* p, int   n)	noexcept { poke2Z(p,uint16(n)); }
	inline void   poke3 (void* p, int32 n)	noexcept { poke3Z(p,uint32(n)); }
	inline void   poke4 (void* p, int32 n)	noexcept { poke4Z(p,uint32(n)); }
	inline void   poke8 (void* p, int64 n)	noexcept { poke8Z(p,uint64(n)); }

	inline void   poke1 (void* p, uint   n) noexcept { poke1Z(p,uint8(n)); }
	inline void   poke2 (void* p, uint   n) noexcept { poke2Z(p,uint16(n)); }
	inline void   poke3 (void* p, uint32 n) noexcept { poke3Z(p,n); }
	inline void   poke4 (void* p, uint32 n) noexcept { poke4Z(p,n); }
	inline void   poke8 (void* p, uint64 n) noexcept { poke8Z(p,n); }
#endif


/* ----	revert bytes in buffer ------------
*/
inline void revert_bytes(void* p, uint sz)
{
	ptr a = ptr(p);
	ptr e = a+(sz-1);
	while(a<e) { std::swap(*a++,*e--); }
}






