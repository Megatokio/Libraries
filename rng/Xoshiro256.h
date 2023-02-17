// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include <math.h>

/*
	Family of RNG to produce 64 random numbers
*/

using Seed256 = uint64[4];
extern uint64 xoshiro256p(Seed256) noexcept;
extern uint64 xoshiro256pp(Seed256) noexcept;
extern uint64 xoshiro256ss(Seed256) noexcept;


// select generator:
#if !defined XOSHIRO256
  #define XOSHIRO256 xoshiro256pp
#endif


class Xoshiro256
{
protected:
	uint64 s[4];
	void   skip(const uint64 bits[4]) noexcept;

public:
	Xoshiro256() noexcept;
	Xoshiro256(const void* q) noexcept;
	Xoshiro256(uint32 seed) noexcept;

	static constexpr uint64 min() { return uint64(0); }
	static constexpr uint64 max() { return ~uint64(0); }

	// full uint64 range:
	uint64 next() noexcept { return XOSHIRO256(s); }
	uint64 operator()() noexcept { return next(); }

	// interval [0 .. [1
	double random() noexcept { return ldexp(double(next()), -64); }

	// interval [0 .. [max
	uint32 random(uint32 max) noexcept;
	uint64 random(uint64 max) noexcept;
	double random(double max) noexcept { return ldexp(next() * max, -64); }

	// interval [min .. max]
	uint32 random(uint32 min, uint32 max) noexcept { return min + random(max - min + 1); }
	uint64 random(uint64 min, uint64 max) noexcept { return min + random(max - min + 1); }
	int32  random(int32 min, int32 max) noexcept { return min + int32(random(uint32(max - min + 1))); }
	int64  random(int64 min, int64 max) noexcept { return min + int64(random(uint64(max - min + 1))); }

	void skip128() noexcept; // skip 2^128 iterations
	void skip192() noexcept; // skip 2^192 iterations
};
