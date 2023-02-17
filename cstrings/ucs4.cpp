// Copyright (c) 2002 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ucs4.h"
#include "ucs2.h"
#include <cmath>

#ifndef NAN
static const float NAN(0.0f / 0.0f);
#endif

// global constants:
#include "unicode/global_constants.h"


namespace ucs4
{
using namespace unicode;


// file "ucs4_general_category.h":
// statistics: count_ucs2 = 2859, thereof: used=2511, gaps=348
// => better include entries for the gaps than store the count
static const ucs4char gc_table_code[] = {
#define ADD_USED(C, N, GC) C,
#define ADD_NONE(C, N, GC) C,
#include "unicode/ucs4_general_category.h"
#undef ADD_USED
#undef ADD_NONE
};
static const GeneralCategory gc_table_category[] = {
#define ADD_USED(C, N, GC) GC,
#define ADD_NONE(C, N, GC) GC,
#include "unicode/ucs4_general_category.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs4_blocks.h":
static const ucs4char blk_table_code[] = {
#define ADD_USED(C, N, BLK) C,
#define ADD_NONE(C, N, BLK) C,
#include "unicode/ucs4_blocks.h"
#undef ADD_USED
#undef ADD_NONE
};
static const Block blk_table_block[] = {
#define ADD_USED(C, N, BLK) BLK,
#define ADD_NONE(C, N, BLK) BLK,
#include "unicode/ucs4_blocks.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs4_simple_lowercase.h":
static const ucs4char slc_table_code[] = {
#define ADD_USED(CODE, COUNT, DIS) CODE,
#define ADD_NONE(CODE, COUNT, DIS) CODE,
#include "unicode/ucs4_simple_lowercase.h"
#undef ADD_USED
#undef ADD_NONE
};
static const int16 slc_table_offset[] = {
#define ADD_USED(C, COUNT, DIS) DIS,
#define ADD_NONE(C, COUNT, DIS) DIS,
#include "unicode/ucs4_simple_lowercase.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs4_simple_uppercase.h":
static const ucs4char suc_table_code[] = {
#define ADD_USED(CODE, COUNT, DIS) CODE,
#define ADD_NONE(CODE, COUNT, DIS) CODE,
#include "unicode/ucs4_simple_uppercase.h"
#undef ADD_USED
#undef ADD_NONE
};
static const int16 suc_table_offset[] = {
#define ADD_USED(CODE, COUNT, DIS) DIS,
#define ADD_NONE(CODE, COUNT, DIS) DIS,
#include "unicode/ucs4_simple_uppercase.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs4_numeric_values.h":
struct NumvalEntry
{
	ucs4char start;
	uint16	 count;
	uint16	 dummy_for_padding; // TODO
	union
	{
		uint32 uint32value;
		uint16 uint16values[2];
		float  floatvalue;
	};
	NumvalEntry(ucs4char start, uint16 count, float value) : start(start), count(count), floatvalue(value) {}
	NumvalEntry(ucs4char start, uint16 count, uint16 value, uint16 incr) :
		start(start),
		count(count),
		uint16values {value, incr}
	{}
};

static const NumvalEntry numval_table[] = {
#define ADD_ONE(CODE, CNT, VALUE, INCR)	  NumvalEntry(CODE, CNT, float32(VALUE)),
#define ADD_NONE(CODE, CNT, VALUE, INCR)  /*gaps not included in table*/
#define ADD_RANGE(CODE, CNT, VALUE, INCR) NumvalEntry(CODE, CNT, VALUE, INCR),
#include "unicode/ucs4_numeric_values.h"
#undef ADD_ONE
#undef ADD_NONE
#undef ADD_RANGE
};


// file "ucs4_scripts.h":
static const ucs4char sc_table_code[] = {
#define ADD_USED(CODE, COUNT, SCRIPT) CODE,
#include "unicode/ucs4_scripts.h"
#undef ADD_USED
};
static const Script sc_table_script[] = {
#define ADD_USED(CODE, COUNT, SCRIPT) SCRIPT,
#include "unicode/ucs4_scripts.h"
#undef ADD_USED
};


static uint find_index(ucs4char n, const ucs4char table[], uint e) noexcept
{
	// helper: get index in table of code ranges
	//	e = initial end value = NELEM(table)

	uint a = 0;
	do {
		uint i = (a + e) / 2;
		if (n < table[i]) e = i;
		else a = i;
	}
	while (a + 1 < e);
	return a;
}

GeneralCategory general_category(ucs4char n) noexcept
{
	// get General Category:

	if (n <= 0xFFFF) return ucs2::general_category(ucs2char(n));
	uint i = find_index(n, gc_table_code, NELEM(gc_table_code));
	return gc_table_category[i];
}

/* ----	is char printable ? -------------------------
		mark_nonspacing and mark_enclosing (and mark_spacing) are also marked 'printable',
		though they should be better combined with their preceding base letter.
*/
bool _is_printable(ucs4char n) noexcept
{
	//	if ( n < 0x700 ) return n!=0x7f && (n&~0x80)>=0x20;

	GeneralCategory m = general_category(n);
	return m >= GcLetter										  /* && m <= GcSpaceSeparator */
		   && !gc_in_range(GcSeparator, m, GcParagraphSeparator); // line and paragraph separators
	//	&& !gc_in_range(GcEnclosingMark, m, GcNonspacingMark);	// combining letters
}

Block block_property(ucs4char n) noexcept
{
	// get coarse block of character
	//
	// Many of these blocks have the same name as one of the scripts because characters of that script
	// are primarily encoded in that block. However, blocks and scripts differ in the following ways:
	// 	• Blocks are simply ranges, and often contain code points that are unassigned.
	// 	• Characters from the same script may be in several different blocks.
	// 	• Characters from different scripts may be in the same block.
	// As a result, for mechanisms such as regular expressions, using script values produces more
	// meaningful results than simple matches based on block names.

	if (n <= 0xFFFF) return ucs2::block_property(ucs2char(n));
	uint i = find_index(n, blk_table_code, NELEM(blk_table_code));
	return blk_table_block[i];
}

ucs4char _to_lower(ucs4char n) noexcept
{
	if (n <= 0xFFFF) return ucs2::_to_lower(ucs2char(n));

	// Die Tabelle gilt nicht für GcLowercaseLetter Zeichen!
	// Andernfalls wachsen die SLC und SUC-Tabellen von 271+219 auf 1278+1245 Einträge
	// oder man muss getrennte Tabellen für gerade und ungerade Codes machen
	// wg. der Code-Ranges mit alternierenden Einträgen.

	if (general_category(n) == GcLowercaseLetter) return n; // doof...
	uint i = find_index(n, slc_table_code, NELEM(slc_table_code));
	return n + uint32(int32(slc_table_offset[i]));
}

ucs4char _to_upper(ucs4char n) noexcept
{
	if (n <= 0xFFFF) return ucs2::_to_upper(ucs2char(n));

	if (general_category(n) == GcUppercaseLetter) return n; // doof...
	uint i = find_index(n, suc_table_code, NELEM(suc_table_code));
	return n + uint32(int32(suc_table_offset[i]));
}

ucs4char _to_title(ucs4char n) noexcept
{
	static_assert(UCS4_STC_CNT == 1, ""); // only one "ADD_NONE" block
	if (n <= 0xFFFF) return ucs2::_to_title(ucs2char(n));


	//	Titlecase ist die Großschreibung des ersten Buchstaben eines Wortes
	//	wie es zB. im Englischen in Überschriften üblich ist.
	//	In den meisten Fällen ist Titlecase == Uppercase.

	if (general_category(n) == GcUppercaseLetter) return n; // doof...
	uint i = find_index(n, suc_table_code, NELEM(suc_table_code));
	return n + uint32(int32(suc_table_offset[i]));
}


static uint find_numval_index(ucs4char n) noexcept
{
	// helper: get index in numval_table[]

	uint e = NELEM(numval_table);
	uint a = 0;
	do {
		uint i = (a + e) / 2;
		if (n < numval_table[i].start) e = i;
		else a = i;
	}
	while (a + 1 < e);
	return a;
}

uint _dec_digit_value(ucs4char n) noexcept
{
	// get decimal digit value
	// ATTN: use only for decimal digits: GC=Nd. No Checking.
	//       Non-Digits like GC=No and Nl result meaningless values!

	if (n <= 0xFFFF) return ucs2::_dec_digit_value(ucs2char(n));

	uint i = find_numval_index(n);
	uint d = n - numval_table[i].start;
	return numval_table[i].uint16values[0] + d; // assuming Numval(code,count=10,value=0,incr=1)
}

float numeric_value(ucs4char n) noexcept
{
	// get numeric value of character
	// incl. Nd, No and Nl etc.
	// return NAN if character has no numeric value

	if (n < 0xB2) return is_in_range('0', n, '9') ? n - '0' : NAN;
	if (n <= 0xFFFF) return ucs2::numeric_value(ucs2char(n));

	uint			   i	  = find_numval_index(n);
	const NumvalEntry& numval = numval_table[i];
	uint			   d	  = n - numval.start;

	if (numval.count == 1) // => single value stored as float
	{
		return d == 0 ? numval.floatvalue : NAN;
	}
	else // => range stored as {value,increment}
	{
		if (d >= numval.count) return NAN;
		return numval.uint16values[0] + d * numval.uint16values[1];
	}
}

Script script_property(ucs4char n) noexcept
{
	// get script system of character
	// code ranges include unassigned character codes to reduce map size
	// if in doubt you must also check that GeneralCategory != GcUnassigned

	if (n <= 0xFFFF) return ucs2::script_property(ucs2char(n));
	uint i = find_index(n, sc_table_code, NELEM(sc_table_code));
	return sc_table_script[i];
}


#if 0



/*	------------------------------------------------------------------------------------------
	Map Unicode Character -> East Asian Width Property

	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Width
	http://www.unicode.org/reports/tr11/tr11-14.html
*/

inline int find_EA_index (ucs2char n, int e) noexcept
{
	int a=0; do { int i=(a+e)/2; if ( n<UCS2_EA_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}
inline int find_EA_index (ucs4char n, int e) noexcept
{
	int a=0; do { int i=(a+e)/2; if ( n<UCS4_EA_Table[i].startcode ) e=i; else a=i; } while (a<e-1);
	return a;
}

Property ea_width_property (ucs1char n) noexcept
{
	if (n<0xa1) return uint8_in_range(0x20,n,0x7e) ? U_ea_na : EaNeutral;
	int i = find_EA_index(ucs2char(n),U_EA_UCS1);
	return n <= UCS2_EA_Table[i].endcode ? Enum(UCS2_EA_Table[i].value) : EaNeutral;
}

Property ea_width_property (ucs2char n) noexcept
{
	if (n<0xa1) return uint8_in_range(0x20,n,0x7e) ? U_ea_na : EaNeutral;
	int i = find_EA_index(n,U_EA_UCS2);
	return n <= UCS2_EA_Table[i].endcode ? Enum(UCS2_EA_Table[i].value) : EaNeutral;
}

Property ea_width_property (ucs4char n) noexcept
{
	if (!(n>>16)) return ea_width_property(ucs2char(n)); if (n<U_EA_UCS4_START) return EaNeutral;
	int i = find_EA_index(n,U_EA_UCS4);
	return n <= UCS4_EA_Table[i].endcode ? Enum(UCS4_EA_Table[i].value) : EaNeutral;
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

uint print_width (ucs4char n) noexcept
{
	Enum w = ea_width_property(n);
	if (w!=EaNeutral) return w==U_ea_w || w==U_ea_f ? 2 : 1;

	Enum m = general_category(n);								// ~ UCS4CharIsPrintable()
	return m>=U_gc_letter /* && m<=U_gc_space_separator */
		&& !uint8_in_range(U_gc_separator,m,U_gc_paragraph_separator)	// line and paragraph separators
		&& !uint8_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark);	// combining letters
}


/*	------------------------------------------------------------------------------------------
	Unicode Character -> Canonical Combining Class (ccc)
	(Enumerated Property)

	http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Canonical_Combining_Class


*/

Enum ccc_property (ucs2char n) noexcept
{
	return n<U_CCC_UCS2_START ? CccNotReordered : property_value( n, UCS2_Ccc, U_CCC_UCS2 );
}

Enum ccc_property (ucs4char n) noexcept
{
	return n>>16
	? n<U_CCC_UCS4_START||n>=U_CCC_UCS4_END ? U_ccc_defaultvalue : property_value( n, UCS4_Ccc, U_CCC_UCS4 )
	: n<U_CCC_UCS2_START||n>=U_CCC_UCS2_END ? U_ccc_defaultvalue : property_value( ucs2char(n), UCS2_Ccc, U_CCC_UCS2 );
}

#endif


} // namespace ucs4
