#pragma once
// Copyright (c) 2018 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"

extern int8	 ALaw_Encode(int16 number);
extern int16 ALaw_Decode(int8 number);
extern int8	 MuLaw_Encode(int16 number);
extern int16 MuLaw_Decode(int8 number);
