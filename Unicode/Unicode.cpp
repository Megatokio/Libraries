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

static_assert(1,"");

#include "kio/kio.h"
#include "Unicode.h"
#include <math.h>


#ifndef NAN
static const float NAN (0.0/0.0);
#endif

struct UCS2_Short { UCS2Char Code; int16 Value; };
struct UCS4_Short { UCS4Char Code; int32 Value; };  // int32 instead of int16 wg. alignment

struct UCS2Range_Short { UCS2Char StartCode; UCS2Char EndCode; int16 Value; };
struct UCS4Range_Short { UCS4Char StartCode; UCS4Char EndCode; int32 Value; };  // int32 wg. alignment


/*	UP: retrieve PropertyValue from UCS2 Character Table
		e = initial end value, e.g. Nelem(Table)
*/
inline U_PropertyValue UCS2_GetPropertyValue ( UCS2Char n, UCS2_Short const* Table, int e )
{
	int a=0; do { int i=(a+e)/2; if( n<Table[i].Code ) e=i; else a=i; } while (a<e-1);
	return U_PropertyValue(Table[a].Value);
}

/*	UP: retrieve PropertyValue from UCS4 Character Table
		e = initial end value, e.g. Nelem(Table)
*/
inline U_PropertyValue UCS4_GetPropertyValue ( UCS4Char n, UCS4_Short const* Table, int e )
{
	int a=0; do { int i=(a+e)/2; if( n<Table[i].Code ) e=i; else a=i; } while (a<e-1);
	return U_PropertyValue(Table[a].Value);
}



/*	------------------------------------------------------------------------------------------
	Map Unicode Character -> East Asian Width Property

	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Width
	http://www.unicode.org/reports/tr11/tr11-14.html
*/

static UCS2Range_Short const UCS2_EA_Table[U_EA_UCS2] =
#include "Includes/EastAsianWidth.h"

static UCS4Range_Short const UCS4_EA_Table[U_EA_UCS4] =
#include "Includes/EastAsianWidth_UCS4.h"

inline int UCS2_FindEAIndex ( UCS2Char n, int e )
{
	int a=0; do { int i=(a+e)/2; if( n<UCS2_EA_Table[i].StartCode ) e=i; else a=i; } while (a<e-1);
	return a;
}
inline int UCS4_FindEAIndex ( UCS4Char n, int e )
{
	int a=0; do { int i=(a+e)/2; if( n<UCS4_EA_Table[i].StartCode ) e=i; else a=i; } while (a<e-1);
	return a;
}


U_PropertyValue UCS1CharEAWidthProperty ( UCS1Char n )
{
	if (n<0xa1) return uchar_in_range(0x20,n,0x7e) ? U_ea_na : U_ea_n;
	int i = UCS2_FindEAIndex(n,U_EA_UCS1);
	return n <= UCS2_EA_Table[i].EndCode ? U_PropertyValue(UCS2_EA_Table[i].Value) : U_ea_n;
}

U_PropertyValue UCS2CharEAWidthProperty ( UCS2Char n )
{
	if (n<0xa1) return uchar_in_range(0x20,n,0x7e) ? U_ea_na : U_ea_n;
	int i = UCS2_FindEAIndex(n,U_EA_UCS2);
	return n <= UCS2_EA_Table[i].EndCode ? U_PropertyValue(UCS2_EA_Table[i].Value) : U_ea_n;
}

U_PropertyValue UCS4CharEAWidthProperty ( UCS4Char n )
{
	if (!(n>>16)) return UCS2CharEAWidthProperty(UCS2Char(n)); if (n<U_EA_UCS4_START) return U_ea_n;
	int i = UCS4_FindEAIndex(n,U_EA_UCS4);
	return n <= UCS4_EA_Table[i].EndCode ? U_PropertyValue(UCS4_EA_Table[i].Value) : U_ea_n;
}


