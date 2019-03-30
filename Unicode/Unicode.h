#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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
*/

#include "kio/kio.h"
#include "cstrings/cstrings.h"
#include "Unicode/Includes/GlobalConstants.h"


#define ulong_in_range(A,N,E)	(ulong((N)-(A))<=ulong((E)-(A)))
#define ushort_in_range(A,N,E)	(ushort((N)-(A))<=ushort((E)-(A)))
#define uchar_in_range(A,N,E)	(uchar((N)-(A))<=uchar((E)-(A)))


/* ********************************************************************************
						TYPE DEFINITIONS
******************************************************************************** */

typedef uint8				UCS1Char;			// == uchar
typedef	UCS1Char const		cUCS1Char;			// == uchar const
typedef UCS1Char *			UCS1CharPtr;		// == uptr
typedef UCS1Char const *	cUCS1CharPtr;		// == cuptr

typedef uint16				UCS2Char;
typedef	UCS2Char const		cUCS2Char;
typedef UCS2Char *			UCS2CharPtr;
typedef UCS2Char const *	cUCS2CharPtr;

typedef uint32				UCS4Char;			// 2010-12-04 kio: uint8/16/32 instead of char/short/long
typedef	UCS4Char const		cUCS4Char;
typedef UCS4Char *			UCS4CharPtr;
typedef UCS4Char const *	cUCS4CharPtr;



/* ********************************************************************************
						CONSTANTS
******************************************************************************** */

// replacement character for illegal, truncated, unprintable etc. characters
const	UCS1Char	UCS1ReplacementChar		= '?';
const	UCS2Char	UCS2ReplacementChar		= 0xfffd;			// official replacement char
const	UCS4Char	UCS4ReplacementChar		= 0xfffd;



/* ****************************************************************
					CONVERSION between UCS-1/2/4
**************************************************************** */

inline	UCS4Char	UCS4CharFromUCS4	( UCS4Char n )			{ return n; }	// 2005-06-09: support for 32 bit added  =>  now this is a nop now
inline	UCS2Char	UCS2CharFromUCS4	( UCS4Char n )			{ return UCS2Char(n)==n ? n : UCS2ReplacementChar; }
inline	UCS1Char	UCS1CharFromUCS4	( UCS4Char n )			{ return UCS1Char(n)==n ? n : UCS1ReplacementChar; }



/* ****************************************************************
					CONVERSION 8-bit CHAR <-> UCS4
**************************************************************** */

typedef UCS2Char		UCS2Table[256];
typedef UCS2Table const	cUCS2Table;

extern	cUCS2Table	ucs2_from_mac_roman;
extern	cUCS2Table	ucs2_from_ascii_ger;
extern	cUCS2Table	ucs2_from_rtos;
extern	cUCS2Table	ucs2_from_cp_437;
extern	cUCS2Table	ucs2_from_atari_st;

inline	UCS2Char	UCS2CharFrom8Bit	( char c, cUCS2Table t)	{ return t[(uchar)c]; }
inline	UCS2Char	UCS2CharFromLatin1	( char c )				{ return (uchar)c; }
inline	UCS2Char	UCS2CharFromAsciiUS	( char c )				{ return c&0x7f; }
inline	UCS2Char	UCS2CharFromMacRoman( char c )				{ return ucs2_from_mac_roman[(uchar)c]; }
inline	UCS2Char	UCS2CharFromAsciiGer( char c )				{ return ucs2_from_ascii_ger[(uchar)c]; }
inline	UCS2Char	UCS2CharFromRtos	( char c )				{ return ucs2_from_rtos[(uchar)c]; }
inline	UCS2Char	UCS2CharFromCp437	( char c )				{ return ucs2_from_cp_437[(uchar)c]; }
inline	UCS2Char	UCS2CharFromAtariST	( char c )				{ return ucs2_from_atari_st[(uchar)c]; }

