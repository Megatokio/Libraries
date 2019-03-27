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
#include "ucs1.h"

namespace ucs1
{

/* best effort mapping inside ucs-1 range:
*/
	#define N3(C)	char(C),char(C+1),char(C+2)
	#define N4(C)	char(C),char(C+1),char(C+2),char(C+3)
	#define N7(C)	N4(C),N3(C+4)
	#define N8(C)	N4(C),N4(C+4)
	#define N16(C)	N8(C),N8(C+8)
	#define N32(C)	N16(C),N16(C+16)

char const uc_table[0x100] =
{
	N32(0), N32(0x20), N32(0x40),
	0x60, N7(0x41), N8(0x48),
	N8(0x50), N3(0x58), 0x7b, N4(0x7c),				// uc(ß) = ß
	N32(0x80), N32(0xa0), N32(0xc0),				// uc(µ) = µ	da: Μ = ucs2
	N16(0xc0), N7(0xd0), char(0xf7), N7(0xd8), 'Y' 	// uc(ÿ) = Y	da: Ÿ = ucs2
};

char const lc_table[0x100] =
{
	N32(0), N32(0x20),
	0x40, N7(0x61), N8(0x68),
	N8(0x70), N3(0x78), 0x5b, N4(0x5c),
	N32(0x60), N32(0x80), N32(0xa0),
	N16(0xe0), N7(0xf0), char(0xd7), N7(0xf8), char(0xdf),
	N32(0xe0)
};

str lowerstr (cstr s) throws
{
	// Convert a string to all lower case

    str t = dupstr(s);
    tolower(t);
    return t;
}

str upperstr (cstr s) throws
{
	// Convert a string to all upper case

    str t = dupstr(s);
    toupper(t);
    return t;
}

bool isupperstr	(cstr s) noexcept
{
	// Test if string is all upper case

    if(s) while(*s) { if(to_upper(*s)!=*s) return no; else s++; }
    return yes;
}

bool islowerstr	(cstr s) noexcept
{
	// Test if string is all lower case

    if(s) while(*s) { if(to_lower(*s)!=*s) return no; else s++; }
    return yes;
}



}; // namespace


















