// this file is included from "create_all.vs"


// name of source file:
var fn_unicode_data_txt  = sourceDir # "UnicodeData.txt"

// generates table:
var unicodeData = {}
/*
	unicodeData items =
	item.Code					numeric
	item.Name					text
	item.GeneralCategory 		text: general category: real property name for enum
	item.DecimalDigitValue		opt.: only if digits come in a block of 10
	item.NumericValue			opt.
	item.SimpleUppercase		opt.: code
	item.SimpleLowercase		opt.: code
	item.SimpleTitlecase		opt.: code if != SimpleUppercase and code itself not a titlecase
	//
	item.Ccc					numeric					TODO Ccc value names
	item.DecompTypeAndMapping	opt.					TODO
	item.BidiClass				text: unmodified value	TODO
	item.BidiMirrored			text: Y or N			TODO
*/


proc readUnicodeData()
{
	vlog("readUnicodeData\n")

	// read the unicode database
	// each lines consist of 15 fields:
	// 	1	Code
	// 	2	Name  	M N
	// 	3	GeneralCategory  E N
	// 	4	Ccc 		N N
	// 	5	BidiClass E N
	// 	6	DecompositionType and DecompositionMapping  E,S  N
	// 	7-9	NumericType, NumericValue   E,N  N
	// 	10	BidiMirrored	B N
	// 	11	Unicode1Name (obsolete)  M I
	// 	12	IsoComment (obsolete)  M I
	// 	13	SimpleUppercaseMapping  S N
	// 	14	SimpleLowercaseMapping  S  N
	// 	15	SimpleTitlecaseMapping  S N

	var data = split(file fn_unicode_data_txt,nl)
	convert data from utf8

	// get the general categories values database for convenience:
	var normalized_gc = normalizedPropertyName("gc")
	var generalCategories = propertyValueNames.@normalized_gc
	if !islist(generalCategories) log "booboo40\n" end 1 then
	if !exists generalCategories.decimalnumber log "booboo54\n" end 1 then

	globals.unicodeData = {}

	var i=1
	do
		while i <= count data
		var zeile = data[i++]
		if zeile=="" || zeile[1]=="#" del data[--i] next then

		zeile = split(zeile,";")
		if count zeile<15
			log "code ",zeile[1], " hat zu wenig Felder\n"
			do while count zeile<15 zeile ##= {""} loop
		then

		var item = {}
		var item.Code = eval("$"#zeile[1])
		var item.Name = zeile[2] // TODO: "<control>"

		var general_category = zeile[3]
			if general_category=="" log "booboo56\n" end 1 then

			//general_category = realPropertyValue("gc",general_category)

			general_category = normalizedName(general_category)
			general_category = generalCategories.@general_category // real name
			if !istext general_category log "booboo58\n" end 1 then

		var item.GeneralCategory = general_category

		if zeile[4]!="" var item.Ccc = eval(zeile[4]) then
		if zeile[5]!="" var item.BidiClass = zeile[5] then
		if zeile[6]!="" var item.DecompTypeAndMapping = zeile[6] then
		if zeile[7]!="" var item.DecimalDigitValue = eval(zeile[7]) then
		if zeile[9]!="" var item.NumericValue = eval(zeile[9]) then
		if zeile[10]!="" var item.BidiMirrored = zeile[10] then

		var item.SimpleUppercase = zeile[13] != "" ? eval("$"#zeile[13]) : item.Code
		var item.SimpleLowercase = zeile[14] != "" ? eval("$"#zeile[14]) : item.Code
		var item.SimpleTitlecase = zeile[15] != "" ? eval("$"#zeile[15]) : item.SimpleUppercase

		// if zeile[13] != ""
		// 	var uc = eval("$"#zeile[13])
		// 	if (uc != item.Code) var item.SimpleUppercase = uc; then
		// then
		// if zeile[14] != ""
		// 	var lc = eval("$"#zeile[14])
		// 	if (lc != item.Code) var item.SimpleLowercase = lc; then
		// then
		// if zeile[15] != "" && zeile[15] != zeile[13]
		// 	var tc = eval("$"#zeile[15]);
		// 	if tc != item.Code var item.SimpleTitlecase = tc; then
		// then

		unicodeData ##= {item}
	loop
}


// run it:

if(verbose) log("run: unicode_data.vs\n") then
readUnicodeData()
vlog("count unicodeData = ",count unicodeData,nl)