extern	char		UCS4CharTo8Bit		( UCS4Char c, cUCS2Table t );
inline	char		UCS4CharToLatin1	( UCS4Char c )			{ return c>>8 ? '?' : c; }
inline	char		UCS4CharToAsciiUS	( UCS4Char c )			{ return c>>7 ? '?' : c; }
inline	char		UCS4CharToAsciiGer	( UCS4Char c )			{ return (c|0x20)<='z' && c!='@' ? c : UCS4CharTo8Bit(c,ucs2_from_ascii_ger); }
inline	char		UCS4CharToRtos		( UCS4Char c )			{ return (c|0x20)<='z' && c!='@' ? c : UCS4CharTo8Bit(c,ucs2_from_rtos); }
inline	char		UCS4CharToMacRoman	( UCS4Char c )			{ return c>>7 ? UCS4CharTo8Bit(c,ucs2_from_mac_roman) : c; }
inline	char		UCS4CharToCp437		( UCS4Char c )			{ return c>>7 ? UCS4CharTo8Bit(c,ucs2_from_cp_437)    : c; }
inline	char		UCS4CharToAtariST	( UCS4Char c )			{ return c>>7 ? UCS4CharTo8Bit(c,ucs2_from_atari_st)  : c; }



/* ********************************************************************************
						PROPERTIES
******************************************************************************** */

//	Unicode Character PROPERTIES

enum	U_PropertyGroup
#include "Unicode/Includes/PropertyGroup_Enum.h"

enum	U_Property
#include "Unicode/Includes/Property_Enum.h"

extern	cstr const	U_PropertyGroup_Names[U_PROPERTYGROUPS];	// Property Group -> Name
extern	uchar const	U_Property_Groups[U_PROPERTIES];			// Property -> Group
extern	cstr const	U_Property_ShortNames[U_PROPERTIES];		// Property -> Short Name
extern	cstr const	U_Property_LongNames[U_PROPERTIES];			// Property -> Long Name

inline	cstr		U_PropertyGroup_Name	( int g )	{ return U_PropertyGroup_Names[g%U_PROPERTYGROUPS]; }
inline	int			U_Property_Group		( int p )	{ return U_Property_Groups[p%U_PROPERTIES]; }
inline	cstr		U_Property_ShortName	( int p )	{ return U_Property_ShortNames[p%U_PROPERTIES]; }
inline	cstr		U_Property_LongName		( int p )	{ return U_Property_LongNames[p%U_PROPERTIES]; }


//	Unicode Character PROPERTY VALUES for Catalog Properties and Enumerated Properties

enum	U_PropertyValue
#include "Unicode/Includes/PropertyValue_Enum.h"

// Short and Long Names: Names may be NULL!
extern cstr const U_PropertyValue_ccc_ShortNames[U_ccc_propertyvalues];
extern cstr const U_PropertyValue_age_ShortNames[U_age_propertyvalues];
extern cstr const U_PropertyValue_bc_ShortNames[U_bc_propertyvalues];
extern cstr const U_PropertyValue_blk_ShortNames[U_blk_propertyvalues];
extern cstr const U_PropertyValue_dt_ShortNames[U_dt_propertyvalues];
extern cstr const U_PropertyValue_ea_ShortNames[U_ea_propertyvalues];
extern cstr const U_PropertyValue_gc_ShortNames[U_gc_propertyvalues];
extern cstr const U_PropertyValue_gcb_ShortNames[U_gcb_propertyvalues];
extern cstr const U_PropertyValue_hst_ShortNames[U_hst_propertyvalues];
extern cstr const U_PropertyValue_jg_ShortNames[U_jg_propertyvalues];
extern cstr const U_PropertyValue_jt_ShortNames[U_jt_propertyvalues];
extern cstr const U_PropertyValue_lb_ShortNames[U_lb_propertyvalues];
extern cstr const U_PropertyValue_nfc_qc_ShortNames[U_nfc_qc_propertyvalues];
extern cstr const U_PropertyValue_nfd_qc_ShortNames[U_nfd_qc_propertyvalues];
extern cstr const U_PropertyValue_nfkc_qc_ShortNames[U_nfkc_qc_propertyvalues];
extern cstr const U_PropertyValue_nfkd_qc_ShortNames[U_nfkd_qc_propertyvalues];
extern cstr const U_PropertyValue_nt_ShortNames[U_nt_propertyvalues];
extern cstr const U_PropertyValue_sc_ShortNames[U_sc_propertyvalues];
extern cstr const U_PropertyValue_sb_ShortNames[U_sb_propertyvalues];
extern cstr const U_PropertyValue_wb_ShortNames[U_wb_propertyvalues];

