// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "ucs2.h"
#include <cmath>

#ifndef NAN
static const float NAN(0.0f / 0.0f);
#endif

// global constants:
#include "unicode/global_constants.h"


namespace ucs2
{
using namespace unicode;


// file "ucs2_general_category.h":
// statistics: count_ucs2 = 2859, thereof: used=2511, gaps=348
// => better include entries for the gaps than store the count
static const ucs2char gc_table_code[] = {
#define ADD_USED(C, N, GC) C,
#define ADD_NONE(C, N, GC) C,
#include "unicode/ucs2_general_category.h"
#undef ADD_USED
#undef ADD_NONE
};
static const GeneralCategory gc_table_category[] = {
#define ADD_USED(C, N, GC) GC,
#define ADD_NONE(C, N, GC) GC,
#include "unicode/ucs2_general_category.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs2_blocks.h":
static const ucs2char blk_table_code[] = {
#define ADD_USED(C, N, BLK) C,
#define ADD_NONE(C, N, BLK) C,
#include "unicode/ucs2_blocks.h"
#undef ADD_USED
#undef ADD_NONE
};
static const Block blk_table_block[] = {
#define ADD_USED(C, N, BLK) BLK,
#define ADD_NONE(C, N, BLK) BLK,
#include "unicode/ucs2_blocks.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs2_simple_lowercase.h":
static const ucs2char slc_table_code[] = {
#define ADD_USED(C, N, DIS) C,
#define ADD_NONE(C, N, DIS) C,
#include "unicode/ucs2_simple_lowercase.h"
#undef ADD_USED
#undef ADD_NONE
};
static const uint16 slc_table_offset[] = {
#define ADD_USED(C, N, DIS) uint16(DIS),
#define ADD_NONE(C, N, DIS) DIS,
#include "unicode/ucs2_simple_lowercase.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs2_simple_uppercase.h":
static const ucs2char suc_table_code[] = {
#define ADD_USED(C, N, DIS) C,
#define ADD_NONE(C, N, DIS) C,
#include "unicode/ucs2_simple_uppercase.h"
#undef ADD_USED
#undef ADD_NONE
};
static const uint16 suc_table_offset[] = {
#define ADD_USED(C, N, DIS) uint16(DIS),
#define ADD_NONE(C, N, DIS) DIS,
#include "unicode/ucs2_simple_uppercase.h"
#undef ADD_USED
#undef ADD_NONE
};


// file "ucs2_numeric_values.h":
struct NumvalEntry
{
	ucs2char start;
	uint16	 count;
	union
	{
		uint32 uint32value;
		uint16 uint16values[2];
		float  floatvalue;
	};
	NumvalEntry(ucs2char start, uint16 count, float value) : start(start), count(count), floatvalue(value) {}
	NumvalEntry(ucs2char start, uint16 count, uint16 value, uint16 incr) :
		start(start),
		count(count),
		uint16values {value, incr}
	{}
};

static const NumvalEntry numval_table[] = {
#define ADD_ONE(CODE, CNT, VALUE, INCR)	  NumvalEntry(CODE, CNT, float32(VALUE)),
#define ADD_NONE(CODE, CNT, VALUE, INCR)  /*gaps not included in table*/
#define ADD_RANGE(CODE, CNT, VALUE, INCR) NumvalEntry(CODE, CNT, VALUE, INCR),
#include "unicode/ucs2_numeric_values.h"
#undef ADD_ONE
#undef ADD_NONE
#undef ADD_RANGE
};


// file "ucs2_scripts.h":
static const ucs2char sc_table_code[] = {
#define ADD_USED(CODE, COUNT, SCRIPT) CODE,
#include "unicode/ucs2_scripts.h"
#undef ADD_USED
};
static const Script sc_table_script[] = {
#define ADD_USED(CODE, COUNT, SCRIPT) SCRIPT,
#include "unicode/ucs2_scripts.h"
#undef ADD_USED
};


static uint find_index(ucs2char n, const ucs2char table[], uint e) noexcept
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

GeneralCategory general_category(ucs2char n) noexcept
{
	// get General Category:

	uint i = find_index(n, gc_table_code, NELEM(gc_table_code));
	return gc_table_category[i];
}

bool _is_printable(ucs2char n) noexcept
{
	// if ( n < 0x700 ) return n!=0x7f && (n&~0x80)>=0x20;

	GeneralCategory m = general_category(n);
	return m >= GcLetter										  /* && m <= GcSpaceSeparator */
		   && !gc_in_range(GcSeparator, m, GcParagraphSeparator); // line and paragraph separators
	//	&& !gc_in_range(GcEnclosingMark, m, GcNonspacingMark);	// combining letters
}

Block block_property(ucs2char n) noexcept
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

	uint i = find_index(n, blk_table_code, NELEM(blk_table_code));
	return blk_table_block[i];
}

ucs2char _to_lower(ucs2char n) noexcept
{
	// Die Tabelle gilt nicht für GcLowercaseLetter Zeichen!
	// Andernfalls wachsen die SLC und SUC-Tabellen von 271+219 auf 1278+1245 Einträge
	// oder man muss getrennte Tabellen für gerade und ungerade Codes machen
	// wg. der Code-Ranges mit alternierenden Einträgen.

	if (general_category(n) == GcLowercaseLetter) return n; // doof...
	uint i = find_index(n, slc_table_code, NELEM(slc_table_code));
	return n + slc_table_offset[i]; // n+offset may overflow / wrap inside ucs2char range
}

ucs2char _to_upper(ucs2char n) noexcept
{
	if (general_category(n) == GcUppercaseLetter) return n; // doof...
	uint i = find_index(n, suc_table_code, NELEM(suc_table_code));
	return n + suc_table_offset[i]; // n+offset may overflow / wrap inside ucs2char range
}

ucs2char _to_title(ucs2char n) noexcept
{
	static_assert(UCS2_STC_CNT == 19, "");
	static_assert(UCS2_STC_ANF == 0x01C4, "");
	static_assert(UCS2_STC_END == 0x1100, "");

	/*	Titlecase ist die Großschreibung des ersten Buchstaben eines Wortes
	wie es zB. im Englischen in Überschriften üblich ist.
	In den meisten Fällen ist Titlecase == Uppercase.

	Ausnahmen (Unicode 12.0):

	Latin character tupels DZ w. caron, LJ, NJ and DZ:
	These are the only letters which actually _have_ a titlecase mapping:
	ADD_USED(0x01C4,1,1)  DZ
	ADD_USED(0x01C5,1,0)  Dz
	ADD_USED(0x01C6,1,-1) dz
	ADD_USED(0x01C7,1,1)  LJ
	ADD_USED(0x01C8,1,0)  Lj
	ADD_USED(0x01C9,1,-1) lj
	ADD_USED(0x01CA,1,1)  NJ
	ADD_USED(0x01CB,1,0)  Nj
	ADD_USED(0x01CC,1,-1) nj
	ADD_NONE(0x01CD,36,0) gap: letters
	ADD_USED(0x01F1,1,1)  DZ
	ADD_USED(0x01F2,1,0)  Dz
	ADD_USED(0x01F3,1,-1) dz

	Georgean lowercase letters:
	these letters map to itself, that is lowercase, not uppercase:
	ADD_USED(0x10D0,43,0)
	ADD_NONE(0x10FB,2,0)  gap: non-letters
	ADD_USED(0x10FD,3,0)
*/
	if (is_in_range(0x01C4, n, 0x1100 - 1))
	{
		if (n >= 0x10D0) return n; // Georgean lowercase letters with titlecase = lowercase
		if (n >= 0x01F4) return _to_upper(n);
		if (n >= 0x01F1) return 0x01F2; // Latin Dz
		if (n >= 0x01CD) return _to_upper(n);
		if (n >= 0x01CA) return 0x01CB; // Latin Nj
		if (n >= 0x01C7) return 0x01C8; // Latin Lj
		else return 0x01C5;				// Latin Dz with caron
	}

	return _to_upper(n);
}

static uint find_numval_index(ucs2char n) noexcept
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

uint _dec_digit_value(ucs2char n) noexcept
{
	// get decimal digit value
	// ATTN: use only for decimal digits: GC=Nd. No Checking.
	//       Non-Digits like GC=No and Nl result meaningless values!

	uint i = find_numval_index(n);
	uint d = n - numval_table[i].start;
	return numval_table[i].uint16values[0] + d; // assuming Numval(code,count=10,value=0,incr=1)
}

float numeric_value(ucs2char n) noexcept
{
	// get numeric value of character
	// incl. Nd, No and Nl etc.
	// return NAN if character has no numeric value

	if (n < 0xB2) return is_in_range('0', n, '9') ? n - '0' : NAN;

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

Script script_property(ucs2char n) noexcept
{
	// get script system of character
	// code ranges include unassigned character codes to reduce map size
	// if in doubt you must also check that GeneralCategory != GcUnassigned

	uint i = find_index(n, sc_table_code, NELEM(sc_table_code));
	return sc_table_script[i];
}


#if 0
static const UCS2_Property ccc_table[] =
  #include "unicode/CanonicalCombiningClass.h"

static const UCS2Range_Property ea_width_table[] =
  #include "unicode/EastAsianWidth.h"

// Canonical Combining Class:
Property ccc_property (ucs2char n) noexcept
{
	// get Canonical Combining Class (ccc) of character
	// (Enumerated Property)

	// http://www.unicode.org/Public/4.1.0/ucd/UnicodeData.txt
	// http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Canonical_Combining_Class

	return property_value(n, ccc_table, NELEM(ccc_table));
}


inline uint find_ea_width_index (ucs2char n) noexcept
{
	uint e = NELEM(ea_width_table);
	uint a = 0; do { uint i=(a+e)/2; if (n<ea_width_table[i].startcode) e=i; else a=i; } while (a+1<e);
	return a;
}

Property ea_width_property (ucs2char n) noexcept
{
	// East Asian Width:
	// Map Unicode Character -> East Asian Width Property
	//
	// http://www.unicode.org/Public/4.1.0/ucd/UCD.html#Width
	// http://www.unicode.org/reports/tr11/tr11-14.html

	uint i = find_ea_width_index(n);
	return n <= ea_width_table[i].endcode ? ea_width_table[i].value : EaNeutral;
}

uint print_width (ucs2char n) noexcept
{
	// print width of char in monospaced font  ->  0, 1, or 2
	//
	// Narrow				always 1: non-E.A. scripts
	//						western script behavior, e.g. for line breaks. rotated in vertical text.
	//
	// Half Width			always 1: like Narrow, but probably from E.A. script
	// 						behave like ideographs. are rotated in vertical text.
	//
	// Wide					always 2: E.A. scripts only, e.g. HAN ideographs
	// 						behave like ideographs. not rotated in v. text, except some few wide punctuations are rotated.
	//
	// Full Width			always 2: like Wide, but probably from non-E.A. script
	//
	// Ambiguous			prefer 1: 1 or 2 in E.A. legacy char set, 1 in non-E.A. script. e.g. Greek, Cyrillic, Maths
	//						ambiguous quotation marks: Wide/Narrow dep. on enclosed text
	// 						behave like Wide or Narrow. Narrow by default.
	//
	// Neutral (non-E.A.)	mostly 1: non-E.A. which do not occur in E.A. legacy char sets.
	//						most of them behave like Narrow.
	// 						unasigned code points.
	//
	// Combining marks are classified by their primary use. E.A.Width of combining marks does not specify print width!
	//						In particular, nonspacing marks do not possess any actual advance width.
	//
	// http://www.unicode.org/reports/tr11/tr11-14.html

	Property w = ea_width_property(n);
	if (w != EaNeutral) return w==U_ea_w || w==U_ea_f ? 2 : 1;

	// --> 1 if printable, else 0:
	Property m = general_category(n);								// ~ UCS4CharIsPrintable()
	return m >= U_gc_letter /* && m<=U_gc_space_separator */
		&& !gc_in_range(U_gc_separator,m,U_gc_paragraph_separator)	// line and paragraph separators
		&& !gc_in_range(U_gc_enclosing_mark,m,U_gc_nonspacing_mark); // combining letters
}

#endif


}; // namespace ucs2
