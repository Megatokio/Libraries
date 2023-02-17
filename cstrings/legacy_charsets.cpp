// Copyright (c) 2002 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

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


					namespace ucs2
{
	char to_8bit(ucs2char c, ucs2table t) noexcept
	{
		// convert UCS2 char -> 8-bit char

		// quick test for mapping to same code:
		if (c <= 0xFFu && c == t[c]) return char(c);

		// search legacy table for mapping to c:
		for (uint i = 0; i < 256; i++)
		{
			if (t[i] == ucs2char(c)) return char(i);
		}

		return _replacementchar();
	}

} // namespace


namespace ucs4
{

char to_8bit(ucs4char c, ucs2table t) noexcept
{
	// convert UCS4 char -> 8-bit char

	// quick test for mapping to same code:
	if (c <= 0xFFu && c == t[c]) return char(c);

	// search legacy table for mapping to c:
	if (c <= 0xFFFFu)
	{
		for (uint i = 0; i < 256; i++)
		{
			if (t[i] == ucs2char(c)) return char(i);
		}
	}

	return _replacementchar();
}

} // namespace ucs4
