#pragma once
// Copyright (c) 2020 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

//#include "kio/kio.h"
#include <atomic>
typedef unsigned int uint;


namespace kio
{

/**
 *  This template class provides a queue to connect two threads.
 *  Writer and reader can - and are expected to be - on different threads.
 *  Writer and reader thread can access the queue without a mutex.
 *  Therefore only one thread can write and only one thread can read.
 *  If multiple threads can read or write a queue, then this class is unsuitable.
 */
template<typename T, uint SIZE>
class Queue
{
	static const uint MASK = SIZE - 1;
	static_assert(SIZE > 0 && (SIZE & MASK) == 0, "size must be a power of 2");

protected:
	T				  buffer[SIZE]; // write -> wp++ -> read -> rp++
	std::atomic<uint> rp = 0;		// only modified by reader
	std::atomic<uint> wp = 0;		// only modified by writer

	static inline void copy(T* z, const T* q, uint n) noexcept // helper: hopefully optimized proper copy
	{
		for (uint i = 0; i < n; i++) { z[i] = std::move(q[i]); }
	}

	void copy_q2b(T*, uint n) noexcept;		  // helper: copy queue to external linear buffer
	void copy_b2q(const T*, uint n) noexcept; // helper: copy external linear buffer to queue

public:
	Queue()	 = default;
	~Queue() = default;

	uint avail() const noexcept { return wp - rp; }
	uint free() const noexcept { return SIZE - avail(); }

#define FENCE std::atomic_thread_fence(std::memory_order_release)

	T get() noexcept
	{
		assert(avail());
		uint i = rp;
		T	 c = std::move(buffer[i & MASK]);
		FENCE;
		rp = i + 1;
		return c;
	}
	void put(T&& c) noexcept
	{
		assert(free());
		uint i			 = wp;
		buffer[i & MASK] = std::move(c);
		FENCE;
		wp = i + 1;
	}

	uint read(T* z, uint n) noexcept
	{
		n = min(n, avail());
		copy_q2b(z, n);
		FENCE;
		rp += n;
		return n;
	}
	uint write(const T* q, uint n) noexcept
	{
		n = min(n, free());
		copy_b2q(q, n);
		FENCE;
		wp += n;
		return n;
	}
};


template<typename T, uint SIZE>
inline void Queue<T, SIZE>::copy_b2q(const T* q, uint n) noexcept
{
	uint wi = wp & MASK;
	uint n1 = SIZE - wi;

	if (n > n1)
	{
		copy(buffer, q + n1, n - n1);
		n = n1;
	}

	copy(buffer + wi, q, n);
}

template<typename T, uint SIZE>
inline void Queue<T, SIZE>::copy_q2b(T* z, uint n) noexcept
{
	uint ri = rp & MASK;
	uint n1 = SIZE - ri;

	if (n > n1)
	{
		copy(z + n1, buffer, n - n1);
		n = n1;
	}

	copy(z, buffer + ri, n);
}

} // namespace kio
