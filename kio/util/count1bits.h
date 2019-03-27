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


INL uint count1bits( uint8 z )
{
	uint rval = z;
	rval = ((rval & 0xAAu) >> 1) + (rval & 0x55u);
	rval = ((rval & 0xCCu) >> 2) + (rval & 0x33u);
	rval = ((rval & 0xF0u) >> 4) + (rval & 0x0Fu);
	return rval;
}

INL uint count1bits( uint16 z )
{
	uint rval = z;
	rval = ((rval & 0xAAAAu) >>  1) + (rval & 0x5555u);
	rval = ((rval & 0xCCCCu) >>  2) + (rval & 0x3333u);
	rval = ((rval & 0xF0F0u) >>  4) + (rval & 0x0F0Fu);
	rval = ((rval & 0xFF00u) >>  8) + (rval & 0x00FFu);
	return rval;
}

INL uint count1bits( uint32 z )
{
	z = ((z & 0xAAAAAAAAu) >>  1) + (z & 0x55555555u);
	z = ((z & 0xCCCCCCCCu) >>  2) + (z & 0x33333333u);
	z = ((z & 0xF0F0F0F0u) >>  4) + (z & 0x0F0F0F0Fu);
	z = ((z & 0xFF00FF00u) >>  8) + (z & 0x00FF00FFu);
	z = ((z & 0xFFFF0000u) >> 16) + (z & 0x0000FFFFu);
	return z;
}

INL uint count1bits( int8 z )	{ return count1bits((uint8)z); }
INL uint count1bits( int16 z )	{ return count1bits((uint16)z); }
INL uint count1bits( int32 z )	{ return count1bits((uint32)z); }


// count bits in array:
//
INL uint32 count1bits( cu8ptr p, uint32 bytes )
{
	uint32 rval = 0;
	uint32 n;
	if(bytes<4) goto b;
	n=0; while(size_t(p)&3)	{ n = (n<<8) + *p++; bytes--; }

a:	rval += count1bits(n);
	if(bytes>=4) { n = *(uint32*)p; p+=4; bytes-=4; goto a; }

b:	n=0; while(bytes) { n = (n<<8) + *p++; bytes--; }
	if(n) goto a;
	return rval;
}


// count bits in array:
// if bits&7 != 0 then the last byte must be filled left-aligned (msbit)
//
INL uint32 count1bitsL( cu8ptr p, uint32 bits )
{
	uint32 rval = 0;
	uint32 n;
	if(bits<32) goto b;
	n=0; while(size_t(p)&3)	{ n = (n<<8) + *p++; bits-=8; }

a:	rval += count1bits(n);
	if(bits>=32) { n = *(uint32*)p; p+=4; bits-=32; goto a; }

b:	n=0; while(bits>=8) { n = (n<<8) + *p++; bits-=8; }
	if(bits) { n = ((n<<8)+*p)>>(8-bits); bits=0; }
	if(n) goto a;
	return rval;
}


// count bits in array:
// if bits&7 != 0 then the last byte must be filled right-aligned (lsb)
//
INL uint32 count1bitsR( cu8ptr p, uint32 bits )
{
	uint32 rval = count1bits(p,bits/8);
	if(bits&7) rval += count1bits( uint8(p[bits/8]<<(8-(bits&7))) );
	return rval;
}







