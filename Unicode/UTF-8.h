#pragma once
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
*/

#include "Unicode.h"
#define INL	inline


/* ********************************************************************************
						TYPE DEFINITIONS
******************************************************************************** */

typedef char const *		UTF8Char;
typedef char const *		cUTF8Char;
typedef char *				UTF8CharPtr;
typedef char const *		cUTF8CharPtr;

typedef	char *				UTF8Str;			// c-style string
typedef char const *		cUTF8Str;			// c-style string literals in utf-8 encoded source files



/* ********************************************************************************
					UTF-8 CHARACTER ENCODING
******************************************************************************** */

// replacement character for illegal, truncated, unprintable etc. characters
UTF8Char const 	UTF8ReplacementChar		= "?";			// offiziell: $FFFD => 3 Bytes => String wächst!!
size_t const  	UTF8ReplacementCharSize	= 1;

// Classification for 1st byte of utf-8 character
INL bool		utf8_is_null		( signed char c )	{ return c == 0; }
INL bool		utf8_is_7bit		( signed char c )	{ return c >= 0; }			// %0xxxxxxx = ascii
INL bool		utf8_no_7bit		( signed char c )	{ return c < 0; }
INL bool		utf8_is_fup			( signed char c )	{ return c < int8(0xc0); }	// %10xxxxxx = fup
INL bool		utf8_no_fup			( signed char c )	{ return c >= int8(0xc0); }
INL bool		utf8_is_starter		( signed char c )	{ return uchar(c)>=0xc0; }
INL bool		utf8_is_c1			( signed char c )	{ return c>=0; }			// == utf8_is_7bit
INL bool		utf8_is_c2			( signed char c )	{ return (c&0xe0)==0xc0; }	// %110xxxxx
INL bool		utf8_is_c3			( signed char c )	{ return (c&0xf0)==0xe0; }	// %1110xxxx
INL bool		utf8_is_c4			( signed char c )	{ return (c&0xf8)==0xf0; }	// %11110xxx
INL bool		utf8_is_c5			( signed char c )	{ return (c&0xfc)==0xf8; }	// %111110xx
INL bool		utf8_is_c6			( signed char c )	{ return uchar(c)>=0xfc; }	// %1111110x 2005-06-11 full 32 bit support
INL bool 		utf8_is_ucs4		( signed char c )	{ return uchar(c)> 0xf0; }	// 2015-01-02 doesn't fit in ucs2?
INL bool 		utf8_requires_c4	( signed char c )	{ return uchar(c)>=0xf0; }	// 2015-01-02 requires processing of c4/c5/c6?


// UCSx char -> 1st char for UTF8
INL char		utf8_fup			( UCS1Char c )		{ return char(0x80) | char(c&0x3f); }
INL char		utf8_starter_c2		( UCS1Char c )		{ return char(0xc0) | char(c>>6);   }
INL char		utf8_starter_c2		( UCS2Char c )		{ return char(0xc0) | char(c>>6);   }
INL char		utf8_starter_c2		( UCS4Char c )		{ return char(0xc0) | char(c>>6);   }
INL char		utf8_starter_c3		( UCS2Char c )		{ return char(0xe0) | char(c>>12);  }

// nominal size of utf-8 character (bytes) based on char[0] of UTF-8 character
// fups  are size = 1
// 2005-06-11: added support for full 32 bit characters
INL int			UTF8CharNominalSize	( char c )			{ int n=1; if (c<0) { c&=~0x02; while(char(c<<n)<0) n++; } return n; }
INL int			UTF8CharNominalSize	( cUTF8CharPtr p )	{ return UTF8CharNominalSize(*p); }

// nominal size of utf-8 character (bytes) based on UCS-1, UCS-2, or UCS-4 character
// 2005-06-11: added support for full 32 bit characters
INL int			UTF8CharNominalSize	( UCS1Char n )		{ return 1 + (n>0x7f); }
INL int			UTF8CharNominalSize	( UCS2Char n )		{ return 1 + (n>0x7f) + (n>0x7ff); }
INL int			UTF8CharNominalSize	( UCS4Char n )		{ if(n<0x80) return 1; int i=16; n>>=11; while(n) { n>>=5; i+=5; } return i/6; }
																			// note:	16/6=2	21/6=3	26/6=4	31/6=5	36/6=6	41/6=6
// find next character in utf-8 string
// skip current assumed non-fup byte and find next
INL void			UTF8SkipChar	( cUTF8CharPtr& p )					{ while (		 utf8_is_fup(*++p) ) {} }
INL cUTF8CharPtr	UTF8NextChar	( cUTF8CharPtr p )					{ while (		 utf8_is_fup(*++p) ) {} return p; }
INL cUTF8CharPtr	UTF8NextChar	( cUTF8CharPtr p, cUTF8CharPtr e )	{ while ( ++p<e && utf8_is_fup(*p) ) {} return p; }