extern cstr const U_PropertyValue_ccc_LongNames[U_ccc_propertyvalues];
extern cstr const U_PropertyValue_age_LongNames[U_age_propertyvalues];
extern cstr const U_PropertyValue_bc_LongNames[U_bc_propertyvalues];
extern cstr const U_PropertyValue_blk_LongNames[U_blk_propertyvalues];
extern cstr const U_PropertyValue_dt_LongNames[U_dt_propertyvalues];
extern cstr const U_PropertyValue_ea_LongNames[U_ea_propertyvalues];
extern cstr const U_PropertyValue_gc_LongNames[U_gc_propertyvalues];
extern cstr const U_PropertyValue_gcb_LongNames[U_gcb_propertyvalues];
extern cstr const U_PropertyValue_hst_LongNames[U_hst_propertyvalues];
extern cstr const U_PropertyValue_jg_LongNames[U_jg_propertyvalues];
extern cstr const U_PropertyValue_jt_LongNames[U_jt_propertyvalues];
extern cstr const U_PropertyValue_lb_LongNames[U_lb_propertyvalues];
extern cstr const U_PropertyValue_nfc_qc_LongNames[U_nfc_qc_propertyvalues];
extern cstr const U_PropertyValue_nfd_qc_LongNames[U_nfd_qc_propertyvalues];
extern cstr const U_PropertyValue_nfkc_qc_LongNames[U_nfkc_qc_propertyvalues];
extern cstr const U_PropertyValue_nfkd_qc_LongNames[U_nfkd_qc_propertyvalues];
extern cstr const U_PropertyValue_nt_LongNames[U_nt_propertyvalues];
extern cstr const U_PropertyValue_sc_LongNames[U_sc_propertyvalues];
extern cstr const U_PropertyValue_sb_LongNames[U_sb_propertyvalues];
extern cstr const U_PropertyValue_wb_LongNames[U_wb_propertyvalues];

inline cstr		U_PropertyValue_DefaultShortName( int v )		{ return tostr(v); }
inline cstr		U_PropertyValue_DefaultLongName	( int v )		{ return usingstr("Property ID %i",v); }
inline cstr		U_PropertyValue_DefaultLongName	( int p,int v )	{ return usingstr("%s ID %i",U_Property_LongName(p),v); }



/* ********************************************************************************
						CHARACTER INFORMATION
******************************************************************************** */

// General Category:

extern U_PropertyValue	UCS1CharGeneralCategory ( UCS1Char n );
extern U_PropertyValue	UCS2CharGeneralCategory ( UCS2Char n );
extern U_PropertyValue	UCS4CharGeneralCategory ( UCS4Char n );


// Character Names:

extern	cstr			UCS1CharCharacterName	( UCS1Char n );
extern	cstr			UCS2CharCharacterName	( UCS2Char n );
extern	cstr			UCS4CharCharacterName	( UCS4Char n );


// Blocks (Ranges) of Characters:

inline U_PropertyValue	UCS1CharBlockProperty	( UCS1Char n )		{ return n>>7 ? U_blk_latin_1_supplement : U_blk_basic_latin; }
extern U_PropertyValue	UCS2CharBlockProperty	( UCS2Char n );
extern U_PropertyValue	UCS4CharBlockProperty	( UCS4Char n );

inline bool				UCS4CharNotInAnyBlock	( UCS4Char n )		{ return UCS4CharBlockProperty(n) == U_blk_no_block; }


// Script Systems:

extern U_PropertyValue	UCS1CharScriptProperty ( UCS1Char n );
extern U_PropertyValue	UCS2CharScriptProperty ( UCS2Char n );
extern U_PropertyValue	UCS4CharScriptProperty ( UCS4Char n );


// Canonical Combining Class:

inline U_PropertyValue	UCS1CharCccProperty		( UCS1Char   )		{ return U_ccc_0; }		// all: U_ccc_0 == U_ccc_Not_Reordered
extern U_PropertyValue	UCS2CharCccProperty		( UCS2Char n );
extern U_PropertyValue	UCS4CharCccProperty		( UCS4Char n );


// East Asian Width:

extern U_PropertyValue	UCS1CharEAWidthProperty ( UCS1Char n );
extern U_PropertyValue	UCS2CharEAWidthProperty ( UCS2Char n );
extern U_PropertyValue	UCS4CharEAWidthProperty ( UCS4Char n );

extern uint				UCS4CharPrintWidth		( UCS4Char n );		// print width of char in monospaced font  ->  0, 1, or 2



/* ********************************************************************************
						NUMERIC VALUE
******************************************************************************** */

// UPs:
	extern uint		ucs2_get_digitvalue		( UCS2Char n );
	extern uint		ucs4_get_digitvalue		( UCS4Char n );
	extern float	ucs2_get_numericvalue	( UCS2Char n );
	extern float	ucs4_get_numericvalue	( UCS4Char n );

inline	bool		UCS4CharIsDecimalDigit	( UCS4Char n )		{ return n<256  ? is_dec_digit(n) : UCS4CharGeneralCategory(n) == U_gc_decimal_number; }
inline	bool		UCS4CharIsNumberLetter	( UCS4Char n )		{ return n<256  ? is_dec_digit(n) : UCS4CharGeneralCategory(n) == U_gc_letter_number; }
inline	bool		UCS4CharHasNumericValue	( UCS4Char n )		{ return n<0xb2 ? is_dec_digit(n)	// digits, numbers & decorated numbers
																: uchar_in_range(U_gc_number, UCS4CharGeneralCategory(n), U_gc_other_number); }
// Get Decimal Digit Value.
// No error checking. Non-Decimal-Digits return meaningless values.
inline	int			UCS1CharDigitValue		( UCS1Char n )		{ return digit_val(n); }
inline	int			UCS2CharDigitValue		( UCS2Char n )		{ return n>>8 ? ucs2_get_digitvalue(n) : digit_val(n); }
inline	int			UCS4CharDigitValue		( UCS4Char n )		{ return n>>8 ? ucs4_get_digitvalue(n) : digit_val(n); }

// Get Digit, Number & Decorated Number value.
// some fractionals. one negative. two NaNs.
inline	float		UCS1CharNumericValue	( UCS1Char n )		{ return uchar_in_range('0',n,'9')  ? digit_val(n) : ucs2_get_numericvalue(n); }
inline	float		UCS2CharNumericValue	( UCS2Char n )		{ return ushort_in_range('0',n,'9') ? digit_val(n) : ucs2_get_numericvalue(n); }
inline	float		UCS4CharNumericValue	( UCS4Char n )		{ return ulong_in_range('0',n,'9')  ? digit_val(n) : ucs4_get_numericvalue(n); }

// quick variants:
inline	bool		UCS4CharIsDec			( UCS4Char n )		{ return ulong_in_range('0',n,'9'); }
inline	bool		UCS4CharIsOct			( UCS4Char n )		{ return ulong_in_range('0',n,'7'); }
inline	bool		UCS4CharIsBin			( UCS4Char n )		{ return ulong_in_range('0',n,'1'); }
inline	bool		UCS4CharIsHex			( UCS4Char n )		{ return ulong_in_range('0',n,'9') || ulong_in_range('a',n|0x20,'f'); }
inline	int			UCS4CharDecVal			( UCS4Char n )		{ return n>>7 ? 99 : digit_val(n);	  }			// 0..9;  NaN>9
inline	int			UCS4CharHexVal			( UCS4Char n )		{ return n>>7 ? 99 : digit_value(n); }			// 0..36; NaN>36


