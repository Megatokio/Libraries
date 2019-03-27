/*	Copyright  (c)	Günter Woigk 1995 - 2019
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

    1995-06-29 kio	started work on pascal string lib
    2000-01-28 kio	started work on c string lib
    2002-01-19 kio	started unix port
    2002-01-26 kio	simplified usingstr(): now using vsnprintf()
    2002-01-26 kio	added xpool[] for unlimited string length support
    2004-12-16 kio	split into functions and qstring part for multi threading safe version for Cocoa
    2006-10-31 kio	renamed QString() to QuickStr() wg. naming collission with Qt.
    2008-05-14 kio	renamed QuickStr() to tempStr()
    2008-05-15 kio	unified thread-safe temporary memory allocation using the new tempMem library
    2010-10-17 kio	skip whitespace after \-escaped linebreak in ConvertedFrom(escaped)
*/

#include "config.h"
#include <math.h>
#include <time.h>
#include "cstrings.h"
#include "../unix/tempmem.h"
#include "../Templates/Array.h"


// hint for decimal POINT:
//
// #include <clocale>
// std::setlocale(LC_ALL, "en_US");


#ifdef HAVE_NAN_H
 #include	<nan.h>
#endif
#ifndef NAN
 static const double NAN = 0.0/0.0;
#endif
static char null = 0;
str emptystr = &null;


// statt UTF-8.h:
inline bool utf8_is_7bit (char c)	{ return int8(c) >= 0; }
inline bool	utf8_no_7bit (char c)	{ return int8(c) < 0;  }
inline bool utf8_is_fup	 (char c)	{ return int8(c) < int8(0xc0); }
inline bool utf8_no_fup	 (char c)	{ return int8(c) >= int8(0xc0); }


str newstr (int n) noexcept
{
	// allocate char[]
	// deallocate with delete[]
	// presets terminating 0

    str c = new char[n+1];
    c[n] = 0;
    return c;
}

str newcopy (cstr s) noexcept
{
	// allocate char[]
	// deallocate with delete[]
	// returns NULL if source string is NULL

    str c = nullptr;
    if (s)
    {
        c = newstr(int(strlen(s)));
        strcpy(c,s);
    }
    return c;
}

str dupstr (cstr s) noexcept
{
	// Create copy of string in tempmem

    if (!s||!*s) return emptystr;
    size_t n = strlen(s);
    str c = tempstr(int(n));
    memcpy(c,s,n);
    return c;
}

str xdupstr (cstr s) noexcept
{
	// Create copy of string in the outer tempmem pool

    if (!s||!*s) return emptystr;
    size_t n = strlen(s);
    str c = xtempstr(int(n));
    memcpy(c,s,n);
    return c;
}

cptr find (cstr target, cstr search) noexcept
{
	// search sub string

    if (!search||!*search) return target;

    int t_len = int(strLen(target));
    int s_len = int(strLen(search));

    for (int i=0; i <= t_len-s_len; i++)
    {
        cptr s = search;
        cptr t = target+i;
        while(*s && *s==*t) { s++; t++; }
        if (*s==0) return target+i;
    }

    return nullptr;	// not found
}

cptr rfind (cstr target, cstr search) noexcept
{
	// search sub string

    int t_len = int(strLen(target));
    int s_len = int(strLen(search));

    if (!search||!*search) return target+t_len;
    cptr se = search+s_len;

    for (int i=t_len-s_len; i>=0; i--)
    {
        cptr s = search;
        cptr t = target+i;
        do { if (s==se) return target+i; } while (*s++==*t++);
    }

    return nullptr;	// not found
}

str spacestr (int n, char c) noexcept
{
	// Create and clear a string

    if (n<=0) return emptystr;
    str s = tempstr(n);
    memset(s,c,uint(n));
    return s;
}

cstr spaces (uint n) noexcept
{
	// return a const space string
	// returned string is const if ≤ 40 char
	// or in tempmem if longer

	static const char _spaces[] = "                                        ";
	static uint maxlen = NELEM(_spaces)-1;

	return n>maxlen ? spacestr(int(n)) : _spaces + maxlen - n;
}

str whitestr (cstr q, char c) noexcept
{
	// create blanked-out copy of string
	// replace all printable characters with space
	// usecase: to place an error indicator exactly beneath an error

    str s = dupstr(q);
    for (ptr p = s; *p; p++)
    {
		if (*uptr(p) > ' ') *p = c;
	}
    return s;
}

