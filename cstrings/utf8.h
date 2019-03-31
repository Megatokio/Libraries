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
using namespace ucs4;
extern const char uc_table[0x80][2];
extern const char lc_table[0x80][2];
extern const char hex_table[16][2];

// helper:
extern ptr _ucs4char_to_utf8 (ucs4char c, ptr z) noexcept; // returns updated ptr z
extern ptr _ucs4char_to_utf8 (ucs4char c) noexcept;


/*
	===== single bytes: =====
*/

// classify byte from utf8 char or string:
inline bool is_fup (char c)			noexcept { return int8(c) < int8(0xc0); }
inline bool no_fup (char c)			noexcept { return int8(c) >= int8(0xc0); }
inline bool is_7bit (char c)		noexcept { return int8(c) >= 0; }			// %0xxxxxxx = ascii


/*
	===== single utf-8 characters: =====
*/

// nominal size of utf-8 character (bytes) based on char[0] of UTF-8 character
// fups are size = 1
inline uint nominal_size (char c)	noexcept { uint n=1; if (c<0) { c &= ~0x02; while(int8(c<<n)<0) n++; } return n; }
inline uint nominal_size (cptr p)	noexcept { return nominal_size(*p); }

// nominal size of utf-8 character (bytes) based on UCS-1, UCS-2, or UCS-4 character
// supports 32 bits
inline uint nominal_size (ucs1char n) noexcept { return 1 + (n>0x7f); }
inline uint nominal_size (ucs2char n) noexcept { return 1 + (n>0x7f) + (n>0x7ff); }
inline uint nominal_size (ucs4char n) noexcept { if(n<0x80) return 1; uint i=16; n>>=11; while(n){n>>=5;i+=5;} return i/6; }

// convert utf8char <-> ucs4char
extern ucs4char utf8_to_ucs4char (cptr s) noexcept;
inline ptr ucs4char_to_utf8 (ucs4char c) noexcept
{
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem
	// c=0 is encoded as 2 bytes

	if (c && c<0x80) return ::charstr(char(c));
	else return _ucs4char_to_utf8(c);
}

inline Enum block_property	(cptr p) noexcept { return ucs4::block_property(utf8_to_ucs4char(p)); }
inline Enum script_property	(cptr p) noexcept { return ucs4::script_property(utf8_to_ucs4char(p)); }
inline Enum ccc_property	(cptr p) noexcept { return ucs4::ccc_property(utf8_to_ucs4char(p)); }
inline Enum general_category(cptr p) noexcept { return ucs4::general_category(utf8_to_ucs4char(p)); }
//inline cstr character_name(cptr p) noexcept { return *p>=0 ? ucs1::character_name(*p) : ucs4::character_name(utf8_to_ucs4char(p)); }
inline Enum ea_width_property(cptr p) noexcept { return ucs4::ea_width_property(utf8_to_ucs4char(p)); }
inline uint print_width		(cptr p) noexcept { return ucs4::print_width(utf8_to_ucs4char(p)); } // 0, 1, or 2


/* ****************************************************************
			replicate and extend cstrings functions:
**************************************************************** */

inline bool is_space	(cptr p) noexcept { return ::is_space(*p); }
inline bool is_letter	(cptr p) noexcept { return *p>=0 ? ::is_letter(*p) : gc_in_range(ucs4::U_gc_letter, p, ucs4::U_gc_lu); }
inline bool is_control	(cptr p) noexcept { return *p>=0 ? ::is_control(*p) : general_category(p) == ucs4::U_gc_control; }

inline bool is_printable (cptr p) noexcept { return *p>=0 ? ::is_printable(*p) : ucs4::is_printable(utf8_to_ucs4char(p)); }
inline bool is_uppercase (cptr p) noexcept { return *p>=0 ? ::is_uppercase(*p) : gc_in_range(ucs4::U_gc_lt, p, ucs4::U_gc_lu); }
inline bool is_lowercase (cptr p) noexcept { return *p>=0 ? ::is_lowercase(*p) : general_category(p) == ucs4::U_gc_ll; }

inline bool is_bin_digit (cptr p) noexcept { return ::is_bin_digit(*p);  }	// '0'..'1'
inline bool is_oct_digit (cptr p) noexcept { return ::is_oct_digit(*p);  }	// '0'..'7'
inline bool is_dec_digit (cptr p) noexcept { return *p>=0 ? ::is_dec_digit(*p) : general_category(p) == ucs4::U_gc_digit; }
inline bool is_hex_digit (cptr p) noexcept { return ::is_hex_digit(*p);  }	// 0-9,a-f,A-F

inline cptr hexchar (int n) noexcept
{
	// create hex digit, (masked legal):
	// returned utf8char is 0-delimited

	return hex_table[n & 15];
}

inline cptr to_lower (cptr p) noexcept
{
	// *Simple* lowercase:
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (*p>=0) { return lc_table[int(*p)]; }
	else return ucs4char_to_utf8(ucs4::simple_lowercase(utf8_to_ucs4char(p)));
}

inline cptr to_upper (cptr p) noexcept
{
	// Simple uppercase:
	// uc(µ)=µ, uc(ÿ)=Y
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (*p>=0) return uc_table[int(*p)];
	else return ucs4char_to_utf8(ucs4::simple_uppercase(utf8_to_ucs4char(p)));
}

inline cptr to_title (cptr p) noexcept
{
	// *Simple* titlecase:
	// tc(µ)=µ, tc(ÿ)=Y
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (*p>=0) return uc_table[int(*p)];
	else return ucs4char_to_utf8(ucs4::simple_titlecase(utf8_to_ucs4char(p)));
}

