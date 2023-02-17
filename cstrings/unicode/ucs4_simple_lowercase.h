
// map code point ranges -> simple lowercase code
// created 2019-05-08 15:16:15 by script create_all.vs
//
// Simple lowercase mapping is when mapping is to a single letter
// ATTN: Mapping is invalid for codes which are already lowercase!
//       this allows to combine code ranges with alternating upper/lowercase letters into one block.
// Macros define code ranges:
//
// ADD_USED ( blk_start, blk_size, code_offset )
// ADD_NONE ( blk_start, blk_size, 0 )


ADD_NONE(0x010000, 1024, 0)
ADD_USED(0x010400, 80, 40)
ADD_NONE(0x010450, 96, 0)
ADD_USED(0x0104B0, 36, 40)
ADD_NONE(0x0104D4, 1964, 0)
ADD_USED(0x010C80, 51, 64)
ADD_NONE(0x010CB3, 3053, 0)
ADD_USED(0x0118A0, 64, 32)
ADD_NONE(0x0118E0, 21856, 0)
ADD_USED(0x016E40, 64, 32)
ADD_NONE(0x016E80, 31360, 0)
ADD_USED(0x01E900, 68, 34)
ADD_NONE(0x01E944, 988860, 0)
