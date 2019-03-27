#pragma once
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

	c-string library

	• this library provides string manipulation for c-style strings.
	• results are typically stored in thread-safe tempMem buffers
	  and are valid until the current TempMemPool is deallocated.
	• they are intended for immediate use,
	  e.g. for printing, intermediate values in string expressions
	  and for returning result strings.
*/


#include "kio/kio.h"
template <class T> class Array;


extern	str  emptystr;				// non-const version of ""


inline	bool is_space (char c)		noexcept { return uchar(c)<=' ' && c!=0; }
inline	bool is_letter (char c)		noexcept { return uchar((c|0x20)-'a')<='z'-'a'; }
inline	bool is_control	(char c)	noexcept { return uchar(c)<0x20 || uchar(c)==0x7f; }
inline	bool is_printable (char c)	noexcept { return uchar(c)>=0x20 && uchar(c)!=0x7f; }
inline	bool is_uppercase (char c)	noexcept { return uchar(c-'A')<='Z'-'A'; }
inline	bool is_lowercase (char c)	noexcept { return uchar(c-'a')<='z'-'a'; }
inline	char to_upper (char c)		noexcept { return uchar(c-'a')<='z'-'a' ? c&~0x20 : c; }
inline	char to_lower (char c)		noexcept { return uchar(c-'A')<='Z'-'A' ? c|0x20 : c; }

inline	bool is_bin_digit (char c)	noexcept { return uchar(c-'0')<='1'-'0'; }	// { return (c|1)=='1'; }
inline	bool is_oct_digit (char c)	noexcept { return uchar(c-'0')<='7'-'0'; }	// { return (c|7)=='7'; }
inline	bool is_dec_digit (char c)	noexcept { return uchar(c-'0')<='9'-'0'; }
inline	bool is_hex_digit (char c)	noexcept { return uchar(c-'0')<='9'-'0' || uchar((c|0x20)-'a') <= 'f'-'a'; }

inline	bool no_bin_digit (char c)	noexcept { return uchar(c-'0')>'1'-'0'; }	// { return (c|1)=='1'; }
inline	bool no_oct_digit (char c)	noexcept { return uchar(c-'0')>'7'-'0'; }	// { return (c|7)=='7'; }
inline	bool no_dec_digit (char c)	noexcept { return uchar(c-'0')>'9'-'0'; }
inline	bool no_hex_digit (char c)	noexcept { return uchar(c-'0')>'9'-'0' && uchar((c|0x20)-'a') > 'f'-'a'; }

inline	uint digit_val (char c)		noexcept { return uchar(c-'0'); }		// char -> digit value: non-digits ≥ 10
inline	uint digit_value (char c)	noexcept { return c<='9'?uchar(c-'0'):uchar((c|0x20)-'a')+10;} // non-digits ≥ 36
inline	char hexchar (int n)		noexcept { n &= 15; return char((n>=10 ? 'A'-10 : '0') + n); } // masked legal


// ---- queries ----
inline	uint strLen		(cstr s)			noexcept { return s ? uint(strlen(s)) : 0; }	// c-string
extern	bool lt			(cstr,cstr)			noexcept;
extern	bool gt			(cstr,cstr)			noexcept;
extern	bool gt_tolower	(cstr,cstr)			noexcept;
extern	bool eq			(cstr,cstr)			noexcept;
extern	bool ne			(cstr,cstr)			noexcept;
inline	bool le			(cstr a,cstr b)		noexcept { return !gt(a,b); }
inline	bool ge			(cstr a,cstr b)		noexcept { return !lt(a,b); }

extern	cptr find		(cstr target, cstr search) noexcept;
extern	cptr rfind		(cstr target, cstr search) noexcept;
inline	ptr  find		(str  target, cstr search) noexcept	{ return ptr(find(cstr(target),search)); }
inline	ptr  rfind		(str  target, cstr search) noexcept	{ return ptr(rfind(cstr(target),search)); }
extern	bool startswith	(cstr,cstr)			noexcept;
extern	bool endswith	(cstr,cstr)			noexcept;
inline	bool contains	(cstr z, cstr s)	noexcept { return find(z,s); }
extern	bool isupperstr	(cstr)				noexcept;
extern	bool islowerstr	(cstr)				noexcept;


// ----	allocate with new[] ----
extern	str  newstr		(int n)				noexcept; // allocate memory with new[]
extern	str	 newcopy	(cstr)				noexcept; // allocate memory with new[] and copy text