// find previous character in utf-8 string
// rskip current assumed non-fup byte and find previous
INL void			UTF8RSkipChar	( cUTF8CharPtr& p )					{ while (           utf8_is_fup(*--p) ) {} }
INL cUTF8CharPtr	UTF8PrevChar	( cUTF8CharPtr p )					{ while (           utf8_is_fup(*--p) ) {} return p; }
INL  UTF8CharPtr	UTF8PrevChar	(  UTF8CharPtr p )					{ while (           utf8_is_fup(*--p) ) {} return p; }
INL cUTF8CharPtr	UTF8PrevChar	( cUTF8CharPtr a, cUTF8CharPtr p )	{ while ( --p>=a && utf8_is_fup(*  p) ) {} return p; }

// find first character in utf-8 string
// skip fups, find next non-fup byte
INL cUTF8CharPtr	UTF8FirstChar	( cUTF8CharPtr a )					{ while (        utf8_is_fup(*a) ) a++; return a; }
INL cUTF8CharPtr	UTF8FirstChar	( cUTF8CharPtr a, cUTF8CharPtr e )	{ while ( a<e && utf8_is_fup(*a) ) a++; return a; }

INL void			UTF8SkipFups	( cUTF8CharPtr& a )					{ while (        utf8_is_fup(*a) ) a++; }
INL void			UTF8SkipFups	( cUTF8CharPtr& a, cUTF8CharPtr e )	{ while ( a<e && utf8_is_fup(*a) ) a++; }
INL void			UTF8SkipFups	(  UTF8CharPtr& a, cUTF8CharPtr e )	{ while ( a<e && utf8_is_fup(*a) ) a++; }

// find last char in utf-8 string
// rskip current non-fup byte and find previous
// identical to UTF8PrevChar()
INL cUTF8CharPtr	UTF8LastChar	( cUTF8CharPtr e )					{ while(utf8_is_fup(*--e)) {} return e; }
INL cUTF8CharPtr	UTF8LastChar	( cUTF8CharPtr a, cUTF8CharPtr e )	{ while(--e>=a&&utf8_is_fup(*e)) {} return e; }

INL void			UTF8RSkipFups	( cUTF8CharPtr& p )					{ while (         utf8_is_fup(*p) ) p++; }
INL void			UTF8RSkipFups	( cUTF8CharPtr& a, cUTF8CharPtr p )	{ while ( p>=a && utf8_is_fup(*p) ) p++; }
INL void			UTF8RSkipFups	(  UTF8CharPtr& a, cUTF8CharPtr p )	{ while ( p>=a && utf8_is_fup(*p) ) p++; }

#if 0	// ----	USAGE ----

/*		utf-8 chars in a string may be corrupted.
		running blind through a string can result in a disaster.
		special care must be taken for the first and the last character in a string.
		also remember the 'golden rule': every non-fup byte makes a character!

	Example 1: running a function on all characters of a 0-terminated c-string.
		c-style strings are 'easy', because the terminating char(0) is a valid utf-8 char
		and automatically stops the parsing of the last utf-8 character in string, even if it is truncated:
*/
	void MyStringFoo ( char const * src )
	{
		ptr a = UTF8FirstChar ( src );			// skip preceding bogus fup bytes

		while ( *a )
		{
			MyCharFoo ( a );					// do foo with the current char
			UTF8SkipChar ( a );					// skip to next
		}
	}

/*	Example 2: running a function on all characters of a buffer without special end marker
		there is no terminating char(0) which would automatically stop the parsing
		of the last utf-8 character, in case it is truncated:
*/
	void MyStringFoo ( char const * bu, char const * bu_end )
	{
		cptr a = UTF8FirstChar ( bu, bu_end );	// skip preceding bogus fup bytes
		if ( a>=e ) return;						// no utf-8 characters in buffer ((maybe some fups))
		cptr e = UTF8LastChar ( bu_end );		// find start of last utf-8 character

		if ( e+UTF8CharNominalSize(e) <= bu_end )
		{
			e = bu_end;							// last character is not truncated => include it in loop
		}										// evtl. trailing bogus fups are ignored

		while ( a<e )
		{
			MyCharFoo ( a );					// do foo with the current char
			UTF8SkipChar ( a );					// skip to next
		}

		if ( e<bu_end )							// don't forget the truncated last char: remember the 'golden rule'!
		{										// running foo on the truncated char directly will run beyond string end!
			MyCharFoo ( UTF8ReplacementChar );	// do foo with a replacement for truncated last char
		}
	}
