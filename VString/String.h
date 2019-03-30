#pragma once
/*	Copyright  (c)	Günter Woigk 2001 - 2015
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


 ---------------------------------------------------------------------------

			class String

			this is a variant of the char-based (UTF-8) class String.
			+ supports UTF-8, UCS-1 (ISO Latin-1), UCS-2 and UCS-4 characters
			+ lots of conversions
			+ indexable characters (which is not possible with UTF-8 Strings)
			- slower than char-based (UTF-8) Strings

---------------------------------------------------------------------------
*/

#include "kio/kio.h"
#include "Unicode/Unicode.h"
#include "Unicode/UTF-8.h"




/* ********************************************************************************
					CHARACTER ENCODING for conversion
******************************************************************************** */

enum CharEncoding
{
	// character encodings:
		ucs1,			iso_latin_1 = ucs1,
		ucs2,
		ucs4,
		ascii_us,		// 7-bit character set
		ascii_ger,		// 7-bit character set
		cp_437,			// the one with the line graphics
		mac_roman,
		atari_st,
		rtos,			// a private one for kio
		utf8,

	// conversions:
		html,
		url,			// no '/' encoding
		url_all,		// also encode '/'
		mime,
		escaped,
		quoted,
		uppercase,		// ?
		lowercase,		// ?
		tab1,tab2,tab3,tab4,
		tab5,tab6,tab7,tab8,tab9,
};




/* ********************************************************************************
					class String
******************************************************************************** */


enum CharSize  { csz1=1, csz2=2, csz4=4 };
enum MatchType { fnm_basic=0, fnm_escape=1, fnm_pathname=2, fnm_period=4, fnm_casefold=8 };


class String
{
	union
	{
	ptr			text;			// ptr -> start of text
	UCS1Char*	ucs1_text;
	UCS2Char*	ucs2_text;
	UCS4Char*	ucs4_text;
	cUCS1Char*	cucs1_text;
	cUCS2Char*	cucs2_text;
	cUCS4Char*	cucs4_text;
	};
	int32		count;			// number of characters
	size_t		data_and_csz;	// data ptr for delete[]; lower 2 bits: csz -1
	String*		next;			// next string sharing same data
	String*		prev;			// prev string sharing same data

	void 		_init_from_utf8	( cptr p, cptr e, bool=no );
	void		_init 			( );
	void 		_init 			( cString& q );
	void 		_init 			( CharSize csz, int32 n );
	void		_move			( String& q );
	void 		_kill			( );
	void		_resize			( int32 newlen );
	int32		compare	  		( cString& q ) const;

	UCS1Char*&	Ucs1			( )						{ return ucs1_text; }
	UCS2Char*&	Ucs2			( )						{ return ucs2_text; }
	UCS4Char*&	Ucs4			( )						{ return ucs4_text; }
	cUCS1Char* const&	Ucs1	( )	const				{ return cucs1_text; }
	cUCS2Char* const&	Ucs2	( )	const				{ return cucs2_text; }
	cUCS4Char* const&	Ucs4	( ) const				{ return cucs4_text; }
	UCS1Char&	Ucs1			( int32 i )				{ return ucs1_text[i]; }
	UCS2Char&	Ucs2			( int32 i )				{ return ucs2_text[i]; }
	UCS4Char&	Ucs4			( int32 i )				{ return ucs4_text[i]; }
	cUCS1Char&	Ucs1			( int32 i ) const		{ return cucs1_text[i]; }
	cUCS2Char&	Ucs2			( int32 i ) const		{ return cucs2_text[i]; }
	cUCS4Char&	Ucs4			( int32 i ) const		{ return cucs4_text[i]; }

	static size_t calc_data_and_csz ( cptr p, CharSize sz ) { assert(!(size_t(p)&3)); return size_t(p)+sz-1; }
	ptr			Data			( ) const				{ return ptr(data_and_csz&size_t(~3)); }
	size_t		DataAndCsz		( )	const				{ return data_and_csz; }
	void		SetData			( cptr p )				{ assert(!(size_t(p)&3)); data_and_csz = (data_and_csz&3)+size_t(p); }
	void		SetCsz			( CharSize sz )			{ data_and_csz = (data_and_csz&size_t(~3))+sz-1; }
	void		SetDataAndCsz	( cptr p, CharSize sz )	{ assert(!(size_t(p)&3)); data_and_csz = size_t(p)+sz-1; }
	void		SetDataAndCsz	( size_t q )			{ data_and_csz = q; }

public:
				~String			( );
				String			( );

				String			( UCS4Char c );						// may be 'not writable'
				String   		( int32 n, CharSize sz );			// uncleared
				String   		( int32 n, UCS4Char c );			// cleared; may be 'not writable'

