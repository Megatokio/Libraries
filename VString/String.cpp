/*	Copyright  (c)	Günter Woigk   2001-2019
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


	=============================================================
		class String
	=============================================================

	2003-08-01 kio	finished work and testing on this string lib variant
	2003-08-02 kio	eliminated StrMem. added operator new() and delete()
	2003-08-05 kio	ToEscaped() no longer escapes spaces
	2003-08-14 kio	fixed bug in ToEscaped()
	2003-08-15 kio	extended NumVal(), new StrVal()
	2003-09-05 kio	void String::ResizeCsz ( CharSize csz )
	2003-09-20 kio	fixed bug in _move(String&)  ((3 days work to find it...))
	2003-10-09 kio	String(cstr,latin_1): convert from utf-8 but assume broken utf-8 is latin-1
	2003-10-11 kio	FNMatch() and Match() to match filenames with patterns ((filename globbing))
	2003-10-15 kio	void String::Replace ( cString& o, cString& n )
	2003-10-18 kio	ToURL() and FromURL()
	2003-10-19 kio	ToTab() and FromTab()
	2003-11-25 kio	To|FromQuoted|Escaped(), StrVal(): now support multiple quoting styles; e.g. «text»
	2003-12-05 kio	To|FromTab(): resync tabs at \n characters
					fixed bugs in _resize() (csz got lost) and ToEscaped() (sparebyte too few)
	2004-06-04 kio	CalcHash()
	2004-06-23 kio	eliminated initialization code --> no more cStrings which were used by creators or so
	2005-06-11 kio	extension to utf-8 encoding to allow full 32 bit round-trip conversions
	2005-06-30 kio	String::Crop(left,right)
	2005-07-12 kio	ReadStringFromFile()
	2010-05-20 kio	korr. StrVal("'\''") error
*/

#define SAFE 1
#define LOGLEVEL 0
#define LOG 0         // for use in project vipsi
#include "kio/kio.h"
#include <sys/stat.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "String.h"
#include "cstrings/utf8.h"
#include "cstrings/legacy_charsets.h"
DEBUG_INIT_MSG;


#ifdef NDEBUG
  #define XXXCHECK(v)	((void)0)
#else
  #define XXXCHECK(v)	(v).Check(__FILE__,__LINE__)
#endif


#define	 RMASK(n)		(~(0xFFFFFFFF<<(n)))				// mask to select n bits from the right




#ifndef NAN
	static const double NAN = 0.0/0.0;
#endif

// typedefs match definitions in standard_types.h:
typedef ucs1char* u8ptr;
typedef ucs2char* u16ptr;
typedef ucs4char* u32ptr;
typedef const ucs1char* cu8ptr;
typedef const ucs2char* cu16ptr;
typedef const ucs4char* cu32ptr;


static ucs4char quotes[][2] =
{	{'<','>'},
	{0x00ab/*«*/,0x00bb/*»*/},
	{0x00bb/*»*/,0x00ab/*«*/},
	{0x2018/*‘*/,0x2019/*’*/},
	{0x201a/*‚*/,0x2018/*‘*/},
	{0x201c/*“*/,0x201d/*”*/},
	{0x201e/*„*/,0x201c/*“*/}
};


// abc[] for String(char)
// spc[] for String(len,char)
#define B1(n)		 		 n,    n+ 1,    n+ 2,    n+ 3
#define B2(n)		 	  B1(n),B1(n+ 4),B1(n+ 8),B1(n+12)
#define B3(n)		 	  B2(n),B2(n+16),B2(n+32),B2(n+48)
static ucs1char abc[] = { B3(0),B3(  64),B3( 128),B3( 192) };
static ucs1char spc[] =
"          ""          ""          ""          ""          ";



#if 0
// ========================================================================================
		STRING POOL
		A string pool for new|delete seems not worth the space and effort here:
		In the whole vipsi project there is not a single occurance of new|delete String.
		This is probably because Strings are basically viewed as extended pointers.
#endif



/* ********************************************************************************
					UTF-8 UTILITIES
******************************************************************************** */


/* ===================================================================================
			UTILITIES
=================================================================================== */

// char -> digit value: ['0'..'9'] ---> [0..9]; non-digits ---> [10..255]
inline	uint8	_digit_val(char c)			{ return uint8 (c-'0'); }
inline	uint8	_digit_val(ucs1char c)	 	{ return uint8 (c-'0'); }
inline	uint16	_digit_val(ucs2char c)	 	{ return uint16(c-'0'); }
inline	uint32	_digit_val(ucs4char c)	 	{ return uint32(c-'0'); }

// char -> digit value: ['0'..'9']['A'..'Z']['a'..'z'] ---> [0...35]; non-digits ---> [36..233]
inline	uint8	_digit_value(char c)		{ return c<='9' ? uint8 (c-'0') : uint8 ((c|0x20)-'a')+10; }
inline	uint8	_digit_value(ucs1char c)	{ return c<='9' ? uint8 (c-'0') : uint8 ((c|0x20)-'a')+10; }
inline	uint16	_digit_value(ucs2char c)	{ return c<='9' ? uint16(c-'0') : uint16((c|0x20)-'a')+10; }
inline	uint32	_digit_value(ucs4char c)	{ return c<='9' ? uint32(c-'0') : uint32((c|0x20)-'a')+10; }

// UCSx char -> 1st char for UTF8
static inline char _utf8_fup		( ucs1char c )	{ return char(0x80) | char(c&0x3f); }
static inline char _utf8_starter_c2	( ucs1char c )	{ return char(0xc0) | char(c>>6);   }
static inline char _utf8_starter_c2	( ucs2char c )	{ return char(0xc0) | char(c>>6);   }
static inline char _utf8_starter_c2	( ucs4char c )	{ return char(0xc0) | char(c>>6);   }
static inline char _utf8_starter_c3	( ucs2char c )	{ return char(0xe0) | char(c>>12);  }


template<typename ZT, typename QT>
static inline void copy_and_convert(ZT* z, const QT* q, int32 n)
{
	assert(n>=0);
	while(n--) { *z++ = static_cast<ZT>(*q++); }
}

inline void ucs4_from_ucs1 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u32ptr(z), cu8ptr(q),n);
}

inline void ucs4_from_ucs2 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u32ptr(z),cu16ptr(q),n);
}

inline void ucs4_from_ucs4 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u32ptr(z),cu32ptr(q),n);
}

inline void ucs2_from_ucs1 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u16ptr(z),cu8ptr(q),n);
}

inline void ucs2_from_ucs2 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u16ptr(z),cu16ptr(q),n);
}

inline void ucs2_from_ucs4 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u16ptr(z),cu32ptr(q),n);
}

inline void ucs1_from_ucs1 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u8ptr(z),cu8ptr(q),n);
}

inline void ucs1_from_ucs2 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u8ptr(z),cu16ptr(q),n);
}

inline void ucs1_from_ucs4 ( void* z, const void* q, int32 n )
{
	copy_and_convert(u8ptr(z),cu32ptr(q),n);
}

#define ucs_from_ucs UCSCopy
//static void ucs_from_ucs ( int zcsz, void* z, int qcsz, const void* q, int32 n )

void UCSCopy (CharSize zcsz, void* z, CharSize qcsz, const void* q, int32 n )
{
	switch(zcsz)
	{
	case csz1:
		switch(qcsz)
		{
			case csz1: ucs1_from_ucs1(z,q,n); return;
			case csz2: ucs1_from_ucs2(z,q,n); return;
			case csz4: ucs1_from_ucs4(z,q,n); return;
		}
	FALLTHROUGH
	case csz2:
		switch(qcsz)
		{
			case csz1: ucs2_from_ucs1(z,q,n); return;
			case csz2: ucs2_from_ucs2(z,q,n); return;
			case csz4: ucs2_from_ucs4(z,q,n); return;
		}
	FALLTHROUGH
	case csz4:
		switch(qcsz)
		{
			case csz1: ucs4_from_ucs1(z,q,n); return;
			case csz2: ucs4_from_ucs2(z,q,n); return;
			case csz4: ucs4_from_ucs4(z,q,n); return;
		}
	}
	IERR();
}

static void ucs_copy ( CharSize csz, void* z, const void* q, int32 n )
{
	switch(csz)
	{
	case csz1: ucs1_from_ucs1(z,q,n); return;
	case csz2: ucs2_from_ucs2(z,q,n); return;
	case csz4: ucs4_from_ucs4(z,q,n); return;
	}
	IERR();
}

template<typename T>
static inline void ucs_clear ( T* z, int32 n, ucs4char filler )
{
	assert(n>=0);
	while(n--) { *z++ = T(filler); }
}

static void ucs_clear ( int csz, void* z, int32 n, ucs4char filler )
{
	switch(csz)
	{
	case csz1: ucs_clear(u8ptr(z),n,filler); return;
	case csz2: ucs_clear(u16ptr(z),n,filler); return;
	case csz4: ucs_clear(u32ptr(z),n,filler); return;
	}
}

static CharSize utf8_req_charsize ( cptr a, cptr e )
{
	// calculate required UCS variant for lossless conversion --------------------
	// returns 1, 2 or 4
	// assumes any 1-byte character and any 2-byte start code <0xc4 is ok for UCS1
	// assumes any 1-, 2- and 3-byte character is ok for UCS2
	// does not check for bogus characters (ill.overlong, truncated)
	// works for full 32 bit characters

	while ( a<e && uchar(*a)<0xC4 ) { a++; } if (a==e) return csz1;
	while ( a<e && uchar(*a)<0xF0 ) { a++; } if (a==e) return csz2; else return csz4;
}

static inline CharSize req_charsize ( ucs4char n )
{
	return n>>16 ? csz4 : n>>8 ? csz2 : csz1;
}

__attribute__((__unused__))
static inline CharSize req_charsize ( ucs2char n )
{
	return n>>8 ? csz2 : csz1;
}



/* ----	Convert UTF-8 to UCS-1, UCS-2, or UCS-4 ---------------------------------------------
		golden rule: every non_fup makes a character!
		overlong encoded characters are not trapped but handled like shortest encoded characters.
		if flag 'latin_1' is set, then handle broken utf-8 as iso-latin-1
  note:	the range [$80…[$C0 is never accepted as latin-1 but always skipped as unexpected fup.
		this is required to obey the golden rule "every non_fup makes a character".
		else the returned string would be longer than pre-computed with utf8_charcount().
*/

static int32 utf8_charcount ( cptr a, cptr e )
{
	// count characters in UTF-8 string

	int32 n = int32(e-a);
	while (a<e) { n -= utf8::is_fup(*a++); }
	return n;
}

static ucs1char* ucs1_from_utf8 ( ucs1char* z, cstr a, cstr e, bool latin_1 = no )
{
	// convert utf-8 to ucs-1

	char c1,c2;

	while (a<e)
	{
		if (utf8::is_7bit(c1=*a++))		{ *z++ = ucs1char(c1); continue; }
		if (utf8::is_fup(c1))			{ continue; }
		if (a>=e || utf8::no_fup(c2=*a) || uchar(c1)>=0xc4 )	{ *z++ = latin_1 ? ucs1char(c1) : '?'; continue; }
		a++; *z++ = ucs1char((c1<<6) + (c2&0x3f));
	}
	return z;
}

static ucs2char* ucs2_from_utf8 ( ucs2char* z, cstr a, cstr e, bool latin_1 = no )
{
	// convert utf-8 to ucs-2

	char c1,c2,c3;

	while(a<e)
	{
		if (utf8::is_7bit(c1=*a++))			{ *z++ = ucs1char(c1); continue; }	// 7-bit ascii
		if (utf8::is_fup(c1))				{ continue; }			// unexpected fup
		if (a>=e || utf8::no_fup(c2=*a))	{ goto x; }
		a++; c2 &= 0x3f; if(uchar(c1)<0xe0)	{ *z++ = ucs2char(((c1&0x1f)<<6) + uchar(c2)); continue; }
		if (a>=e || utf8::no_fup(c3=*a))	{ goto x; }
		a++; c3 &= 0x3f; if(uchar(c1)<0xf0)	{ *z++ = ucs2char(((c1)<<12) + ((c2)<<6) + uchar(c3)); continue; }
x:		*z++ = latin_1 ? ucs2char(c1) : '?';
	}
	return z;
}

/* ----	convert utf-8 to ucs-4 --------------------------------------
		2005-06-09: added support for full 32 bit
*/
static ucs4char* ucs4_from_utf8 ( ucs4char*z, cstr a, cstr e, bool latin_1 = no )
{
	while ( a<e )
	{
		ucs4char c0 = uchar(*a++);
		if( utf8::is_7bit(c0) ) { *z++ = c0; continue; }		// 7-bit ascii char
		if( utf8::is_fup(c0) ) continue;						// unexpected fups

	// c0 is a starter for a 2 … 6 bytes character:
	// 0xC0.. => 2 bytes:	110xxxxx 10xxxxxx
	// 0xE0.. => 3 bytes:	1110xxxx 10xxxxxx 10xxxxxx
	// 0xF0.. => 4 bytes:	11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0xF8.. => 5 bytes:	111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
	// 0xFC.. => 6 bytes:	111111xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
		int  i = 0;								// UTF-8 byte count
		char c = c0 & ~0x02;					// force stop at i=6
		while( char(c<<(++i)) < 0 )				// loop over required fups
		{
			if (a>=e) goto x;
			char c1 = *a; if( utf8::no_fup(c1) ) goto x;
			a++; c0 = (c0<<6) + (c1&0x3F);
		}

	// i  = 2 … 6  = total number of bytes
	// c0 = UCS4 char code; still including the high bits from the starter char
		if(i<6) c0 &= RMASK(2+i*5);				// remove high bits
		*z++ = c0; continue;					// store & continue

	// truncated character => store replacement char
	x:	*z++ = latin_1 ? c0 : '?'; continue;
	}

	return z;
}



