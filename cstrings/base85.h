#pragma once
// Copyright (c) 2013 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


extern uint sizeAfterBase85Encoding(uint qlen);
extern void encodeBase85(cuptr q, uint qlen, uptr z, uint zlen);
extern uint sizeAfterBase85Decoding(uint qlen);
extern int	decodeBase85(cuptr q, uint qlen, uptr z, uint zlen);