// create string from char:
str charstr (char c) noexcept
{
    str s = tempstr(1);
    *s = c;
    return s;
}
str charstr (char c1, char c2) noexcept
{
    str s = tempstr(2);
    s[0] = c1;
    s[1] = c2;
    return s;
}
str charstr (char c1, char c2, char c3) noexcept
{
    str s = tempstr(3);
    s[0] = c1;
    s[1] = c2;
    s[2] = c3;
    return s;
}
str charstr (char c1, char c2, char c3, char c4) noexcept
{
    str s = tempstr(4);
    s[0] = c1;
    s[1] = c2;
    s[2] = c3;
    s[3] = c4;
    return s;
}
str charstr (char c1, char c2, char c3, char c4, char c5) noexcept
{
    str s = tempstr(5);
    s[0] = c1;
    s[1] = c2;
    s[2] = c3;
    s[3] = c4;
    s[4] = c5;
    return s;
}

str replacedstr (cstr s, char old, char nju) noexcept
{
	// Replace any occurance of a character by another

    str t = dupstr(s);
    for (ptr p=t; *p; p++) if (*p==old) *p = nju;
    return t;
}

str lowerstr (cstr s) noexcept
{
	// Convert a string to all lower case

    str t = dupstr(s);
    tolower(t);
    return t;
}

str upperstr (cstr s) noexcept
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

str mulstr (cstr q, uint n) throws // limit_error
{
	// Repeate string n times

    if (!q || !*q || !n) return emptystr;

    size_t len = strlen(q);
    if(len*n > 0xFFFFFFu) throw limit_error("mulstr()",len*n,0xFFFFFFu);	// 16 MB
    str s = tempstr(uint(len*n));
    ptr z = s;

    while (n--) { memcpy(z,q,len); z += len; }

    return s;
}

str catstr (cstr s1, cstr s2) noexcept
{
	// Concatenate 2 strings

    str s = tempstr(strLen(s1)+strLen(s2));
    s[0]=0;
    if (s1) strcpy ( s, s1 );
    if (s2) strcat ( s, s2 );
    return s;
}

str catstr (cstr s1, cstr s2, cstr s3, cstr s4, cstr s5, cstr s6) noexcept
{
	// Concatenate up to 6 strings

    str s = tempstr( strLen(s1)+strLen(s2)+strLen(s3)+strLen(s4)+strLen(s5)+strLen(s6) );
    s[0]=0;
    if (s1) strcpy ( s, s1 );
    if (s2) strcat ( s, s2 );
    if (s3) strcat ( s, s3 );
    if (s4) strcat ( s, s4 );
    if (s5) strcat ( s, s5 );
    if (s6) strcat ( s, s6 );
    return s;
}

str hexstr (uint32 n, uint digits) noexcept
{
	// Convert number to hexadecimal string

    static const char hex[] = "0123456789ABCDEF";
    str c = tempstr(digits);

    while(digits)
    {
        c[--digits] = hex[n&0x0f];
        n >>= 4;
    }
    return c;
}

str hexstr (uint64 n, uint digits) noexcept
{
	// Convert number to hexadecimal string

    static const char hex[] = "0123456789ABCDEF";
    char* c = tempstr(digits);

    while(digits)
    {
        c[--digits] = hex[n&0x0f];
        n >>= 4;
    }
    return c;
}

str binstr (uint value, cstr b0, cstr b1) noexcept
{
	// Convert number to binary string

	assert(b0&&b1);
	uint n = uint(strlen(b0));
	assert(n == strlen(b1));
	str s = tempstr(n); memcpy(s,b0,n);

	while (n--)
	{
		if (b0[n] == b1[n]) n--;
		if (value & 1) s[n] = b1[n];
		value = value >> 1;
	}
	return s;
}

str substr (cstr a, cstr e) noexcept
{
	// create string [a ... [e
	// does not check for 0-characters between a and e

    assert(!a==!e);

    if (e <= a) return emptystr;
    str c = tempstr(int(e-a));
    memcpy(c,a,uint(e-a));
    return c;
}