/* ----	Convert UCS-1, UCS-2, or UCS-4 to UTF-8 ----------------------------
*/

static int32 utf8_bytecount ( const ucs1char* p, int32 n )
{
	// calculate byte-size for utf-8 string

	const ucs1char* e = p + n;
	while(p<e) { n += *p++>>7; }
	return n;
}

static int32 utf8_bytecount ( const ucs2char* p, int32 n )
{
	// calculate byte-size for utf-8 string

	const ucs2char* e = p + n;
	while(p<e) { ucs2char c = *p++; if(c>>7) { n += c>>11 ? 2 : 1; } }
	return n;
}

static int32 utf8_bytecount ( const ucs4char* p, int32 n )
{
	// calculate byte-size for utf-8 string
	// supports full 32 bit

	const ucs4char* e = p + n;
	while (p<e)
	{
		ucs4char c = *p++;
		if(c>>7)		// 2…6 bytes: 1…5 fups
		{
			int i = 10; for( c>>=11; c; c>>=5 ) { i+=5; }
			n += i/6;	// i = num fups =	10/6=1	15/6=2	20/6=3	25/6=4	30/6=5	35/6=5
		}
	}
	return n;
}

static ptr utf8_from_ucs1 ( ptr z, const ucs1char* q, int32 n )
{
	// create utf-8 string from UCS-1 string

	const ucs1char* e = q + n;
	while (q<e)
	{
		ucs1char c = *q++;
		if (int8(c)>=0) *z++ = char(c);
		else { *z++ = _utf8_starter_c2(c); *z++ = _utf8_fup(c); }
	}
	return z;
}

static ptr utf8_from_ucs2 ( ptr z, const ucs2char* q, int32 n )
{
	// create utf-8 string from UCS-2 string

	const ucs2char* e = q+n;
	while (q<e)
	{
		ucs2char c = *q++;
		if( !(c>>7)  )	{ *z++ = char(c); continue; }
		if( !(c>>11) )	{ *z++ = _utf8_starter_c2(c); *z++ = _utf8_fup(uint8(c)); }
		else			{ *z++ = _utf8_starter_c3(c); *z++ = _utf8_fup(uint8(c>>6));  *z++ = _utf8_fup(uint8(c)); }
	}
	return z;
}

static ptr utf8_from_ucs4 ( ptr z, const ucs4char* q, int32 n )
{
	// create utf-8 string from UCS-4 string
	// supports full 32 bit

	const ucs4char* e = q + n;
	while (q<e)
	{
		ucs4char c = *q++;
		if( !(c>>7)  )	{ *z++ = char(c); continue; }								// 1 byte code
		if( !(c>>11) )	{ *z++ = _utf8_starter_c2(c); *z++ = _utf8_fup(uint8(c)); }	// 2 byte code
		else																// 3…6 byte codes
		{																	// num fups = i =	15/6=2	20/6=3	25/6=4	30/6=5	35/6=5
			int i = 15; for( uint m = c>>16; m; m>>=5 ) { i+=5; } i = i/6;	// num bits in fups = i*6
			*z++ = char((0xFF80>>i) | (c>>(i*6)));							// starter
			while(i--) *z++ = _utf8_fup(uint8(c>>(i*6)));					// fups
		}
	}
	return z;
}




/* ===================================================================================
			INTEGRITY TEST
=================================================================================== */


void String::Check ( cstr filename, uint line ) const
{
	// check String integrity
	// (as far as possible)
	// may crash on bogus String
	// will Abort on bogus String

	static int r=0; if(r)return;
	r++;
	xxlog("Check(%s)",CString());
	r--;

	if (next==nullptr||prev==nullptr||(text==nullptr&&count!=0))
	{
		if(next==nullptr) logline("(next==NULL)");
		if(prev==nullptr) logline("(prev==NULL)");
		if(text==nullptr) logline("(text==NULL)");
		abort("String::Check() failed in file %s line %u",filename,line);
	}

	if( Csz()==3 ) abort("String::Check(): invalid csz in file %s line %u",filename,line);

	if( (next!=this)!=(prev!=this) ) abort("String::Check(): (next!=this)!=(prev!=this) in file %s line %u",filename,line);

	if (next!=this||prev!=this)
	{
		if (next->prev!=this) abort("String::Check(): next->prev!=this in file %s line %u",filename,line);
		if (prev->next!=this) abort("String::Check(): prev->next!=this in file %s line %u",filename,line);
		const String *p = this, *q = next;
		do
		{
			if (q->prev!=p)  abort("String::Check(): q->prev!=p in file %s line %u",filename,line);
			p=q; q=q->next;
		}
		while (p!=this);
	}

	if (count)
	{
		if( text < Data() )
		{
			logline(" count=%u, text=0x%08lx, data=0x%08lx ",uint(count),size_t(text),size_t(Data()));
			abort("String::Check(): text<data in file %s line %u",filename,line);
		}
		char c = text[0];
		char d = text[count*Csz()-1];
		d = c;
		c = d;
	}
}



/* ===================================================================================
			PRIVATE MEMBER FUNCTIONS
=================================================================================== */


void String::_init_from_utf8 ( cptr p, cptr e, bool latin_1 )
{
	count  = utf8_charcount ( p, e );
	CharSize csz = utf8_req_charsize(p,e);
	text   = new char[csz*count];
	SetDataAndCsz(text,csz);
	next   = this;
	prev   = this;

	switch( Csz() )
	{
	case csz1: ucs1_from_utf8( Ucs1(),p,e,latin_1 ); break;
	case csz2: ucs2_from_utf8( Ucs2(),p,e,latin_1 ); break;
	case csz4: ucs4_from_utf8( Ucs4(),p,e,latin_1 ); break;
	}
}

void String::_move ( String& q )
{
	XXXCHECK(q);

	if(this!=&q)
	{
		text = q.text;
		count = q.count;
		SetDataAndCsz(q.DataAndCsz());
		if (q.next==&q)					// *****************************
		{								//
			next = prev = this;			//   fixed bug: kio 2003-09-20
		}								//
		else							// *****************************
		{
			next = q.next;
			prev = q.prev;
			next->prev = this;
			prev->next = this;
		}
	}
}


/* ----	compare strings -------------------------------------------------------------------
		return values:
		>0	this >  q
		 0	this == q
		<0	this <  q
*/
int32 String::compare ( cString& s ) const
{
	int32 d = 0;
	int32 n = min(count,s.count);
	int32 i = -1;

	switch (this->Csz())
	{
	case csz1: // this.csz == 1
		switch(s.Csz())
		{
		case csz1: while (++i<n) { d = int32(Ucs1(i)) - int32(s.Ucs1(i)); if(d) break; } break;
		case csz2: while (++i<n) { d = int32(Ucs1(i)) - int32(s.Ucs2(i)); if(d) break; } break;
		case csz4: while (++i<n) { d = int32(Ucs1(i)) - int32(s.Ucs4(i)); if(d) break; } break;
		}
		break;
	case csz2: // this.csz == 2
		switch(s.Csz())
		{
		case csz1: while (++i<n) { d = int32(Ucs2(i)) - int32(s.Ucs1(i)); if(d) break; } break;
		case csz2: while (++i<n) { d = int32(Ucs2(i)) - int32(s.Ucs2(i)); if(d) break; } break;
		case csz4: while (++i<n) { d = int32(Ucs2(i)) - int32(s.Ucs4(i)); if(d) break; } break;
		}
		break;
	case csz4: // this.csz == 4
		switch(s.Csz())
		{
		case csz1: while (++i<n) { d = int32(Ucs4(i)) - int32(s.Ucs1(i)); if(d) break; } break;
		case csz2: while (++i<n) { d = int32(Ucs4(i)) - int32(s.Ucs2(i)); if(d) break; } break;
		case csz4: while (++i<n) { d = int32(Ucs4(i)) - int32(s.Ucs4(i)); if(d) break; } break;
		}
		break;
	}

	return d ? d : count - s.count;
}


/* ----	pattern match ---------------------------------------------------------------
*/
bool Match ( cString& filename, cString& pattern )
{
	int pi=0, pe=pattern.Len();
	int fi=0, fe=filename.Len();

	while(pi<pe&&fi<fe)
	{
	// skip matching characters up to '*'

		ucs4char p = pattern[pi++];
		ucs4char f = filename[fi++];
		if (p!='*') { if (p=='?'||p==f) continue; else return no; }

	// skip more '*' and '?'

		for(;;)
		{
			if (pi==pe) return yes;
			p = pattern[pi++];
			if (p=='*') continue;
			if (p!='?') break;
			if (fi==fe) return no;
			f = filename[fi++];
		}

	// handle "*" in pattern:

	//	p = current char ((next char after "*"))
	//	f = current char
	//	f is advanced by 1 char per loop hoping that the "*" will match the stepped-over characters
	//	this is true if the remainder of pattern matches the remainder of filename
	//	else filename and pattern do not match

		for(;;)
		{
			if( p==f )	// quick test to prevent recursive calls for obvious non-matches
			{
				if ( Match( filename.MidString(fi), pattern.MidString(pi) ) ) return yes;
			}
			if (fi>=fe) return no;
			f = filename[fi++];
		}

		IERR(); // never reached
	}

	while (pi<pe&&pattern[pi]=='*') pi++;
	return pi==pe && fi==fe;
}


/* ---- Filename Match --------------------------------------------------------------
		do the filename globbing thingy
		similar to the system call
		note: arguments pattern and filename are swapped
		note: returns true if they match, false if they don't.
			  this is reverse to fnmatch() !!

		flags =	fnm_escape		=>	unescape strings; else treat "\" as normal char
				fnm_pathname	=>	match "/" with "/" only and not with wildcards
				fnm_period		=>	match leading periods "." with "." only
				fnm_casefold	=>	compare case insensitive
*/
bool FNMatch ( cString& filename, cString& pattern, MatchType flags )
{
// handle escape ?
	if (flags&fnm_escape)
	{
		return FNMatch( filename.FromEscaped(), pattern.FromEscaped(), MatchType(flags-fnm_escape) );
	}

// case insensitive matching?
	if (flags&fnm_casefold)
	{
		return FNMatch( filename.ToLower(), pattern.ToLower(), MatchType(flags-fnm_casefold) );
	}

// match path components only?
	if (flags&fnm_pathname)
	{
		String p_rem = pattern+dirsep;
		String f_rem = filename+dirsep;
		String p,f;

		while ( p_rem.Len()>1 && f_rem.Len()>1 )
		{
			int pi = p_rem.Find(dirsep);
			int fi = f_rem.Find(dirsep);

			p = p_rem.LeftString(pi);  p_rem = p_rem.MidString(pi+1);
			f = f_rem.LeftString(fi);  f_rem = f_rem.MidString(fi+1);

			if (flags&fnm_period)
			{
				bool f1 = f.Len()>=1&&f[0]=='.';
				bool p1 = p.Len()>=1&&p[0]=='.';
				if (f1!=p1) return no;		// pattern or filename starts with an unmatched "."
				if (f1) { f = f.MidString(1); p = p.MidString(1); }
			}
			if (!Match(f,p)) return no;
		}
		return p_rem.Len()==1 && f_rem.Len()==1;
	}

// basic match
	if (flags&fnm_period)
	{
		bool f1 = filename.Len()>=1&&filename[0]=='.';
		bool p1 = pattern.Len() >=1&&pattern[0] =='.';
		if (f1!=p1) return no;		// pattern or filename starts with an unmatched "."
		if (f1) return Match(filename.MidString(1),pattern.MidString(1));
	}
	return Match(filename,pattern);
}


/* ----	resize string ---------------------------------------------------------------
		grow or shrink string to new len
		does not clear padded character positions
*/
void String::_resize ( int32 newlen )
{
	XXXCHECK(*this);

	CharSize csz = Csz();
	ptr newdata = new char[csz*newlen];
	ucs_copy ( csz, newdata, text, min(count,newlen) );

	_kill();

	count  = newlen;
	text   = newdata;
	SetDataAndCsz(newdata,csz);
	next   = this;
	prev   = this;
}



/* ===================================================================================
			PUBLIC MEMBER FUNCTIONS
=================================================================================== */


/* ----	Destructor -------------------------------------------------------------------
*/
String::~String ( )
{
	xlogIn("String::~String()");
	XXXCHECK(*this);
	_kill();
}


/* ----	default creator --------------------------------------------------------------
		creates an empty string
		sharing StrMem empty_strmem
*/
String::String ( )
:
	text(nullptr),
	count(0),
	data_and_csz(calc_data_and_csz(nullptr,csz1)),
	next(this),
	prev(this)
{
	xlogIn("String::String()");
}


