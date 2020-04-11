#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2020
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


	swap halves of int
*/

#include "kio/kio.h"

// evtl. these should be named swapped or byteswapped

inline uint16 swap (uint16 n) noexcept { return uint16((n<<8) | (n>>8)); }
inline uint32 swap (uint32 n) noexcept { return (uint32(swap(uint16(n)))<<16) | (uint32(swap(uint16(n>>16)))); }
inline uint64 swap (uint64 n) noexcept { return (uint64(swap(uint32(n)))<<32) | (uint64(swap(uint32(n>>32)))); }

inline int16  swap (int16 n) noexcept { return int16(swap(uint16(n))); }
inline int32  swap (int32 n) noexcept { return int32(swap(uint32(n))); }
inline int64  swap (int64 n) noexcept { return int64(swap(uint64(n))); }











