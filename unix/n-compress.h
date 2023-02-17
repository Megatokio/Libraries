// Copyright (c) 2022 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"

extern void compress(int fdin, int fdout);
extern void decompress(cstr sourcefilepath, cstr destfilepath);