str leftstr (cstr s, int n) noexcept
{
	// create left substring
	// splits n bytes from the left
	// if source string is shorter, the result is shorter too

	if (n<=0 || !s || !*s) return emptystr;

	cptr a = s;
	cptr e = s+n;
	while (a<e && *a) a++;

	n = int(a-s);
	str c = tempstr(n);
	memcpy(c,s,uint(n));
	return c;
}

str rightstr (cstr s, int n) noexcept
{
	// create right substring
	// splits n bytes from the right
	// if source string is shorter, the result is shorter too

    int m = int(strLen(s)); if (m < n) n = m;
    if (n <= 0) return emptystr;
    str c = tempstr(n);
    memcpy(c,s+m-n,uint(n));
    return c;
}

str midstr (cstr s, int a, int n) noexcept
{
	// create mid substring
	// skip a bytes from the left then split n bytes
	// if source string is shorter, the result is shorter too

    if (a < 0) { n += a; a = 0; }
    n = min(n,int(strLen(s))-a);
    if (n <= 0) return emptystr;
    return substr(s+a,s+a+n);
}

str midstr (cstr s, int a) noexcept
{
	// create mid-to-end substring

    if (!s) return emptystr;
    if (a < 0) a = 0;
    return substr(s+a,strchr(s,0));
}

str usingstr (cstr format, ...) noexcept
{
	// create formatted string

    va_list va;
    va_start(va,format);
    str s = usingstr(format,va);
    va_end(va);
    return s;
}

str usingstr (cstr format, va_list va) noexcept
{
	// note: caller must call va_start() prior and va_end() afterwards.

	// from the GNU autoconf manual:
	// va_copy: The ISO C99 standard provides va_copy for copying va_list variables.
	// It may be available in older environments too, though possibly as __va_copy
	// (e.g., gcc in strict C89 mode). These can be tested with #ifdef.
	// A fallback to memcpy (&dst, &src, sizeof(va_list)) will give maximum portability.

	// va_list is not necessarily just a pointer.
	// It can be a struct (e.g., gcc on Alpha), which means NULL is not portable.
	// Or it can be an array (e.g., gcc in some PowerPC configurations),
	// which means as a function parameter it can be effectively call-by-reference
	// and library routines might modify the value back in the caller
	// (e.g., vsnprintf in the GNU C Library 2.1).

    int err = errno;							// save errno

    va_list va2;								// duplicate va_list
    va_copy(va2,va);

    static char bu[1];
    int n = vsnprintf( bu, 0, format, va2 );	// calc. req. size
    assert(n>=0);

    str z = tempstr(n);
    vsnprintf( z, size_t(n+1), format, va );	// create formatted string

    errno = err;								// restore errno
    return z;
}

str speakingNumberStr (double n) throws
{
	// create beautified text for number

    if (n>=1000000000) return usingstr( "%.3lf GB", n/1000000000 );
    if (n>=1000000   ) return usingstr( "%.3lf MB", n/1000000 );
    if (n>=1000      ) return usingstr( "%.3lf kB", n/1000 );
                       return usingstr( "%.0lf bytes", n );
}

/* ----	convert string ------------------------------
*/
static const char cc[] = "\\\"\a\b\f\n\r\t\v";	// control codes
static const char ec[] = "\\\"abfnrtv";			// escape characters

static cptr str_comp(cstr a, cstr b) noexcept
{
	char c,d;
    while ((c=*a) && (d=*b) && c==d) { a++; b++; }
    return a;
}

str tohtmlstr (cstr s0) noexcept
{
	str  s;
	char c;
	int  i;
	cptr z;

	if (!s0 || !*s0) return emptystr;
	s = str(s0);

	for (i=0; (c=s[i]); i++)
	{
		switch(c)
		{
		default: 	continue;
		case '<':	z = "&lt;";   break;
		case '>':	z = "&gt;";   break;
		case '&':	z = "&amp;";  break;
		case '"':	z = "&quot;"; break;
		case '\n':	z = "<br>";   break;
		}
		s = catstr(leftstr(s,i), z, s+i+1); i+=3;
	}

	return s!=s0 ? s : dupstr(s0);
}

