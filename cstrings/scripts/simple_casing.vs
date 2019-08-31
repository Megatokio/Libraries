// this file is included from "create_all.vs"


/* read upper/lower/titleCase from unicodeData{}
   combine into ranges
   and write header files for UCS-2 and UCS-4

   simple mappings are mappings which map to a single other character

   these files contain macros ADD_USED() and ADD_NONE()
   arguments: first_codepoint, count and code_offset
   for lowercase mapping of "mychar", the code_offset is +(lowercase-mychar)
   for uppercase mapping of "mychar", the code_offset is -(uppercase-mychar)
   this allows code ranges with alternating upper/lowercase letters to be merged into one block
   for titlecase mapping this is tbd.
   if the offset for a mapping exceeds ±0xFF then this might be an absolute code. tbd.

   total uppercase letters: 1788
   total lowercase letters: 2151
   total titlecase letters: 31

   total letters with uppercase mapping: 1407
   total letters with lowercase mapping: 1390
   total letters with titlecase mapping: 8

   total uppercase letters without simple lowercase: 471
   total lowercase letters without simple uppercase: 791

   non-letters with Lu/Ll mapping (Unicode 12.0):
		$0345       YPOGEGRAMMENI/IOTA: Mn / GcNonspacingMark: has Lu mapping
		$2160-$217F Latin Numbers:      Nl / GcLetterNumber:   has Ll/Lu mapping
		$24B6-$24E9 Circled Letters:    So / GcOtherSymbol:    has Ll/Lu mapping

	TITLECASE:
		there are a total of 31 Lt/titlecase letters
	greek titlecase letters with no uppercase mapping:
		in range $1F88 to $1FFC are 27 letters: all have a PROSGEGRAMMENI and a decomposition
		to an uppercase letter without PROSGEGRAMMENI plus appended Mn/non-spacing-mark PROSGEGRAMMENI
		so for the sake of a simple mapping the PROSGEGRAMMENI could have been ignored?
	latin letter tuples:
		in range $01C5 to $01F2 are 4 letter pairs which map to a Lu + Ll in titlecase
		they have a decomposition to these ascii letters
		these are the only letters which actually have a titlecase mapping!
	Georgian letters:
		in range $10D0 to $10FF have uppercase mapping but map to self for titlecase

   note: legal unicode values are $000000 to $010FFFF
   		codes $xxFFFE and $xxFFFF are so-called 'noncharacters' (never assigned by Unicode.org)
   		codes $00FDD0 to $00FDEF  are so-called 'noncharacters' (never assigned by Unicode.org)
   		codes $00D800 to $00DFFF  are surrogates for UTF-16
*/

// names of destination files:
var fn_ucs2_simple_lowercase_h  = destDir # "ucs2_simple_lowercase.h"
var fn_ucs2_simple_uppercase_h  = destDir # "ucs2_simple_uppercase.h"
var fn_ucs2_simple_titlecase_h  = destDir # "ucs2_simple_titlecase.h"
var fn_ucs4_simple_lowercase_h  = destDir # "ucs4_simple_lowercase.h"
var fn_ucs4_simple_uppercase_h  = destDir # "ucs4_simple_uppercase.h"
var fn_ucs4_simple_titlecase_h  = destDir # "ucs4_simple_titlecase.h"

var UCS2_SUC_CNT = "UCS2_SUC_CNT"		// name for global constant
var UCS2_SUC_ANF = "UCS2_SUC_ANF"
var UCS2_SUC_END = "UCS2_SUC_END"

var UCS4_SUC_CNT = "UCS4_SUC_CNT"		// name for global constant
var UCS4_SUC_ANF = "UCS4_SUC_ANF"
var UCS4_SUC_END = "UCS4_SUC_END"

var UCS2_SLC_CNT = "UCS2_SLC_CNT"		// name for global constant
var UCS2_SLC_ANF = "UCS2_SLC_ANF"
var UCS2_SLC_END = "UCS2_SLC_END"

var UCS4_SLC_CNT = "UCS4_SLC_CNT"		// name for global constant
var UCS4_SLC_ANF = "UCS4_SLC_ANF"
var UCS4_SLC_END = "UCS4_SLC_END"

var UCS2_STC_CNT = "UCS2_STC_CNT"		// name for global constant
var UCS2_STC_ANF = "UCS2_STC_ANF"
var UCS2_STC_END = "UCS2_STC_END"

