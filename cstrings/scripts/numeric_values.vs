// this file is included from "create_all.vs"


/* read numeric values from unicodeData{}
   combine decimal digits into ranges
   and write header files for UCS-2 and UCS-4

   these contain macros ADD_ONE(), ADD_RANGE() and ADD_NONE()
   arguments: first_codepoint, count and general_category

   note: legal unicode values are $000000 to $010FFFF
   		 codes $xxFFFE and $xxFFFF are so-called 'noncharacters' (never assigned by Unicode.org)
   		 codes $00FDD0 to $00FDEF  are so-called 'noncharacters' (never assigned by Unicode.org)
   		 codes $00D800 to $00DFFF  are surrogates for UTF-16
*/

// names of destination files:
var fn_ucs2_numeric_values_h  = destDir # "ucs2_numeric_values.h"
var fn_ucs4_numeric_values_h  = destDir # "ucs4_numeric_values.h"

var UCS2_NUM_NUMVAL	= "UCS2_NUMVAL_CNT"		// name for global constant
var UCS4_NUM_NUMVAL	= "UCS4_NUMVAL_CNT"		// name for global constant


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


var numericValues = {}
/*
	numericValues items =
	item.Start				numeric: (first) character code (of a block)
	item.End				numeric: end of block of numeric values incrementing by item.Increment
	item.Value	 			numeric: (first) numeric value
	item.Increment			numeric: increment for values in block

	gaps not included

	--> MACRO(start,end,value,incr)

	ucs2char start
	uint16   count = 1
	float    value

	ucs2char start
	uint16   count > 1
	uint16   value
	uint16   increment
*/


proc readNumericValues()
{
	vlog("readNumericValues\n")

	var globals.numericValues = {}

	var a = 1
	do
		while a <= count unicodeData
		if (!exists unicodeData[a].NumericValue) a++; next; then

		var item  = unicodeData[a]
		var Code  = item.Code
		var Value = item.NumericValue
		var Gc    = item.GeneralCategory  // info only
		var Increment = 0

		if Value == int(Value) && Value >= 0 && Value <= $FFFF
				&& exists unicodeData[a+1].NumericValue
			var next_item = unicodeData[a+1]
			if next_item.Code == Code+1 && next_item.NumericValue >= Value
					&& next_item.NumericValue == int(next_item.NumericValue)
				Increment = next_item.NumericValue - Value
				if Increment > $FFFF Increment = 0 then
			then
		then


		var e = a+1
		do
			while e <= count unicodeData
			while unicodeData[e].Code == Code + (e-a)
			while exists unicodeData[e].NumericValue
			while (unicodeData[e].NumericValue - Value) == (e-a) * Increment && (Value==int(Value) || Increment != 0)
			e++
		loop

		var item = { a=a, e=e, cnt=e-a, Start=Code, Ende=Code+(e-a), Value=Value, Increment=Increment, Gc=Gc }
		numericValues ##= { item }
		a = e
	loop

	vlog("count numericValues = ", count numericValues, nl)
}

proc writeNumericValues()
{
	vlog("writeNumericValues\n")

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

	var text_float = "ADD_ONE  (0x{1}, {2}, {3}, {4})\n"
	var text_range = "ADD_RANGE(0x{1}, {2}, {3}, {4})\n"
	var text_none  = "ADD_NONE (0x{1}, {2}, {3}, {4})\n"

	proc writeUcs2(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,4), item.Start-last_end, 0,0)
			count_ucs2++
		then
		if item.cnt==1
			put#F, Replace(text_float, hexstr(item.Start,4), item.cnt, item.Value, item.Increment)
		else
			put#F, Replace(text_range, hexstr(item.Start,4), item.cnt, item.Value, item.Increment)
		then
		count_ucs2++
		used_ucs2++
		last_end = item.Ende
		longest_used_ucs2 = max(longest_used_ucs2, item.cnt)
	}
	proc writeUcs4(item)
	{
		if (item.Start > last_end)
			put#F, Replace(text_none, hexstr(last_end,6), item.Start-last_end, 0,0)
			count_ucs4++
		then
		if item.cnt==1
			put#F, Replace(text_float, hexstr(item.Start,6), item.cnt, item.Value, item.Increment)
		else
			put#F, Replace(text_range, hexstr(item.Start,6), item.cnt, item.Value, item.Increment)
		then
		count_ucs4++
		used_ucs4++
		last_end = item.Ende
		longest_used_ucs4 = max(longest_used_ucs4, item.cnt)
	}

	var fileinfo = «
	// map (ranges of) code points -> (ranges of) numeric values
	// created »#datestr(now)#« by script »#scriptName#«
	//
	// Macros define code ranges:
	//
	// ADD_ONE  ( blk_start, 1, float_value, 0 )
	// ADD_RANGE( blk_start, blk_size, uint_value, uint_increment )
	// ADD_NONE ( blk_start, blk_size, 0, 0 )\n\n»

	vlog("writing: ",fn_ucs2_numeric_values_h,nl)
	var F
	openout#F, fn_ucs2_numeric_values_h
	put #F, fileinfo
	var i = 1
	do
		until numericValues[i].Start >= $10000
		writeUcs2(numericValues[i++])
	loop
	if (last_end < $10000)
		put#F, Replace(text_none, hexstr(last_end,4), $10000-last_end, 0,0)
		count_ucs2++
		last_end = $10000
	then
	close#F

	vlog("writing: ",fn_ucs4_numeric_values_h,nl)
	var F
	openout#F, fn_ucs4_numeric_values_h
	put #F, fileinfo
	//var i = i
	do
		while i <= count numericValues
		writeUcs4(numericValues[i++])
	loop
		put#F, Replace(text_none, hexstr(last_end,6), $110000-last_end, 0,0)
		count_ucs4++
	close#F

	addGlobalConstant(UCS2_NUM_NUMVAL, count_ucs2, "table ucs2_numeric_values[]: number of entries")
	addGlobalConstant(UCS4_NUM_NUMVAL, count_ucs4, "table ucs4_numeric_values[]: number of entries")

	vlog("count_ucs2 = ",count_ucs2, ", used=",used_ucs2, ", gaps=",(count_ucs2-used_ucs2)   ,nl)
	vlog("longest_used_ucs2 = ",longest_used_ucs2,nl)
	vlog("count_ucs4 = ",count_ucs4, ", used=",used_ucs4, ", gaps=",(count_ucs4-used_ucs4)   ,nl)
	vlog("longest_used_ucs4 = ",longest_used_ucs4,nl)
}


// run it:
if(verbose) log("run: numeric_values.vs\n") then
readNumericValues()
writeNumericValues()



















