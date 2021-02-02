/*	Copyright  (c)	Günter Woigk 2002 - 2021
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

#include "legacy_charsets.h"


// 8-bit charset conversion tables
ucs2table legacy_charset_mac_roman =
#include "legacy_charsets/mac_roman.h"

ucs2table legacy_charset_ascii_ger =
#include "legacy_charsets/ascii_ger.h"

ucs2table legacy_charset_rtos =
#include "legacy_charsets/rtos.h"

ucs2table legacy_charset_cp_437 =
#include "legacy_charsets/cp_437.h"

ucs2table legacy_charset_atari_st =
#include "legacy_charsets/atari_st.h"



namespace ucs2 {

char to_8bit (ucs2char c, ucs2table t) noexcept
{
	// convert UCS2 char -> 8-bit char

	// quick test for mapping to same code:
	if (c <= 0xFFu && c == t[c]) return char(c);

	// search legacy table for mapping to c:
	for (uint i=0; i<256; i++)
	{
		if (t[i] == ucs2char(c)) return char(i);
	}

	return _replacementchar();
}

} // namespace


namespace ucs4 {

char to_8bit (ucs4char c, ucs2table t) noexcept
{
	// convert UCS4 char -> 8-bit char

	// quick test for mapping to same code:
	if (c <= 0xFFu && c == t[c]) return char(c);

	// search legacy table for mapping to c:
	if (c <= 0xFFFFu)
	{
		for (uint i=0; i<256; i++)
		{
			if (t[i] == ucs2char(c)) return char(i);
		}
	}

	return _replacementchar();
}

} // namespace














