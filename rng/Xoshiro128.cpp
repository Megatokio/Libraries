/*
Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)
Modified for his own use by kio in 2020.

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide. This software is distributed without any warranty.

See <http://creativecommons.org/publicdomain/zero/1.0/>.
See	<http://prng.di.unimi.it>
*/

#include "Xoshiro128.h"
#include <math.h>
#include <random>


namespace kio
{

static inline uint32 rotl(const uint32 x, int k) noexcept { return (x << k) | (x >> (32 - k)); }

inline constexpr int msbit(uint32 n) noexcept
{
	int b = 0, i = 16;
	do {
		if (n >> i)
		{
			n >>= i;
			b += i;
		}
	}
	while ((i >>= 1));
	return b;
}

uint32 xoshiro128p(uint32* s) noexcept
{
	// This is xoshiro128+ 1.0, our best and fastest generator for floating-point
	// numbers. We suggest to use its upper bits for floating-point
	// generation, as it is slightly faster than xoshiro128++/xoshiro128**. It
	// passes all tests we are aware of except for the lowest three bits,
	// which might fail linearity tests (and just those), so if low linear
	// complexity is not considered an issue (as it is usually the case) it
	// can be used to generate 32-bit outputs, too.
	//
	// Use a sign test to extract a random Boolean value,
	// and	right shifts to extract subsets of bits.
	// The state must be seeded so that it is not everywhere zero.

	const uint32 result = s[0] + s[3];
	const uint32 t		= s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 11);

	return result;
}

uint32 xoshiro128pp(uint32* s) noexcept
{
	// This is xoshiro128++ 1.0, one of our all-purpose, rock-solid generators.
	// It has excellent (sub-ns) speed, a state (128 bits) that is large
	// enough for any parallel application, and it passes all tests we are
	// aware of.
	//
	// For generating just floating-point numbers, xoshiro128+ is even faster.
	// The state must be seeded so that it is not everywhere zero.

	const uint32 result = rotl(s[0] + s[3], 7) + s[0];
	const uint32 t		= s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 11);

	return result;
}

uint32 xoshiro128ss(uint32* s) noexcept
{
	// This is xoshiro128** 1.1, one of our all-purpose, rock-solid
	// generators. It has excellent (sub-ns) speed, a state (128 bits) that is
	// large enough for any parallel application, and it passes all tests we
	// are aware of.
	//
	// For generating just floating-point numbers, xoshiro128+ is even faster.
	// The state must be seeded so that it is not everywhere zero.

	const uint32 result = rotl(s[1] * 5, 7) * 9;
	const uint32 t		= s[1] << 9;

	s[2] ^= s[0];
	s[3] ^= s[1];
	s[1] ^= s[2];
	s[0] ^= s[3];

	s[2] ^= t;

	s[3] = rotl(s[3], 11);

	return result;
}

uint32 Xoshiro128::random(uint32 max) noexcept
{
	// interval [0 .. [max

	if (max == 0u) return uint32(next());

	const int n = 31 - msbit(max);

	for (;;)
	{
		uint32 r = uint32(next()) >> n;
		if (r < max) return r;
	}
}

void Xoshiro128::_skip(const uint32 bits[4]) noexcept
{
	// helper for jump() and jump_long():

	uint32 s0 = 0;
	uint32 s1 = 0;
	uint32 s2 = 0;
	uint32 s3 = 0;

	for (uint i = 0; i < 4; i++)
		for (uint b = 0; b < 32; b++)
		{
			if (bits[i] & (1u << b))
			{
				s0 ^= state[0];
				s1 ^= state[1];
				s2 ^= state[2];
				s3 ^= state[3];
			}
			next();
		}

	state[0] = s0;
	state[1] = s1;
	state[2] = s2;
	state[3] = s3;
}

void Xoshiro128::jump() noexcept
{
	// This is the short jump function for the generator. It is equivalent
	// to 2^64 calls to next(); it can be used to generate 2^64
	// non-overlapping subsequences for parallel computations.

	static const uint32 bits[] = {0x8764000b, 0xf542d2d3, 0x6fa035c3, 0x77f2db5b};
	_skip(bits);
}

void Xoshiro128::jump_long() noexcept
{
	// This is the long jump function for the generator. It is equivalent to
	// 2^96 calls to next(); it can be used to generate 2^32 starting points,
	// from each of which jump() will generate 2^32 non-overlapping
	// subsequences for parallel distributed computations.

	static const uint32 bits[] = {0xb523952e, 0x0b6f099f, 0xccf5a0ef, 0x1c580662};
	_skip(bits);
}

Xoshiro128::Xoshiro128(const void* q) noexcept
{
	memcpy(state, q, sizeof state);
	next();
}

Xoshiro128::Xoshiro128(uint32 seed) noexcept
{
	srand(seed);
	memset(state, 0xf5, sizeof(state));
	state[0] = unsigned(rand());
	next();
}

Xoshiro128::Xoshiro128() noexcept : Xoshiro128(std::random_device {}()) {}

} // namespace kio