#endif



/* ********************************************************************************
					UCS4 <-> UTF8 CHARACTER CONVERSION
******************************************************************************** */

extern	UCS4Char		UCS4CharFromUTF8		( cUTF8Char s );
extern	UTF8Char		UTF8CharFromUCS4		( UCS4Char n );				// returns UTF8Char in cstring pool
extern	void			UCS4CharToUTF8			( UCS4Char n, UTF8CharPtr& dest );



/* ********************************************************************************
						PROPERTIES
******************************************************************************** */

INL U_PropertyValue	UTF8CharBlockProperty	( UTF8Char p )	{ return UCS4CharBlockProperty(UCS4CharFromUTF8(p) ); }
INL U_PropertyValue	UTF8CharScriptProperty	( UTF8Char p )	{ return UCS4CharScriptProperty(UCS4CharFromUTF8(p) ); }
INL U_PropertyValue	UTF8CharCccProperty		( UTF8Char p )	{ return UCS4CharCccProperty(UCS4CharFromUTF8(p) ); }
INL U_PropertyValue	UTF8CharGeneralCategory	( UTF8Char p )	{ return UCS4CharGeneralCategory(UCS4CharFromUTF8(p) ); }
INL cstr			UTF8CharCharacterName	( UTF8Char p )	{ return *p>=0 ? UCS1CharCharacterName(UCS1Char(*p))
																: UCS4CharCharacterName(UCS4CharFromUTF8(p)); }
INL U_PropertyValue	UTF8CharEAWidthProperty ( UTF8Char p )	{ return UCS4CharEAWidthProperty(UCS4CharFromUTF8(p)); }
INL uint			UTF8CharPrintWidth		( UTF8Char p )	{ return UCS4CharPrintWidth(UCS4CharFromUTF8(p)); }		// 0, 1, or 2



/* ********************************************************************************
						SIMPLE CASING
******************************************************************************** */

// UPs:
	extern	UCS1Char const UCS1_SUC_Table[0x100];	// uc(ß)=ß, uc(µ)=µ, uc(ÿ)=Y
	extern	UCS1Char const UCS1_SLC_Table[0x100];
	extern	UTF8Char	utf8_simple_lowercase	( UTF8Char );
	extern	void		utf8_simple_lowercase	( cUTF8CharPtr, UTF8CharPtr& );
	extern	UTF8Char	utf8_simple_uppercase	( UTF8Char );
	extern	void		utf8_simple_uppercase	( cUTF8CharPtr, UTF8CharPtr& );
	extern	UTF8Char	utf8_simple_titlecase	( UTF8Char );
	extern	void		utf8_simple_titlecase	( cUTF8CharPtr, UTF8CharPtr& );

// Simple Lowercase:
inline UTF8Char	UTF8SimpleLowercase	( UTF8Char p )
{
	return *p>=0 ? ptr(&UCS1_SLC_Table[uint(*p)]) : utf8_simple_lowercase(p);
}
inline void UTF8SimpleLowercase	( cUTF8CharPtr q, UTF8CharPtr& z )
{
	if (*q>=0) *z++ = char(UCS1_SLC_Table[uint(*q)]); else utf8_simple_lowercase(q,z);
}

// Simple Uppercase:
inline UTF8Char	UTF8SimpleUppercase	( UTF8Char p )
{
	return *p>=0 ? ptr(&UCS1_SUC_Table[uint(*p)]) : utf8_simple_uppercase(p);
}
inline void UTF8SimpleUppercase	( cUTF8CharPtr q, UTF8CharPtr& z )
{
	if (*q>=0) *z++ = char(UCS1_SUC_Table[uint(*q)]); else utf8_simple_uppercase(q,z);
}

// Simple Titlecase:
inline UTF8Char UTF8SimpleTitlecase ( UTF8Char p )
{
	return *p>=0 ? ptr(&UCS1_SUC_Table[uint(*p)]) : utf8_simple_titlecase(p);
}
inline void UTF8SimpleTitlecase ( cUTF8CharPtr q, UTF8CharPtr& z )
{
	if (*q>=0) *z++ = char(UCS1_SUC_Table[uint(*q)]); else utf8_simple_titlecase(q,z);
}

// Info:
__attribute__((deprecated))  // utf8.h: use utf8::is_is_uppercase()
inline bool UTF8CharIsUppercase ( UTF8Char p )		// <==>  "will change if converted to lowercase"
{
	return *p>=0 ? is_uppercase(*p) : uchar_in_range(U_gc_lt, UTF8CharGeneralCategory(p), U_gc_lu);
}

