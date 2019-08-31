

// this file is included from "create_all.vs"


if(verbose) log("run: blocks.vs\n") then

proc createBlocks()
{
	var fn_blocks_txt  = sourceDir # "Blocks.txt"	// name of source file
	var fn_ucs2_blocks = destDir # "ucs2_blocks.h"	// name of target file
	var fn_ucs4_blocks = destDir # "ucs4_blocks.h"	// name of target file

	var UCS2_NUM_BLOCKS = "UCS2_BLOCKS_CNT"			// name for global constant
	var UCS4_NUM_BLOCKS = "UCS4_BLOCKS_CNT"			// name for global constant

	var BLOCK 	   = "Blk"							// prefix for names, must end with "_"
	var BLOCK_NONE = ("BlkNone")					// name for unassigned block
	var globals.propertyValueNames.block.none = "BlkNone"

	//log «exists normalizedPropertyNames.Blk = », exists normalizedPropertyNames.Blk, nl
	//log «exists normalizedPropertyNames.blk = », exists normalizedPropertyNames.blk, nl
	//log «exists normalizedPropertyNames.Block = », exists normalizedPropertyNames.Block, nl
	//log «exists normalizedPropertyNames.block = », exists normalizedPropertyNames.block, nl

	//log "normalizedPropertyName(Block) = ", normalizedPropertyName("Block"), nl
	//log "globals.propertyValueNames.block.none = ", exists globals.propertyValueNames.block.none, nl
	//log «propertyValueName("Block","None") = », propertyValueName("Block","None"),nl
	//log «propertyValueName("Blk","CJK Unified Ideographs Extension F") = », propertyValueName("Block","CJK Unified Ideographs Extension F"),nl
	//log "block names = ", propertyValueNames.block, nl

	var Blocks = {}

	proc readBlocks()
	{
		vlog("readBlocks\n")

		// read file:
		var array = convert(split(file fn_blocks_txt) from utf8)

		// remove empty lines and comments:
		var i = count array
		do
			while i >= 1
			var z = array[i--] 	// line of text
			if (z=="" || z[1]=="#" || z[1]<=" ")
				del array[i+1]
				next
			then
			if !find(z,"..") || !find(z,"; ")
				log "line ",i," not understood: ", array[i], nl
				del array[i+1]
				next
			then
		loop

		// now the lines look like: "27F0..27FF; Supplemental Arrows-A"
		// split lines at ".." and "; " and add data to Blocks:

		var globals.Blocks = {}
		i = 0
		do
			while ++i <= count array

			var z = array[i]	// line of text
			z = split(z,"..")
			var a = z[1]		// start of block
			z = split(z[2],"; ")
			var e = z[1]		// end of block
			var n = z[2]		// name of block

			n = propertyValueName(BLOCK,n)
			a = eval("$"#a)
			e = eval("$"#e) + 1

			Blocks ##= {{ Start=a, End=e, Name=n }}
		loop
	}

	proc fillGapsInBlocks()
	{
		vlog("fillGapsInBlocks\n")

		var newBlocks = {}
		var lastEnd = 0	// End of last block
		var i = 0;
		do
			while ++i <= count Blocks
			var block = Blocks[i]
			if block.Start > lastEnd 	// fill gap:
				newBlocks ##= {{ Start=lastEnd, End=block.Start, Name=BLOCK_NONE }}
			then
			newBlocks ##= { block }
			lastEnd = block.End
		loop
		if lastEnd < $110000
			newBlocks ##= {{ Start=lastEnd, End=$110000, Name=BLOCK_NONE }}
		then

		Blocks = newBlocks
	}

	var fileinfo = «
// map code point ranges -> block property
// (similar to script property, but much coarser)
// created »#datestr(now)#« by script »#scriptName#«
//
// Macros define code ranges:
//
// ADD_USED ( start, count, block_property )
// ADD_NONE ( start, count, block_none )
\n\n»

	proc Replace(text,*)
	{
		var i = 1;
		do
			while ++i < count locals
			replace text, "{"#(i-1)#"}", string(locals[i])
		loop
		return text
	}

	proc writeBlocksUCS2()
	{
		vlog("writeBlocksUCS2\n")

		// write blocks to header file:
		// Blocks[i] = { Start=a, End=e, Name="xx" }

		// write lines: "{0x1234,BlockProperty},\n"

		var text = {}
		var i = 1
		do
			var block = Blocks[i++]
			until block.Start >= $10000
			var a = hexstr(block.Start,4)
			var e = block.End-block.Start
			var n = block.Name
			var line = n==BLOCK_NONE ? Replace("ADD_NONE(0x{1},{2},{3})\n",a,e,n)
									 : Replace("ADD_USED(0x{1},{2},{3})\n",a,e,n)
			text ##= {line}
		loop

		addGlobalConstant(UCS2_NUM_BLOCKS, count text, "table ucs2_blocks[]: number of entries")

		text = fileinfo # join(text,"")

		put "writing: ",fn_ucs2_blocks,nl
		file fn_ucs2_blocks = convert(text to utf8)
	}

	proc writeBlocksUCS4()
	{
		vlog("writeBlocksUCS4\n")

		// write ucs4 header file:
		// in:  Blocks[i] = { Start=a, End=e, Name="xx" }
		// out: write lines: "ADD_BLOCK(0x123456,BlockProperty)\n"

		var text = {}
		var i = globalConstants.@UCS2_NUM_BLOCKS[1] + 1;
		if (Blocks[i].Start != $10000)
			log "first UCS-4 block does not start at 0x10000\n"
			text ##= {{Start=$10000, End=Blocks[i].End, Name=BLOCK_NONE}}
		then

		do
			while i <= count Blocks

			var block = Blocks[i++]
			var a = hexstr(block.Start,6)
			var e = block.End-block.Start
			var n = block.Name
			var line = n==BLOCK_NONE ? Replace("ADD_NONE(0x{1},{2},{3})\n",a,e,n)
									 : Replace("ADD_USED(0x{1},{2},{3})\n",a,e,n)
			text ##= {line}
		loop

		addGlobalConstant(UCS4_NUM_BLOCKS, count text, "table ucs4_blocks[]: number of entries")

		text = fileinfo # join(text,"")

		put "writing: ",fn_ucs4_blocks,nl
		file fn_ucs4_blocks = convert(text to utf8)
	}

	readBlocks()
	fillGapsInBlocks()
	writeBlocksUCS2()
	writeBlocksUCS4()

} // proc createBlocks()


// run it:
createBlocks()