/* ********************************************************************************
						SIMPLE CASING
******************************************************************************** */

// UPs:
	extern	UCS1Char const UCS1_SUC_Table[0x100];	// uc(ß)=ß, uc(µ)=µ, uc(ÿ)=Y
	extern	UCS1Char const UCS1_SLC_Table[0x100];
	extern	UCS2Char	ucs2_simple_lowercase	( UCS2Char );
	extern	UCS4Char	ucs4_simple_lowercase	( UCS4Char );
	extern	UCS2Char	ucs2_simple_uppercase	( UCS2Char );
	extern	UCS4Char	ucs4_simple_uppercase	( UCS4Char );
	extern	UCS2Char	ucs2_simple_titlecase	( UCS2Char );
	extern	UCS4Char	ucs4_simple_titlecase	( UCS4Char );

// Simple Lowercase:
inline	UCS1Char	UCS1CharSimpleLowercase	( UCS1Char n )		{ return		   UCS1_SLC_Table[n];					}
inline	UCS2Char	UCS2CharSimpleLowercase	( UCS2Char n )		{ return n<0x100 ? UCS1_SLC_Table[n]  : ucs2_simple_lowercase(n); }
inline	UCS4Char	UCS4CharSimpleLowercase	( UCS4Char n )		{ return n<0x100 ? UCS1_SLC_Table[n]  : ucs4_simple_lowercase(n); }

// Simple Uppercase:
inline	UCS1Char	UCS1CharSimpleUppercase	( UCS1Char n )		{ return		   UCS1_SUC_Table[n]; /* uc(µ)=µ, uc(ÿ)=Y */ }
inline	UCS2Char	UCS2CharSimpleUppercase	( UCS2Char n )		{ return n<0xd7  ? UCS1_SUC_Table[n]  : ucs2_simple_uppercase(n); }
inline	UCS4Char	UCS4CharSimpleUppercase	( UCS4Char n )		{ return n<0xd7  ? UCS1_SUC_Table[n]  : ucs4_simple_uppercase(n); }

// Simple Titlecase:
inline	UCS1Char	UCS1CharSimpleTitlecase	( UCS1Char n )		{ return		   UCS1_SUC_Table[n]; /* tc(µ)=µ, tc(ÿ)=Y */ }
inline	UCS2Char	UCS2CharSimpleTitlecase	( UCS2Char n )		{ return n<0xd7  ? UCS1_SUC_Table[n]  : ucs2_simple_titlecase(n); }
inline	UCS4Char	UCS4CharSimpleTitlecase	( UCS4Char n )		{ return n<0xd7  ? UCS1_SUC_Table[n]  : ucs4_simple_titlecase(n); }

// Info:
inline	bool 		UCS4CharIsLowercase 	( UCS4Char n )		{ return n<0xd7 ? is_lowercase(n) : UCS4CharGeneralCategory(n) == U_gc_ll; }
inline	bool 		UCS4CharIsUppercase 	( UCS4Char n )		{ return n<0xd7 ? is_uppercase(n)
																  : uchar_in_range(U_gc_lt, UCS4CharGeneralCategory(n), U_gc_lu); }


/* ****************************************************************
					UNICODE INFO METHODS
**************************************************************** */

// UPs:
	extern bool		ucs4_is_printable		( UCS4Char n );

inline	bool		UCS4CharIsPrintable		( UCS4Char n )		{ return n<0x700 ? n!=0x7f && (n&~0x80)>=0x20 : ucs4_is_printable(n); }
inline	bool		UCS4CharIsControl		( UCS4Char n )		{ return n==0x7f || (n&~0x80)<0x20; }	// checked
inline	bool		UCS4CharIsLetter		( UCS4Char n )		{ return n<0x80 ? is_letter(n)			// checked
																  : uchar_in_range(U_gc_letter, UCS4CharGeneralCategory(n), U_gc_lu); }

















