#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "cstrings.h"
#include "ucs4.h"


/*
	namespace utf8 enhances some functions in cstrings.cpp
*/

namespace utf8
{
static const char replacementchar = '?'; // offiziell $FFFD -> ucs2, utf8 = 3 bytes

using namespace unicode;
using namespace ucs4;

extern const char uc_table[0x80][2];
extern const char lc_table[0x80][2];
extern const char hex_table[16][2];

// helper:
extern ptr _ucs4char_to_utf8(ucs4char c, ptr z) noexcept; // returns updated ptr z
extern ptr _ucs4char_to_utf8(ucs4char c) noexcept;


/*
	===== single bytes: =====
*/

// classify byte from utf8 char or string:
inline bool is_fup(char c) noexcept { return schar(c) < schar(0xc0); }
inline bool no_fup(char c) noexcept { return schar(c) >= schar(0xc0); }
inline bool is_7bit(char c) noexcept { return schar(c) >= 0; } // %0xxxxxxx = ascii


/*
	===== single utf-8 characters: =====
*/

// nominal size of utf-8 character (bytes) based on char[0] of UTF-8 character
// fups are size = 1
inline uint nominal_size(char c) noexcept
{
	uint n = 1;
	if (int8(c) < 0)
	{
		c &= ~0x02;
		while (int8(c << n) < 0) n++;
	}
	return n;
}
inline uint nominal_size(cptr p) noexcept { return nominal_size(*p); }

// nominal size of utf-8 character (bytes) based on UCS-1, UCS-2, or UCS-4 character
// supports 32 bits
inline uint nominal_size(ucs1char n) noexcept { return 1 + (n > 0x7f); }
inline uint nominal_size(ucs2char n) noexcept { return 1 + (n > 0x7f) + (n > 0x7ff); }
inline uint nominal_size(ucs4char n) noexcept
{
	if (n < 0x80) return 1;
	uint i = 16;
	n >>= 11;
	while (n)
	{
		n >>= 5;
		i += 5;
	}
	return i / 6;
}

// convert utf8char <-> ucs4char
extern ucs4char utf8_to_ucs4char(cptr s) noexcept;
inline ptr		ucs4char_to_utf8(ucs4char c) noexcept
{
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem
	// c=0 is encoded as 2 bytes

	if (c && c < 0x80) return ::charstr(char(c));
	else return _ucs4char_to_utf8(c);
}

inline Block				   block_property(cptr p) noexcept { return ucs4::block_property(utf8_to_ucs4char(p)); }
inline Script				   script_property(cptr p) noexcept { return ucs4::script_property(utf8_to_ucs4char(p)); }
inline CanonicalCombiningClass ccc_property(cptr p) noexcept { return ucs4::ccc_property(utf8_to_ucs4char(p)); }
inline GeneralCategory		   general_category(cptr p) noexcept { return ucs4::general_category(utf8_to_ucs4char(p)); }
//inline cstr character_name(cptr p) noexcept { return *p>=0 ? ucs1::character_name(*p) : ucs4::character_name(utf8_to_ucs4char(p)); }
inline EastAsianWidth ea_width_property(cptr p) noexcept { return ucs4::ea_width_property(utf8_to_ucs4char(p)); }
inline uint			  print_width(cptr p) noexcept { return ucs4::print_width(utf8_to_ucs4char(p)); } // 0, 1, or 2

inline bool gc_in_range(GeneralCategory a, cptr p, GeneralCategory e)
{
	return uint16(general_category(p) - a) <= uint16(e - a);
}


/* ****************************************************************
			replicate and extend cstrings functions:
**************************************************************** */

inline bool is_space(cptr p) noexcept { return ::is_space(*p); }
inline bool is_letter(cptr p) noexcept
{
	return is_ascii(*p) ? ::is_letter(*p) : gc_in_range(GcLetter, p, GcUppercaseLetter);
}
inline bool is_control(cptr p) noexcept { return is_ascii(*p) ? ::is_control(*p) : general_category(p) == GcControl; }

inline bool is_printable(cptr p) noexcept
{
	return is_ascii(*p) ? ::is_printable(*p) : ucs4::is_printable(utf8_to_ucs4char(p));
}
inline bool is_uppercase(cptr p) noexcept
{
	return is_ascii(*p) ? ::is_uppercase(*p) : gc_in_range(GcTitlecaseLetter, p, GcUppercaseLetter);
}
inline bool is_lowercase(cptr p) noexcept
{
	return is_ascii(*p) ? ::is_lowercase(*p) : general_category(p) == GcLowercaseLetter;
}

inline bool is_bin_digit(cptr p) noexcept { return ::is_bin_digit(*p); } // '0'..'1'
inline bool is_oct_digit(cptr p) noexcept { return ::is_oct_digit(*p); } // '0'..'7'
inline bool is_dec_digit(cptr p) noexcept
{
	return is_ascii(*p) ? ::is_dec_digit(*p) : general_category(p) == GcDecimalNumber;
}
inline bool is_hex_digit(cptr p) noexcept { return ::is_hex_digit(*p); } // 0-9,a-f,A-F

inline cptr hexchar(int n) noexcept
{
	// create hex digit, (masked legal):
	// returned utf8char is 0-delimited

	return hex_table[n & 15];
}

inline cptr to_lower(cptr p) noexcept
{
	// *Simple* lowercase:
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (is_ascii(*p)) { return lc_table[uint(*p)]; }
	else return ucs4char_to_utf8(_to_lower(utf8_to_ucs4char(p)));
}

inline cptr to_upper(cptr p) noexcept
{
	// Simple uppercase:
	// uc(µ)=µ, uc(ÿ)=Y
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (is_ascii(*p)) return uc_table[uint(*p)];
	else return ucs4char_to_utf8(_to_upper(utf8_to_ucs4char(p)));
}

inline cptr to_title(cptr p) noexcept
{
	// *Simple* titlecase:
	// tc(µ)=µ, tc(ÿ)=Y
	// returned utf8char is 0-delimited
	// returned utf8char is const or allocated in tempmem

	if (is_ascii(*p)) return uc_table[uint(*p)];
	else return ucs4char_to_utf8(_to_title(utf8_to_ucs4char(p)));
}

inline uint digit_val(cptr p) noexcept __attribute__((deprecated)); // --> dec_digit_value()
inline uint dec_digit_value(cptr p) noexcept
{
	// Get Decimal Digit Value.
	// non-decimal-digits return values ≥ 10.
	// if is_dec_digit() returned true then digit_val() should return a value in range 0 … 9

	return is_ascii(*p) ? ::dec_digit_value(*p) : ucs4::_dec_digit_value(utf8_to_ucs4char((p)));
}

inline uint digit_value(cptr p) noexcept __attribute__((deprecated)); // --> hex_digit_value()
inline uint hex_digit_value(cptr p) noexcept
{
	// Get Digit Value in Number Base 2 … 36:
	// non-digits return value ≥ 36.

	return ::hex_digit_value(*p);
}

inline bool is_number_letter(cptr p) noexcept
{
	// decimal digits, roman numbers

	return is_ascii(*p) ? ::is_dec_digit(*p) : gc_in_range(GcDecimalNumber, p, GcLetterNumber);
}

inline bool has_numeric_value(cptr p) noexcept
{
	// digits, indexes, numbers & decorated numbers

	return is_ascii(*p) ? ::is_dec_digit(*p) : gc_in_range(GcNumber, p, GcOtherNumber);
}

inline float numeric_value(cptr p) noexcept
{
	// Get Digit, Number & Decorated Number value.
	// everything for which has_numeric_value() returned true.
	// some fractionals. one negative. two NaNs.

	return is_ascii(*p) ? float(::dec_digit_value(*p)) : ucs4::numeric_value(utf8_to_ucs4char(p));
}


/*
	===== utf-8 strings: =====
*/

// find start of next character in utf-8 string
// skip current assumed non-fup byte and find next
inline cptr nextchar(cptr p) noexcept
{
	while (is_fup(*++p)) {}
	return p;
}
inline ptr nextchar(ptr p) noexcept
{
	while (is_fup(*++p)) {}
	return p;
}
inline cptr nextchar(cptr p, cptr e) noexcept
{
	while (++p < e && is_fup(*p)) {}
	return p;
}

// find start of previous character in utf-8 string
// rskip current assumed non-fup byte and find previous
inline cptr prevchar(cptr p) noexcept
{
	while (is_fup(*--p)) {}
	return p;
}
inline ptr prevchar(ptr p) noexcept
{
	while (is_fup(*--p)) {}
	return p;
}
inline cptr prevchar(cptr a, cptr p) noexcept
{
	while (--p >= a && is_fup(*p)) {}
	return p;
}


extern uint charcount(cstr) noexcept;				// count characters in utf-8 string; 0-terminated
extern uint charcount(cptr q, uint qsize) noexcept; // count characters in utf-8 string
extern uint max_css(cstr) noexcept;					// character size shift required for utf-8 string
inline uint max_csz(cstr s) noexcept { return 1u << max_css(s); }
extern bool fits_in_ucs1(cstr) noexcept;
extern bool fits_in_ucs2(cstr) noexcept;

inline uint charcount(const ucs1char* q) noexcept
{
	auto a = q;
	if (q)
	{
		while (*q) q++;
	}
	return uint(q - a);
}
inline uint charcount(const ucs2char* q) noexcept
{
	auto a = q;
	if (q)
	{
		while (*q) q++;
	}
	return uint(q - a);
}
inline uint charcount(const ucs4char* q) noexcept
{
	auto a = q;
	if (q)
	{
		while (*q) q++;
	}
	return uint(q - a);
}

extern uint utf8strlen(const ucs1char*, uint) noexcept; // calc. required size for utf-8 string to store ucs text
extern uint utf8strlen(const ucs2char*, uint) noexcept; // ""
extern uint utf8strlen(const ucs4char*, uint) noexcept; // ""

// *** conversion utf8 <-> ucs1 / ucs2 / ucs4 ***

// write into existing buffer:
// 0-terminated. returns ptr _behind_ z[]
extern ptr ucs1_to_utf8(const ucs1char* q, uint qcnt, ptr z) noexcept;
extern ptr ucs2_to_utf8(const ucs2char* q, uint qcnt, ptr z) noexcept;
extern ptr ucs4_to_utf8(const ucs4char* q, uint qcnt, ptr z) noexcept;

// write into existing buffer:
// destination string _not_ 0-terminated. returns ptr _behind_ z[]
extern ucs1char* utf8_to_ucs1(cstr q, uint qsize, ucs1char* z) noexcept; // set errno	source can contain char(0)
extern ucs2char* utf8_to_ucs2(cstr q, uint qsize, ucs2char* z) noexcept; // set errno
extern ucs4char* utf8_to_ucs4(cstr q, uint qsize, ucs4char* z) noexcept; // set errno
inline ucs1char* utf8_to_ucs1(cstr q, ucs1char* z) noexcept
{
	return utf8_to_ucs1(q, strLen(q), z);
} // set errno	source 0-terminated
inline ucs2char* utf8_to_ucs2(cstr q, ucs2char* z) noexcept { return utf8_to_ucs2(q, strLen(q), z); } // set errno
inline ucs4char* utf8_to_ucs4(cstr q, ucs4char* z) noexcept { return utf8_to_ucs4(q, strLen(q), z); } // set errno

// allocate in tempmem:
// 0-terminated. returns ptr to str
extern str		 to_utf8str(const ucs1char* q, uint qcnt); // q: may contain 0
extern str		 to_utf8str(const ucs2char* q, uint qcnt); // ""
extern str		 to_utf8str(const ucs4char* q, uint qcnt); // ""
extern str		 to_utf8str(const ucs1char* q);			   // q: 0-delimited
extern str		 to_utf8str(const ucs2char* q);			   // ""
extern str		 to_utf8str(const ucs4char* q);			   // ""
extern ucs1char* to_ucs1str(cstr q);					   // set errno
extern ucs2char* to_ucs2str(cstr q);					   // set errno
extern ucs4char* to_ucs4str(cstr q);					   // set errno

extern cstr fromhtmlstr(cstr s) throws;
extern cstr detabstr(cstr, uint tabs) noexcept;
extern str	whitestr(cstr, char c = ' ') noexcept;
extern str	unescapedstr(cstr) noexcept; // sets errno

extern bool isupperstr(cstr) noexcept; // TODO
extern bool islowerstr(cstr) noexcept; // TODO

extern void toupper(str) noexcept;	 // TODO
extern void tolower(str) noexcept;	 // TODO
extern str	upperstr(cstr) noexcept; // TODO
extern str	lowerstr(cstr) noexcept; // TODO


} // namespace utf8
