#pragma once
// Copyright (c) 1995 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


inline uint count1bits(uint8 z) noexcept
{
	z = ((z & 0xAAu) >> 1) + (z & 0x55u);
	z = ((z & 0xCCu) >> 2) + (z & 0x33u);
	return (z >> 4) + (z & 0x0Fu);
}

inline uint count1bits(uint16 z) noexcept
{
	z = ((z & 0xAAAAu) >> 1) + (z & 0x5555u);
	z = ((z & 0xCCCCu) >> 2) + (z & 0x3333u);
	z = ((z & 0xF0F0u) >> 4) + (z & 0x0F0Fu);
	return (z >> 8) + (z & 0x00FFu);
}

inline uint count1bits(uint32 z) noexcept
{
	z = ((z & 0xAAAAAAAAu) >> 1) + (z & 0x55555555u);
	z = ((z & 0xCCCCCCCCu) >> 2) + (z & 0x33333333u);
	z = ((z & 0xF0F0F0F0u) >> 4) + (z & 0x0F0F0F0Fu);
	z = ((z & 0xFF00FF00u) >> 8) + (z & 0x00FF00FFu);
	return (z >> 16) + (z & 0x0000FFFFu);
}

inline uint count1bits(uint64 z) noexcept
{
	z = ((z & 0xAAAAAAAAAAAAAAAAuL) >> 1) + (z & 0x5555555555555555uL);
	z = ((z & 0xCCCCCCCCCCCCCCCCuL) >> 2) + (z & 0x3333333333333333uL);
	z = ((z & 0xF0F0F0F0F0F0F0F0uL) >> 4) + (z & 0x0F0F0F0F0F0F0F0FuL);
	z = ((z & 0xFF00FF00FF00FF00uL) >> 8) + (z & 0x00FF00FF00FF00FFuL);
	z = ((z & 0xFFFF0000FFFF0000uL) >> 16) + (z & 0x0000FFFF0000FFFFuL);
	return uint32(z >> 32) + uint32(z);
}

inline uint count1bits(int8 z) noexcept { return count1bits(uint8(z)); }
inline uint count1bits(int16 z) noexcept { return count1bits(uint16(z)); }
inline uint count1bits(int32 z) noexcept { return count1bits(uint32(z)); }
inline uint count1bits(int64 z) noexcept { return count1bits(uint64(z)); }

#ifdef _LINUX
inline uint count1bits(long long z) noexcept { return count1bits(uint64(z)); }
inline uint count1bits(unsigned long long z) noexcept { return count1bits(uint64(z)); }
#else
inline uint count1bits(long z) noexcept { return count1bits(uint64(z)); }
inline uint count1bits(unsigned long z) noexcept { return count1bits(uint64(z)); }
#endif

// ---- count bits in arrays ----

inline uint32 count1bits(cu64ptr a, cu64ptr e) noexcept
{
	uint32 num_bits = 0;
	while (a < e) { num_bits += count1bits(*--e); }
	return num_bits;
}

inline uint32 count1bits(cu32ptr a, cu32ptr e) noexcept
{
#if _sizeof_pointer < 8
	uint32 num_bits = 0;
	while (a < e) { num_bits += count1bits(*--e); }
	return num_bits;
#else
	uint32 num_bits = 0;

	if (size_t(a) & 4 && a < e) { num_bits += count1bits(*a++); }
	if (size_t(e) & 4 && a < e) { num_bits += count1bits(*--e); }

	return num_bits + count1bits(reinterpret_cast<cu64ptr>(a), reinterpret_cast<cu64ptr>(e));
#endif
}

inline uint32 count1bits(cu16ptr a, cu16ptr e) noexcept
{
	uint32 num_bits = 0;

	if (size_t(a) & 2 && a < e) { num_bits += count1bits(*a++); }
	if (size_t(e) & 2 && a < e) { num_bits += count1bits(*--e); }

	return num_bits + count1bits(reinterpret_cast<cu32ptr>(a), reinterpret_cast<cu32ptr>(e));
}

inline uint32 count1bits(cu8ptr a, cu8ptr e) noexcept
{
	uint32 num_bits = 0;

	if (size_t(a) & 1 && a < e) { num_bits += count1bits(*a++); }
	if (size_t(e) & 1 && a < e) { num_bits += count1bits(*--e); }

	return num_bits + count1bits(reinterpret_cast<cu16ptr>(a), reinterpret_cast<cu16ptr>(e));
}

inline uint32 count1bits(cu64ptr p, uint32 count) noexcept { return count1bits(p, p + count); }
inline uint32 count1bits(cu32ptr p, uint32 count) noexcept { return count1bits(p, p + count); }
inline uint32 count1bits(cu16ptr p, uint32 count) noexcept { return count1bits(p, p + count); }
inline uint32 count1bits(cu8ptr p, uint32 count) noexcept { return count1bits(p, p + count); }

inline uint32 count1bitsL(cu8ptr p, uint32 bits) noexcept
{
	// count bits in array:
	// if bits&7 != 0 then the last byte must be filled left-aligned (msbit)

	cu8ptr e		= p + (bits / 8);
	uint   num_bits = (bits &= 7) ? count1bits(uint8(*e >> (8 - bits))) : 0;
	return num_bits + count1bits(p, e);
}

inline uint32 count1bitsR(cu8ptr p, uint32 bits) noexcept
{
	// count bits in array:
	// if bits&7 != 0 then the last byte must be filled right-aligned (lsbit)

	cu8ptr e		= p + (bits / 8);
	uint   num_bits = (bits &= 7) ? count1bits(uint8(*e << (8 - bits))) : 0;
	return num_bits + count1bits(p, e);
}
