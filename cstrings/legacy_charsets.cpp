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














