#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "cstrings.h"
#include "unicode.h"

/*
	UCS-1 characters
*/

namespace ucs1
{
// replacement character for illegal, truncated, unprintable etc. characters
static const ucs1char replacementchar = '?'; // offiziell $FFFD -> ucs2, utf8 = 3 bytes

using namespace unicode;

// helper:
extern const uint8 gc_table[256];
inline bool		   is_letter(ucs1char c) noexcept;

// get character properies:
inline GeneralCategory general_category(ucs1char c) noexcept { return GeneralCategory(gc_table[c]); }
inline Block		   block_property(ucs1char n) noexcept { return n < 0x80 ? BlkBasicLatin : BlkLatin1Supplement; }
inline Script		   script_property(ucs1char c) noexcept { return is_letter(c) ? ScLatin : ScCommon; }
inline uint			   print_width(ucs1char c) noexcept { return general_category(c) != GcControl; } // 0 or 1
inline CanonicalCombiningClass ccc_property(ucs1char) noexcept { return CccNotReordered; }			 // for completeness
inline EastAsianWidth		   ea_width_property(ucs1char) noexcept { return EaNeutral; }			 // for completeness
inline bool					   not_in_any_block(ucs1char) noexcept { return false; }				 // for completeness

// helper:
inline bool is_in_range(ucs1char a, ucs1char c, ucs1char e) noexcept { return uint8(c - a) <= uint8(e - a); }
inline bool gc_in_range(GeneralCategory a, ucs1char c, GeneralCategory e) noexcept
{
	return uint16(general_category(c) - a) <= uint16(e - a);
}


/* ****************************************************************
			replicate and extend cstrings functions:
**************************************************************** */

// is_uppercase() is true for all letters which have a different lowercase letter inside ucs1.
// is_uppercase() is true for 'A' … 'Z', and 'À' … 'Þ' except '×'.
// is_lowercase() is true for all letters which have a different uppercase letter inside ucs1 and 'ß' and 'ÿ'.
// is_lowercase() is true for 'a' … 'z', and 'ß' … 'ÿ' except '÷'.
// is_letter()    is true for all letters which have a different uppercase and lowercase version and for 'ß'.

inline bool is_control(ucs1char c) noexcept { return general_category(c) == GcControl; } // control code incl. 0x00
inline bool is_space(ucs1char c) noexcept { return (--c & 0x7f) < ' '; } // space, nbsp or ctrl excl. 0 and 0x7F
inline bool is_letter(ucs1char c) noexcept { return gc_in_range(GcLetter, c, GcUppercaseLetter); }

inline bool is_printable(ucs1char c) noexcept { return general_category(c) != GcControl; }
inline bool is_lowercase(ucs1char c) noexcept { return general_category(c) == GcLowercaseLetter; }
inline bool is_uppercase(ucs1char c) noexcept { return general_category(c) == GcUppercaseLetter; }

inline bool is_bin_digit(ucs1char c) noexcept { return (c | 1) == '1'; }
inline bool is_oct_digit(ucs1char c) noexcept { return is_in_range('0', c, '7'); }
inline bool is_dec_digit(ucs1char c) noexcept { return is_in_range('0', c, '9'); }
inline bool is_hex_digit(ucs1char c) noexcept { return is_in_range('0', c, '9') || is_in_range('a', c | 0x20, 'f'); }

// digits, indexes, numbers & decorated numbers: 0 - 9, ^2, ^3, ^1, 1/4, 1/2 and 3/4
inline bool has_numeric_value(ucs1char c) noexcept { return gc_in_range(GcNumber, c, GcOtherNumber); }

// decimal digits, roman numbers
inline bool is_number_letter(ucs1char c) noexcept { return is_dec_digit(c); }

// Simple Upper/Lower/Titlecase:
inline ucs1char to_lower(ucs1char c) noexcept { return is_uppercase(c) ? c ^ 0x20 : c; }
inline ucs1char to_upper(ucs1char c) noexcept { return is_uppercase(c ^ 0x20) ? c ^ 0x20 : c; } // not is_lowercase(c)!!
inline ucs1char to_title(ucs1char c) noexcept { return to_upper(c); }

// Get Decimal Digit Value.
// non-decimal-digits return values ≥ 10.
// if is_dec_digit() returned true then decimal_digit_value() should return a value in range 0 … 9
inline uint dec_digit_value(ucs1char c) noexcept { return ::dec_digit_value(char(c)); }

// Get Digit Value in Number Base 2 … 36:
// non-digits return value ≥ 36.
inline uint hex_digit_value(ucs1char c) noexcept { return ::hex_digit_value(char(c)); }

// Get Digit, Number & Decorated Number value.
// everything for which has_numeric_value() returned true. else return NaN
extern float numeric_value(ucs1char c) noexcept;

// create hex digit, (masked legal):
inline ucs1char hexchar(int n) noexcept { return ucs1char(::hexchar(n)); }


/* deprecated:
*/
inline uint digit_val(ucs2char c) __attribute((deprecated));   // --> dec_digit_value()
inline uint digit_value(ucs2char c) __attribute((deprecated)); // --> hex_digit_value()


/* strings:
*/
inline ucs1str tempstr(uint n) noexcept { return ucs1ptr(::tempstr(n)); }
inline ucs1str tempstr(int n) noexcept { return ucs1str(::tempstr(n)); }
inline ucs1str dupstr(ucs1cstr q) noexcept { return ucs1str(::dupstr(cstr(q))); }

extern bool isupperstr(ucs1cstr) noexcept;
extern bool islowerstr(ucs1cstr) noexcept;

inline void toupper(ucs1str s) noexcept
{
	if (s)
		for (; *s; s++) *s = to_upper(*s);
}
inline void tolower(ucs1str s) noexcept
{
	if (s)
		for (; *s; s++) *s = to_lower(*s);
}
extern ucs1str upperstr(ucs1cstr) throws;
extern ucs1str lowerstr(ucs1cstr) throws;

}; // namespace ucs1
