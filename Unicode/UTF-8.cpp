/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

UTF-8 Encoding:
---------------

	UTF-8 encodes UCS-2 or UCS-4 characters into sequences of up to 6 bytes.
	UCS-2 characters can be represented in max. 3 byte long sequences.
	7-bit us-ascii characters map 1:1 to 1-byte UTF-8 characters.

	Each multi-byte character starts with a byte		in range  0xC0 … 0xFF
	and is followed by the defined amount of follow-ups in range  0x80 … 0xBF
	Golden rule in case of broken characters: each non-fup makes a character!

	1 byte:		0x00 … 0x7F		-> 0xxxxxxx						map 1:1 to us ascii
	2 bytes:	0xC0 … 0xDF		-> 110xxxxx 10xxxxxx
	3 bytes:	0xE0 … 0xEF		-> 1110xxxx 10xxxxxx 10xxxxxx
	4 bytes:	0xF0 … 0xF7		-> 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	5 bytes:	0xF8 … 0xFB		-> 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	6 bytes:	0xFC … 0xFF		-> 111111xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

The encoding scheme allows for representing some codes with a regular, shortest	encoding or
	with an irregular, overlong encoding. The standard suggests to treat overlong encodings
	as illegal for security reasons.

Illegal overlong encodings:
	1100000x 10xxxxxx
	11100000 100xxxxx 10xxxxxx
	11110000 1000xxxx 10xxxxxx 10xxxxxx
	11111000 10000xxx 10xxxxxx 10xxxxxx 10xxxxxx
	11111100 100000xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx

Legal unicode character code points are in range  0x0000 … 0x10FFFF only.
	This is max. 21 bits and encodes to max. 4 bytes in UTF-8.

	0000 …			characters, unicode
	D800 …			non-chars,	high surrogate, non-private use
	DB80 …			non-chars,	high surrogate, private use
	DC00 …			non-chars,	low surrogate
	E000 …			characters, private use
	F900 …			characters, unicode

	010000 …		characters, unicode
	0F0000 …		characters, private use plane 15
	100000 …		characters, private use plane 16

	110000 …		non-chars,  illegal code points
	80000000 …		non-chars,  bit 31 set: completely illegal

	Also, there are multiple unassigned code points all through the legal code ranges.

	UTF-8 can store up to 21 bits in up to 3 bytes, which is sufficient for all legal unicode characters.
	UTF-8 can store up to 31 bits in up to 6 bytes, which is sufficient for all positive signed long values.
	UTF-8 in this implementation can store full 32 bits in up to 6 bytes for lossless UTF-8 <-> UCS-4 conversion.
	UCS-2 can store characters of plane 0 only, range 0000 … FFFF.
	UTF-16 is similar to UCS-2 but adds plane 1 to 16 by use of surrogate code pairs.
	UCS-4 can store all legal character codes and beyond.

Surrogate character pairs:
	A code pair of a high and a low surrogate define a code point in plane 1 - 16, range 010000 … 110000.

	D800…DBFF + DC00…DFFF	=	110110xx.xxxxxxxx + 110111yy.yyyyyyyy	=	xxxxyyyy + 10000
*/


#include "UTF-8.h"
#define	RMASK(n)		(~(0xFFFFFFFF<<(n)))								// mask to select n bits from the right


/* ----	convert UTF-8 char to UCS-4 -------------------------------
		stops at next non-fup or at nominal char size
		sets errno and returns replacement char on error
		char(0) is a valid character
		2005-06-09: start codes $FE and $FF for full 32 bit char support added
*/
UCS4Char UCS4CharFromUTF8 ( UTF8Char s )		/*TODO*/
{	UCS4Char n; uint i; char c;

	assert(s);

	n = uchar ( *s );						// UCS-4 char code akku
	if (utf8_is_7bit(n)) return n;			// 7-bit ascii char
	if (utf8_is_fup(n)) goto x1;			// 0x80 … 0xBF: unexpected fups		NOTE: creates a char !!!

// longish character:
	i = 0;									// UTF-8 character size
	c = n & ~0x02;							// force stop at i=6
	while( char(c<<(++i)) < 0 )				// loop over fup bytes
	{
		uchar c1 = *(++s); if( utf8_no_fup(c1) ) goto x3;
		n = (n<<6) + (c1&0x3F);
	}

// now: i = total number of digits
//      n = UCS4 char code with some of the '1' bits from c0
	n &= RMASK(2+i*5);

// ill. overlong encodings:
	if ( n < 1u<<(i*5-4) ) goto x4;		// ill. overlong encoding

// ok => return code
	return n;

// error => return replacement char
	x1:	errno=unexpectedfup;   return UCS4ReplacementChar;
	x3:	errno=truncatedchar;   return UCS4ReplacementChar;
	x4:	errno=illegaloverlong; return UCS4ReplacementChar;
}