var UCS4_STC_CNT = "UCS4_STC_CNT"		// name for global constant
var UCS4_STC_ANF = "UCS4_STC_ANF"
var UCS4_STC_END = "UCS4_STC_END"


/* 	uses table: unicodeData{}

	unicodeData items =
	item.Code					numeric
	item.Name					text
	item.GeneralCategory 		text: general category: real property name for enum
	item.DecimalDigitValue		opt.: only if digits come in a block of 10
	item.NumericValue			opt.
	item.SimpleUppercase		opt.: code
	item.SimpleLowercase		opt.: code
	item.SimpleTitercase		opt.: code if != SimpleUppercase and code itself is not titlecase
	//
	item.Ccc					numeric					TODO Ccc value names
	item.DecompTypeAndMapping	opt.					TODO
	item.BidiClass				text: unmodified value	TODO
	item.BidiMirrored			text: Y or N			TODO
*/


var simpleLowercase = {}
var simpleUppercase = {}
var simpleTitlecase = {}
/*
	simpleUpperCase items =
	simpleLowerCase items =
	simpleTitleCase items =
	item.Start			numeric: character code: start of block
	item.Ende			numeric: character code: end of block (excl.)
	item.Offset			numeric: offset to other case: othercode - mycode
	gaps not included
*/


proc testSimpleCasing()
{
	// test whether all characters with a upper/lower/titlecase mapping are Lu,Ll or Lt

	var Lu = propertyValueName("gc","lu")
	var Ll = propertyValueName("gc","ll")
	var Lt = propertyValueName("gc","lt")

	var lnou=0 // lowercase has no uppercase
	var unol=0 // uppercase has no lowercase
	var lu=0,ll=0,lt=0 // count upper/lower/titlecase characters
	var hasu,hasl,hast // letter has upper/lower/titlecase mapping

	var i=0
	do
		while ++i <= count unicodeData
		var item = unicodeData[i]

		hasu += item.SimpleUppercase != item.Code
		hasl += item.SimpleLowercase != item.Code
		hast += item.SimpleTitlecase != item.SimpleUppercase

		if item.SimpleTitlecase != item.SimpleUppercase
			log "U+",hexstr(item.Code,6)," has Lt mapping",nl
		then

		if item.GeneralCategory == Lu
			lu++
			if item.SimpleLowercase == item.Code
				//log "U+",hexstr(item.Code,6)," is Lu but has no simple Ll mapping\n"
				unol++
			then
		elif item.GeneralCategory == Ll
			ll++
			if item.SimpleUppercase == item.Code
				//log "U+",hexstr(item.Code,6)," is Ll but has no simple Lu mapping\n"
				lnou++
			then
		elif item.GeneralCategory == Lt
			lt++
			if item.SimpleUppercase == item.Code
				log "U+",hexstr(item.Code,6)," is Lt but has no simple Lu mapping\n"
			then
			if item.SimpleLowercase == item.Code
				log "U+",hexstr(item.Code,6)," is Lt but has no simple Ll mapping\n"
			then
		else
			if item.SimpleLowercase != item.Code
				log "U+",hexstr(item.Code,6)," is ",item.GeneralCategory," but has a simple Ll mapping\n"
			then
			if item.SimpleUppercase != item.Code
				log "U+",hexstr(item.Code,6)," is ",item.GeneralCategory," but has a simple Lu mapping\n"
			then
			if item.SimpleTitlecase != item.SimpleUppercase
				log "U+",hexstr(item.Code,6)," is ",item.GeneralCategory," but has a simple Lt mapping\n"
			then
		then
	loop

	put "total Lu: ",lu,nl
	put "total Ll: ",ll,nl
	put "total Lt: ",lt,nl
	put "total with Lu mapping: ",hasu,nl
	put "total with Ll mapping: ",hasl,nl
	put "total with Lt mapping: ",hast,nl
	put "Lu without Ll: ",unol,nl
	put "Ll without Lu: ",lnou,nl

	var i=0
	do
		while ++i <= count unicodeData
		var item = unicodeData[i]
		if item.GeneralCategory == Lu
			if item.SimpleUppercase != item.Code
				log "U+",hexstr(item.Code,6)," is Lu but also has a simple Lu mapping\n"
			then
		elif item.GeneralCategory == Ll
			if item.SimpleLowercase != item.Code
				log "U+",hexstr(item.Code,6)," is Ll but also has a simple Ll mapping\n"
			then
		elif item.GeneralCategory == Lt
			if item.SimpleTitlecase != item.Code
				log "U+",hexstr(item.Code,6)," is Lt but also has a simple Lt mapping\n"
			then
		then
	loop
}