str	escapedstr (cstr s0) noexcept
{
	str  s;
	cptr q;
	ptr  z;
	char c;

	if (!s0 || !*s0) return emptystr;
	z = s = str(s0);

	while ((c=*z))
	{
		if (uchar(c)>=' ' && c!=0x7F && c!='\\' && c!='\"')
		{
			z++;
			continue;
		}

		if ((q=strchr(cc,c)))
		{
			c = ec[q-cc];
			q = s; s = catstr(substr(s,z+1), z); z += s-q; *z++ = '\\'; *z++ = c;
			continue;
		}

		char oc[] = "\\000";
		oc[1] = '0'+(c>>6); c &= (1<<6)-1;
		oc[2] = '0'+(c>>3); c &= (1<<3)-1;
		oc[3] = '0'+(c>>0);
		q = s; s = catstr(substr(s,z),oc,z+1); z += 4+(s-q);
		continue;
	}

	return s!=s0 ? s : dupstr(s0);
}

str quotedstr( cstr s ) noexcept
{
    return catstr( "\"", escapedstr(s), "\"" );
}

str	unescapedstr (cstr s0) noexcept  // sets errno
{
	char c, *q, *z;

	if(!s0||!*s0) return emptystr;
	str s = dupstr(s0);

	q = z = strchr(s,'\\');

	if(q) for(;;)
	{
		while((c=*q++)!='\\')
		{
			*z++ = c;
			if(!c) return s;
		}

		c = *q++;					// c = next char after '\'

		if (is_oct_digit(c))
		{
			// \ooo => octal value for next byte. 9th bit discarded
			// \oo     allowed but not recommended.
			// \o	   allowed but not recommended.

			uint d;
			c = char(digit_val(c));
			d = digit_val(*q); if (d<8) { q++; c = char(c<<3)+char(d); }
			d = digit_val(*q); if (d<8) { q++; c = char(c<<3)+char(d); }
			if (c==0) errno = brokenescapecode;
		}

		else if (c=='x' && is_hex_digit(*q))
		{
			// \xHH => hex coded value for next byte
			// \xH     allowed but not recommended.
			// \x      'x' masked for unknown reason => stores 'x'

			c = char(digit_value(*q++));
			if (is_hex_digit(*q)) c = char(c<<4) + char(digit_value(*q++));
			if (c==0) errno = brokenescapecode;
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
			else errno = brokenescapecode;  	// else self-escaped char
		}

		*z++ = c;
	}
	return s;
}

str	unquotedstr (cstr s0) noexcept // sets errno
{
    char c;
    str  s;
    uint n;

    if (!s0||!*s0) return emptystr;

    s = dupstr(s0);

    n = uint(strlen(s));
    c = s[0];
    if( n>=2 && (c=='"'||c=='\'') && s[n-1]==c )
    {
		// wir ignorieren, dass das schließende Zeichen fehlerhafterweise escaped sein könnte.
		// unescapedstr() will set errno.
		s[n-1] = 0;
		s++;
    }

    return unescapedstr(s);
}

cstr fromhtmlstr (cstr s0) noexcept
{
	// decode &lt; &rt; &amp; &quot; and <br>
	// more versatile decoder in utf8.cpp

    if (!s0||!*s0) return emptystr;

    cptr q = strchr(s0,'&');
    cptr q2 = find(s0,"<br>");
    if (q2 && (q == nullptr || q2 < q) ) q = q2;
    if (q == nullptr) return s0;

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
        *z++ = '&';	// unencoded '&'
    }
}

str fromutf8str (cstr qstr) noexcept // sets errno
{
    if (!qstr || *qstr==0) return emptystr;

    if (*qstr == char(0xef) && *(qstr+1) == char(0xbb) && *(qstr+2) == char(0xbf)) qstr += 3;	// skip BOM

    int	zlen = 0;					// golden rule: every non_fup makes a character!
    for (cptr q=qstr;*q;) { zlen += utf8_no_fup(*q++); }

    str	zstr = tempstr(zlen);
    cptr q = qstr;
    ptr  z = zstr;
    char c1, c2;

    while (*q)
    {
        if (utf8_is_7bit(c1=*q++))			{ *z++ = c1; continue; }
        if (utf8_is_fup(c1))				{ errno = unexpectedfup; continue; }
        if (uint8(c1) >= 0xc4)				{ *z++ = '?'; errno = notindestcharset; continue; }
        if (*q==0 || utf8_no_fup(c2=*q))	{ *z++ = '?'; errno = truncatedchar; continue; }
        q++; *z++ = char((c1<<6) + (c2&0x3f));
    }
    return zstr;
}