				String   		( cUCS1Char* p, int32 chars );
				String   		( cUCS2Char* p, int32 chars );
				String   		( cUCS4Char* p, int32 chars );
				String   		( cUTF8CharPtr p, int32 bytes );
				String   		( cptr p, int32 chars, CharSize sz );

				String   		( cUTF8Str s );						// string from c-string
				String			( cUTF8Str s, CharEncoding );		// string from c-string: interpret bogus utf-8 as latin-1
				String   		( cString& q );						// copy creator
				String   		( cString& q, int32 a, int32 e );	// substring
				String			( cString& q, cuptr txt, int32 chars, CharSize sz );	// special re-interpret substring

	CharSize	ReqCsz			( ) const;
	void		Swap			( String& q );
	void		Copy			( cString& q );
	void		Clear    		( );								// quick clear string: len=0
	void		Truncate		( int32 newlen );
	void		Crop			( int32 left, int32 right );
	void		Resize			( int32 newlen, UCS4Char padding=' ' );

	int32		Len				( ) const							{ return count; }
	CharSize	Csz				( )	const							{ return CharSize((data_and_csz&3)+1); }
	void		ResizeCsz		( CharSize );
	UCS4Char	operator[]  	( int32 i ) const;
	UCS4Char	LastChar		( ) const throws;

	UCS1Char*	UCS1Text		( )									{ assert(Csz()==csz1); return Ucs1(); }
	UCS2Char*	UCS2Text		( )									{ assert(Csz()==csz2); return Ucs2(); }
	UCS4Char*	UCS4Text		( )									{ assert(Csz()==csz4); return Ucs4(); }
	ptr			Text			( )									{ return text; }
	cUCS1Char*	UCS1Text		( ) const							{ assert(Csz()==csz1); return Ucs1(); }
	cUCS2Char*	UCS2Text		( ) const							{ assert(Csz()==csz2); return Ucs2(); }
	cUCS4Char*	UCS4Text		( ) const							{ assert(Csz()==csz4); return Ucs4(); }
	cptr		Text			( ) const							{ return text; }

	bool		IsWritable  	( ) const							{ return (next==this && (data_and_csz>>2)!=0) || count==0; }
	bool		NotWritable  	( ) const							{ return (next!=this || (data_and_csz>>2)==0) && count!=0; }
	void		MakeWritable	( );

	String		SubString   	( int32 a, int32 e ) const			{ return String(*this,a,e); }
	String		MidString   	( int32 a, int32 n ) const			{ return String(*this,a,a+n); }
	String		MidString		( int32 a ) const					{ return String(*this,a,Len()); }
	String		LeftString  	( int32 n ) const					{ return String(*this,int32(0),n); }
	String		RightString 	( int32 n ) const					{ return String(*this,Len()-n,Len()); }

	String&		operator=   	( cString& q );

	String		operator+   	( cString& q ) const;				// concatenate strings
	String&		operator+=  	( cString& q )						{ return *this = *this + q; }

	String		operator*   	( int32 n ) const;					// repeat string contents n times
	String&		operator*=  	( int32 n )							{ return *this = *this * n; }

	bool		operator==  	( cString& q ) const				{ return count==q.count && compare(q)==0; }
	bool		operator!=  	( cString& q ) const				{ return count!=q.count || compare(q)!=0; }
	bool		operator<   	( cString& q ) const				{ return compare(q)<0; }
	bool		operator>   	( cString& q ) const				{ return compare(q)>0; }
	bool		operator>=  	( cString& q ) const				{ return compare(q)>=0; }
	bool		operator<=  	( cString& q ) const				{ return compare(q)<=0; }

	str			CString			( ) const;
	double		NumVal			( int32* index_inout=nullptr ) const;
	String		StrVal			( int32* index_inout=nullptr ) const;

	int32		Find			( UCS4Char c, int32 startidx= 0 ) const;
	int32		RFind			( UCS4Char c, int32 startidx= 0x7fffffff ) const;
	int32		Find			( cString& s, int32 startidx= 0 ) const;
	int32		RFind			( cString& s, int32 startidx= 0x7fffffff ) const;
	int32		Find			( cUTF8Str s, int32 startidx= 0 ) const;
	int32		RFind			( cUTF8Str s, int32 startidx= 0x7fffffff ) const;

	void		Replace     	( UCS4Char o, UCS4Char n );
	void		Replace     	( cString& o, cString& n );
	void		Swap			( UCS4Char o, UCS4Char n );