/* ----	create string from single character ------------------------------------------
		charsize is choosen to fit
		returned String may be 'not writable'
*/
String::String ( ucs4char c )
{
	xlogIn("String::String(char)");

	count = 1;
	next  = this;
	prev  = this;

	if (c == ucs1char(c))
	{
		ucs1_text = abc+c;
		SetDataAndCsz(nullptr,csz1);
	}
	else if (c == ucs2char(c))
	{
		ucs2_text = new ucs2char[1];
		SetDataAndCsz(text,csz2);
		ucs2_text[0] = ucs2char(c);
	}
	else
	{
		ucs4_text = new ucs4char[1];
		SetDataAndCsz(text,csz4);
		ucs4_text[0] = c;
	}
}


/* ----	create string from UCS-1 buffer data ----------------------------------------
		the buffer data is copied.
		to avoid this, use strings for your buffers!
*/
String::String ( const ucs1char* p, int32 n )
{
	xlogIn("String::String(ucs1ptr,len)");

	if (n>0)
	{
		_init(csz1,n);
		memcpy(text, p, size_t(n)*csz1);
	}
	else _init();
}


/* ----	create string from UCS-2 buffer data ----------------------------------------
		the buffer data is copied.
		to avoid this, use strings for your buffers!
*/
String::String ( const ucs2char* p, int32 n )
{
	xlogIn("String::String(ucs2ptr,len)");

	if (n>0)
	{
		_init(csz2,n);
		memcpy(text, p, size_t(n)*csz2);
	}
	else _init();
}


/* ----	create string from UCS-4 buffer data ----------------------------------------
		the buffer data is copied.
		to avoid this, use strings for your buffers!
*/
String::String ( const ucs4char* p, int32 n )
{
	xlogIn("String::String(ucs4ptr,len)");

	if (n>0)
	{
		_init(csz4,n);
		memcpy(text, p, size_t(n)*csz4);
	}
	else _init();
}


/* ----	create string from UCS-1, -2 or -4 buffer data ----------------------------------------
		the buffer data is copied.
		to avoid this, use strings for your buffers!
*/
String::String ( cptr p, int32 n, CharSize cs )
{
	xlogIn("String::String(ptr,len,csz)");

	if (n>0)
	{
		_init(cs,n);
		memcpy(text, p, size_t(n)*cs);
	}
	else _init();
}


/* ----	create string from UTF-8 buffer data ----------------------------------------
		the UTF-8 encoded text is converted
		to UCS-1, -2 or -4 as required.
*/
String::String ( cstr p, int32 n )
{
	xlogIn("String::String(utf8ptr,len)");

	if (n)
	{
		assert(p);
		_init_from_utf8(p,p+n);
	}
	else
	{
		_init();
	}
}


/* ----	create uncleared string ---------------------------------------------------
*/
String::String ( int32 n, CharSize csz )
{
	xlogIn("String::String(len,csz)");

	if(n>0)	_init(csz,n);
	else	_init();
}


/* ----	create cleared string -----------------------------------------------------
		created string may be 'not writable'
*/
String::String ( int32 n, ucs4char c )
{
	xlogIn("String::String(len,char)");

	next = prev = this;
	count  = n;

	if(n<=0)
	{
		count  = 0;
		Ucs1() = nullptr;
		SetDataAndCsz(nullptr,csz1);
	}
	else if (c==' ' && uint32(n)<NELEM(spc) )
	{
		ucs1_text = spc;
		SetDataAndCsz(nullptr,csz1);
	}
	else if (c == ucs1char(c))
	{
		ucs1_text = new ucs1char[n];
		SetDataAndCsz(text,csz1);
		ucs_clear(ucs1_text,n,c);
	}
	else if (c == ucs2char(c))
	{
		ucs2_text = new ucs2char[n];
		SetDataAndCsz(text,csz2);
		ucs_clear(ucs2_text,n,c);
	}
	else
	{
		ucs4_text = new ucs4char[n];
		SetDataAndCsz(text,csz4);
		ucs_clear(ucs4_text,n,c);
	}
}


/* ----	create string from c-style text literal ------------------------------------
*/
String::String ( cstr s )
{
	xlogIn("String::String(\"%s\")",s);

	if (s&&*s)	_init_from_utf8( s,strchr(s,0) );
	else		_init();
}


/* ----	create string from c-style text literal ------------------------------------
		special variant which handles broken utf-8 $C0++ characters as latin-1
		note that this does not break the golden rule "each non-fup makes a char"
		because characters $80..$BF are not handled this way!
		which is not such a problem because all letters in latin-1 are $C0..$FF
*/
String::String ( cstr s, CharEncoding ce )
{
	xlogIn("String::String(utf8str,encoding)");

	assert(ce==ISO_LATIN_1);
	if (s&&*s)	_init_from_utf8( s,strchr(s,0), yes );
	else		_init();
}


/* ----	copy creator ---------------------------------------------------------------
*/
String::String ( cString& q )
{
	xlogIn("String::String(String)");

	XXXCHECK(q);
	_init(q);
}


/* ---- create substring -----------------------------------------------------------
		the original string and the substring share a single StrMem
		=> both become not_writable!
		this can be purposely exploited for left-side substrings in assignments
*/
String::String ( cString& q, int32 a, int32 e )
{
	xlogIn("String::String(String,a,e)");

	XXXCHECK(q);
	if (a<0)	   a = 0;
	if (e>q.Len()) e = q.Len();
	if (a>=e)
	{
		_init();
	}
	else
	{
		text   = q.text + a*q.Csz();
		count  = e - a;
		SetDataAndCsz(q.DataAndCsz());
		next   = q.next;
		prev   = const_cast<String*>(&q);
		next->prev = prev->next = this;
	}
}


/* ---- create substring -----------------------------------------------------------
		the original string and the substring share a single StrMem
		=> both become not_writable!
		special re-interpret version for vipsi, used for string literals in proc bodies
*/
String::String ( cString& q, cuptr txt, int32 cnt, CharSize cs )
{
	xlogIn("String::String(String,ptr,len,csz)");

	XXXCHECK(q);
	if ( cnt )
	{
		assert( ptr(txt)        >= q.text                 );
		assert( ptr(txt)+cnt*cs <= q.text+q.count*q.Csz() );

	#if _ALIGNMENT_REQUIRED
		_init(cs,cnt);
		memcpy(text,txt,cs*cnt);
	#else
		text   = ptr(txt);
		count  = cnt;
		SetDataAndCsz(q.Data(),cs);
		next   = q.next;
		prev   = const_cast<String*>(&q);
		next->prev = prev->next = this;
	#endif
	}
	else
	{
		_init();
	}
}


CharSize String::ReqCsz() const
{
	XXXCHECK(*this);
	switch(Csz())
	{
	case csz1:
		break;

	case csz2:
		{	const ucs2char* a = Ucs2();
			const ucs2char* e = a+Len();
			while (a<e) { if (*--e>>8) return csz2; }
		}	break;

	case csz4:
		{	const ucs4char* a = Ucs4();
			const ucs4char* e = a+Len();
			while (a<e)
			{
				if (*--e>>8)
				{
					e++; while (a<e) if (*--e>>16) return csz4;
					return csz2;
				}
			}
		}	break;
	}

	return csz1;
}


void String::ResizeCsz ( CharSize csz )
{
	XXXCHECK(*this);
	if (Csz()!=csz)
	{
		ptr newtext = new char[csz*Len()];
		ucs_from_ucs ( csz, newtext, Csz(), text, Len() );
		_kill();
		text	= newtext;
	//	count	= count;
		SetDataAndCsz(newtext,csz);
		next   = this;
		prev   = this;
		XXXCHECK(*this);
	}
}


/* ----	quick clear string ------------------------------------------------------------
*/
void String::Clear ( )
{
	XXXCHECK(*this);
	_kill();
	_init();
	XXXCHECK(*this);
}


/* ----	quick swap strings ------------------------------------------------------------
*/
void String::Swap ( String& that )
{
	XXXCHECK(*this);
	XXXCHECK(that);
	char z[sizeof(String)];
	String& temp = *(String*)z;

	temp._move(*this);
	this->_move(that);
	that._move(temp);
	XXXCHECK(*this);
	XXXCHECK(that);
}


void String::Copy ( cString& q )
{
	// quick copy string

	XXXCHECK(*this);
	XXXCHECK(q);

	if (this != &q)
	{
		_kill();
		_init(q);
		XXXCHECK(*this);
		XXXCHECK(q);
	}
}


void String::Truncate ( int32 newlen )
{
	// truncate string

	XXXCHECK(*this);

	if (newlen <= 0)
	{
		_kill();
		_init();
		XXXCHECK(*this);
	}
	else if (newlen<count)
	{
		count = newlen;
	}
}


void String::Crop ( int32 left, int32 right )
{
	// crop string from left and right side

	XXXCHECK(*this);

	if (count)
	{
		if (count > left + right)
		{
			count -= right + right;
			text += Csz() * left;
		}
		else
		{
			_kill();
			_init();
		}
		XXXCHECK(*this);
	}
}


void String::Resize ( int32 newlen, ucs4char padding )
{
	// resize string
	// shrink or grow string to new len
	// if string grows, use the supplied character for padding

	if (newlen <= count)
	{
		Truncate(newlen);
	}
	else
	{
		XXXCHECK(*this);
		CharSize ocsz = Csz();
		CharSize ncsz = max( ocsz, req_charsize(padding) );
		ptr newdata = new char[ncsz*newlen];
		ucs_from_ucs ( ncsz, newdata, ocsz, text, count );
		ucs_clear    ( ncsz, newdata+count*ncsz, newlen-count, padding );
		_kill();
		count = newlen;
		text  = newdata;
		SetDataAndCsz(newdata,ncsz);
		next  = this;
		prev  = this;
		XXXCHECK(*this);
	}
}


ucs4char String::operator[] ( int32 i ) const
{
	// access character at given position
	// the index must be in range

	XXXCHECK(*this);
	assert(i < count);

	switch (Csz())
	{
	case csz1: return ucs1_text[i];
	case csz2: return ucs2_text[i];
	case csz4: return ucs4_text[i];
	}
	IERR();
}


ucs4char String::LastChar ( ) const throws
{
	// access last character in string
	// the string must not be empty

	XXXCHECK(*this);
	assert(count > 0);

	int32 i = count -1;
	switch (Csz())
	{
	case csz1: return ucs1_text[i];
	case csz2: return ucs2_text[i];
	case csz4: return ucs4_text[i];
	}
	IERR();
}


/* ----	make writable ------------------------------------------------------------
*/
void String::MakeWritable ( )
{
	XXXCHECK(*this);
	if (NotWritable())
	{
		CharSize csz=Csz();
		ptr newdata = new char[count*csz];
		memcpy ( newdata, text, csz*count );
		_kill();
		text = newdata;
		SetDataAndCsz(newdata,csz);
		next = this;
		prev = this;
		XXXCHECK(*this);
	}
}


/* ----	assignment ----------------------------------------------------------------------
*/
String& String::operator= ( cString& q )
{
	xlogIn("String::Operator=(cString&)");
	XXXCHECK(*this);
	XXXCHECK(q);
	if(this!=&q)
	{
		_kill();
		_init(q);
		XXXCHECK(*this);
		XXXCHECK(q);
	}
	return *this;
}


/* ----	concatenate ----------------------------------------------------------------------
*/
String String::operator+ ( cString& q ) const
{
	XXXCHECK(*this);
	XXXCHECK(q);
	CharSize qcsz = q.Csz();
	CharSize  csz =   Csz();
	CharSize zcsz = max(csz,qcsz);
	String z(count+q.count,zcsz);

	ucs_from_ucs ( zcsz, z.text,             csz,   text,   count );
	ucs_from_ucs ( zcsz, z.text+count*zcsz, qcsz, q.text, q.count );
	XXXCHECK(z);
	return z;
}


String String::operator* ( int32 n ) const
{
	// repetition
	// optimized for String.Len()==1
	// no additional optimization for String==" "
	// because SpaceString() will be used for that most times

	XXXCHECK(*this);

	if (count<=1) { return count<=0 ? emptyString : String(n,operator[](0)); }
	if (    n<=1) { return     n<=0 ? emptyString : *this; }

	assert( n <= 0x7FFFFFFF / count);

	CharSize csz = Csz();
	String s(n*count, csz);
	cptr q = text;
	ptr  z = s.text;
	int32 l = count;
	do { ucs_copy(csz,z,q,l); z += l*csz; } while (--n);

	XXXCHECK(s);
	return s;
}


str String::CString ( ) const
{
	// create c-string from string
	// the returned c-string is allocated
	// in the temporary c-string pool

	XXXCHECK(*this);

	switch(Csz())
	{
	case csz1:
	{
		int32 n = utf8_bytecount( Ucs1(), Len() );
		str s = tempstr(n);
		IFDEBUG( ptr p = ) utf8_from_ucs1( s, Ucs1(), Len() );
		assert(p==s+n);
		return s;
	}
	case csz2:
	{
		int32 n = utf8_bytecount( Ucs2(), Len() );
		str s = tempstr(n);
		IFDEBUG( ptr p = ) utf8_from_ucs2( s, Ucs2(), Len() );
		assert(p==s+n);
		return s;
	}
	case csz4:
	{
		int32 n = utf8_bytecount( Ucs4(), Len() );
		str s = tempstr(n);
		IFDEBUG( ptr p = ) utf8_from_ucs4( s, Ucs4(), Len() );
		assert(p==s+n);
		return s;
	}
	}
	IERR();
}