/* print width of char in monospaced font  ->  0, 1, or 2

	Narrow				always 1: non-E.A. scripts
								  western script behavior, e.g. for line breaks. rotated in vertical text.

	Half Width			always 1: like Narrow, but probably from E.A. script
								  behave like ideographs. are rotated in vertical text.

	Wide				always 2: E.A. scripts only, e.g. HAN ideographs
								  behave like ideographs. not rotated in v. text, except some few wide punctuations are rotated.

	Full Width			always 2: like Wide, but probably from non-E.A. script

	Ambiguous			prefer 1: 1 or 2 in E.A. legacy char set, 1 in non-E.A. script. e.g. Greek, Cyrillic, Maths
								  ambiguous quotation marks: Wide/Narrow dep. on enclosed text
								  behave like Wide or Narrow. Narrow by default.

	Neutral (non-E.A.)	mostly 1: non-E.A. which do not occur in E.A. legacy char sets.
								  most of them behave like Narrow.
								  unasigned code points.

	Combining marks are classified by their primary use. E.A.Width of combining marks does not specify print width!
								  In particular, nonspacing marks do not possess any actual advance width.

						http://www.unicode.org/reports/tr11/tr11-14.html
*/
uint UCS4CharPrintWidth ( UCS4Char n )
{
	U_PropertyValue w = UCS4CharEAWidthProperty(n); if (w!=U_ea_n) return w==U_ea_w || w==U_ea_f ? 2 : 1;

	U_PropertyValue m = UCS4CharGeneralCategory(n);						// ~ UCS4CharIsPrintable()
	return m>=U_gc_letter /* && m<=U_gc_space_separator */
		&& !uchar_in_range(U_gc_separator,m,U_gc_paragraph_separator)	// line and paragraph separators
		&& !uchar_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark);	// combining letters
}




/*	------------------------------------------------------------------------------------------
	Map Unicode Character -> Block Property

	Many of these blocks have the same name as one of the scripts because characters of that script
	are primarily encoded in that block. However, blocks and scripts differ in the following ways:
		• Blocks are simply ranges, and often contain code points that are unassigned.
		• Characters from the same script may be in several different blocks.
		• Characters from different scripts may be in the same block.
	As a result, for mechanisms such as regular expressions, using script values produces more
	meaningful results than simple matches based on block names.

	http://www.unicode.org/Public/4.1.0/ucd/Blocks.txt
	http://www.unicode.org/reports/tr24/#Blocks
*/

static UCS4_Short const Blocks[U_BLOCKS_UCS4] =
#include "Includes/Blocks.h"

U_PropertyValue	UCS2CharBlockProperty ( UCS2Char n )
{
	return UCS4_GetPropertyValue( n, Blocks, U_BLOCKS_UCS2 );
}

U_PropertyValue	UCS4CharBlockProperty ( UCS4Char n )
{
	return UCS4_GetPropertyValue( n, Blocks, U_BLOCKS_UCS4 );
}




/*	------------------------------------------------------------------------------------------
	Unicode Character -> Script Property

	http://www.unicode.org/Public/4.1.0/ucd/Scripts.txt
	http://www.unicode.org/reports/tr24/
*/

static UCS2_Short const UCS2_Scripts[U_SCRIPTS_UCS2] =
#include "Includes/Scripts.h"

static UCS4_Short const UCS4_Scripts[U_SCRIPTS_UCS4] =
#include "Includes/Scripts_UCS4.h"


U_PropertyValue UCS1CharScriptProperty ( UCS1Char n )
{
	return UCS2_GetPropertyValue( n, UCS2_Scripts, U_SCRIPTS_UCS1 );
}

U_PropertyValue UCS2CharScriptProperty ( UCS2Char n )
{
	return UCS2_GetPropertyValue( n, UCS2_Scripts, n>>8 ? U_SCRIPTS_UCS2 : U_SCRIPTS_UCS1 );
}

U_PropertyValue UCS4CharScriptProperty ( UCS4Char n )
{
	return n>>16 ? UCS4_GetPropertyValue( n, UCS4_Scripts, U_SCRIPTS_UCS4 )
	: UCS2_GetPropertyValue( UCS2Char(n), UCS2_Scripts, U_SCRIPTS_UCS2 );
}




/*	------------------------------------------------------------------------------------------
	Unicode Character -> Canonical Combining Class (ccc)
	(Enumerated Property)

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Canonical_Combining_Class


*/
static UCS2_Short const UCS2_Ccc[U_CCC_UCS2] =
#include "Includes/CanonicalCombiningClass.h"

static UCS4_Short const UCS4_Ccc[U_CCC_UCS4] =
#include "Includes/CanonicalCombiningClassUCS4.h"

U_PropertyValue	UCS2CharCccProperty ( UCS2Char n )
{
	return n<U_CCC_UCS2_START ? U_ccc_0 : UCS2_GetPropertyValue( n, UCS2_Ccc, U_CCC_UCS2 );
}

