
// map code point ranges -> simple uppercase code
// created 2019-05-08 15:16:15 by script create_all.vs
//
// Simple uppercase mapping is when mapping is to a single letter
// ATTN: Mapping is invalid for codes which are already uppercase!
//       this allows to combine code ranges with alternating upper/lowercase letters into one block.
// Macros define code ranges:
//
// ADD_USED ( blk_start, blk_size, code_offset )
// ADD_NONE ( blk_start, blk_size, 0 )


ADD_NONE(0x010000, 1064, 0)
ADD_USED(0x010428, 40, -40)
ADD_NONE(0x010450, 136, 0)
ADD_USED(0x0104D8, 36, -40)
ADD_NONE(0x0104FC, 1988, 0)
ADD_USED(0x010CC0, 51, -64)
ADD_NONE(0x010CF3, 3021, 0)
ADD_USED(0x0118C0, 32, -32)
ADD_NONE(0x0118E0, 21888, 0)
ADD_USED(0x016E60, 32, -32)
ADD_NONE(0x016E80, 31394, 0)
ADD_USED(0x01E922, 34, -34)
ADD_NONE(0x01E944, 988860, 0)