proc readSimpleCasing()
{
	vlog("readSimpleCasing\n")

	var globals.simpleLowercase = {}
	var globals.simpleUppercase = {}
	var globals.simpleTitlecase = {}

	var gc_uppercase = propertyValueName("gc","lu")
	var gc_lowercase = propertyValueName("gc","ll")
	var gc_titlecase = propertyValueName("gc","lt")

	// read simpleLowercase[]:
	var a = 1
	do
		while a <= count unicodeData
		var item = unicodeData[a]
		var Code = item.Code
		var Lower = item.SimpleLowercase
		var Delta = Lower - Code
		var e = a
		do
			while e <= count unicodeData
			while Delta==0 || unicodeData[e].Code == Code + (e-a)
			if (unicodeData[e].GeneralCategory == gc_lowercase) e++; next; then
			while unicodeData[e].SimpleLowercase == unicodeData[e].Code + Delta
			e++
		loop

		if Delta
			simpleLowercase ##= {{ a=a, e=e, cnt=e-a, Start=Code, Ende=Code+(e-a), Delta=Delta }}
		then
		a = e
	loop

	// read simpleUppercase[]:
	var a = 1
	do
		while a <= count unicodeData
		var item = unicodeData[a]
		var Code = item.Code
		var Upper = item.SimpleUppercase
		var Delta = Upper - Code
		var e = a
		do
			while e <= count unicodeData
			while Delta==0 || unicodeData[e].Code == Code + (e-a)
			if (unicodeData[e].GeneralCategory == gc_uppercase) e++; next; then
			while unicodeData[e].SimpleUppercase == unicodeData[e].Code + Delta
			e++
		loop

		if Delta
			simpleUppercase ##= {{ a=a, e=e, cnt=e-a, Start=Code, Ende=Code+(e-a), Delta=Delta }}
		then
		a = e
	loop

	// read simpleTitlecase[]:
	var a = 1
	do
		while a <= count unicodeData
		var item = unicodeData[a]
		var Upper = item.SimpleUppercase
		var Title = item.SimpleTitlecase
		if Upper==Title a++ next then

		var Code = item.Code
		var Delta = Title - Code
		var e = a
		do
			while e <= count unicodeData
			while unicodeData[e].Code == Code + (e-a)
			//if (unicodeData[e].GeneralCategory == gc_titlecase) e++; next; then
			while unicodeData[e].SimpleTitlecase != unicodeData[e].SimpleUppercase
			while unicodeData[e].SimpleTitlecase == unicodeData[e].Code + Delta
			e++
		loop

		if Upper!=Title
			simpleTitlecase ##= {{ a=a, e=e, cnt=e-a, Start=Code, Ende=Code+(e-a), Delta=Delta }}
		then
		a = e
	loop

	vlog("count simpleLowercase = ", count simpleLowercase, nl)
	vlog("count simpleUppercase = ", count simpleUppercase, nl)
	vlog("count simpleTitlecase = ", count simpleTitlecase, nl)
}

proc writeSimpleCasing()
{
	vlog("writeSimpleCasing\n")

	proc Replace(text,*)
	{
		var i = 1;
		do
			while ++i < count locals
			replace text, "{"#(i-1)#"}", string(locals[i])
		loop
		return text
	}

	var text_used = "ADD_USED(0x{1},{2},{3})\n"	// has mapping
	var text_none = "ADD_NONE(0x{1},{2},{3})\n" // no mapping

	proc writeUcs2(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,4), item.Start-last_end, 0)
			count_ucs2++
		then
		put#F, Replace(text_used, hexstr(item.Start,4), item.cnt, item.Delta)
		count_ucs2++
		used_ucs2++
		last_end = item.Ende
		longest_used_ucs2 = max(longest_used_ucs2, item.cnt)
	}
	proc writeUcs4(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,6), item.Start-last_end, 0)
			count_ucs4++
		then
		put#F, Replace(text_used, hexstr(item.Start,6), item.cnt, item.Delta)
		count_ucs4++
		used_ucs4++
		last_end = item.Ende
		longest_used_ucs4 = max(longest_used_ucs4, item.cnt)
	}

	var fileinfo = «