double String::NumVal ( int32* idx_io ) const
{
	cuptr  q, qe, a;
	uchar  c, bu[41];
	int32  idx = idx_io ? *idx_io : 0;
	int    n   = min(int(NELEM(bu))-1,count-idx);
	double f   = 0.0;
	bool   neg;

	XXXCHECK(*this);

	switch(Csz())
	{
	case csz4:
	{
		int i; const ucs4char* p4 = ucs4_text+idx;
		for (i=0;i<n;i++) { if(p4[i]>>8) break; }
		q=bu; n=i; ucs1_from_ucs4(bu,p4,n); break;
	}
	case csz2:
	{
		int i; const ucs2char* p2 = ucs2_text+idx;
		for (i=0;i<n;i++) { if(p2[i]>>8) break; }
		q=bu; n=i; ucs1_from_ucs2(bu,p2,n); break;
	}
	case csz1:
	{
		const ucs1char* p1 = ucs1_text+idx;
		q=bu; ucs1_from_ucs1(bu,p1,n); break;
	}
	}
	XXXCHECK(*this);

	qe  = q + n;
	*uptr(qe) = 0;

	c   = ' ';    while( q<qe && c<=' ' )     { c = *q++; }
	neg = c=='-'; if( q<qe && (neg||c=='+') ) { c = *q++; }

	if (is_dec_digit(c))		// 123.456e78
	{
		ptr endptr;
		if( c=='0' && q<qe && to_upper(*q)=='X' )	// strtod() stolpert über 0x…
		{
			endptr = (ptr)q;
			f = 0;
		}
		else
		{
			f = strtod((cptr)q-1,&endptr);
			if (errno && (errno==HUGE_VAL||errno==ERANGE)) /*ForceError(numbertoobig);*/ errno=ok;
		}
		if (idx_io) *idx_io += endptr-(ptr)bu;
		return neg ? -f : f;
	}

	if (c=='$')			// $abc.def
	{
		a = q;
		while ( q<qe && (n=_digit_value(*q))<16 ) { q++; f = ldexp(f,4) + n; }
		if (q==a) goto x;

		if (q<qe&&*q=='.')
		{
			a = ++q;
			while ( q<qe && (n=_digit_value(*q))<16 ) { q++; f = ldexp(f,4) + n; }
			if(q>a) f = ldexp(f,4*(a-q));
		}
		if (idx_io) *idx_io += q-bu;
		return neg ? -f : f;
	}

	if (c=='%')			// %101010.101010
	{
		a = q;
		while ( q<qe && (n=_digit_val(*q))<2 ) { q++; f = ldexp(f,1) + n; }
		if (q==a) goto x;

		if (q<qe&&*q=='.')
		{
			a = ++q;
			while ( q<qe && (n=_digit_val(*q))<2 ) { q++; f = ldexp(f,1) + n; }
			if(q>a) f = ldexp(f,1*(a-q));
		}
		if (idx_io) *idx_io += q-bu;
		return neg ? -f : f;
	}

	if (c=='\'') 			// 'abcd'
	{
		a = q;
		bool esc=no;
		for(;;)
		{	if(q>=qe) goto x;				// not terminated
			c = *q;
			if(c=='\'') break;				// terminator
			if(c=='\\') { esc=yes; q++; }	// skip next byte. if multiple chars follow (e.g. octal digits) this will do fine too
			q++;
		}
		if(esc)
		{
			String z(a,q-a);	// digits without '
			z = z.FromQuoted();
			a = z.UCS1Text();
			cuptr e = a+z.Len();			// 2010-05-20 korr. kio
			if(e==a||e-a>6) goto x;
			while(a<e) { f = ldexp(f,8) + *a++; }
		}
		else
		{
			if(q==a||q-a>6) goto x;
			while(a<q) { f = ldexp(f,8) + *a++; }
		}
		if (idx_io) *idx_io += q+1-bu;
		return neg ? -f : f;
	}

x:	errno = notanumber;
	return NAN;
}


String String::StrVal ( int32* idx_io ) const
{
	XXXCHECK(*this);
	int32 idx = idx_io ? *idx_io : 0;	// loop index
	int32 end = count;					// source end index

	while( idx<end && operator[](idx)<=' ' ) { idx++; }
	if( end-idx<2 ) { errno = notastring; return emptyString; }

	ucs4char leftquote,rightquote; leftquote=rightquote=operator[](idx);
	if( leftquote!='"' && leftquote!='\'' )
	{
		uint i;
		for( i=0; i<NELEM(quotes); i++ ) { if (quotes[i][0] == leftquote) break; }
		if (i<NELEM(quotes)) rightquote = quotes[i][1];
		else { errno = notastring; return emptyString; }
	}

	int32 anf = ++idx;					// string start index
	bool esc = no;						// escaped characters present flag

	while (idx<end)
	{
		ucs4char c = operator[](idx++);

		if (c==rightquote)
		{
			if (idx_io) *idx_io = idx;
			String s = SubString(anf,idx-1);
			if (esc) s = s.FromEscaped();
			CharSize csz  = s.Csz();
			CharSize rcsz = s.ReqCsz();
			if (rcsz==csz) return s;
			String z(s.Len(),rcsz);
			ucs_from_ucs(rcsz,z.Text(),csz,s.Text(),s.Len());
			return z;
		}

		if (c!='\\') continue;
		if (idx==end) break;
		c = operator[](idx++);
		if (idx==end) break;
		esc = yes;

// --- handling of escaped characters must match String::FromEscaped():
	// \o..  =>  o = 0..7
	// \xH.. =>  H = 0..F
	// \cX   =>  X = <any_char>
	// \<return> => discard return ((10 or 13)) and following white space  ((control codes))
	// \"  or  \\  or well known abreviation  or s.th. escaped for unknown reason

		if (c=='c') { idx++; continue; }		// \cX  and  X = <any_char>

		if (c==10||c==13)						// \<return>
		{
			while (idx<end && operator[](idx)<=' ') idx++;
			continue;
		}

		// others: just skip 1 char: already done.
	}

	errno = nofinalquotes;
	return emptyString;
}


/* ----	find character in string --------------------------------------------------------------------
		returns index of character found
		returns -1 if not found
*/
int32 String::Find ( ucs4char c, int32 idx ) const
{
	XXXCHECK(*this);
	if (idx<0) idx=0;

	switch(Csz())
	{
	case csz1:
		if (c==ucs1char(c))
		{
			for (const ucs1char* p1 = ucs1_text; idx<count; idx++)
			{ if (p1[idx]==ucs1char(c)) return idx; }
		}
		break;
	case csz2:
		if (c==ucs2char(c))
		{
			for (const ucs2char* p2 = ucs2_text; idx<count; idx++)
			{ if (p2[idx]==ucs2char(c)) return idx; }
		}
		break;
	case csz4:
		{
			for (const ucs4char* p4 = ucs4_text; idx<count; idx++)
			{ if (p4[idx]==ucs4char(c)) return idx; }
		}
		break;
	}
	return -1;  	// not found
}

int32 String::RFind ( ucs4char c, int32 idx ) const
{
	XXXCHECK(*this);
	if (idx >= count) idx = int32(count-1);

	switch (Csz())
	{
	case csz1:
		if (c==ucs1char(c))
		{
			for (const ucs1char* p1 = ucs1_text; idx>=0; idx--)
			{ if (p1[idx]==c) return idx; }
		}
		break;
	case csz2:
		if (c==ucs2char(c))
		{
			for (const ucs2char* p2 = ucs2_text; idx>=0; idx--)
			{ if (p2[idx]==c) return idx; }
		}
		break;
	case csz4:
		{
			for (const ucs4char* p4 = ucs4_text; idx>=0; idx--)
			{ if (p4[idx]==c) return idx; }
		}
		break;
	}
	return -1;  	// not found
}


/* ----	find substring in string --------------------------------------------------------------------
		returns index of substring found
		returns -1 if not found
*/
int32 String::Find ( cString& s, int32 idx ) const
{
	XXXCHECK(*this);
	XXXCHECK(s);
	int32 qn = count;
	int32 sn = s.count;
	int32 n  = qn - sn;

	if (idx<0) idx=0;
	if (idx>n) return -1;
	if (sn==0) return idx;

	ucs4char s0 = s[0];

	while ( idx<=n )
	{
		idx = Find(s0,idx); if(idx==-1) break;
		if (s==MidString(idx,sn)) return idx;
		idx++;
	}
	return -1;
}

int32 String::RFind ( cString& s, int32 idx ) const
{
	XXXCHECK(*this);
	XXXCHECK(s);
	int32 qn = count;
	int32 sn = s.count;
	int32 n  = qn - sn;

	if (idx>n) idx=n;
	if (idx<0) return -1;
	if (sn==0) return idx;

	ucs4char s0 = s[0];

	while ( idx>=0 )
	{
		idx = RFind(s0,idx); if(idx==-1) break;
		if (s==MidString(idx,sn)) return idx;
		idx--;
	}
	return -1;
}


/* ----	find c-string in string --------------------------------------------------------------------
		returns index of c-string found
		returns -1 if not found
*/
int32 String::Find ( cstr s, int32 idx ) const
{
	XXXCHECK(*this);
	return Find(String(s),idx);		// to be optimized...
}

int32 String::RFind (cstr s, int32 idx ) const
{
	XXXCHECK(*this);
	return RFind(String(s),idx);		// to be optimized...
}


void String::Replace ( ucs4char o, ucs4char n )
{
	XXXCHECK(*this);
	MakeWritable();
	if(req_charsize(o)>Csz()) return;
	CharSize rcsz = req_charsize(n);
	if (rcsz>Csz())
	{
		ptr newdata = new char[rcsz*count];
		ucs_from_ucs ( rcsz, newdata, Csz(), text, count );
		_kill();
		text = newdata;
		SetDataAndCsz(newdata,rcsz);
		next = this;
		prev = this;
		XXXCHECK(*this);
	}

	switch(Csz())
	{
	case csz1:
		{	ucs1char* a = Ucs1();
			ucs1char* e = a + Len();
			while ( --e>=a ) { if (*e==ucs1char(o)) *e = ucs1char(n); }
		}	return;
	case csz2:
		{	ucs2char* a = Ucs2();
			ucs2char* e = a + Len();
			while ( --e>=a ) { if (*e==ucs2char(o)) *e = ucs2char(n); }
		}	return;
	case csz4:
		{	ucs4char* a = Ucs4();
			ucs4char* e = a + Len();
			while ( --e>=a ) { if (*e==ucs4char(o)) *e = ucs4char(n); }
		}	return;
	}
	IERR();
}

void String::Replace ( cString& o, cString& n )
{
	XXXCHECK(*this);
	XXXCHECK(o);
	XXXCHECK(n);

	if (o.Len()==0) return;

	for ( int32 i=0; (i=Find(o,i))>=0; i+=n.Len() )
	{
		*this = LeftString(i) + n + MidString(i+o.Len());
	}
}


void String::Swap ( ucs4char o, ucs4char n )
{
	XXXCHECK(*this);
	MakeWritable();
	CharSize rcsz = max(req_charsize(n),req_charsize(o));
	if (rcsz>Csz())
	{
		ptr newdata = new char[rcsz*count];
		ucs_from_ucs ( rcsz, newdata, Csz(), text, count );
		_kill();
		text = newdata;
		SetDataAndCsz(newdata,rcsz);
		next = this;
		prev = this;
		XXXCHECK(*this);
	}
	ucs4char x = o^n;

	switch(Csz())
	{
	case csz1:
		{	ucs1char* a = Ucs1();
			ucs1char* e = a + Len();
			ucs1char c;
			while ( --e >= a ) { c = *e; if (c==ucs1char(o)||c==ucs1char(n)) *e = ucs1char(c^x); }
		}	return;
	case csz2:
		{	ucs2char* a = Ucs2();
			ucs2char* e = a + Len();
			ucs2char c;
			while ( --e >= a ) { c = *e; if (c==ucs2char(o)||c==ucs2char(n)) *e = ucs2char(c^x); }
		}	return;
	case csz4:
		{	ucs4char* a = Ucs4();
			ucs4char* e = a + Len();
			ucs4char c;
			while ( --e >= a ) { c = *e; if (c==ucs4char(o)||c==ucs4char(n)) *e = ucs4char(c^x); }
		}	return;
	}
	IERR();
}