/* ----	convert UCS4 code -> UTF-8 character --------------
		returns 1 .. 6 byte 0-terminated string with UTF-8 encoding
		except for n==0, where char(0) itself is the result.
		returned string may be allocated in qstring pool
		2005-06-09: support for full 32 bit added
*/
UTF8Char UTF8CharFromUCS4 ( UCS4Char n )		/*TODO*/
{
// 7-bit ascii:
	if (n<0x80) return charstr(n);	// n==0 => ""; else 1-char string

// 2-byte codes:
	if ( n<(1u<<11) ) { str s=tempstr(2); s[0]=0xC0+(n>>6); s[1]=0x80+(n&0x3f); return s; }

// multi-byte codes 3 to 6 bytes:
	uint i = 21; for( ushort m = n>>16; m; m>>=5 ) { i+=5; } i = i/6;		// utf-8 character size
	str s = tempstr(i);
	char c0 = 0x80;
	while (--i>0) { c0 >>= 1; s[i] = 0x80+(n&0x3f); n >>= 6; }
	s[0] = c0 + n;
	return s;
}


void UCS4CharToUTF8 ( UCS4Char n, UTF8CharPtr& z )		/*TODO*/
{
	if ( n<(1<<7)  ) { *z++ = n; return; }									// 1 byte code		(7 bit ascii)
	if ( n<(1<<11) ) { *z++ = 0xC0+(n>>6); *z++ = 0x80+(n&0x3f); return; }	// 2 byte code
// 3…6 byte codes
// note the trick to make 32 bits a 6 byte code too:					// i = num fups =	15/6=2	20/6=3	25/6=4	30/6=5	35/6=5
	int i = 15; for( ushort m = n>>16; m; m>>=5 ) { i+=5; } i = i/6;	// i = num fups; num bits stored in fups = i*6
	*z++ = (char(0x80)>>i) + (n>>(i*6));								// starter (non_fup)
	while(i--) { *z++ = 0x80 + ((n>>(i*6))&0x3f); }						// fups
}



/* ********************************************************************************
					SIMPLE CASING
******************************************************************************** */


void utf8_simple_uppercase		( cUTF8CharPtr q, UTF8CharPtr& z )	{ UCS4CharToUTF8(UCS4CharSimpleUppercase(UCS4CharFromUTF8(q)),z); }
void utf8_simple_lowercase		( cUTF8CharPtr q, UTF8CharPtr& z )	{ UCS4CharToUTF8(UCS4CharSimpleLowercase(UCS4CharFromUTF8(q)),z); }
void utf8_simple_titlecase		( cUTF8CharPtr q, UTF8CharPtr& z )	{ UCS4CharToUTF8(UCS4CharSimpleTitlecase(UCS4CharFromUTF8(q)),z); }
UTF8Char utf8_simple_uppercase	( UTF8Char q )						{ str s = tempstr(6); ptr z = s; utf8_simple_uppercase(q,z); return s;  }
UTF8Char utf8_simple_lowercase	( UTF8Char q )						{ str s = tempstr(6); ptr z = s; utf8_simple_lowercase(q,z); return s;  }
UTF8Char utf8_simple_titlecase	( UTF8Char q )						{ str s = tempstr(6); ptr z = s; utf8_simple_titlecase(q,z); return s;  }