str toutf8str (cstr qstr) noexcept
{
    if (qstr==nullptr || *qstr==0) return nullptr;

    int	zlen = 0;
    cptr q=qstr; while(*q) zlen += utf8_no_7bit(*q++); zlen += q-qstr;
    str	zstr = tempstr(zlen);
    ptr z = zstr;
    q = qstr;

    while(*q)
    {
        char c = *q++;
        if(utf8_is_7bit(c)) { *z++ = c; continue; }	// 7-bit ascii
        *z++ = char(0xC0 + (uchar(c)>>6));		// 2-char utf8 code
        *z++ = char(0x80 + (c&0x3f));
    }
    return zstr;
}

/* ====	date&time =============================================
        note: time_t is always in UTC  (seconds since 1970-1-1 0:00 GMT)
*/
static int next_number (cptr& c, cptr e) noexcept
{
    int  n = 0;
    while (c<e && no_dec_digit(*c)) { c++; }
    while (c<e && is_dec_digit(*c)) { n = n*10 + *c++ -'0'; }
    return n;
}

time_t dateval (cstr datestr) noexcept
{
	// datestr = "year month day hour minute second" in local timezone
	// optional from right to left
	// separated by any non-digit
	// year 20XX may be abbreviated as year XX
	// month must be numeric (not a name)

	// interesting reading: http://www.catb.org/esr/time-programming/

	tm d;
	cptr c = datestr;
	cptr e = strchr(datestr,0);

	int n = next_number(c,e);
	if (n<70) n += 2000; else if (n<100) n += 1900;
	d.tm_year = n - 1900;

	d.tm_mon  = next_number(c,e) - 1;
	d.tm_mday = next_number(c,e);
	d.tm_hour = next_number(c,e);
	d.tm_min  = next_number(c,e);
	d.tm_sec  = next_number(c,e);
    d.tm_isdst  = -1;	// 3-way flag:  -1 -> DST status unknown; 0 -> use DST=off; 1 -> use DST=on

	//d.tm_wday = 0;	output field
	//d.tm_yday = 0;	output field
	//#if !defined(_SOLARIS)
	//d.tm_gmtoff = 0;	output field
	//d.tm_zone   = "";	output field
	//#endif

	return mktime(&d);		// local time
	//return timegm(&d);	// UTC
}

str datetimestr (time_t secs) noexcept
{
	tm d; localtime_r(&secs,&d);
	return usingstr( "%04u-%02u-%02u %02u:%02u:%02u",
				d.tm_year+1900, d.tm_mon+1, d.tm_mday, d.tm_hour, d.tm_min, d.tm_sec );
}

str datestr (time_t secs) noexcept
{
    tm d; localtime_r(&secs,&d);
    return usingstr( "%04u-%02u-%02u", d.tm_year+1900, d.tm_mon+1, d.tm_mday );
}

str timestr (time_t secs) noexcept
{
    tm d; localtime_r(&secs,&d);
    return usingstr( "%02u:%02u:%02u", d.tm_hour, d.tm_min, d.tm_sec );
}

ON_INIT(tzset);	// because localtime_r() is not guaranteed to call tzset()


str durationstr (time_t secs) noexcept
{
	uint s = uint(secs);	 if (s<300) return usingstr("%u sec.",          s);
	uint m = s/60; s = s%60; if (m<60)	return usingstr("%um:%02us",        m,s);
	uint h = m/60; m = m%60; if (h<24)	return usingstr("%uh:%02um:%02us",  h,m,s);
	uint d = h/24; h = h%24;			return usingstr("%ud:%02uh:%02um",d,h,m  );
}

str durationstr (double secs) noexcept
{
    return secs >= 300 ? durationstr( time_t(secs) ) : usingstr( "%.3f sec.",secs );
}


bool lt (cptr a, cptr b) noexcept
{
	if (a && b)
	{
	    while(*a == *b && *a) { a++; b++; }
		return *a < *b;
	}
	else
	{
		return b && *b && !(a && *a);		// nullptr == ""
	}
}

bool gt (cptr a, cptr b) noexcept
{
	if (a && b)
	{
	    while(*a == *b && *a) { a++; b++; }
		return *a > *b;
	}
	else
	{
		return a && *a && !(b && *b);		// nullptr == ""
	}
}