U_PropertyValue	UCS4CharCccProperty ( UCS4Char n )
{
	return n>>16 ?
		n<U_CCC_UCS4_START || n>=U_CCC_UCS4_END ?
			U_ccc_defaultvalue
			: UCS4_GetPropertyValue( n, UCS4_Ccc, U_CCC_UCS4 )
		: n<U_CCC_UCS2_START || n>=U_CCC_UCS2_END ?
			U_ccc_defaultvalue
			: UCS2_GetPropertyValue( UCS2Char(n), UCS2_Ccc, U_CCC_UCS2 );
}




/*	------------------------------------------------------------------------------------------
	Unicode Character -> General Category
	(Enumerated Property)

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#General_Category_Values

	Note:	UCS2_GC_Table[] enthält die speziellen Einträge
				• U_gc_luello für Lu on even position, Ll on odd position und
				• U_gc_luolle für Lu on odd  position, Lo on even position
			da es einige Bereiche gibt, in denen Ll und Lu alternieren.
			Diese kommen nur im Bereich für UCS2 ohne UCS1 vor.
*/
static UCS2_Short const UCS2_GC_Table[U_GC_UCS2] =
#include "Includes/GeneralCategory.h"

static UCS4_Short const UCS4_GC_Table[U_GC_UCS4] =
#include "Includes/GeneralCategory_UCS4.h"


U_PropertyValue	UCS1CharGeneralCategory ( UCS1Char n )
{
	return UCS2_GetPropertyValue( n, UCS2_GC_Table, U_GC_UCS1 );
}

U_PropertyValue	UCS2CharGeneralCategory ( UCS2Char n )
{
	if (!(n>>8)) return UCS2_GetPropertyValue( n, UCS2_GC_Table, U_GC_UCS1 );
	U_PropertyValue v = UCS2_GetPropertyValue( n, UCS2_GC_Table, U_GC_UCS2 );
	return int8(v) >= 0 ? v : int8(v) == U_gc_luello && n&1 ? U_gc_ll : U_gc_lu;
}

U_PropertyValue	UCS4CharGeneralCategory ( UCS4Char n )
{
	if (!(n>>8)) return UCS2_GetPropertyValue( UCS2Char(n), UCS2_GC_Table, U_GC_UCS1 );
	if ( n>>16 ) return UCS4_GetPropertyValue( n, UCS4_GC_Table, U_GC_UCS4 );
	U_PropertyValue v = UCS2_GetPropertyValue( UCS2Char(n), UCS2_GC_Table, U_GC_UCS2 );
	return int8(v) >= 0 ? v : int8(v) == U_gc_luello && n&1 ? U_gc_ll : U_gc_lu;
}



/*	------------------------------------------------------------------------------------------
	Simple Case Mapping

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Case_Mappings
*/

static UCS2_Short const UCS2_SUC_Even_Table[] =
#include "Includes/SimpleUppercase_Even.h"

static UCS2_Short const UCS2_SLC_Even_Table[] =
#include "Includes/SimpleLowercase_Even.h"

static UCS2_Short const UCS2_SUC_Odd_Table[] =
#include "Includes/SimpleUppercase_Odd.h"

static UCS2_Short const UCS2_SLC_Odd_Table[] =
#include "Includes/SimpleLowercase_Odd.h"

#if U_STC_UCS2 != 12
static UCS2_Short const UCS2_STC_Table[] =
#include "Includes/SimpleTitlecase.h"
#endif

#if U_SUC_UCS4 != 3
static UCS4_Short const UCS4_SUC_Table[] =
#include "Includes/SimpleUppercase_UCS4.h"
#endif

#if U_SLC_UCS4 != 3
static UCS4_Short const UCS4_SLC_Table[] =
#include "Includes/SimpleLowercase_UCS4.h"
#endif

#define N3(C)	C,C+1,C+2
#define N4(C)	C,C+1,C+2,C+3
#define N7(C)	N4(C),N3(C+4)
#define N8(C)	N4(C),N4(C+4)
#define N16(C)	N8(C),N8(C+8)
#define N32(C)	N16(C),N16(C+16)
UCS1Char const UCS1_SUC_Table[0x100] = {
	N32(0), N32(0x20), N32(0x40),
	0x60, N7(0x41), N8(0x48),
	N8(0x50), N3(0x58), 0x7b, N4(0x7c),				// uc(ß) = ß
	N32(0x80), N32(0xa0), N32(0xc0),				// uc(µ) = µ	da: Μ = ucs2
	N16(0xc0), N7(0xd0), 0xf7, N7(0xd8), 'Y' };		// uc(ÿ) = Y	da: Ÿ = ucs2
