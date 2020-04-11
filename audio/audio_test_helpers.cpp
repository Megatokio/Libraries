/*	Copyright  (c)	Günter Woigk 2018 - 2020
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

#define LOGLEVEL 1
#define SAFETY 1
#include "kio/kio.h"
#include "audio_test_helpers.h"


// A-Law and µLaw reference implementations from
// http://dystopiancode.blogspot.de/2012/02/pcm-law-and-u-law-companding-algorithms.html

int8 ALaw_Encode(int16 number)
{
   const uint16 ALAW_MAX = 0xFFF;
   uint16 mask = 0x800;
   uint8 sign = 0;
   uint8 position = 11;
   uint8 lsb = 0;

   if (number < 0)
   {
	  number = -number;
	  sign = 0x80;
   }
   if (number > ALAW_MAX)
   {
	  number = ALAW_MAX;
   }
   for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--);
   lsb = (number >> ((position == 4) ? (1) : (position - 4))) & 0x0f;
   return (sign | ((position - 4) << 4) | lsb) ^ 0x55;
}

int16 ALaw_Decode(int8 number)
{
   uint8 sign = 0x00;
   uint8 position = 0;
   int16 decoded = 0;

   number^=0x55;
   if(number&0x80)
   {
	  number&=~(1<<7);
	  sign = -1;
   }
   position = ((number & 0xF0) >>4) + 4;
   if(position!=4)
   {
	  decoded = ((1<<position)|((number&0x0F)<<(position-4))
				|(1<<(position-5)));
   }
   else
   {
	  decoded = (number<<1)|1;
   }
   return (sign==0)?(decoded):(-decoded);
}

int8 MuLaw_Encode(int16 number)
{
   const uint16 MULAW_MAX = 0x1FFF;
   const uint16 MULAW_BIAS = 33;
   uint16 mask = 0x1000;
   uint8 sign = 0;
   uint8 position = 12;
   uint8 lsb = 0;

   if (number < 0)
   {
	  number = -number;
	  sign = 0x80;
   }
   number += MULAW_BIAS;
   if (number > MULAW_MAX)
   {
	  number = MULAW_MAX;
   }
   for (; ((number & mask) != mask && position >= 5); mask >>= 1, position--)
		;
   lsb = (number >> (position - 4)) & 0x0f;
   return (~(sign | ((position - 5) << 4) | lsb));
}

int16 MuLaw_Decode(int8 number)
{
   const uint16 MULAW_BIAS = 33;
   uint8 sign = 0, position = 0;
   int16 decoded = 0;

   number = ~number;
   if (number & 0x80)
   {
	  number &= ~(1 << 7);
	  sign = -1;
   }
   position = ((number & 0xF0) >> 4) + 5;
   decoded = ((1 << position) | ((number & 0x0F) << (position - 4))
			 | (1 << (position - 5))) - MULAW_BIAS;
   return (sign == 0) ? (decoded) : (-(decoded));
}




















