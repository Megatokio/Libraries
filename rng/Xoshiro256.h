// Copyright (c) 2020 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include <math.h>

/*
	Family of RNG to produce 64 random numbers
*/

namespace kio
{

using RNGState256 = uint64[4];
extern uint64 xoshiro256p(RNGState256) noexcept;
extern uint64 xoshiro256pp(RNGState256) noexcept;
extern uint64 xoshiro256ss(RNGState256) noexcept;


// select generator:
#if !defined XOSHIRO256
  #define XOSHIRO256 xoshiro256pp
#endif


class Xoshiro256
{
public:
	uint64 s[4];

	Xoshiro256() noexcept;
	Xoshiro256(const void* q) noexcept;
	Xoshiro256(uint32 seed) noexcept;

	// 0 .. uint64max:
	uint64 next() noexcept { return XOSHIRO256(s); }

	// interval [0 .. [1
	double random() noexcept { return ldexp(double(next()), -64); }
	float  randomf() noexcept { return ldexp(float(next()), -64); }

	// interval [0 .. [max
	uint32 random(uint32 max) noexcept;
	uint64 random(uint64 max) noexcept;
	double random(double max) noexcept { return max * random(); }
	float  random(float max) noexcept { return max * randomf(); }

	// fast forward:
	void jump() noexcept;	   // skip 2^128 iterations
	void jump_long() noexcept; // skip 2^192 iterations

private:
	void _skip(const uint64 bits[4]) noexcept;
};

} // namespace kio
