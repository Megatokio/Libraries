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

#include "Libraries/kio/kio.h"
#include "utf8.h"
#include "../unix/tempmem.h"


static cptr str_comp(cstr a, cstr b) noexcept
{
	char c,d;
    while ((c=*a) && (d=*b) && c==d) { a++; b++; }
    return a;
}


inline ptr ucs4char_to_utf8 (uint32 n, ptr z)
{
	if (n<(1<<7))  { *z++ = char(n); return z; }										// 1 byte code
	if (n<(1<<11)) { *z++ = char(0xC0+(n>>6)); *z++ = char(0x80+(n&0x3f)); return z; }	// 2 byte code
	// 3…6 byte codes
	int i = 15; for (uint m = n>>16; m; m>>=5) { i+=5; } i = i/6;	// i = num fups; num bits stored in fups = i*6
	*z++ = char(0x80>>i) + char(n>>(i*6));							// starter (non_fup)
	while (i--) { *z++ = char(0x80 + ((n>>(i*6))&0x3f)); }			// fups
	return z;
}


namespace utf8 {

// statt UTF-8.h:
inline bool is_starter (char c) { return uint8(c) >= 0xc0; }
inline bool is_7bit (char c)	{ return int8(c) >= 0; }
inline bool	no_7bit (char c)	{ return int8(c) < 0;  }
inline bool is_fup	(char c)	{ return int8(c) < int8(0xc0); }
inline bool no_fup	(char c)	{ return int8(c) >= int8(0xc0); }


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
				while (p<q+6 && *p && *p!=';' && is_hex_digit(*p)) { n = (n<<4) + digit_value(*p++); }
			}
			else
			{
				while (p<q+6 && *p && *p!=';' && is_dec_digit(*p)) { n = (n<<4) + digit_val(*p++); }
			}
			if (*p == ';')
			{
				q = p+1;
				z = ucs4char_to_utf8(n,z);
				continue;
			}
		}

        *z++ = '&';	// unencoded '&'
    }
}

cstr detabstr (cstr s, uint tabstops)
{
	// expand tabs to spaces
	// returns the original string if there were no tabs found

	assert( tabstops>=1 && tabstops<=99 );
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
	uint zlen = qlen+tabstops*4;
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

		uint n = tabstops - (z-z0)%tabstops;
		if(qe-q > zstr+zlen-(z+n))
		{
			zlen += tabstops*4;
			cstr zalt = zstr;
			zstr = tempstr(zlen); strcpy(zstr,zalt);
			z0 += zstr-zalt; z += zstr-zalt;
		}
		while(n--) *z++ = ' ';
	}

	*z = 0;
	return zstr;
}

str whitestr (cstr q, char c) throws
{
	// create blanked-out copy of string
	// replace all printable characters with space
	// usecase: to place an error indicator exactly beneath an error

    str s = dupstr(q);
    for (ptr p = s; *p; p++)
    {
		if (*uptr(p) > ' ') *p = c;
		else if(is_starter(*p))
		{
			*p = c;
			ptr e = p+1; while (is_fup(*e)) { e++; }
			strcpy(p+1,e);
		}
	}
    return s;
}

}; // namespace















