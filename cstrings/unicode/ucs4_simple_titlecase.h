
// map code point ranges -> simple titlecase code
// created 2019-05-08 15:16:15 by script create_all.vs
//
// Simple titlecase mapping is when mapping is to a single letter
// ATTN: Mapping is invalid for codes which are already titlecase!
//       this allows to combine code ranges with alternating upper/lowercase letters into one block.
// Macros define code ranges:
//
// ADD_USED ( blk_start, blk_size, code_offset )
// ADD_NONE ( blk_start, blk_size, 0 )


ADD_NONE(0x010000, 1048576, 0)
