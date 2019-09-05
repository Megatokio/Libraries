#pragma once
/*	Copyright  (c)	Günter Woigk 2013 - 2019
                    mailto:kio@little-bat.de

	This file is free software.

 	Permission to use, copy, modify, distribute, and sell this software
 	and its documentation for any purpose is hereby granted without fee,
 	provided that the above copyright notice appears in all copies and
 	that both that copyright notice, this permission notice and the
 	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "md5.h"
#include "kio/kio.h"
#include "unix/FD.h"


extern void calc_md5  (FD& fd, uint64 start, uint64 count, uint8 md5[16]) throws;
extern void calc_md5  (const uint8 data[], uint32 count, uint8 md5[16]);
extern void check_md5 (const uint8 data[], uint32 count, const uint8 md5[16], cstr msg) throws;
























