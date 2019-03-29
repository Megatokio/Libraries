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
#include "Libraries/unix/tempmem.h"
#include "utf8.h"


static cptr str_comp(cstr a, cstr b) noexcept
{
	char c,d;
	while ((c=*a) && (d=*b) && c==d) { a++; b++; }
	return a;
}


namespace utf8 {

//	0fup: c < %10000000		c <= %01111111		c = %0xxxxxxx	ucs < 0x80
//	fup:  c < %11000000		c <= %10111111		c = %10xxxxxx
//	1fup: c < %11100000		c <= %11011111		c = %110xxxxx	ucs < 0x800
//	2fup: c < %11110000		c <= %11101111		c = %1110xxxx	ucs < 0x10000
//	3fup: c < %11111000		c <= %11110111		c = %11110xxx
//	4fup: c < %11111100		c <= %11111011		c = %111110xx
//	5fup:										c = %111111xx

uint charcount (cstr q) noexcept
{
	// count characters in utf-8 string
	// the "Golden Rule": every non-fup makes a char

	uint rval = 0;
	if(q) while (char c = *q++)
	{
		if (no_fup(c)) rval++;
	}
	return rval;
}
uint max_csz (cstr q) noexcept
{
	// calculate required character size (ucs1, ucs2 or ucs4) to store utf-8 string

	if(q) while (char c = *q++)
	{
		if (uchar(c) <= 0xC3) continue;
		if (uchar(c) >= 0xF0) return 4;
		while ((c = *q++))
		{
			if (uchar(c) >= 0xF0) return 4;
		}
		return 2;
	}
	return 1;
}
bool fits_in_ucs1 (cstr q) noexcept
{
	// test whether utf-8 string q can be encoded to ucs1
	// note: if q contains broken characters then these will be replaced with '?' not $FFFD
	//       and thus will not break the result of this function

	if(q) while (char c = *q++)
	{
		if (uchar(c) > 0xC3) return no;
	}
	return yes;
}
bool fits_in_ucs2 (cstr q) noexcept
{
	// test whether utf-8 string q can be encoded to ucs2

	if(q) while (char c = *q++)
	{
		if (uchar(c) >= 0xF0) return no;
	}
	return yes;
}

uint utf8len (ucs1char const* q, uint cnt) noexcept
{
	// calculate required size for an utf-8 string to store ucs1 string

	uint rval = cnt;
	while(cnt--)
	{
		if(*q++ >= 0x80) rval++;
	}
	return rval;
}
uint utf8len (ucs2char const* q, uint cnt) noexcept
{
	// calculate required size for an utf-8 string to store ucs2 string

	uint rval = cnt;
	while(cnt--)
	{
		uint16 c = *q++;
		if(c < 0x80) continue;
		rval += c<0x800 ? 1 : 2;
	}
	return rval;
}
uint utf8len (ucs4char const* q, uint cnt) noexcept
{
	// calculate required size for an utf-8 string to store ucs4 string

	uint rval = cnt;
	while(cnt--)
	{
		uint32 c = *q++;
		if(c < 0x80) continue;
		rval += c<0x800 ? 1 : c<0x10000 ? 2 : c<0x200000 ? 3 : c<0x4000000 ? 4 : 5;
	}
	return rval;

}

char* ucs1_to_utf8 (ucs1char const* q, uint qcnt, char* z) noexcept
{
	// encode ucs1 text to utf-8
	// this can encode any 1-byte data to utf-8
	// if $00 is part of the source data it will be encoded as 2 non-zero bytes
	// return: ptr behind z[]

	while (qcnt--)
	{
		uint c = *q++;
		if (c && c < 0x80) { *z++ = char(c); continue; }

		*z++ = 0xC0 + char(c>>6);
		*z++ = char(0x80 + (c & 0x3F));
	}
	*z = 0;
	return z;
}
char* ucs2_to_utf8 (ucs2char const* q, uint qcnt, char* z) noexcept
{
	// encode ucs2 text to utf-8
	// this can encode any 2-byte data to utf-8
	// if $00 is part of the source data it will be encoded as 2 non-zero bytes
	// return: ptr -> chr0 at end of utf8 text

	while(qcnt--)
	{
		uint c = *q++;
		if (c && c <  0x80) { *z++ = char(c); continue; }

		if (c < 0x800) { *z++ = 0xC0 + char(c>>6);  goto f1; }
		else		   { *z++ = 0xE0 + char(c>>12); goto f2; }

		f2:	*z++ = char(0x80 + ((c>>6)  & 0x3F));
		f1:	*z++ = char(0x80 + ((c)     & 0x3F));
	}
	*z = 0;
	return z;
}
char* ucs4_to_utf8 (ucs4char const* q, uint qcnt, char* z) noexcept
{
	// encode ucs4 text to utf-8
	// this can encode any 4-byte data to utf-8
	// if $00 is part of the source data it will be encoded as 2 non-zero bytes
	// return: ptr -> chr0 at end of utf8 text

	while(qcnt--)
	{
		uint32 c = *q++;
		if (c && c <  0x80) { *z++ = char(c); continue; }

		if (c <      0x800) { *z++ = 0xC0 + char(c>>6);  goto f1; }
		if (c <    0x10000) { *z++ = 0xE0 + char(c>>12); goto f2; }
		if (c <   0x200000) { *z++ = 0xF0 + char(c>>18); goto f3; }
		if (c <  0x4000000) { *z++ = 0xF8 + char(c>>24); goto f4; }
		else				{ *z++ = 0xFC + char(c>>30); goto f5; }		// full 32 bit encoded

		f5:	*z++ = char(0x80 + ((c>>24) & 0x3F));
		f4:	*z++ = char(0x80 + ((c>>18) & 0x3F));
		f3:	*z++ = char(0x80 + ((c>>12) & 0x3F));
		f2:	*z++ = char(0x80 + ((c>>6)  & 0x3F));
		f1:	*z++ = char(0x80 + ((c)     & 0x3F));
	}
	*z = 0;
	return z;
}

ucs4char utf8_to_ucs4char (cptr q) noexcept
{
	if(!q) return 0;

	uint32 c = *cuptr(q++);
	if (c < 0x80) { return c; }
	if (c < 0xC0) { errno = unexpectedfup; return 0; }

	uint n;
	if (c < 0xE0) { c &= 0x1F; n = 1; } else
	if (c < 0xF0) { c &= 0x0F; n = 2; } else
	if (c < 0xF8) { c &= 0x07; n = 3; } else
	if (c < 0xFC) { c &= 0x03; n = 4; } else
				  {            n = 5; }			// full 32 bit decoded
	do
	{
		if (is_fup(*q))
		{
			c = (c<<6) + (*cuptr(q++) & 0x3F);
			continue;
		}
		else
		{
			c = replacementchar;
			errno = truncatedchar;
			break;
		}
	}
	while(--n);

	return c;
}

ucs4char* utf8_to_ucs4 (cptr q, ucs4char* z) noexcept
{
	// decode utf-8 string into ucs4 buffer
	// the buffer must be large enough to hold the decoded text
	// • sets errno for broken characters: unexpectedfup and truncatedchar
	// • illegal overlong encodings are not trapped
	// • combining characters etc. are not handled
	// return:	ptr -> behind dest
	//			no null-char written
	//
	// possible approach:
	// • precalculate required size of buffer[] with char_count()
	// • provide buffer[strlen(q)] and truncate at size returned by this function

	if(q) while (uint32 c = *cuptr(q++))
	{
		uint n;
		if (c < 0x80) { *z++ = c; continue; }
		if (c < 0xC0) { errno = unexpectedfup; continue; }

		if (c < 0xE0) { c &= 0x1F; n = 1; } else
		if (c < 0xF0) { c &= 0x0F; n = 2; } else
		if (c < 0xF8) { c &= 0x07; n = 3; } else
		if (c < 0xFC) { c &= 0x03; n = 4; } else
					  {            n = 5; }			// full 32 bit decoded
		do
		{
			if (is_fup(*q))
			{
				c = (c<<6) + (*cuptr(q++) & 0x3F);
				continue;
			}
			else
			{
				c = replacementchar;
				errno = truncatedchar;
				break;
			}
		}
		while(--n);

		*z++ = c;
	}

	return z;
}
ucs2char* utf8_to_ucs2 (cptr q, ucs2char* z) noexcept
{
	// decode utf-8 string into ucs2 buffer
	// the buffer must be large enough to hold the decoded text
	// • sets errno for broken characters: unexpectedfup, truncatedchar etc.
	// • illegal overlong encodings are not trapped
	// • combining characters etc. are not handled

	if(q) while (uint c = *cuptr(q++))
	{
		uint n;
		if (c < 0x80) { *z++ = ucs2char(c); continue; }
		if (c < 0xC0) { errno = unexpectedfup; continue; }

		if (c < 0xE0) { c &= 0x1F; n = 1; } else
		if (c < 0xF0) { c &= 0x0F; n = 2; }
		else
		{
			errno = notindestcharset;
			*z++ = replacementchar;
			n = c <= 0xF7 ? 3 : c <= 0xFB ? 4 : 5;
			while (n-- && is_fup(*q)) { q++; }
			continue;
		}

		while(n--)
		{
			if (is_fup(*q))
			{
				c = (c<<6) + (*cuptr(q++) & 0x3F);
				continue;
			}
			else
			{
				errno = truncatedchar;
				c = replacementchar;
				break;
			}
		}

		*z++ = ucs2char(c);
	}

	return z;
}
ucs1char* utf8_to_ucs1 (cptr q, ucs1char* z) noexcept
{
	// decode utf-8 string into ucs1 buffer
	// the buffer must be large enough to hold the decoded text

	if(q) while (uint c = *cuptr(q++))
	{
		if (c <= 0x7F) { *z++ = ucs1char(c); continue; }
		if (c <= 0xBF) { errno = unexpectedfup; continue; }

		if (c <= 0xc3)	// %110000xx + %10xxxxxx
		{
			if (is_fup(*q))
			{
				*z++ = ucs1char((c<<6) + (*cuptr(q++) & 0x3F));
			}
			else
			{
				errno = truncatedchar;
				*z++ = replacementchar;
			}
			continue;
		}

		errno = notindestcharset;
		*z++ = replacementchar;

		uint n = c < 0xE0 ? 1 : c < 0xF0 ? 2 : c < 0xF8 ? 3 : c < 0xFC ? 4 : 5;
		while (n-- && is_fup(*q)) { q++; }
	}

	return z;
}

str to_utf8(ucs1char const* q, uint qcnt) throws
{
	str s = tempstr(utf8len(q,qcnt));
	ucs1_to_utf8(q,qcnt,s);
	return s;
}
str to_utf8(ucs2char const* q, uint qcnt) throws
{
	str s = tempstr(utf8len(q,qcnt));
	ucs2_to_utf8(q,qcnt,s);
	return s;
}
str to_utf8(ucs4char const* q, uint qcnt) throws
{
	str s = tempstr(utf8len(q,qcnt));
	ucs4_to_utf8(q,qcnt,s);
	return s;
}

str to_utf8 (ucs1char const* q)
{
	return to_utf8(q,charcount(q));
}
str to_utf8 (ucs2char const* q)
{
	return to_utf8(q,charcount(q));
}
str to_utf8 (ucs4char const* q)
{
	return to_utf8(q,charcount(q));
}

ucs1char* to_ucs1(cstr q) throws
{
	uint cnt = charcount(q);
	uint8* z = reinterpret_cast<ucs1char*>(tempmem(cnt+1));
	utf8_to_ucs1(q,z);
	z[cnt] = 0;
	return z;
}
ucs2char* to_ucs2(cstr q) throws
{
	uint cnt = charcount(q);
	uint16* z = reinterpret_cast<ucs2char*>(tempmem(cnt*2+2));
	utf8_to_ucs2(q,z);
	z[cnt] = 0;
	return z;
}
ucs4char* to_ucs4(cstr q) throws
{
	uint cnt = charcount(q);
	uint32* z = reinterpret_cast<ucs4char*>(tempmem(cnt*4+4));
	utf8_to_ucs4(q,z);
	z[cnt] = 0;
	return z;
}

cstr fromhtmlstr (cstr s0) throws
{
	// decode &lt; &rt; &amp; &quot; and <br>
	// decode &#123; and &#x123;

    if (!s0||!*s0) return emptystr;

    cptr q = strchr(s0,'&');
    cptr q2 = find(s0,"<br>");
    if(q2 && (q==nullptr || q2<q) ) q = q2;
    if (q==nullptr) return s0;
    str  s = dupstr(s0);
    ptr  z = s + (q-s0);

    for (;;)
    {
	    char c;
        while ((c=*q++) != '&')
        {
			if(c=='<' && *str_comp("br>", q)==0) { *z++ = '\n'; q+=3; continue; }
			*z++ = c;
			if (!c) return s;
		}

        if (*str_comp("gt;",  q)==0) { *z++ = '>'; q+=3; continue; }
        if (*str_comp("lt;",  q)==0) { *z++ = '<'; q+=3; continue; }
        if (*str_comp("amp;", q)==0) { *z++ = '&'; q+=4; continue; }
        if (*str_comp("quot;",q)==0) { *z++ = '"'; q+=5; continue; }
        //if (*str_comp("apos;",q)==0) { *z++ = '\''; q+=5; continue; }	// XML

		if(*q=='#')		// &#1234;  or  &#x1234;
		{
			uint32 n = 0;
			cptr p = q+1;

			if(*p == 'x')
			{
				p++;
				while (p<q+6 && *p && *p!=';' && ::is_hex_digit(*p)) { n = (n<<4) + ::digit_value(*p++); }
			}
			else
			{
				while (p<q+6 && *p && *p!=';' && ::is_dec_digit(*p)) { n = (n<<4) + ::digit_val(*p++); }
			}
			if (*p == ';')
			{
				q = p+1;
				z = ucs4_to_utf8(&n,1,z);
				continue;
			}
		}

        *z++ = '&';	// unencoded '&'
    }
}

cstr detabstr (cstr s, uint tabs) noexcept
{
	// expand tabs to spaces
	// returns the original string if there were no tabs found

	assert( tabs>=1 && tabs<=99 );
	if (s==nullptr) return nullptr;
	if (strchr(s,'\t') == nullptr) return s;

	// expand tabs to spaces
	// all tab characters are replaced by 1 to 'tabstops' spaces
	// to fill up to next multiple of 'tabstops'
	// tabstops are re-synced at line breaks
	// all characters which are not '\t', '\n' or '\r' are assumed to be printable
	// escape sequences etc. will mess up the column counting anyway.
	// utf8 FUPs are not counted as characters.

	uint qlen = uint(strlen(s));
	uint zlen = qlen+tabs*4;
	str  zstr = tempstr(zlen);

	cptr q  = s;
	cptr qe = q + qlen;
	ptr  z0 = zstr;
	ptr  z  = z0;
	char c;

	while ((c=*q++))
	{
		if (is_fup(c))		{ *z++ = c; z0++; continue; }
		if (c==13 || c==10) { *z++ = c; z0 = z; continue; }
		if (c != '\t')		{ *z++ = c; continue; }

		uint n = tabs - (z-z0)%tabs;
		if(qe-q > zstr+zlen-(z+n))
		{
			zlen += tabs*4;
			cstr zalt = zstr;
			zstr = tempstr(zlen); strcpy(zstr,zalt);
			z0 += zstr-zalt; z += zstr-zalt;
		}
		while(n--) *z++ = ' ';
	}

	*z = 0;
	return zstr;
}

str whitestr (cstr q, char c) noexcept
{
	// create blanked-out copy of string
	// replace all printable characters with space
	// usecase: to place an error indicator exactly beneath an error

    str rval = dupstr(q);
    for (ptr p = rval; *p; p++)
    {
		if(*p >= 0)
		{
			if (*p > ' ') *p = c;
			continue;
		}

		ptr z; for(z=p; *p; p++)
	    {
			if (no_fup(*p))
			{
				*z++ = *p > ' ' ? c : *p;
			}
	    }
		*z = 0;
	}
    return rval;
}

str unescapedstr (cstr s0) noexcept  // sets errno
{
	// preserves escaped char(0) as unicode 0xC0 + 0x80

	if (!s0 || !*s0) return emptystr;

	char c, *q, *z;
	str s = dupstr(s0);

	q = z = strchr(s,'\\');

	if(q) for(;;)
	{
		while ((c=*q++)!='\\')
		{
			*z++ = c;
			if (!c) return s;
		}

		c = *q++;					// c = next char after '\'

		if (::is_oct_digit(c))
		{
			// \ooo => octal value for next byte. 9th bit discarded
			// \oo     allowed but not recommended.
			// \o	   allowed but not recommended. typically used for chr(0)

			uint d;
			c = char(::digit_val(c));
			d = digit_val(q); if (d<8) { q++; c = char(c<<3)+char(d); }
			d = digit_val(q); if (d<8) { q++; c = char(c<<3)+char(d); }
			if (c==0) { *z++ = char(0xC0); c = char(0x80); }
		}

		else if (c=='x' && is_hex_digit(q))
		{
			// \xHH => hex coded value for next byte
			// \xH     allowed but not recommended.
			// \x      'x' masked for unknown reason => stores 'x'

			c = char(::digit_value(*q++));
			if (is_hex_digit(q)) c = char(c<<4) + char(::digit_value(*q++));
			if (c==0) { *z++ = char(0xC0); c = char(0x80); }
		}

		else if ((c|0x20) == 'u' && is_hex_digit(q))
		{
			// 'u' -> ucs2 character (4 hex digits)
			// 'U' -> ucs4 character (8 hex digits)

			ucs4char d = 0;
			uint n = c=='U' ? 8 : 4;
			do { d = (d<<4) + digit_value(q++); } while (--n && is_hex_digit(q));
			z = ucs4_to_utf8(&d,1,z);
			continue;
		}

		else if (!c)
		{
			// remove '\' at end of string:

			errno = brokenescapecode;
			continue;
		}

		else if (c=='\n' || c=='\r')
		{
			// '\n' => store '\n' and skip white space

			while (*q && *q<=' ') q++;
			continue;	// don't store the '\n'
		}

		else
		{
			// standard escape codes or s.th. escaped for unknown reason:

			static const char ec[] = "\\\"'?abfnrtv";			// escape character
			static const char cc[] = "\\\"'\?\a\b\f\n\r\t\v";	// result
			cptr p = strchr(ec,c);
			if (p) c = cc[p-ec];				// valid escaped sequence
			else errno = brokenescapecode;		// else self-escaped char
		}

		*z++ = c;
	}
	return s;
}

}; // namespace















