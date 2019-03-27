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

#include <math.h>
#include "Libraries/kio/kio.h"
#include "ucs4.h"


namespace ucs4 {

#ifndef NAN
static const float NAN (0.0f/0.0f);
#endif

struct UCS2_Short { ucs2char code; int16 value; };
struct UCS4_Short { ucs4char code; int16 value; };

struct UCS2Range_Short { ucs2char startcode; ucs2char endcode; int16 value; };
struct UCS4Range_Short { ucs4char startcode; ucs4char endcode; int16 value; };

struct UCS2_NumVal { ucs2char startcode; uchar generalcategory; uchar count; int16 codedvalue; };
struct UCS4_NumVal { ucs4char startcode; uchar generalcategory; uchar count; int16 codedvalue; };

#include "Libraries/Unicode/Includes/GlobalConstants.h"

static UCS2_NumVal const UCS2_NumVal_Table[U_NUMERIC_UCS2] =
#include "Libraries/Unicode/Includes/NumericValue.h"

static UCS4_NumVal const UCS4_NumVal_Table[U_NUMERIC_UCS4] =
#include "Libraries/Unicode/Includes/NumericValue_UCS4.h"

static UCS2Range_Short const UCS2_EA_Table[U_EA_UCS2] =
#include "Libraries/Unicode/Includes/EastAsianWidth.h"

static UCS4Range_Short const UCS4_EA_Table[U_EA_UCS4] =
#include "Libraries/Unicode/Includes/EastAsianWidth_UCS4.h"

static UCS4_Short const Blocks[U_BLOCKS_UCS4] =
#include "Libraries/Unicode/Includes/Blocks.h"

static UCS2_Short const UCS2_Scripts[U_SCRIPTS_UCS2] =
#include "Libraries/Unicode/Includes/Scripts.h"

static UCS4_Short const UCS4_Scripts[U_SCRIPTS_UCS4] =
#include "Libraries/Unicode/Includes/Scripts_UCS4.h"

static UCS2_Short const UCS2_Ccc[U_CCC_UCS2] =
#include "Libraries/Unicode/Includes/CanonicalCombiningClass.h"

static UCS4_Short const UCS4_Ccc[U_CCC_UCS4] =
#include "Libraries/Unicode/Includes/CanonicalCombiningClassUCS4.h"

static UCS2_Short const UCS2_GC_Table[U_GC_UCS2] =
#include "Libraries/Unicode/Includes/GeneralCategory.h"

static UCS4_Short const UCS4_GC_Table[U_GC_UCS4] =
#include "Libraries/Unicode/Includes/GeneralCategory_UCS4.h"

static UCS2_Short const UCS2_SUC_Even_Table[] =
#include "Libraries/Unicode/Includes/SimpleUppercase_Even.h"

static UCS2_Short const UCS2_SLC_Even_Table[] =
#include "Libraries/Unicode/Includes/SimpleLowercase_Even.h"

static UCS2_Short const UCS2_SUC_Odd_Table[] =
#include "Libraries/Unicode/Includes/SimpleUppercase_Odd.h"

static UCS2_Short const UCS2_SLC_Odd_Table[] =
#include "Libraries/Unicode/Includes/SimpleLowercase_Odd.h"

#if U_STC_UCS2 != 12
static UCS2_Short const UCS2_STC_Table[] =
#include "Libraries/Unicode/Includes/SimpleTitlecase.h"
#endif

#if U_SUC_UCS4 != 3
static UCS4_Short const UCS4_SUC_Table[] =
#include "Libraries/Unicode/Includes/SimpleUppercase_UCS4.h"
#endif

#if U_SLC_UCS4 != 3
static UCS4_Short const UCS4_SLC_Table[] =
#include "Libraries/Unicode/Includes/SimpleLowercase_UCS4.h"
#endif



/*	UP: retrieve PropertyValue from UCS2 Character Table
		e = initial end value, e.g. Nelem(Table)
*/
inline Enum propertyValue ( ucs2char n, UCS2_Short const* Table, int e )
{
	int a=0; do { int i=(a+e)/2; if(n<Table[i].code) e=i; else a=i; } while (a<e-1);
	return Enum(Table[a].value);
}

/*	UP: retrieve PropertyValue from UCS4 Character Table
		e = initial end value, e.g. Nelem(Table)
*/
inline Enum propertyValue ( ucs4char n, UCS4_Short const* Table, int e )
{
	int a=0; do { int i=(a+e)/2; if(n<Table[i].code) e=i; else a=i; } while (a<e-1);
	return Enum(Table[a].value);
}



/*	------------------------------------------------------------------------------------------
	Map Unicode Character -> East Asian Width Property

	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Width
	http://www.unicode.org/reports/tr11/tr11-14.html
*/

inline int findEAIndex (ucs2char n, int e)
{
	int a=0; do { int i=(a+e)/2; if( n<UCS2_EA_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}
inline int findEAIndex (ucs4char n, int e)
{
	int a=0; do { int i=(a+e)/2; if( n<UCS4_EA_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}

Enum eaWidthProperty (ucs1char n)
{
	if (n<0xa1) return uint8_in_range(0x20,n,0x7e) ? U_ea_na : U_ea_n;
	int i = findEAIndex(ucs2char(n),U_EA_UCS1);
	return n <= UCS2_EA_Table[i].endcode ? Enum(UCS2_EA_Table[i].value) : U_ea_n;
}

Enum eaWidthProperty (ucs2char n)
{
	if (n<0xa1) return uint8_in_range(0x20,n,0x7e) ? U_ea_na : U_ea_n;
	int i = findEAIndex(n,U_EA_UCS2);
	return n <= UCS2_EA_Table[i].endcode ? Enum(UCS2_EA_Table[i].value) : U_ea_n;
}

Enum eaWidthProperty (ucs4char n)
{
	if (!(n>>16)) return eaWidthProperty(ucs2char(n)); if (n<U_EA_UCS4_START) return U_ea_n;
	int i = findEAIndex(n,U_EA_UCS4);
	return n <= UCS4_EA_Table[i].endcode ? Enum(UCS4_EA_Table[i].value) : U_ea_n;
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

uint printWidth (ucs4char n)
{
	Enum w = eaWidthProperty(n);
	if (w!=U_ea_n) return w==U_ea_w || w==U_ea_f ? 2 : 1;

	Enum m = generalCategory(n);								// ~ UCS4CharIsPrintable()
	return m>=U_gc_letter /* && m<=U_gc_space_separator */
		&& !uint8_in_range(U_gc_separator,m,U_gc_paragraph_separator)	// line and paragraph separators
		&& !uint8_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark);	// combining letters
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

Enum blockProperty (ucs4char n)	{ return propertyValue( n, Blocks, U_BLOCKS_UCS4 ); }


/*	------------------------------------------------------------------------------------------
	Unicode Character -> Script Property

	http://www.unicode.org/Public/4.1.0/ucd/Scripts.txt
	http://www.unicode.org/reports/tr24/
*/

Enum scriptProperty (ucs1char n)	{ return propertyValue(ucs2char(n), UCS2_Scripts, U_SCRIPTS_UCS1); }
Enum scriptProperty (ucs2char n)	{ return propertyValue(n, UCS2_Scripts, n>>8 ? U_SCRIPTS_UCS2 : U_SCRIPTS_UCS1); }
Enum scriptProperty (ucs4char n)	{ return n>>16 ? propertyValue(n, UCS4_Scripts, U_SCRIPTS_UCS4)
													: propertyValue(ucs2char(n), UCS2_Scripts, U_SCRIPTS_UCS2); }


/*	------------------------------------------------------------------------------------------
	Unicode Character -> Canonical Combining Class (ccc)
	(Enumerated Property)

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Canonical_Combining_Class


*/

Enum cccProperty (ucs2char n)
{
	return n<U_CCC_UCS2_START ? U_ccc_0 : propertyValue( n, UCS2_Ccc, U_CCC_UCS2 );
}
Enum cccProperty (ucs4char n)
{
	return n>>16
	? n<U_CCC_UCS4_START||n>=U_CCC_UCS4_END ? U_ccc_defaultvalue : propertyValue( n, UCS4_Ccc, U_CCC_UCS4 )
	: n<U_CCC_UCS2_START||n>=U_CCC_UCS2_END ? U_ccc_defaultvalue : propertyValue( ucs2char(n), UCS2_Ccc, U_CCC_UCS2 );
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

Enum generalCategory (ucs1char n)
{
	return propertyValue( n, UCS2_GC_Table, U_GC_UCS1 );
}
Enum generalCategory (ucs2char n)
{
	if (!(n>>8)) return propertyValue( n, UCS2_GC_Table, U_GC_UCS1 );
	Enum v = propertyValue( n, UCS2_GC_Table, U_GC_UCS2 );
	return int8(v)>=0 ? v : int8(v)==U_gc_luello&&n&1 ? U_gc_ll : U_gc_lu;
}
Enum generalCategory (ucs4char n)
{
	if (n <= 0xff) return propertyValue( ucs2char(n), UCS2_GC_Table, U_GC_UCS1 );
	if (n > 0xffff) return propertyValue( n, UCS4_GC_Table, U_GC_UCS4 );
	Enum v = propertyValue( ucs2char(n), UCS2_GC_Table, U_GC_UCS2 );
	return int8(v) >= 0 ? v : int8(v) == U_gc_luello && n&1 ? U_gc_ll : U_gc_lu;
}


/*	------------------------------------------------------------------------------------------
	Simple Case Mapping

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Case_Mappings
*/

inline ucs2char simple_uc (ucs2char n)
{
	return n&1
		? n + propertyValue(n, UCS2_SUC_Odd_Table,  U_SUC_ODD_UCS2 )
		: n + propertyValue(n, UCS2_SUC_Even_Table, U_SUC_EVEN_UCS2);
}

inline ucs2char simple_lc (ucs2char n)
{
	return n&1
		? n + propertyValue(n, UCS2_SLC_Odd_Table,  U_SLC_ODD_UCS2 )
		: n + propertyValue(n, UCS2_SLC_Even_Table, U_SLC_EVEN_UCS2);
}

inline ucs4char simple_uc (ucs4char n)
{
	#if U_SUC_UCS4 != 3
	return n + UCS4_GetPropertyValue( n, UCS4_SUC_Table, U_SUC_UCS4 );
	#else
	return is_in_range(U_SUC_UCS4_START, n, U_SUC_UCS4_END-1) ? n-40 : n;
	//return uint32(n-U_SUC_UCS4_START) < uint32(U_SUC_UCS4_END-U_SUC_UCS4_START) ? n-40 : n;
	#endif
}

inline ucs4char simple_lc (ucs4char n)
{
	#if U_SLC_UCS4 != 3
	return n + UCS4_GetPropertyValue( n, UCS4_SLC_Table, U_SLC_UCS4 );
	#else
	return is_in_range(U_SLC_UCS4_START, n, U_SLC_UCS4_END-1) ? n+40 : n;
	//return uint32(n-U_SLC_UCS4_START) < uint32(U_SLC_UCS4_END-U_SLC_UCS4_START) ? n+40 : n;
	#endif
}

ucs2char simple_uppercase (ucs2char n)	{ return simple_uc(n); }
ucs4char simple_uppercase (ucs4char n)	{ return n > 0xffff ? simple_uc(n) : simple_uc(ucs2char(n)); }
ucs2char simple_lowercase (ucs2char n)	{ return simple_lc(n); }
ucs4char simple_lowercase (ucs4char n)	{ return n > 0xffff ? simple_lc(n) : simple_lc(ucs2char(n)); }

ucs2char simple_titlecase (ucs2char n)
{
	#if U_STC_UCS2 == 12	//	01C4/5/6 -> 01C5, 01C7/8/9 -> 01C8, 01CA/B/C -> 01CB, 01F1/2/3 -> 01F2
	return not_in_range(0x01C4, n, 0x01F4-1) || is_in_range(0x01cd, n, 0x01f1-1)
	//return uint16(n-0x01C4) >= uint16(0x01F4-0x01C4) || uint16(n-0x01cd) < uint16(0x01f1-0x01cd)
			? simple_uc(n) : 0x01c5 + (n-0x01c4)/3;
	#else
	if n>=U_STC_UCS2_START && n<U_STC_UCS2_END
	{
		int a=0, e=U_STC_UCS2; do { int i=(a+e)/2; if( n<UCS2_STC_Table[i].Code ) e=i; else a=i; } while (a<e-1);
		if( n==UCS2_STC_Table[a].Code) return ucs2char( UCS2_STC_Table[a].Value );
	}
	return ucs2_simple_uc(n);
	#endif
}

ucs4char simple_titlecase (ucs4char n)
{
	return n>>16 ? simple_uc(n)		// solange es keine TC im UCS4-Bereich gibt ((wird im Script geprüft))
				 : simple_titlecase(ucs2char(n));
}


/*	------------------------------------------------------------------------------------------
	Numeric Value

	CodedValue = %BBBB.BBBB.EEEE.DDDD	->	Value = BBBBBBBB * 10eEEEE / DDDD
	CodedValue == 0						->	NaN (kommt 2x vor)
	Count>1								->	Value[i] = (BBBBBBBB+i) * 10eEEEE / DDDD
*/

inline int numValIndex (ucs2char n, int e)
{
	int a=0; do { int i=(a+e)/2; if( n<UCS2_NumVal_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}
inline int numValIndex (ucs4char n, int e)
{	int a=0; do { int i=(a+e)/2; if( n<UCS4_NumVal_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}

/*	get decimal digit value
	Nur für Dezimalziffern gedacht: GC=Nd. No Checking.
	Non-Digits, auch GC=No und Nl, resultieren idR. in sinnlosen Rückgabewerten.
*/
uint _dec_digit_value (ucs2char n)
{
	int i = numValIndex( n, U_NUMERIC_UCS2 );
	uint d = n - UCS2_NumVal_Table[i].startcode;
	return (uint16(UCS2_NumVal_Table[i].codedvalue)>>8) + d;
}

uint _dec_digit_value (ucs4char n)
{
	if(n <= 0xffff) return _dec_digit_value(ucs2char(n));
	int i = numValIndex(n, U_NUMERIC_UCS4);
	uint d = n - UCS4_NumVal_Table[i].startcode;
	return (uint16(UCS4_NumVal_Table[i].codedvalue)>>8) + d;
}

/*	decode coded numeric value from table
	return value or NaN
	note: NaN kommt nur für 2 Einträge vor:
		  0x09F8 = BENGALI CURRENCY NUMERATOR ONE LESS THAN THE DENOMINATOR
		  0x2183 = ROMAN NUMERAL REVERSED ONE HUNDRED
		  beide haben CodedValue = 0x0000 und Count=1
		  => decoded_value() wird NaN zurückgeben
*/
inline float decoded_value (int coded_value, uint offset)
{
	static float const dec[] = { 1,10,100,1000,10000, 0,0,0 };
	int  N = coded_value >> 8;
	uint E =(coded_value & 0x0070) >> 4;
	int  D = coded_value & 0x000F;
	return (N+int(offset)) * dec[E] / D;
}

float _numeric_value (ucs2char n)		// returns Value or NaN
{
	int i = numValIndex( n, U_NUMERIC_UCS2 );
	uint d = n - UCS2_NumVal_Table[i].startcode;
	if( d >= UCS2_NumVal_Table[i].count ) return NAN;
	return decoded_value(UCS2_NumVal_Table[i].codedvalue,d);
}

float _numeric_value (ucs4char n)		// returns Value or NaN
{
	if (n <= 0xffff) return _numeric_value(ucs2char(n));
	int i = numValIndex(n, U_NUMERIC_UCS4);
	uint d = n - UCS4_NumVal_Table[i].startcode;
	if (d >= UCS4_NumVal_Table[i].count) return NAN;
	return decoded_value(UCS4_NumVal_Table[i].codedvalue,d);
}


/* ********************************************************************************
					CLASSIFY CHARACTER
******************************************************************************** */

/* ----	is char printable ? -------------------------
		also non-latin
		mark_nonspacing and mark_enclosing (and mark_spacing) are also marked 'printable',
		though they should be better combined with their preceding base letter.
*/
bool _is_printable (ucs4char n)
{
//	if( n < 0x700 ) return n!=0x7f && (n&~0x80)>=0x20;
	Enum m = generalCategory(n);
	return m>=U_gc_letter /* && m<=U_gc_space_separator */
		&& !uint8_in_range(U_gc_separator,m,U_gc_paragraph_separator);	// line and paragraph separators
	//	&& !uchar_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark);	// combining letters
}


} // namespace













