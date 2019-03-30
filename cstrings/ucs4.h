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
#include "cstrings.h"
#include "ucs1.h"


typedef uint8  ucs1char;
typedef uint16 ucs2char;
typedef uint32 ucs4char;

#define uint32_in_range(A,N,E)	(uint32((N)-(A))<=uint32((E)-(A)))
#define uint16_in_range(A,N,E)	(uint16((N)-(A))<=uint16((E)-(A)))
#define uint8_in_range(A,N,E)	(uint8((N)-(A))<=uint8((E)-(A)))

#define is_in_range(A,N,E)		(uint((N)-(A)) <= uint((E)-(A)))
#define not_in_range(A,N,E)		(uint((N)-(A)) > uint((E)-(A)))
#define gc_in_range(A,C,E)		(uint(generalCategory(C)-(A)) <= uint((E)-(A)))


/*
	namespace ucs4 enhances some functions in cstrings.cpp
*/

namespace ucs4
{

// Unicode Character PROPERTY VALUES for Catalog Properties and Enumerated Properties
enum Enum
#include "Libraries/Unicode/Includes/PropertyValue_Enum.h"

// General Category:
extern Enum	generalCategory (ucs1char);
extern Enum	generalCategory (ucs2char);
extern Enum	generalCategory (ucs4char);

// Blocks (Ranges) of Characters:
inline Enum	blockProperty	(ucs1char n)	{ return n>>7 ? U_blk_latin_1_supplement : U_blk_basic_latin; }
extern Enum	blockProperty	(ucs2char);
extern Enum	blockProperty	(ucs4char);
inline bool	notInAnyBlock	(ucs4char n)	{ return blockProperty(n) == U_blk_no_block; }

// Script Systems:
extern Enum	scriptProperty	(ucs1char);
extern Enum	scriptProperty	(ucs2char);
extern Enum	scriptProperty	(ucs4char);

// Canonical Combining Class:
inline Enum	cccProperty		(ucs1char)		{ return U_ccc_0; }	// all: U_ccc_0 == U_ccc_Not_Reordered
extern Enum	cccProperty		(ucs2char);
extern Enum	cccProperty		(ucs4char);

// East Asian Width:
extern Enum	eaWidthProperty (ucs1char);
extern Enum	eaWidthProperty (ucs2char);
extern Enum	eaWidthProperty (ucs4char);
extern uint	printWidth		(ucs4char);		// print width of char in monospaced font  ->  0, 1, or 2

// helper:
extern uint	 _dec_digit_value (ucs2char); 	// no error checking
extern uint	 _dec_digit_value (ucs4char); 	// ""
extern float _numeric_value	(ucs2char);		// ""
extern float _numeric_value	(ucs4char);		// ""
extern bool	 _is_printable  (ucs4char);
//
//extern	ucs1char const UCS1_SUC_Table[0x100];	// uc(ß)=ß, uc(µ)=µ, uc(ÿ)=Y
//extern	ucs1char const UCS1_SLC_Table[0x100];
//
extern	ucs2char simple_lowercase (ucs2char);
extern	ucs4char simple_lowercase (ucs4char);
extern	ucs2char simple_uppercase (ucs2char);
extern	ucs4char simple_uppercase (ucs4char);
extern	ucs2char simple_titlecase (ucs2char);
extern	ucs4char simple_titlecase (ucs4char);



/* ****************************************************************
			replicate and extend cstrings functions:
**************************************************************** */

inline	bool is_space	  (ucs4char c)	noexcept { return is_in_range(1,c,' '); }
inline	bool is_letter	  (ucs4char c)	noexcept { return c<0x80 ? is_in_range('a',c|0x20,'z') : gc_in_range(U_gc_letter,c,U_gc_lu); }
inline	bool is_control	  (ucs4char c)	noexcept { return c<0x80 ? c<0x20 || c==0x7f : generalCategory(c) == U_gc_control; }

inline	bool is_printable (ucs4char c)	noexcept { return c<0x700 ? c!=0x7fu && (c&~0x80u)>=0x20 : _is_printable(c); }
inline	bool is_lowercase (ucs4char c)	noexcept { return c<0xd7 ? is_in_range('a',c,'z') : generalCategory(c) == U_gc_ll; }
inline	bool is_uppercase (ucs4char c)	noexcept { return c<0xd7 ? is_in_range('A',c,'Z') : gc_in_range(U_gc_lt,c,U_gc_lu); }

// Simple Lowercase:
inline	ucs1char to_lower (ucs1char n)	noexcept { return		    ucs1char(ucs1::lc_table[n]); }
inline	ucs2char to_lower (ucs2char n)	noexcept { return n<0x100 ? ucs1char(ucs1::lc_table[n]) : simple_lowercase(n); }
inline	ucs4char to_lower (ucs4char n)	noexcept { return n<0x100 ? ucs1char(ucs1::lc_table[n]) : simple_lowercase(n); }

// Simple Uppercase:
inline	ucs1char to_upper (ucs1char n)	noexcept { return		   ucs1char(ucs1::uc_table[n]); /* uc(µ)=µ, uc(ÿ)=Y */ }
inline	ucs2char to_upper (ucs2char n)	noexcept { return n<0xd7 ? ucs1char(ucs1::uc_table[n]) : simple_uppercase(n); }
inline	ucs4char to_upper (ucs4char n)	noexcept { return n<0xd7 ? ucs1char(ucs1::uc_table[n]) : simple_uppercase(n); }

// Simple Titlecase:
inline	ucs1char to_title (ucs1char n)	noexcept { return		   ucs1char(ucs1::uc_table[n]); /* tc(µ)=µ, tc(ÿ)=Y */ }
inline	ucs2char to_title (ucs2char n)	noexcept { return n<0xd7 ? ucs1char(ucs1::uc_table[n]) : simple_titlecase(n); }
inline	ucs4char to_title (ucs4char n)	noexcept { return n<0xd7 ? ucs1char(ucs1::uc_table[n]) : simple_titlecase(n); }

inline	bool is_bin_digit (ucs4char c)	noexcept { return is_in_range('0',c,'1'); }
inline	bool is_oct_digit (ucs4char c)	noexcept { return is_in_range('0',c,'7'); }
inline  bool is_dec_digit (ucs4char c)	noexcept { return c<256 ? is_in_range('0',c,'9') : generalCategory(c) == U_gc_digit; }
inline	bool is_hex_digit (ucs4char c)	noexcept { return is_in_range('0',c,'9') || is_in_range('a',c|0x20,'f'); }

inline	bool no_bin_digit (ucs4char c)	noexcept { return not_in_range('0',c,'1'); }
inline	bool no_oct_digit (ucs4char c)	noexcept { return not_in_range('0',c,'7'); }
inline  bool no_dec_digit (ucs4char c)	noexcept { return c<256 ? not_in_range('0',c,'9') : generalCategory(c) != U_gc_digit; }
inline	bool no_hex_digit (ucs4char c)	noexcept { return not_in_range('0',c,'9') && not_in_range('a',c|0x20,'f'); }

// Get Decimal Digit Value.
// non-decimal-digits return values ≥ 10.
// if is_dec_digit() returned true then digit_val() should return a value in range 0 … 9
inline	uint digit_val	  (ucs1char c)	noexcept { return c-'0'; }
inline	uint digit_val	  (ucs2char c)	noexcept { return c<256 ? c-'0' : _dec_digit_value(c); }
inline	uint digit_val	  (ucs4char c)	noexcept { return c<256 ? c-'0' : _dec_digit_value(c); }

// Get Digit Value in Number Base 2 … 36:
// non-digits return value ≥ 36.
inline	uint digit_value  (ucs4char c)	noexcept { return c <= '9' ? c-'0' : (c|0x20) -'a' +10; }

// decimal digits, roman numbers
inline  bool is_number_letter(ucs4char c) noexcept { return c<256  ? is_in_range('0',c,'9') : gc_in_range(U_gc_digit, c, U_gc_letter_number); }

// digits, indexes, numbers & decorated numbers
inline  bool has_numeric_value (ucs4char c) noexcept { return c<0xb2 ? is_in_range('0',c,'9') : gc_in_range(U_gc_number, c, U_gc_other_number); }

// Get Digit, Number & Decorated Number value.
// everything for which has_numeric_value() returned true.
// some fractionals. one negative. two NaNs.
inline float numeric_value (ucs1char c)	noexcept { return c <= '9' ? c-'0' : _numeric_value(ucs2char(c)); }
inline float numeric_value (ucs2char c)	noexcept { return c <= '9' ? c-'0' : _numeric_value(c); }
inline float numeric_value (ucs4char c)	noexcept { return c <= '9' ? c-'0' : _numeric_value(c); }

// create hex digit, (masked legal):
inline  ucs4char hexchar  (int n)		noexcept { n &= 15; return ucs4char((n>=10 ? 'A'-10 : '0') + n); }
}










