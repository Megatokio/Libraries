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


/*
	namespace ucs1 enhances some functions in cstrings.cpp
	the functions in this namespace work with 'char' not 'ucs1char'
	for easier use in normal string handling.
*/

namespace ucs1
{
extern char const uc_table[0x100];	// simple uppercase table
extern char const lc_table[0x100];	// simple lowercase table

// is_uppercase() is true for all letters which have a different lowercase letter inside ucs1.
// is_uppercase() is true for 'A' … 'Z', and 'À' … 'Þ' except '×'.
//
// is_lowercase() is true for all letters which have a different uppercase letter inside ucs1 and 'ß' and 'ÿ'.
// is_lowercase() is true for 'a' … 'z', and 'ß' … 'ÿ' except '÷'.
//
// is_letter() is true for all letters which have a different uppercase and lowercase version and for 'ß'.

inline	bool is_uppercase (char c)	noexcept { return c != lc_table[uchar(c)]; }
inline	bool is_lowercase (char c)	noexcept { return c != uc_table[uchar(c)] || c==char(0xdf)/*ß*/; }
inline	bool is_letter (char c)		noexcept { return to_upper(c) != to_lower(c) || c==char(0xdf)/*ß*/; }

inline	char to_lower (char c)		noexcept { return lc_table[uchar(c)]; }
inline	char to_upper (char c)		noexcept { return uc_table[uchar(c)]; }

extern	bool isupperstr	(cstr)		noexcept;
extern	bool islowerstr	(cstr)		noexcept;

inline	void toupper (str s)		throws	{ if(s) for( ;*s;s++ ) *s = to_upper(*s); }
inline	void tolower (str s)		throws	{ if(s) for( ;*s;s++ ) *s = to_lower(*s); }
extern	str	 upperstr (cstr)		throws;
extern	str	 lowerstr (cstr)		throws;
}










