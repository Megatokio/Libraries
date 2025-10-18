// Copyright (c) 2015 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include <memory>

template<class T>
class RCPtr;


/*	Smart Pointer for locking Volatile Objects
	==========================================

	This file provides:
	NVPtr<T>   a smart pointer for locking of volatile objects.
	nvptr<T>() a convenience function to create a NVPtr with auto type deduction prior to c++17.
	NV<T>      a convenience function to cast away 'volatile' if you think you are allowed to do so.

	The idea is to define non-const objects which are accessible from multiple threads as 'volatile'
	because from each thread's perspective they can change state spontaneously.
	The NVPtr locks a volatile object until the NVPtr is destroyed or reassigned.
	operator->() provides access to non-volatile object.

	Requires:
	A class must provide the functions lock() and unlock().


	Usage example
	-------------

	extern volatile Foo* foo;
	void test()
	{
		NVPtr<Foo> p{foo};
		p->bar();
		p->bar2();
	}
	void test2()
	{
		nvptr(foo)->bar();
	}
*/


// convenience const casts:

template<class T>
T* NV(volatile T* o)
{
	return const_cast<T*>(o);
}
template<class T>
T& NV(volatile T& o)
{
	return const_cast<T&>(o);
}
template<class T>
T* NV(std::shared_ptr<volatile T>& o)
{
	return const_cast<T*>(o.get());
}
template<class T>
T* NV(RCPtr<volatile T>& o)
{
	return const_cast<T*>(o.get());
}

// locking pointer:

template<class T>
class NVPtr
{
	T* p = nullptr;

	void lock()
	{
		if (p) p->lock();
	}
	void unlock()
	{
		if (p) p->unlock();
	}

public:
	NO_COPY(NVPtr);
	NVPtr() noexcept : p(nullptr) {}
	NVPtr(NVPtr&& q) noexcept : p(q.p) { q.p = nullptr; }
	explicit NVPtr(volatile T* p) : p(NV(p)) { lock(); }
	//NVPtr(volatile T& q) : p(NV(q)) { lock(); }
	NVPtr(volatile T* p, int nsec) : p(p && NV(p)->trylock(nsec) ? NV(p) : nullptr) {}
	~NVPtr() { unlock(); }

	NVPtr& operator=(NVPtr&& q)
	{
		assert(this != &q);
		unlock();
		p	= q.p;
		q.p = nullptr;
		return *this;
	}

	NVPtr& operator=(volatile T* q)
	{
		if (p != q)
		{
			unlock();
			p = q;
			lock();
		}
		return *this;
	}

	T* operator->() const noexcept { return p; }
	T& operator*() const noexcept { return *p; }
	T* ptr() const noexcept { return p; }
	T& ref() const noexcept { return *p; }

	operator T&() const noexcept { return *p; }
	operator T*() const noexcept { return p; }

	operator bool() const noexcept { return p; }
};


//	#include <mutex>
//	template<>
//	inline NVPtr<std::timed_mutex>::NVPtr(volatile std::timed_mutex* p, int nsec) : p(NV(p))
//	{
//		if (p && !NV(p)->try_lock_for(std::chrono::nanoseconds(nsec))) p = nullptr;
//	}


// convenience:
// (auto deducted type)

template<class T>
NVPtr<T> nvptr(volatile T* o)
{
	return NVPtr<T>(o);
}

template<class T>
NVPtr<T> nvptr(const RCPtr<volatile T>& o)
{
	return NVPtr<T>(o.get());
}

template<class T>
NVPtr<T> nvptr(std::shared_ptr<volatile T>& o)
{
	return NVPtr<T>(o.get());
}


/*
























*/
