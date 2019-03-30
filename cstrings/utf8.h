#pragma once
/*	Copyright  (c)	Günter Woigk 2018 - 2019
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
*/

#include "Libraries/kio/kio.h"
#include "Libraries/cstrings/cstrings.h"
#include "ucs4.h"


typedef uint8  ucs1char;
typedef uint16 ucs2char;
typedef uint32 ucs4char;

/*
	namespace utf8 enhances some functions in cstrings.cpp
*/

namespace utf8
{
static const char replacementchar = '?';	// offiziell $FFFD -> ucs2, utf8 = 3 bytes

inline bool is_fup (char c)			noexcept { return int8(c) < int8(0xc0); }
inline bool	no_fup (char c)			noexcept { return int8(c) >= int8(0xc0); }

inline cptr nextchar  (cptr p)		noexcept { while (is_fup(*++p) ) {} return p; }
inline ptr  nextchar  (ptr p)		noexcept { while (is_fup(*++p) ) {} return p; }

extern uint charcount (cstr)		noexcept; // count characters in utf-8 string
extern uint max_csz	  (cstr)		noexcept; // required ucs* character size to store utf-8 string
extern bool fits_in_ucs1 (cstr)		noexcept;
extern bool fits_in_ucs2 (cstr)		noexcept;

inline uint charcount (ucs1char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }
inline uint charcount (ucs2char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }
inline uint charcount (ucs4char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }

extern uint utf8len	(ucs1char const*, uint) noexcept; // calc. required size for utf-8 string to store ucs text
extern uint utf8len	(ucs2char const*, uint) noexcept; // ""
extern uint utf8len	(ucs4char const*, uint) noexcept; // ""

// *** conversion utf8 <-> ucs1 / ucs2 / ucs4 ***

// write into existing buffer:
// 0-terminated. returns ptr _behind_ z[]
extern ptr ucs1_to_utf8 (ucs1char const* q, uint qcnt, ptr z) noexcept;
extern ptr ucs2_to_utf8 (ucs2char const* q, uint qcnt, ptr z) noexcept;
extern ptr ucs4_to_utf8 (ucs4char const* q, uint qcnt, ptr z) noexcept;

// write into existing buffer:
// _not_ 0-terminated. returns ptr _behind_ z[]
extern ucs1char* utf8_to_ucs1 (cstr q, ucs1char* z)	noexcept; // set errno
extern ucs2char* utf8_to_ucs2 (cstr q, ucs2char* z)	noexcept; // set errno
extern ucs4char* utf8_to_ucs4 (cstr q, ucs4char* z)	noexcept; // set errno

// allocate in tempmem:
// 0-terminated. returns ptr to str
extern str to_utf8 (ucs1char const* q, uint qcnt);	// q: may contain 0
extern str to_utf8 (ucs2char const* q, uint qcnt);	// ""
extern str to_utf8 (ucs4char const* q, uint qcnt);	// ""
extern str to_utf8 (ucs1char const* q);				// q: 0-delimited
extern str to_utf8 (ucs2char const* q);				// ""
extern str to_utf8 (ucs4char const* q);				// ""
extern ucs1char* to_ucs1 (cstr q);					// set errno
extern ucs2char* to_ucs2 (cstr q);					// set errno
extern ucs4char* to_ucs4 (cstr q);					// set errno


extern	cstr fromhtmlstr (cstr s)			throws;
extern	cstr detabstr	(cstr, uint tabs)	noexcept;
extern	str	 whitestr	(cstr, char c=' ')	noexcept;
extern	str	 unescapedstr(cstr)				noexcept; // sets errno

extern	bool isupperstr	(cstr)				noexcept; // TODO
extern	bool islowerstr	(cstr)				noexcept; // TODO

extern	void toupper	(str)				noexcept; // TODO	{ if(s) for( ;*s;s++ ) *s = to_upper(*s); }
extern	void tolower	(str)				noexcept; // TODO	{ if(s) for( ;*s;s++ ) *s = to_lower(*s); }
extern	str	 upperstr	(cstr)				noexcept; // TODO
extern	str	 lowerstr	(cstr)				noexcept; // TODO


extern	ucs4char utf8_to_ucs4char	(cptr s) noexcept;

inline ucs4::Enum blockProperty		(cptr p)	{ return ucs4::blockProperty(utf8_to_ucs4char(p) ); }
inline ucs4::Enum scriptProperty	(cptr p)	{ return ucs4::scriptProperty(utf8_to_ucs4char(p) ); }
inline ucs4::Enum cccProperty		(cptr p)	{ return ucs4::cccProperty(utf8_to_ucs4char(p) ); }
inline ucs4::Enum generalCategory	(cptr p)	{ return ucs4::generalCategory(utf8_to_ucs4char(p) ); }
//inline cstr	  characterName		(cptr p)	{ return *p>=0 ? ucs1::characterName(*p)
//					 											: ucs4::characterName(UCS4CharFromUTF8(p)); }
inline ucs4::Enum eaWidthProperty	(cptr p)	{ return ucs4::eaWidthProperty(utf8_to_ucs4char(p)); }
inline uint		  printWidth		(cptr p)	{ return ucs4::printWidth(utf8_to_ucs4char(p)); } // 0, 1, or 2


inline	bool is_printable (cptr p) { return *p>=0 ? is_in_range(0x20,*p,0x7e) : ucs4::is_printable(utf8_to_ucs4char(p)); }
inline	bool is_control	  (cptr p) { return *p>=0 ? not_in_range(0x20,*p,0x7e) : generalCategory(p) == ucs4::U_gc_control; }
inline	bool is_letter	  (cptr p) { return *p>=0 ? ::is_letter(*p) : is_in_range(ucs4::U_gc_letter, generalCategory(p), ucs4::U_gc_lu); }

inline	bool is_dec_digit (cptr p)	{ return *p>=0 ? ::is_dec_digit(*p) : generalCategory(p) == ucs4::U_gc_digit; }
inline	bool is_oct_digit (cptr p)	{ return ::is_oct_digit(*p);  }	// '0'..'7'
inline	bool is_bin_digit (cptr p)	{ return ::is_bin_digit(*p);  }	// '0'..'1'
inline	bool is_hex_digit (cptr p)	{ return ::is_hex_digit(*p);  }	// 0-9,a-f,A-F
inline	uint digit_val	  (cptr p)	{ return ::digit_val(*p);     }	// 0..9;  NaN>9
inline	uint digit_value  (cptr p)	{ return ::digit_value(*p);   }	// 0..36; NaN>36

// decimal digits, roman numbers
inline bool is_number_letter (cptr p)
{
	return *p>=0 ? is_in_range('0',*p,'9')
			: is_in_range(ucs4::U_gc_digit, generalCategory(p), ucs4::U_gc_letter_number);
}
inline bool has_numeric_value (cptr p)
{
	return *p>=0 ? ::is_dec_digit(*p)
		: is_in_range(ucs4::U_gc_number, generalCategory(p), ucs4::U_gc_other_number);
}

// Get Digit, Number & Decorated Number value.
// some fractionals. one negative. two NaNs.
inline float numeric_value (cptr p)
{
	return is_in_range('0',*p,'9') ? ::digit_val(*p) : ucs4::_numeric_value(utf8_to_ucs4char(p));
}

// Info:
inline bool is_uppercase ( cptr p )		// <==>  "will change if converted to lowercase"
{
	return *p>=0 ? ::is_uppercase(*p) : is_in_range(ucs4::U_gc_lt, generalCategory(p), ucs4::U_gc_lu);
}

inline bool is_lowercase ( cptr p )		// <==>  "will change if converted to uppercase"
{
	return *p>=0 ? ::is_lowercase(*p) : generalCategory(p) == ucs4::U_gc_ll;
}

} // namespace





