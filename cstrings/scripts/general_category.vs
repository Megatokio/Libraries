// this file is included from "create_all.vs"


/* read GeneralCategory from unicodeData{}
   combine into ranges
   and write header files for UCS-2 and UCS-4

   these contain macros ADD_USED() and ADD_NONE()
   arguments: first_codepoint, count and general_category

   note: legal unicode values are $000000 to $010FFFF
   		 codes $xxFFFE and $xxFFFF are so-called 'noncharacters' (never assigned by Unicode.org)
   		 codes $00FDD0 to $00FDEF  are so-called 'noncharacters' (never assigned by Unicode.org)
   		 codes $00D800 to $00DFFF  are surrogates for UTF-16
*/

// names of destination files:
var fn_ucs2_general_category_h  = destDir # "ucs2_general_category.h"
var fn_ucs4_general_category_h  = destDir # "ucs4_general_category.h"

var UCS2_NUM_GC	= "UCS2_GC_CNT"		// name for global constant
var UCS4_NUM_GC	= "UCS4_GC_CNT"		// name for global constant


/* 	uses table: unicodeData{}

	unicodeData items =
	item.Code					numeric
	item.Name					text
	item.GeneralCategory		text: general category: real property name for enum
	item.DecimalDigitValue		opt.: only if digits come in a block of 10
	item.NumericValue			opt.
	item.SimpleUppercase		opt.: code
	item.SimpleLowercase		opt.: code
	item.SimpleTitercase		opt.: code if != SimpleUppercase
	//
	item.Ccc					numeric					TODO Ccc value names
	item.DecompTypeAndMapping	opt.					TODO
	item.BidiClass				text: unmodified value	TODO
	item.BidiMirrored			text: Y or N			TODO
*/


var generalCategory = {}
/*
	generalCategory items =
	item.Start			numeric character code: start of block
	item.Ende			numeric character code: end of block (excl.)
	item.GeneralCategory text: General Category
	gaps not included
*/


proc readGeneralCategory()
{
	vlog("readGeneralCategory\n")

	var globals.generalCategory = {}

	var a = 1
	var d = 0		// add. offset for gap in <private use> categories
	do
		while a <= count unicodeData
		var GC = unicodeData[a].GeneralCategory
		var Code = unicodeData[a].Code
		var Name = unicodeData[a].Name
		var e = a
		do
			while e <= count unicodeData
			while unicodeData[e].Code == Code + (e+d-a)
			while unicodeData[e].GeneralCategory == GC

			if find(unicodeData[e].Name," First>")
				d += unicodeData[e+1].Code - unicodeData[e].Code - 1
			then
			e++
		loop

		generalCategory ##= {{ a=a, e=e, cnt=e+d-a, Start=Code, Ende=Code+(e+d-a), GeneralCategory = GC }}
		a = e
		d = 0
	loop

	vlog("count generalCategory = ", count generalCategory, nl)
}

proc writeGeneralCategory()
{
	vlog("writeGeneralCategory\n")

	proc Replace(text,*)
	{
		var i = 1;
		do
			while ++i < count locals
			replace text, "{"#(i-1)#"}", string(locals[i])
		loop
		return text
	}

	var count_ucs2 = 0, longest_used_ucs2=0, used_ucs2=0
	var count_ucs4 = 0, longest_used_ucs4=0, used_ucs4=0
	var last_end = 0
	var gc_unassigned = propertyValueName("gc","unassigned")

	var text_used = "ADD_USED(0x{1},{2},{3})\n"
	var text_none = "ADD_NONE(0x{1},{2},{3})\n"

	proc writeUcs2(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,4), item.Start-last_end, gc_unassigned)
			count_ucs2++
		then
		put#F, Replace(text_used, hexstr(item.Start,4), item.cnt, item.GeneralCategory)
		count_ucs2++
		used_ucs2++
		last_end = item.Ende
		longest_used_ucs2 = max(longest_used_ucs2, item.cnt)
	}
	proc writeUcs4(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,6), item.Start-last_end, gc_unassigned)
			count_ucs4++
		then
		put#F, Replace(text_used, hexstr(item.Start,6), item.cnt, item.GeneralCategory)
		count_ucs4++
		used_ucs4++
		last_end = item.Ende
		longest_used_ucs4 = max(longest_used_ucs4, item.cnt)
	}

	var fileinfo = «
	// map code point ranges -> GeneralCategory property
	// created »#datestr(now)#« by script »#scriptName#«
	//
	// Macros define code ranges:
	//
	// ADD_USED ( blk_start, blk_size, general_category )
	// ADD_NONE ( blk_start, blk_size, gc_unassigned )\n\n»

	vlog("writing: ",fn_ucs2_general_category_h,nl)
	var F
	openout#F, fn_ucs2_general_category_h
	put #F, fileinfo
	var i = 1
	do
		until generalCategory[i].Start >= $10000
		writeUcs2(generalCategory[i++])
	loop
	if (last_end < $10000)
		put#F, Replace(text_none, hexstr(last_end,4), $10000-last_end, gc_unassigned)
		count_ucs2++
		last_end = $10000
	then
	close#F

	vlog("writing: ",fn_ucs4_general_category_h,nl)
	var F
	openout#F, fn_ucs4_general_category_h
	put #F, fileinfo
	//var i = i
	do
		while i <= count generalCategory
		writeUcs4(generalCategory[i++])
	loop
		put#F, Replace(text_none, hexstr(last_end,6), $110000-last_end, gc_unassigned)
		count_ucs4++
	close#F

	addGlobalConstant(UCS2_NUM_GC, count_ucs2, "table ucs2_general_category[]: number of entries")
	addGlobalConstant(UCS4_NUM_GC, count_ucs4, "table ucs4_general_category[]: number of entries")

	vlog("count_ucs2 = ",count_ucs2, ", used=",used_ucs2, ", gaps=",(count_ucs2-used_ucs2)   ,nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, ", used=",used_ucs4, ", gaps=",(count_ucs4-used_ucs4)   ,nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)
}


// run it:
if(verbose) log("run: general_category.vs\n") then
readGeneralCategory()
writeGeneralCategory()



