/* ----	convert string to uppercase ------------------------------------
		charsize is not changed except:
		if Ucs1()-size string contains "ÿ" charsize is changed to Ucs2()
*/
String String::ToUpper ( ) const
{
	XXXCHECK(*this);
	if(count<=0) return emptyString;
	String s(count,Csz());
	memcpy ( s.text, text, uint32(count)*Csz() );
	XXXCHECK(s);

	switch ( Csz() )
	{
	case csz1:
		{
			ucs1char* za = s.Ucs1();
			ucs1char* ze = za + count;
			ucs1char  c;

			while (za<ze)
			{
				c = *--ze;
				if ( c<'a'  ) continue;
				if ( c<='z' ) { *ze = c^0x20; continue; }
				if ( c<0xe0 ) continue;
				if ((c|8)!=0xff ) { *ze = c^0x20; continue; }		// if not ÷ or ÿ
				if ( c   ==0xff )									// if ÿ
				{
					ptr newdata = new char[csz2*s.count];
					ucs2_from_ucs1 ( newdata, s.text, s.count );
					s._kill();
					s.text = newdata;
					s.SetDataAndCsz(newdata,csz2);
					s.next = &s;
					s.prev = &s;
					goto c2;
				}
			}
			XXXCHECK(s);
			return s;
		}
	case csz2:
		{
	c2:		ucs2char* za = s.Ucs2();
			ucs2char* ze = za + count;
			ucs2char  c;

			while (za<ze)
			{
				c = *--ze;
				if ( c>='a' ) *ze = ucs4::to_upper(c);
			}
			XXXCHECK(s);
			return s;
		}
	case csz4:
		{
			ucs4char* za = s.Ucs4();
			ucs4char* ze = za + count;
			ucs4char  c;

			while (za<ze)
			{
				c = *--ze;
				if ( c>='a' ) *ze = ucs4::to_upper(c);
			}
			XXXCHECK(s);
			return s;
		}
	}
	IERR();
}


/* ----	convert string to lowercase ------------------------------------
		charsize is not changed
*/
String String::ToLower ( ) const
{
	XXXCHECK(*this);
	if(count<=0) return emptyString;
	String s(count,Csz());
	memcpy ( s.text, text, uint32(count)*Csz() );
	XXXCHECK(s);

	switch ( Csz() )
	{
	case csz1:
		{
			ucs1char* za = s.Ucs1();
			ucs1char* ze = za + count;
			ucs1char  c;

			while (za<ze)
			{
				c = *--ze ^ 0x20;
				if ( c<'a'  ) continue;
				if ( c<='z' ) { *ze = c; continue; }
				if ( c<0xe0 ) continue;
				if ((c|8)!=0xff ) { *ze = c; continue; }	 // if not × or ß
			}
			XXXCHECK(s);
			return s;
		}
	case csz2:
		{
			ucs2char* za = s.Ucs2();
			ucs2char* ze = za + count;
			ucs2char  c;

			while (za<ze)
			{
				c = *--ze;
				if ( (c^0x20)>='a' ) *ze = ucs4::to_lower(c);
			}
			XXXCHECK(s);
			return s;
		}
	case csz4:
		{
			ucs4char* za = s.Ucs4();
			ucs4char* ze = za + count;
			ucs4char  c;

			while (za<ze)
			{
				c = *--ze;
				if ( (c^0x20)>='a' ) *ze = ucs4::to_lower(c);
			}
			XXXCHECK(s);
			return s;
		}
	}
	IERR();
}


/* ----	convert string to html ------------------------------------------
		replaces <, >, & and " with names
		string may grow in length
		charsize is preserved
		note: no replacement of non-ucs-1 characters because
			  this would render non-Latin text unreadable.
*/
String String::ToHtml ( ) const
{
	cptr t; ssize_t i;

	XXXCHECK(*this);
	if ( count==0 ) return emptyString;
	int32	 n   = 20;						// spare characters: string may grow!
	String   s   = String(count+n,Csz());

	switch ( Csz() )
	{
	case csz1:
		{
			ucs1char   c;
			const ucs1char* q  = Ucs1();
			const ucs1char* qe = q + count;
			ucs1char*  z  = s.Ucs1();
			ucs1char*  ze = z + s.count -5;

			while ( q<qe )
			{
				switch(c=*q++)
				{
					default: *z++ = c; continue;
					case '<':	t = "&lt;";   break;
					case '>':	t = "&gt;";   break;
					case '&':	t = "&amp;";  break;
					case '"':	t = "&quot;"; break;
				}

				while( (c = *cuptr(t++)) ) *z++ = c;

				if (qe-q <= ze-z) continue;

				i = z - s.Ucs1();
				s._resize(s.count+(n*=2));
				z  = s.Ucs1() + i;
				ze = s.Ucs1() + s.count -5;
			}
			s.count = int32(z - s.Ucs1());
			XXXCHECK(s);
			return s;
		}

	case csz2:
		{
			ucs2char   c;
			const ucs2char* q  = Ucs2();
			const ucs2char* qe = q + count;
			ucs2char*  z  = s.Ucs2();
			ucs2char*  ze = z + s.count -5;

			while ( q<qe )
			{
				switch(c=*q++)
				{
					default: *z++ = c; continue;
					case '<':	t = "&lt;";   break;
					case '>':	t = "&gt;";   break;
					case '&':	t = "&amp;";  break;
					case '"':	t = "&quot;"; break;
				}

				while( (c = *cuptr(t++)) ) *z++ = c;

				if (qe-q <= ze-z) continue;

				i = z - s.Ucs2();
				s._resize(s.count+(n*=2));
				z  = s.Ucs2() + i;
				ze = s.Ucs2() + s.count -5;
			}
			s.count = int32(z - s.Ucs2());
			XXXCHECK(s);
			return s;
		}

	case csz4:
		{
			ucs4char   c;
			const ucs4char* q  = Ucs4();
			const ucs4char* qe = q + count;
			ucs4char*  z  = s.Ucs4();
			ucs4char*  ze = z + s.count -5;

			while ( q<qe )
			{
				switch(c=*q++)
				{
					default: *z++ = c; continue;
					case '<':	t = "&lt;";   break;
					case '>':	t = "&gt;";   break;
					case '&':	t = "&amp;";  break;
					case '"':	t = "&quot;"; break;
				}

				while( (c = *cuptr(t++)) ) *z++ = c;

				if (qe-q <= ze-z) continue;

				i = z - s.Ucs4();
				s._resize(s.count+(n*=2));
				z  = s.Ucs4() + i;
				ze = s.Ucs4() + s.count -5;
			}
			s.count = int32(z - s.Ucs4());
			XXXCHECK(s);
			return s;
		}
	}

	IERR();
}


struct HtmlInfo { char name[10]; ucs2char code; };
static HtmlInfo htmlinfo[] =
{
#define H(UTF8,BESCHR,NAME,UCS2) { NAME, UCS2 }
#include "Libraries/cstrings/legacy_charsets/html.h"
#undef H
};


/* ----	convert string from Html ----------------------------------------
		expand entity names and character codes
		result string may have increased charsize
		result string length may shrink
*/
String String::FromHtml ( ) const
{	ucs4char n,m;
	uint i,j;

	XXXCHECK(*this);
	if ( count==0 ) return emptyString;
	String   s   = String(count,Csz());

	switch ( Csz() )
	{
	case csz1:
		{
			const ucs1char* q  = Ucs1();
			const ucs1char* qe = q + count;
			ucs1char*  z  = s.Ucs1();
			ucs1char   c;
			const ucs1char  *q0,*qs;

			while ( q<qe )
			{
				c = *q++; if (c!='&') { *z++ = c; continue; }

				q0 = q;						// remember in case of invalid char ref.
				qs = q;						// ptr -> ';'
				while ( qs<qe && *qs!=';' ) qs++;
				if ( qs>=qe || qs>q+10 ) 	// unmasked '&' => copy 'as is'
				{ x1: q = q0; *z++ = '&'; continue; }

				if (*q=='#')				// UCS4 number
				{
					q++; n = 0;				// UCS4 code akku
					if ((*q|0x20)=='x')	for( q++; (m=_digit_value(*q))<16; q++ ) { n = (n<<4) + m; }
					else 				for(    ; (m=_digit_val(*q))<10;   q++ ) { n =  n*10  + m; }
					if (q!=qs) goto x1;
				}
				else						// named entity
				{
					for ( i=0; i<NELEM(htmlinfo); i++ )
					{
						cptr p = htmlinfo[i].name;
						for ( j=0; p[j] && p[j]==q[j]; j++ ) {}
						if (p[j]==0 && q+j==qs) break;
					}
					if (i==NELEM(htmlinfo)) goto x1;
					n = htmlinfo[i].code;
					q = qs;
				}
				if (n>>8)
				{
					String zz(count,req_charsize(n));
					ucs_from_ucs ( zz.Csz(),zz.text, csz1,text, count );
					return zz.FromHtml();
				}
				q++; *z++ = ucs1char(n); continue;
			}

			s.count = int32(z - s.Ucs1());
			XXXCHECK(s);
			return s;
		}

	case csz2:
		{
			const ucs2char* q  = Ucs2();
			const ucs2char* qe = q + count;
			ucs2char*  z  = s.Ucs2();
			ucs2char   c;
			const ucs2char  *q0,*qs;

			while ( q<qe )
			{
				c = *q++; if (c!='&') { *z++ = c; continue; }

				q0 = q;						// remember in case of invalid char ref.
				qs = q;						// ptr -> ';'
				while ( qs<qe && *qs!=';' ) qs++;
				if ( qs>=qe || qs>q+10 ) 	// unmasked '&' => copy 'as is'
				{ x2: q = q0; *z++ = '&'; continue; }

				if (*q=='#')			// UCS4 number
				{
					q++; n = 0;			// UCS4 code akku
					if ((*q|0x20)=='x')	for( q++; (m=_digit_value(*q))<16; q++ ) { n = (n<<4) + m; }
					else 				for(    ; (m=_digit_val(*q))<10;   q++ ) { n =  n*10  + m; }
					if (q!=qs) goto x2;
				}
				else						// named entity
				{
					for ( i=0; i<NELEM(htmlinfo); i++ )
					{
						cptr p = htmlinfo[i].name;
						for ( j=0; p[j] && uchar(p[j])==q[j]; j++ ) {}
						if (p[j]==0 && q+j==qs) break;
					}
					if (i==NELEM(htmlinfo)) goto x2;
					n = htmlinfo[i].code;
					q = qs;
				}
				if (n>>8)
				{
					String zz(count,csz4);
					ucs4_from_ucs2 ( zz.Ucs4(), Ucs2(), count );
					return zz.FromHtml();
				}
				q++; *z++ = ucs2char(n); continue;
			}

			s.count = int32(z - s.Ucs2());
			XXXCHECK(s);
			return s;
		}

	case csz4:
		{
			const ucs4char* q  = Ucs4();
			const ucs4char* qe = q + count;
			ucs4char*  z  = s.Ucs4();
			ucs4char   c;
			const ucs4char  *q0,*qs;

			while ( q<qe )
			{
				c = *q++; if (c!='&') { *z++ = c; continue; }

				q0 = q;						// remember in case of invalid char ref.
				qs = q;						// ptr -> ';'
				while ( qs<qe && *qs!=';' ) qs++;
				if ( qs>=qe || qs>q+10 ) 	// unmasked '&' => copy 'as is'
				{ x4: q = q0; *z++ = '&'; continue; }

				if (*q=='#')			// UCS4 number
				{
					q++; n = 0;			// UCS4 code akku
					if ((*q|0x20)=='x')	for( q++; (m=_digit_value(*q))<16; q++ ) { n = (n<<4) + m; }
					else 				for(    ; (m=_digit_val(*q))<10;   q++ ) { n =  n*10  + m; }
					if (q!=qs) goto x4;
				}
				else						// named entity
				{
					for ( i=0; i<NELEM(htmlinfo); i++ )
					{
						cptr p = htmlinfo[i].name;
						for ( j=0; p[j] && (uchar)p[j]==q[j]; j++ ) {}
						if (p[j]==0 && q+j==qs) break;
					}
					if (i==NELEM(htmlinfo)) goto x4;
					n = htmlinfo[i].code;
					q = qs;
				}
				q++; *z++ = n; continue;
			}

			s.count = int32(z - s.Ucs4());
			XXXCHECK(s);
			return s;
		}
	}		// case

	IERR();
}


static const char ec[] = "\\\"'?abfnrtv";			// escape character
static const char cc[] = "\\\"'\?\a\b\f\n\r\t\v";	// result