// ---- allocate in TempMemPool ----
extern	str	 tempstr	(uint n)			noexcept; // tempmem.h
inline	str	 tempstr	(int size)			noexcept { assert(size>=0); return tempstr(uint(size)); }
inline	str	 tempstr	(ulong size)		noexcept { assert(size==uint(size)); return tempstr(uint(size)); }
inline	str	 tempstr	(long size)			noexcept { assert(size>=0); return tempstr(ulong(size)); }
inline	str	 xtempstr	(int n)				noexcept; // tempmem.h
extern	str	 xtempstr	(uint n)			noexcept; // tempmem.h
extern	str	 spacestr	(int n, char c=' ')	noexcept;
extern	cstr spaces		(uint n)			noexcept;
extern	str	 whitestr	(cstr, char c=' ')	noexcept; // also in utf8
extern	str	 dupstr		(cstr)				noexcept;
extern	str	 xdupstr    (cstr)				noexcept;

extern	str	 substr		(cptr a, cptr e)	noexcept;
inline	str	 substr		(cuptr a, cuptr e)	noexcept { return substr(cptr(a),cptr(e)); }	// convenience method
extern	str  mulstr 	(cstr, uint n)		throws;	  // limit_error
extern	str  catstr 	(cstr, cstr)		noexcept;
extern	str  catstr 	(cstr, cstr, cstr, cstr=nullptr, cstr=nullptr, cstr=nullptr) noexcept;
extern	str  midstr 	(cstr, int a, int n) noexcept;
extern	str  midstr 	(cstr, int a)		noexcept;
extern	str  leftstr 	(cstr, int n)		noexcept;
extern	str  rightstr 	(cstr, int n)		noexcept;
inline char	 lastchar	(cstr s)			noexcept { return s&&*s ? s[strlen(s)-1] : 0; }

inline	void toupper	(str s)				noexcept { if(s) for( ;*s;s++ ) *s = to_upper(*s); }
inline	void tolower	(str s)				noexcept { if(s) for( ;*s;s++ ) *s = to_lower(*s); }
extern	str	 upperstr	(cstr)				noexcept;
extern	str	 lowerstr	(cstr)				noexcept;
extern	str	 replacedstr(cstr, char oldchar, char newchar) noexcept;
extern	str	 quotedstr	(cstr)				noexcept;
extern	str	 unquotedstr(cstr)				noexcept; // sets errno
extern	str	 escapedstr	(cstr)				noexcept;
extern	str	 unescapedstr(cstr)				noexcept; // sets errno
extern	str	 tohtmlstr	(cstr)				noexcept;
extern	cstr fromhtmlstr(cstr)				noexcept; // may return original string
extern	str	 toutf8str	(cstr)				noexcept;
extern	str  fromutf8str(cstr)				noexcept; // ucs1, sets errno
extern	str  unhexstr	(cstr)				noexcept; // may return nullptr
extern	str	 base64str	(cstr)				noexcept;
extern	str	 unbase64str(cstr)				noexcept; // may return nullptr
extern	cstr croppedstr	(cstr)				noexcept; // may return (substring of) original string
extern	cstr detabstr	(cstr, uint tabs)	noexcept; // may return original string

extern	str	 usingstr	(cstr fmt, va_list)	noexcept __printflike(1,0);
extern	str	 usingstr	(cstr fmt, ...)		noexcept __printflike(1,2);

inline	str	 tostr		(float n)			noexcept { return usingstr("%.10g", double(n)); }
inline	str	 tostr		(double n)			noexcept { return usingstr("%.14g", n); }
inline	str	 tostr		(long double n)		noexcept { return usingstr("%.22Lg",n); }
inline	str	 tostr		(int n)				noexcept { return usingstr("%i", n); }
inline	str	 tostr		(unsigned int n)	noexcept { return usingstr("%u", n); }
inline	str  tostr		(long n)			noexcept { return usingstr("%li", n); }
inline	str	 tostr		(unsigned long n)	noexcept { return usingstr("%lu", n); }
inline	str	 tostr		(long long n)		noexcept { return usingstr("%lli", n); }
inline	str  tostr		(unsigned long long n)	noexcept { return usingstr("%llu", n); }
inline	cstr tostr		(cstr s)			noexcept { return s ? quotedstr(s) : "nullptr"; }

