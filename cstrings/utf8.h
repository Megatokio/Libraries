/*	Copyright  (c)	Günter Woigk 2018 - 2018
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

#ifndef UTF8_H
#define UTF8_H
#include "Libraries/kio/kio.h"
#include "Libraries/cstrings/cstrings.h"


typedef uint8  ucs1char;
typedef uint16 ucs2char;
typedef uint32 ucs4char;

/*
	namespace utf8 enhances some functions in cstrings.cpp
*/

namespace utf8
{
const char replacementchar = '?';	// offiziell $FFFD -> ucs2, utf8 = 3 bytes

inline bool is_fup (char c)			noexcept { return int8(c) < int8(0xc0); }
inline bool	no_fup (char c)			noexcept { return int8(c) >= int8(0xc0); }

extern uint charcount (cstr)		noexcept; // count characters in utf-8 string
extern uint max_csz	  (cstr)		noexcept; // required ucs* character size to store utf-8 string
extern bool fits_ucs1 (cstr)		noexcept;
extern bool fits_ucs2 (cstr)		noexcept;

inline uint charcount (ucs1char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }
inline uint charcount (ucs2char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }
inline uint charcount (ucs4char const* q)	noexcept { uint n=0; if(q) while(*q++) n++; return n; }

extern uint utf8len	(ucs1char const*, uint) noexcept; // calc. required size for utf-8 string to store ucs text
extern uint utf8len	(ucs2char const*, uint) noexcept; // ""
extern uint utf8len	(ucs4char const*, uint) noexcept; // ""

// *** conversion utf8 <-> ucs1 / ucs2 / ucs4 ***

// write into existing buffer:
// 0-terminated. returns ptr _behind_ z[]
extern ptr ucs1_to_utf8 (ucs1char const* q, uint qcnt, ptr z) noexcept;
extern ptr ucs2_to_utf8 (ucs2char const* q, uint qcnt, ptr z) noexcept;
extern ptr ucs4_to_utf8 (ucs4char const* q, uint qcnt, ptr z) noexcept;

// write into existing buffer:
// _not_ 0-terminated. returns ptr _behind_ z[]
extern ucs1char* utf8_to_ucs1 (cstr q, ucs1char* z)	noexcept; // set errno
extern ucs2char* utf8_to_ucs2 (cstr q, ucs2char* z)	noexcept; // set errno
extern ucs4char* utf8_to_ucs4 (cstr q, ucs4char* z)	noexcept; // set errno

// allocate in tempmem:
// 0-terminated. returns ptr to str
extern str to_utf8 (ucs1char const* q, uint qcnt);	// q: may contain 0
extern str to_utf8 (ucs2char const* q, uint qcnt);	// ""
extern str to_utf8 (ucs4char const* q, uint qcnt);	// ""
extern str to_utf8 (ucs1char const* q);				// q: 0-delimited
extern str to_utf8 (ucs2char const* q);				// ""
extern str to_utf8 (ucs4char const* q);				// ""
extern ucs1char* to_ucs1 (cstr q);					// set errno
extern ucs2char* to_ucs2 (cstr q);					// set errno
extern ucs4char* to_ucs4 (cstr q);					// set errno


extern	cstr fromhtmlstr (cstr s)			throws;
extern	cstr detabstr	(cstr, uint tabwidth) throws;
extern	str	 whitestr	(cstr, char c=' ')	throws;

extern	bool isupperstr	(cstr)				noexcept; // TODO
extern	bool islowerstr	(cstr)				noexcept; // TODO

inline	void toupper	(str)				throws; // TODO	{ if(s) for( ;*s;s++ ) *s = to_upper(*s); }
inline	void tolower	(str)				throws; // TODO	{ if(s) for( ;*s;s++ ) *s = to_lower(*s); }
extern	str	 upperstr	(cstr)				throws; // TODO
extern	str	 lowerstr	(cstr)				throws; // TODO
};


#endif // UTF8_H