UCS1Char const UCS1_SLC_Table[0x100] = {
	N32(0), N32(0x20),
	0x40, N7(0x61), N8(0x68),
	N8(0x70), N3(0x78), 0x5b, N4(0x5c),
	N32(0x60), N32(0x80), N32(0xa0),
	N16(0xe0), N7(0xf0), 0xd7, N7(0xf8), 0xdf,
	N32(0xe0) };


inline UCS2Char ucs2_simple_uc ( UCS2Char n )
{	return n + UCS2_GetPropertyValue( n, n&1?UCS2_SUC_Odd_Table:UCS2_SUC_Even_Table, n&1?U_SUC_ODD_UCS2:U_SUC_EVEN_UCS2 ); }

inline UCS2Char ucs2_simple_lc ( UCS2Char n )
{	return n + UCS2_GetPropertyValue( n, n&1?UCS2_SLC_Odd_Table:UCS2_SLC_Even_Table, n&1?U_SLC_ODD_UCS2:U_SLC_EVEN_UCS2 ); }

inline UCS4Char ucs4_simple_uc ( UCS4Char n )
#if U_SUC_UCS4 != 3
{	return n + UCS4_GetPropertyValue( n, UCS4_SUC_Table, U_SUC_UCS4 ); }
#else
{	return ulong(n-U_SUC_UCS4_START) < ulong(U_SUC_UCS4_END-U_SUC_UCS4_START) ? n-40 : n; }
#endif

inline UCS4Char ucs4_simple_lc ( UCS4Char n )
#if U_SLC_UCS4 != 3
{	return n + UCS4_GetPropertyValue( n, UCS4_SLC_Table, U_SLC_UCS4 ); }
#else
{	return ulong(n-U_SLC_UCS4_START) < ulong(U_SLC_UCS4_END-U_SLC_UCS4_START) ? n+40 : n; }
#endif


UCS2Char ucs2_simple_uppercase ( UCS2Char n )
{
	return ucs2_simple_uc(n);
}

UCS4Char ucs4_simple_uppercase ( UCS4Char n )
{
	return n>>16 ? ucs4_simple_uc(n) : ucs2_simple_uc(UCS2Char(n));
}

UCS2Char ucs2_simple_lowercase ( UCS2Char n )
{
	return ucs2_simple_lc(n);
}

UCS4Char ucs4_simple_lowercase ( UCS4Char n )
{
	return n>>16 ? ucs4_simple_lc(n) : ucs2_simple_lc(UCS2Char(n));
}

UCS2Char ucs2_simple_titlecase ( UCS2Char n )
{
#if U_STC_UCS2 == 12	//	01C4/5/6 -> 01C5, 01C7/8/9 -> 01C8, 01CA/B/C -> 01CB, 01F1/2/3 -> 01F2
	return uint16(n-0x01C4) >= uint16(0x01F4-0x01C4) || uint16(n-0x01cd) < uint16(0x01f1-0x01cd)
			? ucs2_simple_uc(n) : 0x01c5 + (n-0x01c4)/3;
#else
	if n>=U_STC_UCS2_START && n<U_STC_UCS2_END
	{
		int a=0, e=U_STC_UCS2; do { int i=(a+e)/2; if( n<UCS2_STC_Table[i].Code ) e=i; else a=i; } while (a<e-1);
		if( n==UCS2_STC_Table[a].Code) return UCS2Char( UCS2_STC_Table[a].Value );
	}
	return ucs2_simple_uc(n);
#endif
}

UCS4Char ucs4_simple_titlecase ( UCS4Char n )
{
	return n>>16 ? ucs4_simple_uc(n)				// solange es keine TC im UCS4-Bereich gibt ((wird im Script geprüft))
				 : ucs2_simple_titlecase(UCS2Char(n));
}



/*	------------------------------------------------------------------------------------------
	Numeric Value

	CodedValue = %BBBB.BBBB.EEEE.DDDD	->	Value = BBBBBBBB * 10eEEEE / DDDD
	CodedValue == 0						->	NaN (kommt 2x vor)
	Count>1								->	Value[i] = (BBBBBBBB+i) * 10eEEEE / DDDD
*/

struct UCS2_NumVal { UCS2Char StartCode; uchar GeneralCategory; uchar Count; uint16 CodedValue; };
struct UCS4_NumVal { UCS4Char StartCode; uchar GeneralCategory; uchar Count; uint16 CodedValue; };

