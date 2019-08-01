#pragma once
/*	Copyright  (c)	Günter Woigk 1995 - 2019
					mailto:kio@little-bat.de

	This file is free software
	License: BSD-2-Clause, see https://opensource.org/licenses/BSD-2-Clause


	swap halves of int
*/

#include "kio/kio.h"


inline uint16 swap	(uint16 n)	{ return (n<<8) | (n>>8); }
inline uint32 swap	(uint32 n)	{ return (uint32(swap((uint16)n))<<16) | (uint32(swap((uint16)(n>>16)))); }
inline uint64 swap	(uint64 n)	{ return (uint64(swap((uint32)n))<<32) | (uint64(swap((uint32)(n>>32)))); }
inline int16  swap	(int16 n)	{ return swap(uint16(n)); }
inline int32  swap	(int32 n)	{ return swap(uint32(n)); }
inline int64  swap	(int64 n)	{ return swap(uint64(n)); }