inline uint digit_val (cptr p) noexcept
{
	// Get Decimal Digit Value.
	// non-decimal-digits return values ≥ 10.
	// if is_dec_digit() returned true then digit_val() should return a value in range 0 … 9

	return *p>=0 ? ::digit_val(*p) : ucs4::_dec_digit_value(utf8_to_ucs4char((p)));
}

inline uint digit_value  (cptr p) noexcept
{
	// Get Digit Value in Number Base 2 … 36:
	// non-digits return value ≥ 36.

	return ::digit_value(*p);
}

inline bool is_number_letter (cptr p) noexcept
{
	// decimal digits, roman numbers

	return *p>=0 ? ::is_dec_digit(*p) : gc_in_range(ucs4::U_gc_digit, p, ucs4::U_gc_letter_number);
}

inline bool has_numeric_value (cptr p) noexcept
{
	// digits, indexes, numbers & decorated numbers

	return *p>=0 ? ::is_dec_digit(*p) : gc_in_range(ucs4::U_gc_number, p, ucs4::U_gc_other_number);
}

inline float numeric_value (cptr p) noexcept
{
	// Get Digit, Number & Decorated Number value.
	// everything for which has_numeric_value() returned true.
	// some fractionals. one negative. two NaNs.

	return *p>=0 ? ::digit_val(*p) : ucs4::_numeric_value(utf8_to_ucs4char(p));
}


/*
	===== utf-8 strings: =====
*/

// find start of next character in utf-8 string
// skip current assumed non-fup byte and find next
inline cptr nextchar  (cptr p)		noexcept { while (is_fup(*++p)) {} return p; }
inline ptr  nextchar  (ptr p)		noexcept { while (is_fup(*++p)) {} return p; }
inline cptr nextchar  (cptr p, cptr e) noexcept { while (++p<e && is_fup(*p)) {} return p; }

// find start of previous character in utf-8 string
// rskip current assumed non-fup byte and find previous
inline cptr prevchar (cptr p)		noexcept { while (is_fup(*--p)) {} return p; }
inline ptr  prevchar (ptr p)		noexcept { while (is_fup(*--p)) {} return p; }
inline cptr prevchar (cptr a, cptr p) noexcept { while (--p >= a && is_fup(*p)) {} return p; }


extern uint charcount (cstr)		noexcept; // count characters in utf-8 string
extern uint max_csz	  (cstr)		noexcept; // required ucs* character size to store utf-8 string
extern bool fits_in_ucs1 (cstr)		noexcept;
extern bool fits_in_ucs2 (cstr)		noexcept;

inline uint charcount (const ucs1char* q) noexcept { auto a = q; if(q) { while(*q) q++; } return uint(q-a); }
inline uint charcount (const ucs2char* q) noexcept { auto a = q; if(q) { while(*q) q++; } return uint(q-a); }
inline uint charcount (const ucs4char* q) noexcept { auto a = q; if(q) { while(*q) q++; } return uint(q-a); }

extern uint utf8strlen (const ucs1char*, uint) noexcept; // calc. required size for utf-8 string to store ucs text
extern uint utf8strlen (const ucs2char*, uint) noexcept; // ""
extern uint utf8strlen (const ucs4char*, uint) noexcept; // ""

// *** conversion utf8 <-> ucs1 / ucs2 / ucs4 ***

// write into existing buffer:
// 0-terminated. returns ptr _behind_ z[]
extern ptr ucs1_to_utf8 (const ucs1char* q, uint qcnt, ptr z) noexcept;
extern ptr ucs2_to_utf8 (const ucs2char* q, uint qcnt, ptr z) noexcept;
extern ptr ucs4_to_utf8 (const ucs4char* q, uint qcnt, ptr z) noexcept;

// write into existing buffer:
// _not_ 0-terminated. returns ptr _behind_ z[]
extern ucs1char* utf8_to_ucs1 (cstr q, ucs1char* z)	noexcept; // set errno
extern ucs2char* utf8_to_ucs2 (cstr q, ucs2char* z)	noexcept; // set errno
extern ucs4char* utf8_to_ucs4 (cstr q, ucs4char* z)	noexcept; // set errno

// allocate in tempmem:
// 0-terminated. returns ptr to str
extern str to_utf8str (const ucs1char* q, uint qcnt);	// q: may contain 0
extern str to_utf8str (const ucs2char* q, uint qcnt);	// ""
extern str to_utf8str (const ucs4char* q, uint qcnt);	// ""
extern str to_utf8str (const ucs1char* q);				// q: 0-delimited
extern str to_utf8str (const ucs2char* q);				// ""
extern str to_utf8str (const ucs4char* q);				// ""
extern ucs1char* to_ucs1str (cstr q);					// set errno
extern ucs2char* to_ucs2str (cstr q);					// set errno
extern ucs4char* to_ucs4str (cstr q);					// set errno

extern cstr fromhtmlstr (cstr s)			throws;
extern cstr detabstr	(cstr, uint tabs)	noexcept;
extern str	whitestr	(cstr, char c=' ')	noexcept;
extern str	unescapedstr(cstr)				noexcept; // sets errno

extern bool isupperstr (cstr) noexcept; // TODO
extern bool islowerstr (cstr) noexcept; // TODO

extern void toupper	(str)	noexcept; // TODO
extern void tolower	(str)	noexcept; // TODO
extern str	upperstr (cstr)	noexcept; // TODO
extern str	lowerstr (cstr)	noexcept; // TODO






} // namespace