static UCS2_NumVal const UCS2_NumVal_Table[U_NUMERIC_UCS2] =
#include "Includes/NumericValue.h"

static UCS4_NumVal const UCS4_NumVal_Table[U_NUMERIC_UCS4] =
#include "Includes/NumericValue_UCS4.h"


inline uint UCS2_GetNumValIndex ( UCS2Char n, uint e )
{
	uint a=0; do { uint i=(a+e)/2; if( n<UCS2_NumVal_Table[i].StartCode ) e=i; else a=i; } while (a+1<e);
	return a;
}

inline uint UCS4_GetNumValIndex ( UCS4Char n, uint e )
{
	uint a=0; do { uint i=(a+e)/2; if( n<UCS4_NumVal_Table[i].StartCode ) e=i; else a=i; } while (a+1<e);
	return a;
}


/*	get decimal digit value
	Nur für Dezimalziffern gedacht: GC=Nd. No Checking.
	Non-Digits, auch GC=No und Nl, resultieren idR. in sinnlosen Rückgabewerten.
*/
uint ucs2_get_digitvalue ( UCS2Char n )
{
	uint i = UCS2_GetNumValIndex( n, U_NUMERIC_UCS2 );
	uint d = n - UCS2_NumVal_Table[i].StartCode;
	return (UCS2_NumVal_Table[i].CodedValue>>8) + d;
}

uint ucs4_get_digitvalue( UCS4Char n )
{
	if(!(n>>16)) return ucs2_get_digitvalue(UCS2Char(n));
	uint i = UCS4_GetNumValIndex( n, U_NUMERIC_UCS4 );
	uint d = n - UCS4_NumVal_Table[i].StartCode;
	return (UCS4_NumVal_Table[i].CodedValue>>8) + d;
}


/*	decode coded numeric value from table
	return value or NaN
	note: NaN kommt nur für 2 Einträge vor:
		  0x09F8 = BENGALI CURRENCY NUMERATOR ONE LESS THAN THE DENOMINATOR
		  0x2183 = ROMAN NUMERAL REVERSED ONE HUNDRED
		  beide haben CodedValue = 0x0000 und Count=1
		  => decoded_value() wird NaN zurückgeben
	note: there is one negative value:
		  TIBETAN DIGIT HALF ZERO {0x0F33, U_gc_no, 1,  0xFF02}
		  which equals -0.5
*/
static int const dec[] = { 1,10,100,1000,10000 };

inline float decoded_value ( uint coded_value, uint offset )
{
	int N = int8(coded_value >> 8);
	float rval = N + int(offset);

	uint E = coded_value & 0x00F0;
	if (E!=0) rval *= dec[E>>4];

	uint D = coded_value & 0x000F;
	if (D!=1) rval /= D;

	return rval;
}

float ucs2_get_numericvalue ( UCS2Char n )		// returns Value or NaN
{
	uint i = UCS2_GetNumValIndex( n, U_NUMERIC_UCS2 );
	uint d = n - UCS2_NumVal_Table[i].StartCode;
	if (d >= UCS2_NumVal_Table[i].Count) return NAN;
	return decoded_value(UCS2_NumVal_Table[i].CodedValue,d);
}

float ucs4_get_numericvalue ( UCS4Char n )		// returns Value or NaN
{
	if (!(n>>16)) return ucs2_get_numericvalue(UCS2Char(n));
	uint i = UCS4_GetNumValIndex( n, U_NUMERIC_UCS4 );
	uint d = n - UCS4_NumVal_Table[i].StartCode;
	if (d >= UCS4_NumVal_Table[i].Count) return NAN;
	return decoded_value(UCS4_NumVal_Table[i].CodedValue,d);
}















/* ********************************************************************************
					CLASSIFY CHARACTER
******************************************************************************** */


/* ----	is char printable ? -------------------------
		also non-latin
		mark_nonspacing and mark_enclosing (and mark_spacing) are also marked 'printable',
		though they should be better combined with their preceding base letter.
*/
bool ucs4_is_printable ( UCS4Char n )		/*checked*/
{
//	if( n < 0x700 ) return n!=0x7f && (n&~0x80)>=0x20;
	U_PropertyValue m = UCS4CharGeneralCategory(n);
	return m>=U_gc_letter /* && m<=U_gc_space_separator */
		&& !uchar_in_range(U_gc_separator,m,U_gc_paragraph_separator);	// line and paragraph separators
	//	&& !uchar_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark);	// combining letters
}