bool gt_tolower (cptr a, cptr b) noexcept
{
    while(*a && to_lower(*a)==to_lower(*b)) { a++; b++; }
    return to_lower(*a) > to_lower(*b);
}

bool eq (cptr s, cptr t) noexcept
{
    if (s && t)
    {
        while (*s) if (*s++ != *t++) return false;
        return *t == 0;
    }
    else
    {
		return !(s && *s) && !(t && *t);	// nullptr == ""
    }
}

bool ne (cptr s, cptr t) noexcept
{
    if (s && t)
    {
        while (*s) if (*s++ != *t++) return true;
        return *t != 0;
    }
    else
    {
        return (s && *s) || (t && *t);		// nullptr == ""
    }
}

str hexstr (cptr s, uint n) noexcept
{
	assert(s!=nullptr || n==0);

	ptr z = tempstr(n*2) + n*2;
	s += n;
	while(n--) { char c = *--s; *--z = hexchar(c); *--z = hexchar(c>>4); }
	return z;
}

str unhexstr (cstr s) noexcept
{
	// returns nullptr on any error

    if (!s) return nullptr;

    int n = int(strlen(s));
    if(n&1) return nullptr;
    n = n/2;
    str z = tempstr(n);
    while (*s)
    {
        uint c1 = digit_value(*s++);
        uint c2 = digit_value(*s++);
        if ((c1|c2) >> 4) return nullptr;
        *z++ = char((c1<<4) + c2);
    }
    return z-n;
}