// map code point ranges -> simple {1} code
// created »#datestr(now)#« by script »#scriptName#«
//
// Simple {1} mapping is when mapping is to a single letter
// ATTN: Mapping is invalid for codes which are already {1}!
//       this allows to combine code ranges with alternating upper/lowercase letters into one block.
// Macros define code ranges:
//
// ADD_USED ( blk_start, blk_size, code_offset )
// ADD_NONE ( blk_start, blk_size, 0 )
\n\n»

	// Write file "simple_uppercase.h":

	var count_ucs2 = 0, longest_used_ucs2=0, used_ucs2=0
	var count_ucs4 = 0, longest_used_ucs4=0, used_ucs4=0
	var last_end = 0

	vlog("writing: ",fn_ucs2_simple_uppercase_h,nl)
	var F
	openout#F, fn_ucs2_simple_uppercase_h
	put #F, Replace(fileinfo,"uppercase")
	var i = 1
	do
		until simpleUppercase[i].Start >= $10000
		writeUcs2(simpleUppercase[i++])
	loop
	if (last_end < $10000)
		put#F, Replace(text_none, hexstr(last_end,4), $10000-last_end, 0)
		count_ucs2++
		last_end = $10000
	then
	close#F

	var ucs2_a = simpleUppercase[1].Start
	var ucs2_e = simpleUppercase[i-1].Ende
	var ucs4_a = simpleUppercase[i].Start
	var ucs4_e = simpleUppercase[count simpleUppercase].Ende

	vlog("writing: ",fn_ucs4_simple_uppercase_h,nl)
	var F
	openout#F, fn_ucs4_simple_uppercase_h
	put #F, Replace(fileinfo,"uppercase")
	//var i = i
	do
		while i <= count simpleUppercase
		writeUcs4(simpleUppercase[i++])
	loop
		put#F, Replace(text_none, hexstr(last_end,6), $110000-last_end, 0)
		count_ucs4++
	close#F

	addGlobalConstant(UCS2_SUC_CNT, count_ucs2, "table ucs2_simple_uppercase[]: number of entries")
	addGlobalConstant(UCS2_SUC_ANF, ucs2_a,     "table ucs2_simple_uppercase[]: first code in table")
	addGlobalConstant(UCS2_SUC_END, ucs2_e,     "table ucs2_simple_uppercase[]: first code after table")

	addGlobalConstant(UCS4_SUC_CNT, count_ucs4, "table ucs4_simple_uppercase[]: number of entries")
	addGlobalConstant(UCS4_SUC_ANF, ucs4_a,     "table ucs4_simple_uppercase[]: first code in table")
	addGlobalConstant(UCS4_SUC_END, ucs4_e,     "table ucs4_simple_uppercase[]: first code after table")

	vlog("count_ucs2 = ",count_ucs2, ", used=",used_ucs2, ", gaps=",(count_ucs2-used_ucs2)   ,nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, ", used=",used_ucs4, ", gaps=",(count_ucs4-used_ucs4)   ,nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)

	// Write file "simple_lowercase.h":

	var count_ucs2 = 0, longest_used_ucs2=0, used_ucs2=0
	var count_ucs4 = 0, longest_used_ucs4=0, used_ucs4=0
	var last_end = 0

	vlog("writing: ",fn_ucs2_simple_lowercase_h,nl)
	var F
	openout#F, fn_ucs2_simple_lowercase_h
	put #F, Replace(fileinfo,"lowercase")
	var i = 1
	do
		until simpleLowercase[i].Start >= $10000
		writeUcs2(simpleLowercase[i++])
	loop
	if (last_end < $10000)
		put#F, Replace(text_none, hexstr(last_end,4), $10000-last_end, 0)
		count_ucs2++
		last_end = $10000
	then
	close#F

	var ucs2_a = simpleLowercase[1].Start
	var ucs2_e = simpleLowercase[i-1].Ende
	var ucs4_a = simpleLowercase[i].Start
	var ucs4_e = simpleLowercase[count simpleLowercase].Ende

	vlog("writing: ",fn_ucs4_simple_lowercase_h,nl)
	var F
	openout#F, fn_ucs4_simple_lowercase_h
	put #F, Replace(fileinfo,"lowercase")
	//var i = i
	do
		while i <= count simpleLowercase
		writeUcs4(simpleLowercase[i++])
	loop
		put#F, Replace(text_none, hexstr(last_end,6), $110000-last_end, 0)
		count_ucs4++
	close#F

	addGlobalConstant(UCS2_SLC_CNT, count_ucs2, "table ucs2_simple_lowercase[]: number of entries")
	addGlobalConstant(UCS2_SLC_ANF, ucs2_a,     "table ucs2_simple_lowercase[]: first code in table")
	addGlobalConstant(UCS2_SLC_END, ucs2_e,     "table ucs2_simple_lowercase[]: first code after table")

	addGlobalConstant(UCS4_SLC_CNT, count_ucs4, "table ucs4_simple_lowercase[]: number of entries")
	addGlobalConstant(UCS4_SLC_ANF, ucs4_a,     "table ucs4_simple_lowercase[]: first code in table")
	addGlobalConstant(UCS4_SLC_END, ucs4_e,     "table ucs4_simple_lowercase[]: first code after table")

	vlog("count_ucs2 = ",count_ucs2, ", used=",used_ucs2, ", gaps=",(count_ucs2-used_ucs2)   ,nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, ", used=",used_ucs4, ", gaps=",(count_ucs4-used_ucs4)   ,nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)


	// Write file "simple_titlecase.h":

	var count_ucs2 = 0, longest_used_ucs2=0, used_ucs2=0
	var count_ucs4 = 0, longest_used_ucs4=0, used_ucs4=0
	var last_end = 0

	vlog("writing: ",fn_ucs2_simple_titlecase_h,nl)
	var F
	openout#F, fn_ucs2_simple_titlecase_h
	put #F, Replace(fileinfo,"titlecase")
	var i = 1
	do
		while i <= count simpleTitlecase
		while simpleTitlecase[i].Start < $10000
		writeUcs2(simpleTitlecase[i++])
	loop
	if (last_end < $10000)
		put#F, Replace(text_none, hexstr(last_end,4), $10000-last_end, 0)
		count_ucs2++
		last_end = $10000
	then
	close#F

	var ucs2_a = simpleTitlecase[1].Start
	var ucs2_e = simpleTitlecase[i-1].Ende
	var at_end = i == count simpleTitlecase + 1
	var ucs4_a = at_end ? $10000 : simpleTitlecase[i].Start
	var ucs4_e = at_end ? $10000 : simpleTitlecase[count simpleTitlecase].Ende

	vlog("writing: ",fn_ucs4_simple_titlecase_h,nl)
	var F
	openout#F, fn_ucs4_simple_titlecase_h
	put #F, Replace(fileinfo,"titlecase")
	//var i = i
	do
		while i <= count simpleTitlecase
		writeUcs4(simpleTitlecase[i++])
	loop
		put#F, Replace(text_none, hexstr(last_end,6), $110000-last_end, 0)
		count_ucs4++
	close#F

	addGlobalConstant(UCS2_STC_CNT, count_ucs2, "table ucs2_simple_titlecase[]: number of entries")
	addGlobalConstant(UCS2_STC_ANF, ucs2_a,     "table ucs2_simple_titlecase[]: first code in table")
	addGlobalConstant(UCS2_STC_END, ucs2_e,     "table ucs2_simple_titlecase[]: first code after table")

	addGlobalConstant(UCS4_STC_CNT, count_ucs4, "table ucs4_simple_titlecase[]: number of entries")
	addGlobalConstant(UCS4_STC_ANF, ucs4_a,     "table ucs4_simple_titlecase[]: first code in table")
	addGlobalConstant(UCS4_STC_END, ucs4_e,     "table ucs4_simple_titlecase[]: first code after table")

	vlog("count_ucs2 = ",count_ucs2, ", used=",used_ucs2, ", gaps=",(count_ucs2-used_ucs2)   ,nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, ", used=",used_ucs4, ", gaps=",(count_ucs4-used_ucs4)   ,nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)
}


// run it:
if(verbose) log("run: simple_casing.vs\n") then

//testSimpleCasing()
readSimpleCasing()
writeSimpleCasing()



















