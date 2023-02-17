#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "cstrings.h"
#include "unicode.h"

/*
	UCS-4 characters
*/

namespace ucs4
{
// replacement character for illegal, truncated, unprintable etc. characters
static const ucs4char replacementchar = 0xfffd; // offiziell $FFFD -> ucs2, utf8 = 3 bytes

using namespace unicode;

// helper:
extern uint		_dec_digit_value(ucs4char) noexcept;
extern bool		_is_printable(ucs4char) noexcept;
extern ucs4char _to_lower(ucs4char) noexcept;
extern ucs4char _to_upper(ucs4char) noexcept;
extern ucs4char _to_title(ucs4char) noexcept;

// get character properies:
extern GeneralCategory		   general_category(ucs4char) noexcept;
extern Block				   block_property(ucs4char) noexcept;
inline bool					   not_in_any_block(ucs4char n) noexcept { return block_property(n) == BlkNoBlock; }
extern Script				   script_property(ucs4char) noexcept;
extern CanonicalCombiningClass ccc_property(ucs4char) noexcept;
extern EastAsianWidth		   ea_width_property(ucs4char) noexcept;
extern uint					   print_width(ucs4char) noexcept;

// helper:
inline bool is_in_range(uint a, uint c, uint e) { return uint(c - a) <= uint(e - a); }
inline bool gc_in_range(GeneralCategory a, ucs4char c, GeneralCategory e)
{
	return uint16(general_category(c) - a) <= uint16(e - a);
}


/* ****************************************************************
			replicate and extend cstrings functions:
**************************************************************** */

inline bool is_control(ucs4char c) noexcept
{
	return c < 0x80 ? c < 0x20 || c == 0x7f : general_category(c) == GcControl;
}
inline bool is_space(ucs4char c) noexcept { return (--c & 0x7f) < ' '; } // space, nbsp or ctrl excl. 0 and 0x7F
inline bool is_letter(ucs4char c) noexcept
{
	return c < 0x80 ? is_in_range('a', c | 0x20, 'z') : gc_in_range(GcLetter, c, GcUppercaseLetter);
}
inline bool is_ascii(ucs4char c) noexcept { return c <= 0x7F; }

inline bool is_printable(ucs4char c) noexcept
{
	return c < 0x700 ? c != 0x7fu && (c & ~0x80u) >= 0x20 : _is_printable(c);
}
inline bool is_lowercase(ucs4char c) noexcept
{
	return c < 0xd7 ? is_in_range('a', c, 'z') : general_category(c) == GcLowercaseLetter;
}
inline bool is_uppercase(ucs4char c) noexcept
{
	return c < 0xd7 ? is_in_range('A', c, 'Z') : gc_in_range(GcTitlecaseLetter, c, GcUppercaseLetter);
}

inline bool is_bin_digit(ucs4char c) noexcept { return is_in_range('0', c, '1'); }
inline bool is_oct_digit(ucs4char c) noexcept { return is_in_range('0', c, '7'); }
inline bool is_dec_digit(ucs4char c) noexcept
{
	return c < 256 ? is_in_range('0', c, '9') : general_category(c) == GcDecimalNumber;
}
inline bool is_hex_digit(ucs4char c) noexcept { return is_in_range('0', c, '9') || is_in_range('a', c | 0x20, 'f'); }

// decimal digits, indexes, numbers & decorated numbers
inline bool has_numeric_value(ucs4char c) noexcept
{
	return c < 0xb2 ? is_in_range('0', c, '9') : gc_in_range(GcNumber, c, GcOtherNumber);
}

// decimal digits, roman numbers
inline bool is_number_letter(ucs4char c) noexcept
{
	return c < 256 ? is_in_range('0', c, '9') : gc_in_range(GcDecimalNumber, c, GcLetterNumber);
}

// Simple Upper/Lower/Titlecase:
inline ucs4char to_lower(ucs4char c) noexcept { return c < 0x80 ? ucs1char(::to_lower(char(c))) : _to_lower(c); }
inline ucs4char to_upper(ucs4char c) noexcept { return c < 0x80 ? ucs1char(::to_upper(char(c))) : _to_upper(c); }
inline ucs4char to_title(ucs4char c) noexcept { return c < 0x80 ? ucs1char(::to_upper(char(c))) : _to_title(c); }

// Get Decimal Digit Value.
// non-decimal-digits return values ≥ 10.
// if is_dec_digit() returned true then decimal_digit_value() should return a value in range 0 … 9
inline uint dec_digit_value(ucs4char c) noexcept { return c < 256 ? c - '0' : _dec_digit_value(c); }

// Get Digit Value in Number Base 2 … 36:
// non-digits return value ≥ 36.
inline uint hex_digit_value(ucs4char c) noexcept { return c < 256 ? ::hex_digit_value(char(c)) : 66; }

// Get Digit, Number & Decorated Number value.
// everything for which has_numeric_value() returned true. else return NaN
extern float numeric_value(ucs4char c) noexcept;

// create hex digit, (masked legal):
inline ucs4char hexchar(int n) noexcept
{
	n &= 15;
	return ucs4char((n >= 10 ? 'A' - 10 : '0') + n);
}


inline uint digit_val(ucs4char c) __attribute((deprecated));			// --> dec_digit_value()
inline uint digit_value(ucs4char c) noexcept __attribute((deprecated)); // --> hex_digit_value()

} // namespace ucs4