static const int8 base64[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//static const int8 base64url[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
#define x 0xff
static const uint8 unbase64[128] =
{ 	x,x,x,x,x,x,x,x,        x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,         x,x,x,x,x,x,x,x,
    x,x,x,x,x,x,x,x,        x,x,x,62,x,62,x,63,      52,53,54,55,56,57,58,59, 60,61,x,x,x,x,x,x,
    x,0,1,2,3,4,5,6,        7,8,9,10,11,12,13,14,    15,16,17,18,19,20,21,22, 23,24,25,x,x,x,x,63,
    x,26,27,28,29,30,31,32, 33,34,35,36,37,38,39,40, 41,42,43,44,45,46,47,48, 49,50,51,x,x,x,x,x };
#undef x

str base64str (cstr s0) noexcept
{
	// base64 encode a string
    // base64 encoding uses only 3 special characters: '+'  and  '/'  and  '='
    // note: the original encoding can add line breaks

    if (!s0) return nullptr;

    int slen = int(strlen(s0));
    int zlen = (slen+2)/3*4;

    str z = tempstr(zlen);

    cuptr s = cuptr(s0);
    cuptr e = s + slen-slen%3;

    while (s<e)
    {
        uint32 c = *s++;
               c = (c<<8) + *s++;
               c = (c<<8) + *s++;

        *z++ = base64[ c>>18    ];
        *z++ = base64[(c>>12)&63];
        *z++ = base64[(c>>6 )&63];
        *z++ = base64[ c     &63];
    }
    if (*s)
    {
        uint32 c = *s++;
        bool   f = *s;
        c = c<<8; if(f) c += *s++;
        c = c<<8;

        *z++ =     base64[ c>>18    ];
        *z++ =     base64[(c>>12)&63];
        *z++ = f ? base64[(c>>6 )&63] : '=';		// indicate padding
        *z++ =                          '=';		// indicate padding
    }

    return z-zlen;
}

str unbase64str (cstr s0) noexcept
{
	// returns nullptr on any error

    if (!s0) return nullptr;

    int slen = int(strlen(s0));
    if(slen%4) return nullptr;

    int padd = s0[slen-1] != '=' ? 0 : s0[slen-2] != '=' ? 1 : 2;
    int zlen = slen/4*3-padd;

    cuptr s = cuptr(s0);

    str  z = tempstr(zlen);
    cptr e = z + zlen-zlen%3;

    while (z<e)
    {
        uint c1 = *s++;
        uint c2 = *s++;
        uint c3 = *s++;
        uint c4 = *s++;
        if ((c1|c2|c3|c4) & 0x80) return nullptr;

        c1 = unbase64[c1];
        c2 = unbase64[c2];
        c3 = unbase64[c3];
        c4 = unbase64[c4];
        if ((c1|c2|c3|c4) & 0x80) return nullptr;

        uint32 n = (c1<<18) + (c2<<12) + (c3<<6) + c4;
        *z++ = char(n>>16);
        *z++ = char(n>>8);
        *z++ = char(n);
    }

    assert((*s==0)==(padd==0));

    if(padd)
    {
        uint c1 = *s++;
        uint c2 = *s++;
        uint c3 = *s++;
        if ((c1|c2|c3)&0x80) return nullptr;

        c1 = unbase64[c1];
        c2 = unbase64[c2];
        c3 = padd==2 ? 0 : unbase64[c3];
        if ((c1|c2|c3)&0x80) return nullptr;

        uint32 n = (c1<<18) + (c2<<12) + (c3<<6) + 0;
        *z++ = char(n>>16);
        if (padd==1)
        *z++ = char(n>>8);
        //*z++ = char(n);

        //*z++ = char((c1<<2) + (c2>>4));
        //if (padd==2)
        //*z++ = char((c2<<4) + (c3>>2));
    }

    return z-zlen;
}

bool startswith (cstr a, cstr b) noexcept
{
    if(!b) return yes;		// leerer suchstring
    if(!a) return !*b;		// leeres target? => nur ok wenn leerer suchstring

    while(*b)
    {
        if(*a++ != *b++) return no;
    }
    return yes;
}

bool endswith (cstr a, cstr b) noexcept
{
    if(!b) return yes;		// leerer suchstring
    if(!a) return !*b;		// leeres target? => nur ok wenn leerer suchstring

    cstr b0 = b;
    a = strchr(a,0);
    b = strchr(b,0);

    while(b>b0)
    {
        if(*--a != *--b) return no;
    }
    return yes;
}

void _split (Array<str>& array, ptr a, ptr e) throws
{
	// split string at "well known" line separators
    // the memory of the source string is used for the array strings!
	// NOTE: the line delimiters in the source string are overwritten with 0!
	// ATTN: the character at ptr e is overwritten with 0 if the buffer was not 0-terminated!

    assert(a!=nullptr);
    array.purge();

    static const int line_separators = 0x3411; // 0b0011010000010001;

    while(a<e)
    {
        // append not-yet-0-delimited string:
        array.append(a);

        // search for line end:
        char c = 0;
        for (; a<e; a++)
        {
            c = *a; if (uint8(c)>13) continue;
            if ((1<<c) & line_separators) break;
        }

        // poke cstring delimiter at string end:
        *a++ = 0;
        if ((c==10||c==13) && a<e && *a+c==23) a++;
    }
}

void _split (Array<str>& array, ptr a, ptr e, char c) throws
{
	// split string at separator
    // the memory of the source string is used for the array strings!
	// NOTE: the line delimiters in the source string are overwritten with 0!
	// ATTN: the character at ptr e is overwritten with 0 if the buffer was not terminated with char c!

    assert(a!=nullptr);
    array.purge();

    while(a<e)
    {
        // append not-yet-0-delimited string:
        array.append(a);

        // search for line end:
        a = ptr(memchr(a,c,size_t(e-a)));
        if(!a) a = e;

        // poke cstring delimiter at string end:
        *a++ = 0;
    }
}

void split (Array<str>& array, cptr a, cptr e) throws
{
	// split string at separator

	ptr b = substr(a,e);
    _split(array, b, b+(e-a));
}

void split (Array<str>& array, cptr a, cptr e, char c) throws
{
	// split string at separator

	ptr b = substr(a,e);
    _split(array, b, b+(e-a), c);
}

void split (Array<str>& dest, cstr s) throws
{
	// split c-string

	split(dest, s, strchr(s,0));
}

void split (Array<str>& dest, cstr s, char c) throws
{
	split(dest, s, strchr(s,0), c);
}

str join (Array<cstr> const& q, cstr s, bool final) throws
{
	uint slen = uint(strlen(s));
	uint len = slen * (q.count() - 1 + final);
	for (uint i=0; i<q.count(); i++) { len += strlen(q[i]); }
	str rval = tempstr(len);

	ptr p = rval;
	for (uint i=0; i<q.count(); i++)
	{
		if (i) { strcpy(p,s); p += slen; }
		strcpy(p,q[i]); p = strchr(p,0);
	}
	if (final) { strcpy(p,s); p += slen; }

	return rval;
}

str join (Array<cstr> const& q) throws
{
	return join(q,"");
}

str join (Array<cstr> const& q, char c, bool final) throws
{
#if 0
	char s[2] = { c, 0 };
	return join(q,s,final);
#else
	uint len = q.count() - 1 + final;
	for (uint i=0; i<q.count(); i++) { len += strlen(q[i]); }
	str rval = tempstr(len);

	ptr p = rval;
	for (uint i=0; i<q.count(); i++)
	{
		if (i) *p++ = c;
		strcpy(p,q[i]); p = strchr(p,0);
	}
	if (final) *p = c;

	return rval;
#endif
}

cstr croppedstr (cstr s) noexcept
{
	// remove white spaces at start and end of string
	// may return original string

	if (!s) return nullptr;

    while (*s && *uptr(s) <= ' ') s++;
    if (*s == 0) return emptystr;

    cptr p = strchr(s,0);
	while (*uptr(p-1) <= ' ') p--;
    if (*p == 0) return s;		// return (rightstr of) original string
	else return substr(s,p);	// reallocate
}

uint strcpy (ptr z, cptr q, uint sz) noexcept
{
	// strncpy variant which always terminates the destination string
    // Returns the resulting string size or the buffer size, if the string was truncated.
    // The string is always delimited with a 0 character unless sz = 0.

	assert(z||sz==0);
	if(!q) q = "";

    ptr za = z, ze = za+sz;
    while(z<ze) { if((*z++=*q++)==0) return uint(--z-za); }
    if(sz) *--z = 0;
    return sz;
}

uint strcat (ptr z, cptr q, uint sz) noexcept
{
	// strncat variant which always terminates the destination string
	// Returns the resulting string size or the buffer size, if the string was truncated.
	// The string is always delimited with a 0 character
	// 	unless sz = 0 or if no '\0' is found in z within sz characters,
	// 	then strcat returns sz and the returned string was and is not terminated!
	// Note: you can blindly concatenate multiple strings with strcat, it will not overflow!

	assert(z||sz==0);
	if(!q) q = "";

    ptr za = z, ze = za+sz;
    while (z<ze && *z) z++;
    return uint(z-za + strcpy(z,q,uint(ze-z)));
}

cstr detabstr (cstr s, uint tabstops) noexcept
{
	// expand tabs to spaces
	// returns the original string if there were no tabs found
	// all tab characters are replaced by 1 to 'tabstops' spaces
	// to fill up to next multiple of 'tabstops'
	// tabstops are re-synced at line breaks
	// all characters which are not '\t', '\n' or '\r' are assumed to be printable
	// escape sequences etc. will mess up the column counting anyway.

	assert( tabstops>=1 && tabstops<=99 );
	if (s==nullptr) return nullptr;
	if (strchr(s,'\t') == nullptr) return s;

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


#ifdef INCLUDE_DEPRECATED

char NextChar ( char const *& p )
{	char c;
    c = *p++; if (c!='\\') return c;	// plain char
    c = *p++;
    if( (c-'0')&~7 )					// no octal digit
    {
        cptr q = strchr(ec,c);
        if (c==0) p--;					// dont advance text pointer if "\" at end of text
        if (q) return cc[q-ec]; 		// control code
        return c;						// unkn. escape char / char escaped for unknown reason
    }
    else								// octal digit
    {
        uchar n = c-'0';
        if (((c=*p++-'0')&~7)==0) { n=n*8+c; if (((c=*p++-'0')&~7)==0) return n*8+c; }
        p--; return n;
    }
}


str ConvertedTo ( cstr s, strconvtype ctype )
{
    switch(ctype)
    {
    case str_noconv:	return newCopy(s);
    case str_html:		return ToHtml(s);
    case str_escaped:	return Escaped(s);
    case str_quoted:	return quotedstr(s);
    default:			IERR();
    }
}

str ConvertedFrom ( cstr s, strconvtype ctype )
{
    switch(ctype)
    {
    case str_noconv:	return newCopy(s);
    case str_html:		return FromHtml(s);
    case str_escaped:	return Unescaped(s);
    case str_quoted:	return Unquoted(s);
    default:			IERR();
    }
}

#endif