/* ----	escape control codes et.al. -----------------------------------------
		replace control codes by "\ooo" octal numbers
		replace "well known" control codes by "\x" names
		replace char(0x7f) by "\177" octal number
		replace "\" by "\\"
		if leftquote!=0 replace <rightquote> by "\<rightquote>"  -> idR: " -> \"
		resulting string may grow in length
		resulting string charsize preserved
		if leftquote!=0 is true then leftquote and matching rightquote are added at both ends
		*NOTE*: only non-printable 8-bit characters are replaced (=> ANSI-C)
*/
String String::ToEscaped ( ucs4char leftquote ) const
{
	XXXCHECK(*this);

	xlogIn("String::ToEscaped()");

	ucs4char rightquote = leftquote;
	if (leftquote&&leftquote!='"'&&leftquote!='\'')
	{
		uint i;
		for( i=0; i<NELEM(quotes); i++ )
		{
			if (quotes[i][0] == leftquote) break;
		}
		if (i<NELEM(quotes)) rightquote = quotes[i][1];
		else leftquote = rightquote = '"';
	}

	if (count==0) return leftquote ? String(leftquote)+String(rightquote) : emptyString;

	if(leftquote>0xffu)
	{
		CharSize rcsz = req_charsize(leftquote);
		if (rcsz>Csz())
		{
			String z(count,rcsz);
			ucs_from_ucs ( rcsz, z.text, Csz(), text, count );
			return z.ToEscaped(leftquote);
		}
	}

	int		 n   = 20;						// spare bytes: result string may grow
	String	 s   = String(count+n,Csz());

	switch ( Csz() )
	{
	case csz1:
		{
			ucs1char   c;
			const ucs1char* q  = Ucs1();			// source pointer (q='quelle')
			const ucs1char* qe = q+count;		// source end
			ucs1char*  z  = s.Ucs1();		// target pointer (z='ziel')
			ucs1char*  ze = z +s.count -4;	// target end -3 -1

			if (leftquote) *z++ = ucs1char(leftquote);

			while(q<qe)
			{
				c = *q++;

				if( (c&0x60) && c!=0x7f && c!='\\' && c!=rightquote )
				{
					*z++ = c; continue;
				}

				if (qe-q > ze-z)
				{
					int32 i = int32(z - s.Ucs1());
					s._resize(s.count+(n*=2));
					z  =  s.Ucs1() +i;
					ze =  s.Ucs1() +s.count-4;
				}

				*z++ = '\\';
				if(rightquote&&c==rightquote) { *z++ = c; continue; }
				cptr p = c ? strchr(cc,c) : nullptr;
				if (p) { *z++ = ucs1char(ec[p-cc]); continue; }
				z[2] = '0'+(c&7); c>>=3;
				z[1] = '0'+(c&7); c>>=3;
				z[0] = '0'+ c;
				z+=3;
			}

			if (rightquote) *z++ = ucs1char(rightquote);

			s.count = int32(z - s.Ucs1());
			break;
		}

	case csz2:
		{
			ucs2char   c;
			const ucs2char* q  = Ucs2();			// source pointer (q='quelle')
			const ucs2char* qe = q+count;		// source end
			ucs2char*  z  = s.Ucs2();		// target pointer (z='ziel')
			ucs2char*  ze = z +s.count -4;	// target end: space -3 for "\123" -1 for rightquote

			if (leftquote) *z++ = ucs2char(leftquote);

			while(q<qe)
			{
				c = *q++;

				if( (c&0xFF60) && c!=0x7f && c!='\\' && c!=rightquote )
				{
					assert(z < s.Ucs2()+s.Len());
					*z++ = c; continue;
				}

				if (qe-q > ze-z)
				{
					int32 i = int32(z - s.Ucs2());
					s._resize(s.count+(n*=2));
					z  =  s.Ucs2() +i;
					ze =  s.Ucs2() +s.count-4;
				}

				*z++ = '\\';
				if(rightquote&&c==rightquote) { *z++ = c; continue; }
				cptr p = strchr(cc,c);
				if (p) { *z++ = ucs2char(ec[p-cc]); continue; }
				assert(z+3 <= s.Ucs2()+s.Len());
				z[2] = '0'+(c&7); c>>=3;
				z[1] = '0'+(c&7); c>>=3;
				z[0] = '0'+ c;
				z+=3;
			}

			if (rightquote) *z++ = ucs2char(rightquote);

			assert(z < s.Ucs2()+s.Len());
			s.count = int32(z - s.Ucs2());
			break;
		}

	case csz4:
		{
			ucs4char   c;
			const ucs4char* q  = Ucs4();			// source pointer (q='quelle')
			const ucs4char* qe = q+count;		// source end
			ucs4char*  z  = s.Ucs4();		// target pointer (z='ziel')
			ucs4char*  ze = z +s.count -4;	// target end -3 -1

			if (leftquote) *z++ = leftquote;

			while(q<qe)
			{
				c = *q++;

				if( (c&0xFFFFFF60) && c!=0x7f && c!='\\' && c!=rightquote )
				{
					*z++ = c; continue;
				}

				if (qe-q > ze-z)
				{
					int32 i = z - s.Ucs4();
					s._resize(s.count+(n*=2));
					z  =  s.Ucs4() +i;
					ze =  s.Ucs4() +s.count-4;
				}

				*z++ = '\\';
				if(rightquote&&c==rightquote) { *z++ = c; continue; }
				cptr p = strchr(cc,c);
				if (p) { *z++ = ec[p-cc]; continue; }
				z[2] = '0'+(c&7); c>>=3;
				z[1] = '0'+(c&7); c>>=3;
				z[0] = '0'+ c;
				z+=3;
			}

			if (rightquote) *z++ = rightquote;

			s.count = z - s.Ucs4();
			break;
		}
	}

	XXXCHECK(s);
	return s;
}


/* ----	undo \-escape sequences ---------------------------------------
		undoes "\ooo" octal numbers
		undoes "\xXX" hexa-decimal numbers
		undoes "\cX" control codes
		discards "\<return><whitespace>"
		undoes "well known" control codes with "\x" names
		undoes '\\' and '\"' (resp. any other passed quote char)
		resulting string may shrink
		resulting string charsize is preserved
		if "leftquote!=0" then matching quotes at both ends are removed
		*NOTE*: octal and hexadecimal escape sequences are only expanded
		to characters in range of ucs-1 (max. 3 octal digits, max. 2 hex. digits)
*/
String String::FromEscaped ( ucs4char leftquote ) const
{
	xlogIn("String::FromEscaped()");

	XXXCHECK(*this);
	if (count==0) return emptyString;

	ucs4char rightquote = leftquote;
	if (leftquote&&leftquote!='"'&&leftquote!='\'')
	{
		uint i;
		for( i=0; i<NELEM(quotes); i++ )
		{
			if (quotes[i][0] == leftquote) break;
		}
		if (i<NELEM(quotes)) rightquote = quotes[i][1];
		else leftquote = rightquote = '"';
	}

	CharSize csz = Csz();
	String   s   = String(count,csz);

	switch ( csz )
	{
	case csz1:
		{
			ucs1char   c,d;
			const ucs1char* q  = Ucs1();
			const ucs1char* qe = q+count;
			ucs1char*  z  = s.Ucs1();

			if(leftquote&&count>=2&&*q==leftquote&&qe[-1]==rightquote) { q++; qe--; }

			for ( ; q<qe; *z++=c )
			{
			a1:	c = *q++;
				if (c!='\\') continue;

			// '\' escaped byte:
				if (q>=qe) continue;	// at eof: just store the '\'
				c = *q++;
				if (c>>7) continue;		// no valid \-sequence => masked for unknown reason => store as is

			// \ooo => octal value for next byte. 9th bit discarded
			// \oo     allowed but not recommended.
			// \o	   allowed but not recommended.
				if (is_oct_digit(c))
				{
					c = _digit_val(c);
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=(c<<3)+d;         } }
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=ucs1char(c<<3)+d; } }
					continue;
				}

			// \xHH => hex coded value for next byte
			// \xH     allowed too but not recommended.
			// \x      'x' masked for unknown reason => stores 'x'
				if (c=='x')
				{
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=d;        } }
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=(c<<4)+d; } }
					continue;
				}

			// \cX => control code value for next byte
			// \c     (at eof) => stores 'c'
				if (c=='c')
				{
					if (q<qe) c = *q++ & 0x1f;
					continue;
				}

			// \<return> => discard return and following white space  ((control codes))
				if (c==13||c==10)
				{
					while( q<qe && *q<=' ' ) q++;
					if (q<qe) goto a1;		// don't just 'continue': this would store c = '\n' !
					else break;
				}

			// "well known abreviations" or '"' '\' or s.th. escaped for unknown reason:
				cptr p = strchr(ec,c);
				if (p) c = cc[p-ec];
			}

			s.count = int32(z - s.Ucs1());
			XXXCHECK(s);
			return s;
		}

	case csz2:
		{
			ucs2char   c,d;
			const ucs2char* q  = Ucs2();
			const ucs2char* qe = q+count;
			ucs2char*  z  = s.Ucs2();

			if(leftquote&&count>=2&&*q==leftquote&&qe[-1]==rightquote) { q++; qe--; }

			for ( ; q<qe; *z++=c )
			{
			a2:	c = *q++;
				if (c!='\\') continue;

			// '\' escaped byte:
				if (q>=qe) continue;	// at eof: just store the '\'
				c = *q++;
				if (c>>7) continue;		// no valid \-sequence => masked for unknown reason => store as is

			// \ooo => octal value for next byte. 9th bit discarded
			// \oo     allowed but not recommended.
			// \o	   allowed but not recommended.
				if (is_oct_digit(c))
				{
					c = _digit_val(c);
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=(c<<3)+d;         } }
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=ucs1char(c<<3)+d; } }
					continue;
				}

			// \xHH => hex coded value for next byte
			// \xH     allowed too but not recommended.
			// \x      'x' masked for unknown reason => stores 'x'
				if (c=='x')
				{
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=d;        } }
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=(c<<4)+d; } }
					continue;
				}

			// \cX => control code value for next byte
			// \c     (at eof) => stores 'c'
				if (c=='c')
				{
					if (q<qe) c = *q++ & 0x1f;
					continue;
				}

			// \<return> => discard return and following white space  ((control codes))
				if (c==13||c==10)
				{
					while( q<qe && *q<=' ' ) q++;
					if (q<qe) goto a2;		// don't just 'continue': this would store c = '\n' !
					else break;
				}

			// "well known abreviations" or '"' '\' or s.th. escaped for unknown reason:
				cptr p = strchr(ec,c);
				if (p) c = cc[p-ec];
			}

			s.count = int32(z - s.Ucs2());
			XXXCHECK(s);
			return s;
		}

	case csz4:
		{
			ucs4char   c,d;
			const ucs4char* q  = Ucs4();
			const ucs4char* qe = q+count;
			ucs4char*  z  = s.Ucs4();

			if(leftquote&&count>=2&&*q==leftquote&&qe[-1]==rightquote) { q++; qe--; }

			for ( ; q<qe; *z++=c )
			{
			a4:	c = *q++;
				if (c!='\\') continue;

			// '\' escaped byte:
				if (q>=qe) continue;	// at eof: just store the '\'
				c = *q++;
				if (c>>7) continue;		// no valid \-sequence => masked for unknown reason => store as is

			// \ooo => octal value for next byte. 9th bit discarded
			// \oo     allowed but not recommended.
			// \o	   allowed but not recommended.
				if (is_oct_digit(c))
				{
					c = _digit_val(c);
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=(c<<3)+d;         } }
					if (q<qe) { d=_digit_val(*q); if (d<8) { q++; c=ucs1char(c<<3)+d; } }
					continue;
				}

			// \xHH => hex coded value for next byte
			// \xH     allowed too but not recommended.
			// \x      'x' masked for unknown reason => stores 'x'
				if (c=='x')
				{
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=d;        } }
					if (q<qe) { d=_digit_value(*q); if (d<16) { q++; c=(c<<4)+d; } }
					continue;
				}

			// \cX => control code value for next byte
			// \c     (at eof) => stores 'c'
				if (c=='c')
				{
					if (q<qe) c = *q++ & 0x1f;
					continue;
				}

			// \<return> => discard return and following white space  ((control codes))
				if (c==13||c==10)
				{
					while( q<qe && *q<=' ' ) q++;
					if (q<qe) goto a4;		// don't just 'continue': this would store c = '\n' !
					else break;
				}

			// "well known abreviations" or '"' '\' or s.th. escaped for unknown reason:
				cptr p = strchr(ec,c);
				if (p) c = cc[p-ec];
			}

			s.count = int32(z - s.Ucs4());
			XXXCHECK(s);
			return s;
		}
	}

	IERR();
}


/* ----	export to utf-8 ------------------------------------
		assumes that data will be written in binary mode
		thus multi-byte encodings will be spread over multiple csz1 characters
*/
String String::ToUTF8 ( ) const
{
	XXXCHECK(*this);

	switch(Csz())
	{
	case csz1:
	  {
		int32 n = utf8_bytecount( Ucs1(), Len() );
		if (n==Len()) return *this;
		String s(n,csz1);
		IFDEBUG( ptr p = ) utf8_from_ucs1( s.Text(), Ucs1(), Len() );
		assert(p == s.Text()+s.Len());
		return s;
	  }
	case csz2:
	  {
		int32 n = utf8_bytecount( Ucs2(), Len() );
		String s(n,csz1);
		IFDEBUG( ptr p = ) utf8_from_ucs2( s.Text(), Ucs2(), Len() );
		assert(p == s.Text()+s.Len());
		return s;
	  }
	case csz4:
	  {
		int32 n = utf8_bytecount( Ucs4(), Len() );
		String s(n,csz1);
		IFDEBUG( ptr p = ) utf8_from_ucs4( s.Text(), Ucs4(), Len() );
		assert(p == s.Text()+s.Len());
		return s;
	  }
	}

	IERR();
	return emptyString;		// for the sake of the glorious gcc
}


/* ----	import from utf-8 ---------------------------------
		assumes that data was read in binary mode
		thus multi-byte encodings are still spread over multiple csz1 characters
*/
String String::FromUTF8() const
{
	if(Csz()!=csz1) return ToUCS1().FromUTF8();

	cptr text = Text();
	int32 len = Len();

	if(len>=3 && peek3X(text)==0x00efbbbf) { text+=3; len-=3; }		// skip BOM

	return String(cstr(text), len);
}