extern	str	 binstr		(uint n, cstr b0="00000000", cstr b1="11111111") noexcept;
extern	str	 hexstr 	(uint32 n, uint len) noexcept;
inline	str  hexstr		(int32 n, uint len)  noexcept { return hexstr(uint32(n),len); }
extern	str	 hexstr 	(uint64 n, uint len) noexcept;
inline	str	 hexstr		(int64 n, uint len)	noexcept { return hexstr(uint64(n),len); }
#ifndef _LINUX
inline	str	 hexstr		(long n, uint len)  noexcept { return hexstr(uint64(n),len); }
inline	str	 hexstr		(ulong n, uint len) noexcept { return hexstr(uint64(n),len); }
#endif
extern	str  hexstr		(cptr, uint len)	noexcept;
inline	str  hexstr		(cstr s)			noexcept { return hexstr(s,strLen(s)); }	// must not contain nullbyte

//template<class T> str hexstr (T* p, uint cnt) throws AMBIGUITY: reinterpret vs. static cast!
//template<class T> str hexstr (T n, uint len)  throws AMBIGUITY: reinterpret vs. static cast!

extern	str	 charstr	(char)				noexcept;
extern	str	 charstr	(char,char)			noexcept;
extern	str	 charstr	(char,char,char)	noexcept;
extern	str	 charstr	(char,char,char,char) noexcept;
extern	str	 charstr	(char,char,char,char,char) noexcept;

extern	str	 datestr	(time_t secs)		noexcept; // returned string is in local time
extern	str	 timestr	(time_t secs)		noexcept; // returned string is in local time
extern	str	 datetimestr (time_t secs)		noexcept; // returned string is in local time
extern	time_t dateval	 (cstr localtimestr) noexcept;
extern	str	 durationstr (time_t secs)		noexcept;
inline	str	 durationstr (int secs)			noexcept { return durationstr(time_t(secs)); }
#ifndef _LINUX
inline	str	 durationstr (int64 secs)		noexcept { return durationstr(time_t(secs)); }
#endif
extern	str	 durationstr (float64 secs)		noexcept;
inline	str	 durationstr (float32 secs)		noexcept { return durationstr(float64(secs)); }

// NOTE: split() reuses the source buffer and overwrites line delimiters with 0, evtl. overwriting char at ptr e!
extern	void _split (Array<str>& z, ptr a, ptr e)			throws; // split at line breaks
extern	void _split (Array<str>& z, ptr a, ptr e, char c)	throws; // split at char

extern	void split (Array<str>& z, cptr a, cptr e)			throws; // split at line breaks
extern	void split (Array<str>& z, cptr a, cptr e, char c)	throws; // split at char
extern	void split (Array<str>& z, cstr s)					throws; // split c-string at line breaks
extern	void split (Array<str>& z, cstr s, char c)			throws; // split c-string at char

inline	void split (Array<cstr>& z, cptr a, cptr e)			throws { split(reinterpret_cast<Array<str>&>(z),a,e); }
inline	void split (Array<cstr>& z, cptr a, cptr e, char c)	throws { split(reinterpret_cast<Array<str>&>(z),a,e,c); }
inline	void split (Array<cstr>& z, cstr s)					throws { split(reinterpret_cast<Array<str>&>(z),s); }
inline	void split (Array<cstr>& z, cstr s, char c)			throws { split(reinterpret_cast<Array<str>&>(z),s,c); }

extern	uint strcpy	(ptr z, cptr q, uint buffersize) noexcept;
extern	uint strcat	(ptr z, cptr q, uint buffersize) noexcept;

extern str join (Array<cstr> const& q) throws;
extern str join (Array<cstr> const& q, char, bool final=no) throws;
extern str join (Array<cstr> const& q, cstr, bool final=no) throws;

// _________________________________________________________________________
//

extern	str	 speakingNumberStr (double) throws __attribute__((deprecated)); // define in appl which needs this

template<class T> inline str numstr (T n) throws __attribute__((deprecated));	// use tostr()
template<class T> inline str numstr (T n) throws { return tostr(n); }			// use tostr()

inline	bool sameStr (cstr a, cstr b) noexcept  __attribute__((deprecated));	// use eq()
inline	bool sameStr (cstr a, cstr b) noexcept { return eq(a,b); }				// use eq()

inline cptr findStr	 (cstr target, cstr search) noexcept __attribute__((deprecated));	// use find()
inline cptr findStr	 (cstr target, cstr search) noexcept {return find(target,search);}

inline cptr rFindStr (cstr target, cstr search) noexcept __attribute__((deprecated));	// use rfind()
inline cptr rFindStr (cstr target, cstr search) noexcept {return rfind(target,search);}











