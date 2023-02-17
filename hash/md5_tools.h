#pragma once
// Copyright (c) 2013 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "md5.h"
#include "unix/FD.h"


extern void calc_md5(FD& fd, uint64 start, uint64 count, uint8 md5[16]) throws;
extern void calc_md5(const uint8 data[], uint32 count, uint8 md5[16]);
extern void check_md5(const uint8 data[], uint32 count, const uint8 md5[16], cstr msg) throws;