/* ----	export to ucs1 ------------------------------------
		assumes that data will be written in binary mode
*/
String String::ToUCS1 ( ) const
{
	if (Csz()==csz1) return *this;

	String s(*this);
	s.MakeWritable();
	if(s.Csz()==csz2)
	{ u16ptr p = s.UCS2Text(); for (int i=0;i<s.Len();i++) if(p[i]>0xff) p[i]='?'; }
	else
	{ u32ptr p = s.UCS4Text(); for (int i=0;i<s.Len();i++) if(p[i]>0xff) p[i]='?'; }
	s.ResizeCsz(csz1);
	XXXCHECK(s);
	return s;
}


/* ----	import from ucs1 ---------------------------------
		assumes that data was read in binary mode
*/
String String::FromUCS1 ( ) const
{
	if (Csz()==csz1) return *this;

	String s(*this);
	s.ResizeCsz(csz1);
	XXXCHECK(s);
	return s;
}


/* ----	export to ucs2 ---------------------------------
		assumes that data will be written in binary mode
		thus assuming that a character must be split into 2 bytes
*/
String String::ToUCS2 ( ) const
{
	String s(*this);
	if (s.Csz()>csz2)
	{
		s.MakeWritable();
		u32ptr p = s.UCS4Text(); for (int i=0;i<s.Len();i++) if(p[i]>0xffff) p[i]='?';
	}
	s.ResizeCsz(csz2);
	s.SetCsz(csz1); s.count*=csz2;
	XXXCHECK(s);
	return s;
}


/* ----	import from ucs2 ---------------------------------
		assumes that data was read in binary mode
		thus assuming that 2 bytes make a character
*/
String String::FromUCS2 ( ) const
{
	String s(*this); s.ResizeCsz(csz1);
	s.SetCsz(csz2); s.count/=csz2;		// note: bogus final odd byte gets lost
	XXXCHECK(s);
	return s;
}


/* ----	export to ucs4 ---------------------------------
		assumes that data will be written in binary mode
		thus assuming that a character must be split into 4 bytes
*/
String String::ToUCS4 ( ) const
{
	String s(*this);
	s.ResizeCsz(csz4);
	s.SetCsz(csz1); s.count*=csz4;
	XXXCHECK(s);
	return s;
}


/* ----	import from ucs4 ---------------------------------
		assumes that data was read in binary mode
		thus assuming that 4 bytes make a character
*/
String String::FromUCS4 ( ) const
{
	String s(*this); s.ResizeCsz(csz1);
	s.SetCsz(csz4); s.count/=csz4;		// note: bogus final odd bytes get lost
	XXXCHECK(s);
	return s;
}


/* ----	export to url-encoded ----------------------------------

	-->	application/x-www-form-urlencoded

		convert to UTF-8 and then mask non-ascii, control- and URI special characters
		resulting data is 7-bit clean ascii
		and can be written either in binary mode or with us-ascii, ucs-1 or utf-8 conversion
		note: if preserve_dirsep==true then '/' is taken as the directory separator and is not encoded!
		note: it may be impossible to access some files containing non-ascii characters
			  on a fileserver which assumes latin-1 or so.
		note: you cannot url-encode full urls like "http://user:passwd@host.org:port/path/file.htm#anchor?querry=data"
			  because ':' '@' '#' '?' would be encoded!
			  you must encode the path/filename and any other component -if required- separately
*/
String String::ToUrl ( bool preserve_dirsep ) const
{
/*	from "man:/url":

	The following characters are reserved, that is, they may appear in a URI but their use is limited
	to their reserved purpose (conflicting data must be escaped before forming the URI):

	; / ? : @ & = + $ ,

	Unreserved characters may be included in a URI. Unreserved characters include upper and
	lower case English letters, decimal digits, and the following limited set of punctuation marks
	and symbols:

	- _ . ! ~ * ' ( )

	All other characters must be escaped. An escaped octet is encoded as a character triplet,
	consisting of the percent character "%" followed by the two hexadecimal digits representing
	the octet code (you can use upper or lower case letters for the hexadecimal digits). For example,
	a blank space must be escaped as "%20", a tab character as "%09", and the "&" as "%26". Because
	the percent "%" character always has the reserved purpose of being the escape indicator, it must
	be escaped as "%25". It is common practice to escape space characters as the plus symbol (+) in
	query text; this practice isn't uniformly defined in the relevant RFCs (which recommend %20 instead)
	but any tool accepting URIs with query text should be prepared for them. A URI is always shown in
	its "escaped" form.

	Unreserved characters can be escaped without changing the semantics of the URI, but this should not
	be done unless the URI is being used in a context that does not allow the unescaped character to
	appear. For example, "%7e" is sometimes used instead of "~" in an http URL path, but the two are
	equivalent for an http URL.

	For URIs which must handle characters outside the US ASCII character set,
	the HTML 4.01 specification (section B.2) and IETF RFC 2718 (section 2.2.5)
	recommend the following approach:

	1. translate the character sequences into UTF-8 (IETF RFC 2279) - see utf-8(7) - and then
	2. use the URI escaping mechanism, that is, use the %HH encoding for unsafe octets.
*/

	String z = ToUTF8(); assert(z.Csz()==csz1);
	String z3 = "%00";   assert(z3.Csz()==csz1);
	static const char hex[] = "0123456789ABCDEF";
	cstr unreserved = "/0123456789-_.!~*'()";
	if (!preserve_dirsep) unreserved++;

	for( int i=z.Len(); --i >= 0; )
	{
		char c = z.text[i];
		if( is_letter(c) ) continue;
		if( c && strchr(unreserved,c) ) continue;
		z3.text[1] = hex[(c>>4)&15];
		z3.text[2] = hex[c&15];
		z = z.LeftString(i) + z3 + z.MidString(i+1);
	}

	return z;
}


/* ---- import from url-encoded ----------------------------------

	--> application/x-www-form-urlencoded

		step 1: decode url-encoding
		step 2: decode utf-8 encoding

		note: you should split protocoll, host, user, password, path/filename, anchor
			  and query beforehand because path/filename may contain url-special characters

		2004-05-11 kio:
			  removed replacement of '+' by space
			  this is not part of url encoding
			  for query strings replace '+' yourself first!
*/
String String::FromUrl ( ) const
{
	String z = *this;
	if (z.Csz()!=csz1) z.ResizeCsz(csz1);

	assert(z.Csz()==csz1);

	for( int i=0; i<z.Len()-2; i++ )
	{
		ucs1char c = z.Ucs1()[i];
		if( c!='%' ) continue;
		char a = z.text[i+1]; if (no_hex_digit(a)) continue;
		char b = z.text[i+2]; if (no_hex_digit(b)) continue;
		z = z.LeftString(i) + CharString(_digit_value(a)*16+_digit_value(b)) + z.MidString(i+3);
	}

	return z.FromUTF8();
}


String String::ToMime() const
{
	logline("String::ToMime() not yet implemented");		// ***TODO***
	return *this;
}


String String::FromMime() const
{
	logline("String::FromMime() not yet implemented");		// ***TODO***
	return *this;
}


/* ----	replace by tabs -----------------------------------
		any sequence of 1 .. 'tabstops' spaces ending on a multiple
		of 'tabstops' is replaced by a single tab character
		tabstops are re-synced at line breaks
*/
String String::ToTab( int tabstops ) const
{
	assert( tabstops>=1 && tabstops<=99 );

	String zstr(Len(),Csz());
	int32 q0 = 0;
	int32 q  = 0;
	int32 z  = 0;
	int32 qe = Len();

	switch(Csz())
	{
	case csz1:
		while (q0<qe)
		{
			while(q<qe&&q<q0+tabstops)
			{
				ucs1char c = Ucs1(q++);
				if (c>13)							{ zstr.Ucs1(z++) = c; continue; }
				if (c==13||c==10||c==0)				{ zstr.Ucs1(z++) = c; break; }
				if (c!='\t')						{ zstr.Ucs1(z++) = c; continue; }
				while (zstr.Ucs1(z-1)==' ') {z--;}	{ zstr.Ucs1(z++) = c; break; }
			}
			if (zstr.Ucs1(z-1)==' ')
			{
				while (zstr.Ucs1(z-1)==' ') { z--; }
				zstr.Ucs1(z++) = '\t';
			}
			q0 = q;
		}
		break;

	case csz2:
		while (q0<qe)
		{
			while(q<qe&&q<q0+tabstops)
			{
				ucs2char c = Ucs2(q++);
				if (c>13)							{ zstr.Ucs2(z++) = c; continue; }
				if (c==13||c==10||c==0)				{ zstr.Ucs2(z++) = c; break; }
				if (c!='\t')						{ zstr.Ucs2(z++) = c; continue; }
				while (zstr.Ucs2(z-1)==' ') {z--;}	{ zstr.Ucs2(z++) = c; break; }
			}
			if (zstr.Ucs2(z-1)==' ')
			{
				while (zstr.Ucs2(z-1)==' ') { z--; }
				zstr.Ucs2(z++) = '\t';
			}
			q0 = q;
		}
		break;

	case csz4:
		while (q0<qe)
		{
			while(q<qe&&q<q0+tabstops)
			{
				ucs4char c = Ucs4(q++);
				if (c>13)							{ zstr.Ucs4(z++) = c; continue; }
				if (c==13||c==10||c==0)				{ zstr.Ucs4(z++) = c; break; }
				if (c!='\t')						{ zstr.Ucs4(z++) = c; continue; }
				while (zstr.Ucs4(z-1)==' ') {z--;}	{ zstr.Ucs4(z++) = c; break; }
			}
			if (zstr.Ucs4(z-1)==' ')
			{
				while (zstr.Ucs4(z-1)==' ') z--;
				zstr.Ucs4(z++) = '\t';
			}
			q0 = q;
		}
		break;
	}

	return zstr.LeftString(z);
}


/* ----	expand tabs to spaces ------------------------------
		all tab characters are replaced by 1 to 'tabstops' spaces
		to fill up to next multiple of 'tabstops'
		tabstops are re-synced at line breaks

		***TODO***	very slow for long texts with many tabs
					maybe calc. required zstr.Len() first
					or use a higher/increasing multiple of 'tabstops' than 4
*/
String String::FromTab( int tabstops ) const
{
	assert( tabstops>=1 && tabstops<=99 );

	if(Find('\t')<0) return *this;

	String zstr( Len()+tabstops*4, Csz() );

	int32 q  = 0;
	int32 qe = Len();
	int32 z0 = 0;
	int32 z  = 0;

	switch(Csz())
	{
	case csz1:
		while(q<qe)
		{
			ucs1char c = Ucs1(q++);
			if(c>13)			   { zstr.Ucs1(z++) = c; continue; }
			if(c==13||c==10||c==0) { zstr.Ucs1(z++) = c; z0=z; continue; }
			if(c!='\t')			   { zstr.Ucs1(z++) = c; continue; }

			int n = tabstops - (z-z0)%tabstops;
			if( qe-q > zstr.Len()-(z+n) ) zstr._resize(zstr.Len()+tabstops*4);
			while(n--) zstr.Ucs1(z++) = ' ';
		}
		break;

	case csz2:
		while(q<qe)
		{
			ucs2char c = Ucs2(q++);
			if(c>13)			   { zstr.Ucs2(z++) = c; continue; }
			if(c==13||c==10||c==0) { zstr.Ucs2(z++) = c; z0=z; continue; }
			if(c!='\t')			   { zstr.Ucs2(z++) = c; continue; }

			int n = tabstops - (z-z0)%tabstops;
			if( qe-q > zstr.Len()-(z+n) ) zstr._resize(zstr.Len()+tabstops*4);
			while(n--) zstr.Ucs2(z++) = ' ';
		}
		break;

	case csz4:
		while(q<qe)
		{
			ucs4char c = Ucs4(q++);
			if(c>13)			   { zstr.Ucs4(z++) = c; continue; }
			if(c==13||c==10||c==0) { zstr.Ucs4(z++) = c; z0=z; continue; }
			if(c!='\t')			   { zstr.Ucs4(z++) = c; continue; }

			int n = tabstops - (z-z0)%tabstops;
			if( qe-q > zstr.Len()-(z+n) ) zstr._resize(zstr.Len()+tabstops*4);
			while(n--) zstr.Ucs4(z++) = ' ';
		}
		break;
	}

	return zstr.LeftString(z);
}


