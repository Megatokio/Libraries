#pragma once
// Copyright (c) 2015 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <mutex>


/*	NVPtr<T>
	retains and locks a volatile object
	until the NVPtr is destroyed or reassigned 
	operator->() provides access to non-volatile object

	class T must provide:
		- void lock() 
		- void unlock()
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


// locking pointer:

template<class T>
class NVPtr
{
	typedef volatile T vT;

	T* p;

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
	NVPtr() : p(nullptr) {}
	NVPtr(NVPtr&& q) : p(q.p) { q.p = nullptr; }
	NVPtr(vT* p) : p(NV(p)) { lock(); }
	NVPtr(vT& q) : p(NV(&q)) { lock(); }
	~NVPtr() { unlock(); }

	NVPtr(vT* p, int nsec) : p(NV(p))
	{
		if (p && !p->trylock(nsec)) p = nullptr;
	}

	NVPtr& operator=(NVPtr&& q)
	{
		assert(this != &q);
		unlock();
		p	= q.p;
		q.p = nullptr;
		return *this;
	}

	NVPtr& operator=(vT* q)
	{
		if (p != q)
		{
			unlock();
			p = q;
			lock();
		}
		return *this;
	}

	T* operator->() const { return p; }
	T& operator*() const { return *p; }
	T* ptr() const { return p; }
	T& ref() const { return *p; }

	operator T&() const { return *p; }
	operator T*() const { return p; }

	bool isNotNull() const { return p != nullptr; }
	bool isNull() const { return p == nullptr; }

	operator bool() const { return p; }
};

template<>
inline NVPtr<std::timed_mutex>::NVPtr(volatile std::timed_mutex* p, int nsec) : p(NV(p))
{
	if (p && !NV(p)->try_lock_for(std::chrono::nanoseconds(nsec))) p = nullptr;
}


// convenience:
// (auto deduced type)

template<class T>
NVPtr<T> nvptr(volatile T* o)
{
	return NVPtr<T>(o);
}

template<class T>
NVPtr<T> nvptr(std::shared_ptr<volatile T>& o)
{
	return NVPtr<T>(o.get());
}


/*
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
  
*/