	String		ToUpper  		( )	const;
	String		ToLower  		( )	const;
	String		ToHtml			( ) const;
	String		FromHtml		( ) const;
	String		ToEscaped		( UCS4Char addquotes=no ) const;
	String		FromEscaped		( UCS4Char rmvquotes=no ) const;
	String		ToQuoted		( UCS4Char leftquote='"' ) const	{ return ToEscaped(leftquote); }
	String		FromQuoted		( UCS4Char leftquote='"' ) const	{ return FromEscaped(leftquote); }
	String		ToUCS1			( ) const;
	String		FromUCS1		( ) const;
	String		ToUCS2			( ) const;
	String		FromUCS2		( ) const;
	String		ToUCS4			( ) const;
	String		FromUCS4		( ) const;
	String		ToUTF8			( ) const;
	String		FromUTF8		( ) const;
	String		ToUrl			( bool preserve_dirsep=yes ) const;
	String		FromUrl			( ) const;
	String		ToMime			( ) const;
	String		FromMime		( ) const;
	String		ToTab			( int tabstops=8 ) const;
	String		FromTab			( int tabstops=8 ) const;

	String		ConvertedTo		( CharEncoding e ) const;
	String		ConvertedFrom	( CharEncoding e ) const;

	void		ReadFromFile	( cstr cpath,    off_t max_sz = 1 MB );
	void		ReadFromFile	( cString& path, off_t max_sz = 1 MB );

	uint32 		CalcHash 		( ) const;

	void		Check			( cstr filename, uint line_no ) const;

friend  class   Var;
friend	class	VarData;
friend	void	TestStringClass	(uint&,uint&);

static	void	PurgeCaches		( );
};

extern void		TestStringClass	( );



/* ********************************************************************************
				NON-MEMBER FUNCTIONS
******************************************************************************** */

#define			emptyString	String()						// String() benötigt keine Initialisierung!
//extern String	emptyString;								// Bummer: erst valide nach Initialisierung!

inline String	CharString	( UCS4Char c )					{ return String(c); }
inline String	SpaceString	( int32 n, UCS4Char c=' ' )		{ return String(n,c); }

extern String  	NumString	( double n );
extern String  	NumString	( long n );
extern String  	NumString	( ulong n );
inline String  	NumString	( int n )						{ return NumString(long(n)); }
inline String  	NumString	( uint n )						{ return NumString(ulong(n)); }
extern String  	HexString	( ulong n, int digits=0 );
extern String	HexString	( double d, int digits=0 );
extern String	BinString	( double d, int digits=0 );
inline String	HexString	( uint n, int digits=0 )		{ return HexString(ulong(n),digits); }

inline String	SubString	( cString& s, int32 a, int32 e)	{ return s.SubString(a,e); }
inline String	MidString	( cString& s, int32 a, int32 n)	{ return s.MidString(a,n); }
inline String	MidString	( cString& s, int32 a )			{ return s.MidString(a); }
inline String	LeftString	( cString& s, int32 n )			{ return s.LeftString(n); }
inline String	RightString	( cString& s, int32 n )			{ return s.RightString(n); }

inline String	ToUpper		( cString& s )					{ return s.ToUpper(); }
inline String	ToLower		( cString& s )					{ return s.ToLower(); }
inline String	ToHtml		( cString& s )					{ return s.ToHtml(); }
inline String	FromHtml	( cString& s )					{ return s.FromHtml(); }
inline String	ToQuoted	( cString& s, UCS4Char f='"' )	{ return s.ToQuoted(f); }
inline String	FromQuoted	( cString& s, UCS4Char f='"' )	{ return s.FromQuoted(f); }
inline String	ToEscaped	( cString& s, UCS4Char f=no )	{ return s.ToEscaped(f); }
inline String	FromEscaped	( cString& s, UCS4Char f=no )	{ return s.FromEscaped(f); }

inline str		CString		( cString& s )					{ return s.CString(); }
inline double	NumVal		( cString& s )					{ return s.NumVal(); }
extern bool		Match		( cString& filename, cString& pattern );
extern bool		FNMatch		( cString& filename, cString& pattern, MatchType=fnm_basic );

extern void		UCSCopy		( CharSize zcsz, void* z, CharSize qcsz, const void* q, int32 n );



/* ****************************************************************
				INLINE FUNCTIONS ((private for class Var))
**************************************************************** */

inline void String::_init ( )
{
	xxlogIn("String::_init()");

	text   = nullptr;
	count  = 0;
	SetDataAndCsz(nullptr,csz1);
	next   = this;
	prev   = this;
}


inline void String::_init ( cString& q )
{
	xxlogIn("String::_init(cString&)");

	text   = q.text;
	count  = q.count;
	SetDataAndCsz(q.DataAndCsz());
	next   = q.next;
	prev   = const_cast<String*>(&q);
	next->prev = prev->next = this;
}


inline void String::_init ( CharSize sz, int32 n )
{
	xxlogIn("String::_init(CharSize,n)");

	count  = n;
	text   = new char[sz*n];
	SetDataAndCsz(text,sz);
	next   = this;
	prev   = this;
}


inline void String::_kill()
{
	xxlogIn("String::_kill()");

	if (next==this)
	{
		delete[] Data();
	}
	else
	{
		next->prev = prev;
		prev->next = next;
	}
}






























