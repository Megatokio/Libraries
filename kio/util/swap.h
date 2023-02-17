#pragma once
// Copyright (c) 1995 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	swap halves of int
*/

#include "kio/kio.h"

// evtl. these should be named swapped or byteswapped

inline uint16 swap(uint16 n) noexcept { return uint16((n << 8) | (n >> 8)); }
inline uint32 swap(uint32 n) noexcept { return (uint32(swap(uint16(n))) << 16) | (uint32(swap(uint16(n >> 16)))); }
inline uint64 swap(uint64 n) noexcept { return (uint64(swap(uint32(n))) << 32) | (uint64(swap(uint32(n >> 32)))); }

inline int16 swap(int16 n) noexcept { return int16(swap(uint16(n))); }
inline int32 swap(int32 n) noexcept { return int32(swap(uint32(n))); }
inline int64 swap(int64 n) noexcept { return int64(swap(uint64(n))); }
