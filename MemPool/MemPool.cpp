// Copyright (c) 2008 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "MemPool.h"

#define ALIGNMENT_MASK (_MAX_ALIGNMENT - 1)


MemPool::MemPool() noexcept : freesize(0), data(nullptr) { xlogIn("new MemPool"); }

MemPool::~MemPool() noexcept
{
	xlogIn("~MemPool");
	purge();
}

/*static*/ inline ptr MemPool::new_data(size_t bytes) noexcept { return new char[sizeof(ptr) + bytes] + sizeof(ptr); }

/*static*/ inline void MemPool::delete_data(ptr data) noexcept { delete[] (data - sizeof(ptr)); }

/*static*/ inline ptr& MemPool::prev_data(ptr data) noexcept { return *reinterpret_cast<ptr*>(data - sizeof(ptr)); }

void MemPool::purge() noexcept
{
	while (data)
	{
		ptr prev = prev_data(data);
		delete_data(data);
		data = prev;
	}
	freesize = 0;
}

ptr MemPool::alloc(size_t bytes) throws
{
	xxlogline("MemPool.alloc %zu bytes", bytes);

	if (bytes <= freesize) // fits in current buffer?
	{
		freesize -= bytes;
		return data + freesize;
	}

	if (bytes <= MAXREQ_SIZE) // small request?
	{
		ptr newdata		   = new_data(BUFFER_SIZE);
		prev_data(newdata) = data;
		data			   = newdata;
		freesize		   = BUFFER_SIZE - bytes;
		return newdata + freesize;
	}

	// large request

	ptr newdata = new_data(bytes);
	if (data)
	{
		prev_data(newdata) = prev_data(data); // neuen Block 'unterheben'
		prev_data(data)	   = newdata;
	}
	else
	{
		prev_data(newdata) = nullptr;
		data			   = newdata;
		freesize		   = 0; // superfluous
	}
	return newdata;
}

ptr MemPool::allocmem(size_t bytes) throws
{
	ptr	   p = alloc(bytes);
	size_t n = freesize & ALIGNMENT_MASK;
	freesize -= n;
	return p - n;
}

str MemPool::allocstr(size_t len) throws
{
	ptr p  = alloc(len + 1);
	p[len] = 0;
	return p;
}

str MemPool::dupstr(cstr q) throws
{
	if (q == nullptr) return nullptr;
	size_t len = strlen(q) + 1;
	ptr	   z   = alloc(len);
	memcpy(z, q, len);
	return z;
}


#ifndef NDEBUG

/*static*/ void MemPool::test()
{
	ptr p1 = new_data(33);
	ptr p2 = new_data(31);
	assert((uintptr_t(p1) & ALIGNMENT_MASK) == 0);
	assert((uintptr_t(p2) & ALIGNMENT_MASK) == 0);
	delete_data(p1);
	delete_data(p2);
}

ON_INIT(MemPool::test);

#endif