/* ----	export string to any known encoding ---------------------------

	note on conversion to UCS1, UCS2 and UCS4:
		1) to resize the CharSize of a String use ResizeCsz()
		2) converting a String to UCS1, UCS2 or UCS4 assumes that it will
		   be written without further conversion to a file or stream.
		   therefore characters are spread to multiple csz1 characters:
		   e.g. convert to ucs4:
				-> 4 UCS1Chars
		   not:	-> 1 ucs4char
*/
String String::ConvertedTo ( CharEncoding e ) const
{
	const ucs2char* ctable;
	XXXCHECK(*this);

	switch(int(e))
	{
	case UCS1:		return ToUCS1();
	case UCS2:		return ToUCS2();
	case UCS4:		return ToUCS4();
	case UTF8:		return ToUTF8();
	case URL:		return ToUrl();
	case URL_ALL:	return ToUrl(no);
	case MIME:		return ToMime();
	case HTML:		return ToHtml();
	case QUOTED:	return ToQuoted();
	case ESCAPED:	return ToEscaped();
	case UPPERCASE:	return ToUpper();
	case LOWERCASE:	return ToLower();

	case ASCII_US:
		{	String s = ConvertedTo(UCS1);
			s.MakeWritable();
			u8ptr z = s.Ucs1();
			for (int32 i=0;i<s.Len();i++) if (z[i]>>7) z[i] ='?';
			XXXCHECK(s);
			return s;
		}

	case ASCII_GER:	ctable = legacy_charset_ascii_ger; goto b8;
	case CP_437:	ctable = legacy_charset_cp_437;	   goto b8;
	case MAC_ROMAN:	ctable = legacy_charset_mac_roman; goto b8;
	case ATARI_ST:	ctable = legacy_charset_atari_st;  goto b8;
	case RTOS:		ctable = legacy_charset_rtos;	   goto b8;
b8:		{
			String s(Len(),csz1);
			ptr z = s.text;
			switch(Csz())
			{
			case csz1: { cu8ptr q = Ucs1(); for(int32 i=0;i<Len();i++) z[i] = ucs4::to_8bit(q[i],ctable); } break;
			case csz2: { cu16ptr q = Ucs2(); for(int32 i=0;i<Len();i++) z[i] = ucs4::to_8bit(q[i],ctable); } break;
			case csz4: { cu32ptr q = Ucs4(); for(int32 i=0;i<Len();i++) z[i] = ucs4::to_8bit(q[i],ctable); } break;
			}
			XXXCHECK(s);
			return s;
		}

	default:
		if (e>=TAB1 && e<=TAB9) return ToTab(e-TAB1+1);

		xlog("String::ConvertedTo(%i): not yet implemented",int(e));
		TODO();
	}
}


/* ----	import string from any known encoding ---------------------------
*/
String String::ConvertedFrom ( CharEncoding e ) const
{
	const ucs2char* ctable;

	XXXCHECK(*this);
	switch(int(e))
	{
	case UCS1:		return FromUCS1();
	case UCS2:		return FromUCS2();
	case UCS4:		return FromUCS4();
	case UTF8:		return FromUTF8();
	case HTML:		return FromHtml();
	case URL_ALL:
	case URL:		return FromUrl();
	case MIME:		return FromMime();
	case QUOTED:	return FromQuoted();
	case ESCAPED:	return FromEscaped();

	case UPPERCASE:						// senseless
	case LOWERCASE:	return *this;		// senseless

	case ASCII_US:
		{
			String s(*this);
			s.ResizeCsz(csz1);
			s.MakeWritable();
			u8ptr z = s.Ucs1();
			for (int32 i=0;i<s.Len();i++) { if(z[i]>>7) z[i]&=0x7F; }
			XXXCHECK(s);
			return s;
		}

	case ASCII_GER:	ctable = legacy_charset_ascii_ger; goto b8;
	case CP_437:	ctable = legacy_charset_cp_437;	   goto b8;
	case MAC_ROMAN:	ctable = legacy_charset_mac_roman; goto b8;
	case ATARI_ST:	ctable = legacy_charset_atari_st;  goto b8;
	case RTOS:		ctable = legacy_charset_rtos;	   goto b8;
b8:		{
			String s(*this);
			s.ResizeCsz(csz1);
			s.MakeWritable();
			ucs1char* z = s.ucs1_text;
			int32 i=0;
			for (;i<s.Len();i++)
			{
				ucs2char c = ucs4::from_8bit(char(z[i]),ctable);
				if (c>255) goto b16;
				z[i] = ucs1char(c);
			}
			return s;
b16:		s.ResizeCsz(csz2);
			ucs2char* zz = s.ucs2_text;
			for (;i<s.Len();i++) { zz[i] = ucs4::from_8bit(char(zz[i]),ctable); }
			XXXCHECK(s);
			return s;
		}

	default:
		if (e>=TAB1 && e<=TAB9) return FromTab(e-TAB1+1);

		xlog("String::ConvertedFrom(%i): not yet implemented",(int)e);
		TODO();
	}
}



/* ----	purge caches to free up memory -----------------------------------------------
*/
void String::PurgeCaches ( )
{
	/* nothing to do */
}





/* ===================================================================================
			NON MEMBER FUNCTIONS
=================================================================================== */


/*
String CharString ( ucs4char c )
{
	return c<256 ? String(abc+c,1) : String(c);
}
*/


/*
String SpaceString ( long n, ucs4char c )
{
	XXXTRAP( spaceString!=String(spaceString.Len(),' ') );
	return c==' '&&n<=spaceString.Len() ? spaceString.LeftString(n) : String(n,c);
}
*/


String HexString ( double d, int digits )	// ***TODO*** fractional part ?
{
	cstr hex;
	if(d<0)
	{
		d = -1-d;
		hex="FEDCBA9876543210";
		if(!digits) { digits=1; if(d!=0.0) { (void)frexp(d,&digits); digits=(digits+4)>>2; } }
	}
	else
	{
		hex="0123456789ABCDEF";
		if(!digits) { digits=1; if(d!=0.0) { (void)frexp(d,&digits); digits=(digits+3)>>2; } }
	}
	d = floor(d);
	String s(digits,csz1);
	do { s.Text()[--digits] = hex[ uint( ldexp( modf( ldexp(d,-4), &d ), 4) ) ]; } while(digits);
	return s;
}

String BinString ( double d, int digits )	// ***TODO*** fractional part ?
{
	cstr bin;
	if(d<0)
	{
		d = -1-d;
		bin="10";
		if(!digits) { digits=1; if(d!=0.0) { (void)frexp(d,&digits); digits+=1; } }
	}
	else
	{
		bin="01";
		if(!digits) { digits=1; if(d!=0.0) { (void)frexp(d,&digits); } }
	}
	d = floor(d);
	String s(digits,csz1);
	do { s.Text()[--digits] = bin[ uint( ldexp( modf( ldexp(d,-1), &d ), 1) ) ]; } while(digits);
	return s;
}

String NumString ( double n )
{
//	max. 21 char
	String z(24,csz1);
	z.Truncate( sprintf(z.Text(),"%.14g",n) );
	XXXCHECK(z);
	return z;
}

String NumString ( long n )
{
// max. 12 chars
	String z(12,csz1);
	z.Truncate( sprintf(z.Text(),"%li",n) );
	XXXCHECK(z);
	return z;
}

String NumString ( ulong n )
{
// max. 11 chars
	String z(12,csz1);
	z.Truncate( sprintf(z.Text(),"%lu",n) );
	XXXCHECK(z);
	return z;
}

String HexString ( ulong n, int digits )
{
	if(!digits) { digits = n>>16 ? 8 : n>>8 ? 4 : 2; }

	String z(digits,csz1);
	while(digits>0)
	{
		z.Text()[--digits] = "0123456789ABCDEF"[n&0x0f];
		n >>= 4;
	}
	XXXCHECK(z);
	return z;
}

static off_t file_size(int fd) noexcept
{
	struct stat fs;
	if (fstat(fd,&fs)) return -1;			// error
	return fs.st_size;
}

// get working directory or NULL
static cstr workingdirpath()
{
	char s[MAXPATHLEN+1];
	s[MAXPATHLEN] = 0;
	return dupstr( getcwd(s,MAXPATHLEN) );
}

// get user home directory or NULL
static cstr homedirpath()
{
	return getenv("HOME");
}

static cstr quick_fullpath( cstr path )
{
	if(path[0]=='~' && path[1]=='/') { cstr h=homedirpath(); if(h) path = catstr(h,path+1); }
	if(path[0]!='/')				 { path = catstr(workingdirpath(),"/",path); }
	return path;
}

void String::ReadFromFile( cstr cpath, off_t max_sz )
{
	Clear();
	errno = ok;
	if(!cpath||!*cpath) { errno=ENOENT; return; }	// no such file or directory --> a required component of the filepath is missing
	cpath = quick_fullpath(cpath); if(errno) return;
	int fd = open( cpath,O_RDONLY ); if(fd==-1) return;

	off_t n  = 0;									// n  = bytes read
	off_t sz = file_size(fd);						// sz = file size
		if( sz<=0 )		{ goto x; }					// -1: open failed; 0: empty file
		if( sz>max_sz ) { errno=EFBIG; goto x; }	// file too large

	_kill();
	_init(csz1,sz);
	//if(errno) goto x; if(Len()!=sz) { errno=ENOMEM; goto x; }		// out of memory

	while ( n<sz )
	{
		ssize_t r = read ( fd, Text()+n, sz-n );
		if (r!=-1) { n+=r; continue; }
		if (errno!=EINTR && errno!=EAGAIN) break;
	}
	if(n<sz) Truncate(n); else errno=ok;

x:	close(fd);
}


void String::ReadFromFile( cString& path, off_t max_sz )
{
	ReadFromFile( path.CString(), max_sz );
}



/* ----	32-Bit-Primzahlen -----------------------------------------------------------------
*/
static uint32 prim[] =
{
	4082442179u, 3134562593u, 2883627547u, 2567654303u, 3311094179u, 2660468347u, 3352679063u, 2179288373u,
	2219291651u, 3264466963u, 2267587997u, 3759131197u, 3484443481u, 2157055477u, 3623644897u, 3408306677u,
	3534620147u, 2609476411u, 4009698493u, 3676609817u, 4210158697u, 3710124419u, 3206152063u, 2490176341u,
	2320400059u, 3324063619u, 4000354447u, 3516401387u, 2444461157u, 3457331093u, 3272600677u, 4237679587u,
//	3816520987u, 3575769173u, 2924379011u, 2681534827u, 2203088519u, 2380368763u, 3636635803u, 3410680799u,
//	2868811903u, 4186747393u, 3412143391u, 3731664617u, 4011100739u, 3534395411u, 2853505093u, 2550920611u,
//	2924953399u, 4161939961u, 3073271633u, 2700841183u, 4248979811u, 3829758953u, 2579473499u, 2975565143u,
//	3918994669u, 3294543181u, 3675058111u, 2316572107u, 2480194517u, 2157852997u, 3446492953u, 4195433141u,
//	2699673269u, 3752947819u, 4292241403u, 3643396103u, 3865278289u, 2421157601u, 2576148571u, 4076712377u,
//	3363242011u, 2390061899u, 2737177889u, 2855086903u, 2487113467u, 3520803367u, 2158248611u, 3679906903u,
	3047975057u, 2315343463u, 3301686491u
};


#if 0
proc NewPrime(bits)
{
	do
		var n = (0.5+random(0.5)) << bits
			n = int(n>>1) << 1 +1
		var e = int sqrt n
		var q = 1
		do
			if (q+=2)>e return n then
			until fract(n/q)==0
		loop
	loop
}
#endif

/* 	Berechne einen Hashwert für den angegebenen Namen

	Die Hashes sind prinzipbedingt nicht eindeutig.
	Der Hash des leeren Strings ist 0.
	0 ist aber auch der Hash für andere Strings, da: Hashes sind nicht eindeutig!

	Methode:
	Alle Bits aus allen Zeichen des Strings werden möglichst wild auf die Bits des Hashes verteilt.
	Dazu werden die Zeichen mit ständig neuen Zahlen multipliziert, die möglichst chaotisch bzgl.
	ihrer gesetzten Bits sind. Außerdem sollten sie ungerade sein: gesetztes Bit 0.
	Durch die Multiplikation wird jedes Bit eines Zeichens auf all die Bits repliziert, die im
	Multiplikator gesetzt sind, verschoben um die eigene Bitposition.
	Bit 0 im Zeichen wird auf alle Bits des Hashes verteilt, die höherwertigen Bits auf immer weniger.
	Das macht aber nichts, da Buchstaben überwiegend Ascii sind und somit überwiegend Bits 0-6 variieren.
	Es sollten alle Zeichen des Strings in den Hash eingehen, um systematischen Problemen vorzubeugen.
	z.B. wenn nur Zeichen 1-20 eingehen, wären die Hashes für Variablennamen, die alle mit dem gleichen
	Wortstamm anfangen, gleich.
	Außerdem wird nach Erschöpfung der Primzahlenliste der Hash geshiftet, damit z.B. zwei identische
	Zeichenblocks, die genau diese Zeichenzahl haben, sich nicht systematisch immer zu 0 auslöschen.
	Das Shiften bedeutet quasi, dass der bisherige Hash nocheinmal mit einer Zahl multipliziert wird,
	und das entspricht dem Effekt, als wären alle bisherigen Primes mit dieser Zahl multipliziert gewesen.
*/
uint32 String::CalcHash( ) const
{
	xxlogIn("String::CalcHash()");

	uint32 hash = 0;
	int a,e,i,n;

	for( e=0; e<Len(); )
	{
		hash = (hash<<7) | (hash>>(32-7));

		a  = e;
		e += NELEM(prim); if (e>Len()) e=Len();
		n  = e-a;

		switch (Csz())
		{
		case csz1:	{ const ucs1char* p = Ucs1()+a; for ( i=0; i<n; i++ ) hash ^= p[i] * prim[i]; } break;
		case csz2:	{ const ucs2char* p = Ucs2()+a; for ( i=0; i<n; i++ ) hash ^= p[i] * prim[i]; } break;
		case csz4:	{ const ucs4char* p = Ucs4()+a; for ( i=0; i<n; i++ ) hash ^= p[i] * prim[i]; } break;
		}
	}


	xxlogline("return: $%08lx",hash);
	return hash;
}





























