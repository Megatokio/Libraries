
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


ADD_NONE(0x0000, 452, 0)
ADD_USED(0x01C4, 1, 1)
ADD_USED(0x01C5, 1, 0)
ADD_USED(0x01C6, 1, -1)
ADD_USED(0x01C7, 1, 1)
ADD_USED(0x01C8, 1, 0)
ADD_USED(0x01C9, 1, -1)
ADD_USED(0x01CA, 1, 1)
ADD_USED(0x01CB, 1, 0)
ADD_USED(0x01CC, 1, -1)
ADD_NONE(0x01CD, 36, 0)
ADD_USED(0x01F1, 1, 1)
ADD_USED(0x01F2, 1, 0)
ADD_USED(0x01F3, 1, -1)
ADD_NONE(0x01F4, 3804, 0)
ADD_USED(0x10D0, 43, 0)
ADD_NONE(0x10FB, 2, 0)
ADD_USED(0x10FD, 3, 0)
ADD_NONE(0x1100, 61184, 0)
