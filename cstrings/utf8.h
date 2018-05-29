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

/*
	namespace utf8 enhances some functions in cstrings.cpp
*/

namespace utf8
{
extern	cstr fromhtmlstr (cstr s)			throws;
extern	cstr detabstr (cstr, uint tabwidth) throws;
extern	str	 whitestr (cstr, char c=' ')	throws;

extern	bool isupperstr	(cstr)				noexcept; // TODO
extern	bool islowerstr	(cstr)				noexcept; // TODO

inline	void toupper	(str)				throws; // TODO	{ if(s) for( ;*s;s++ ) *s = to_upper(*s); }
inline	void tolower	(str)				throws; // TODO	{ if(s) for( ;*s;s++ ) *s = to_lower(*s); }
extern	str	 upperstr	(cstr)				throws; // TODO
extern	str	 lowerstr	(cstr)				throws; // TODO

};


#endif // UTF8_H