__attribute__((deprecated))  // utf8.h: use utf8::is_is_lowercase()
inline bool UTF8CharIsLowercase ( UTF8Char p )		// <==>  "will change if converted to uppercase"
{
	return *p>=0 ? is_lowercase(*p) : UTF8CharGeneralCategory(p) == U_gc_ll;
}



/* ****************************************************************
					NUMERIC VALUE
**************************************************************** */

// quick variants
__attribute__((deprecated))  // utf8.h: use utf8::is_dec_digit()
inline bool UTF8CharIsDec	( UTF8Char p )	{ return is_dec_digit(*p);  }	// '0'..'9'

__attribute__((deprecated))  // utf8.h: use utf8::is_oct_digit()
inline bool UTF8CharIsOct	( UTF8Char p )	{ return is_oct_digit(*p);  }	// '0'..'7'

__attribute__((deprecated))  // utf8.h: use utf8::is_bin_digit()
inline bool UTF8CharIsBin	( UTF8Char p )	{ return is_bin_digit(*p);  }	// '0'..'1'

__attribute__((deprecated))  // utf8.h: use utf8::is_hex_digit()
inline bool UTF8CharIsHex	( UTF8Char p )	{ return is_hex_digit(*p);  }	// 0-9,a-f,A-F

__attribute__((deprecated))  // utf8.h: use utf8::digit_val()
inline uint UTF8CharDecVal	( UTF8Char p )	{ return digit_val(*p);     }	// 0..9; NaN>9

__attribute__((deprecated))  // utf8.h: use utf8::digit_value()
inline uint UTF8CharHexVal	( UTF8Char p )	{ return digit_value(*p);   }	// 0..36; NaN>36

__attribute__((deprecated))  // utf8.h: use utf8::is_dec_digit()
inline bool UTF8CharIsDecimalDigit	( UTF8Char p )
{
//	return *p>=0 ? is_dec_digit(*p) : UTF8CharGeneralCategory(p) == U_gc_decimal_number;
	return *p>=0 ? is_dec_digit(*p) : UTF8CharGeneralCategory(p) == U_gc_digit;
}

__attribute__((deprecated))  // utf8.h: use utf8::is_number_letter()
inline bool UTF8CharIsNumberLetter	( UTF8Char p )
{
	return *p>=0 ? is_dec_digit(*p) : UTF8CharGeneralCategory(p) == U_gc_letter_number;
}

__attribute__((deprecated))  // utf8.h: use utf8::has_numeric_value()
inline bool UTF8CharHasNumericValue	( UTF8Char p )  // digits, numbers & decorated numbers
{
	return *p>=0 ? is_dec_digit(*p) : uchar_in_range(U_gc_number, UTF8CharGeneralCategory(p), U_gc_other_number);
}

// Get Decimal Digit Value.
// No error checking. Non-Decimal-Digits return meaningless values.
__attribute__((deprecated))  // utf8.h: use utf8::digit_value()
inline uint UTF8CharDigitValue ( UTF8Char p )
{
	return *p>=0 ? digit_val(*p) : ucs4_get_digitvalue(UCS4CharFromUTF8(p));
}

// Get Digit, Number & Decorated Number value.
// some fractionals. one negative. two NaNs.
__attribute__((deprecated))  // utf8.h: use utf8::numeric_value()
inline float UTF8CharNumericValue ( UTF8Char p )
{
	return uchar_in_range('0',*p,'9')  ? digit_val(*p) : ucs4_get_numericvalue(UCS4CharFromUTF8(p));
}



/* ****************************************************************
					UNICODE INFO METHODS
**************************************************************** */

__attribute__((deprecated))  // #include utf8.h and use utf8::is_printable()
inline bool UTF8CharIsPrintable ( UTF8Char p )
{
	return *p>=0 ? *p>=0x20 && *p!=0x7f : ucs4_is_printable(UCS4CharFromUTF8(p));
}

__attribute__((deprecated))  // #include utf8.h and use utf8::is_control()
inline bool UTF8CharIsControl ( UTF8Char p )
{
	return *p>=0 ? *p<0x20  || *p==0x7f : UTF8CharGeneralCategory(p) == U_gc_control;
}

__attribute__((deprecated))  // #include utf8.h and use utf8::is_letter()
inline bool UTF8CharIsLetter ( UTF8Char p )
{
	return *p>=0 ? is_letter(*p) : uchar_in_range(U_gc_letter, UTF8CharGeneralCategory(p), U_gc_lu);
}
















