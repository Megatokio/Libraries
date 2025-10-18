// Copyright (c) 2015 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Shared Pointers and Weak Smart Pointers
	=======================================

	This file provides:
	RCPtr<T>:	shared pointer to objects using reference counting.
	WeakPtr<T>:	non-locking pointer to objects managed by RCPtrs for observers.

	Requires:
	For use with RCPtrs a class must provide a data member, typically provided by macro RCDATA.

	Options:
	#define NO_THREADS: use a not thread-safe implementation for the smart pointers.
	#define NO_WEAKPTR: macro RCDATA has by default no support for WeakPtrs.

	Advantages of RCPtr over std::shared_ptr:
	- supports my old stuff
	- no 2nd heap allocation
	- slicker syntax
	- configurable, e.g. no WeakPtrs
	- pure pointer to managed object can be stored in a RCPtr again (helps with complicated casting)

	Disadvantages:
	- less well tested
	- needs a data member in the class
	  -> cannot be used directly for existing classes: needs a wrapper class with RCDATA
	  -> not directly usable with arrays (i never needed this, arrays almost always need unique_ptrs)
	- WeakPtrs: destruction and deallocation of objects can be split in 2 operations
	  -> memory is not freed until the last WeakPtr goes away
	  -> deallocation is done with wrong type -> fails for heaps which use sized alloc/dealloc (rare)
	  -> fails for custom allocators for classes


	Mixed info:

	As with all pointers, the inner workings are thread-safe, not the access to the smart pointer itself.
	If multiple threads can access and modify a smart pointer at the same time, then this must be locked.

	Use of RCPtr and WeakPtr is lock-free (non-blocking).
	The only locking operation is the conversion of a WeakPtr into an RCPtr.

	Support for RCPtr to a class is achieved by adding macro RCDATA to the class definition:

	- macro RCDATA is RCDATA_WITHWEAK or RCDATA_NOWEAK depending on whether NO_WEAKPTR is #defined.
	- macro RCDATA_WITHWEAK
	- macro RCDATA_NOWEAK
	These macros also define a member function 'refcnt()' which returns the hard count.


	Usage example
	-------------

	class Foo
	{
		RCDATA
		int a,b,c; // my stuff
	};

	OR:

	class Foo : public RCObject
	{
		int a,b,c; // my stuff
	};

	RCPtr<Foo> foo = new Foo();
	WeakPtr<Foo> foo2{foo};

	void test()
	{
		RCPtr<Foo> p{foo2};
		if (p) p->bar();
		else foo2 = nullptr;
	}


	Locking of objects and mutex (spinlock) in WeakPtr
	--------------------------------------------------

	the WeakPtr locks the memory by incrementing _rcdata.wc ("weak count").
	the RCPtr additionally locks the object by incrementing _rcdata.hc ("hard count").
	normally both counters must be atomic.
	if the RCPtr decrements _rcdata.hc to zero it must destroy the object.
	if the RCPtr or WeakPtr decrements _rcdata.wc to zero it must free the memory.

	The only locking operation:

	In constructor RCPtr(WeakPtr) the hard count hc must be increased, but _only_ if it is non-zero.
	We can also blindly increase hc and undo it if ++hc == 1.
	In the first case we have a race condition with the possible destruction of the last RCPtr on another thread,
	In the second case there is a race condition with a second thread performing RCPtr(WeakPtr) at the same time.
	If we put a mutex around the second operation, then the mutex is only needed here. fine!
	But in order to avoid unneeded mutex lock/unlock, we test for hc!=0 beforehand anyway.

	With both tests the total chances for failure (without lock) is miniscule.
	If all RCPtr(WeakPtr) conversions for a class happen only on the same thread,
	  e.g. for observers running on a 'gui thread', then this cannot happen at all.
	If all RCPtr destructions and all RCPtr(WeakPtr) conversions for a class happen
	  only on the same _two_ threads then this cannot happen too.
	Then you could use RCDATA_WEAK_NOLOCK in your class definition. But you shouldn't. You'll forget about it.
*/


#pragma once
#include "kio/cdefs.h"
#include <type_traits>
#include <utility>

using uint = unsigned int;

#ifndef NO_THREADS
  #include <atomic>
  #include <thread>
using rc_uint32_t = std::atomic<uint32_t>;
using rc_uint16_t = std::atomic<uint16_t>;
using rc_bool_t	  = std::atomic<bool>;
#else
using rc_uint32_t = uint32_t;
using rc_uint16_t = uint16_t;
using rc_bool_t	  = bool;
#endif

struct RCDataNoWeak
{
	union
	{
		rc_uint32_t hc {0}; // hard (locking) refs
		rc_uint32_t wc;		// total == hard refs
	};
	static rc_bool_t lock; // n.ex.

	RCDataNoWeak() noexcept = default;
	RCDataNoWeak(const RCDataNoWeak&) noexcept {}
	RCDataNoWeak(RCDataNoWeak&&) noexcept {}
	RCDataNoWeak& operator=(const RCDataNoWeak&) noexcept { return *this; }
	RCDataNoWeak& operator=(RCDataNoWeak&&) noexcept { return *this; }
};

struct RCDataWeakNoLock
{
	rc_uint32_t		 wc {0}; // total = hard + weak refs
	rc_uint32_t		 hc {0}; // hard (locking) refs
	static rc_bool_t lock;	 // n.ex.

	RCDataWeakNoLock() noexcept = default;
	RCDataWeakNoLock(const RCDataWeakNoLock&) noexcept {}
	RCDataWeakNoLock(RCDataWeakNoLock&&) noexcept {}
	RCDataWeakNoLock& operator=(const RCDataWeakNoLock&) noexcept { return *this; }
	RCDataWeakNoLock& operator=(RCDataWeakNoLock&&) noexcept { return *this; }
};

struct RCDataWeakWithLock
{
	rc_uint32_t wc {0};	  // total = hard + weak refs
	rc_uint16_t hc {0};	  // hard (locking) refs
	rc_bool_t	lock {0}; // lock for hc while upgrading WeakPtr
	uint8_t		padding {0};

	RCDataWeakWithLock() noexcept = default;
	RCDataWeakWithLock(const RCDataWeakWithLock&) noexcept {}
	RCDataWeakWithLock(RCDataWeakWithLock&&) noexcept {}
	RCDataWeakWithLock& operator=(const RCDataWeakWithLock&) noexcept { return *this; }
	RCDataWeakWithLock& operator=(RCDataWeakWithLock&&) noexcept { return *this; }
};

// clang-format off
#define RCDATA_NOWEAK						\
  mutable RCDataNoWeak _rcdata;				\
  static constexpr bool _has_wc	  = false;	\
  static constexpr bool _has_lock = false;	\
  uint refcnt() volatile const noexcept {return _rcdata.hc;} \
  template<typename> friend class RCPtr;
#define RCDATA_WEAK_NOLOCK					\
  mutable RCDataWeakNoLock _rcdata;			\
  static constexpr bool _has_wc	  = true;	\
  static constexpr bool _has_lock = false;	\
  uint refcnt() volatile const noexcept {return _rcdata.hc;} \
  template<typename> friend class WeakPtr;	\
  template<typename> friend class RCPtr;
#define RCDATA_WEAK_WITHLOCK				\
  mutable RCDataWeakWithLock _rcdata;		\
  static constexpr bool _has_wc	  = true;	\
  static constexpr bool _has_lock = true;	\
  uint refcnt() volatile const noexcept {return _rcdata.hc;} \
  template<typename> friend class WeakPtr;	\
  template<typename> friend class RCPtr;
// clang-format on


#ifdef NO_THREADS
  #define RCDATA_WITHWEAK RCDATA_WEAK_NOLOCK
#else
  #define RCDATA_WITHWEAK RCDATA_WEAK_WITHLOCK
#endif

#ifdef NO_WEAKPTR
  #define RCDATA RCDATA_NOWEAK
#else
  #define RCDATA RCDATA_WITHWEAK
#endif


#ifdef NO_THREADS
extern void rc_lock_spinlock(volatile rc_bool_t& lock) noexcept;   // n.ex.
extern void rc_unlock_spinlock(volatile rc_bool_t& lock) noexcept; // n.ex.
#else

inline void rc_lock_spinlock(volatile rc_bool_t& lock) noexcept
{
	for (int i = 0; lock.exchange(true, std::memory_order_acquire); i++)
	{
		// we didn't get the lock:
		// wait until we see that it's free.
		// this happens only very rarely.
		// if we block too long then yield to other thread:
		while (lock.load(std::memory_order_relaxed))
		{
			if (i > 20) std::this_thread::yield();
		}
	}
}

inline void rc_unlock_spinlock(volatile rc_bool_t& lock) noexcept
{
	lock.store(false, std::memory_order_release); //
}
#endif


template<typename T>
class RCPtr;

/*	__________________________________________________________________________________
	template class WeakPtr stores a weak reference to a object managed by RCPtr.
	WeakPtr can only be created from RCPtr or other WeakPtr, not from a pure pointer.
	The stored pointer cannot be accessed directly.
	To access the stored pointer a RCPtr must be created from the WeakPtr.
*/
template<typename T>
class WeakPtr
{
	template<typename T2>
	friend class RCPtr;

	T* p {nullptr};

	static void retain(T* p) noexcept
	{
		if (p) ++p->_rcdata.wc;
	}
	static void release(T* p) noexcept
	{
		if (p && --p->_rcdata.wc == 0) delete reinterpret_cast<const volatile char*>(p);
	}

public:
	uint refcnt() const noexcept { return p ? uint(p->_rcdata.hc) : 0; }   // hard count
	uint totalcnt() const noexcept { return p ? uint(p->_rcdata.wc) : 0; } // total = hard + weak count

	WeakPtr() noexcept : p(nullptr) {}
	WeakPtr(const WeakPtr& q) noexcept : p(q.p) { retain(p); }
	WeakPtr(WeakPtr&& q) noexcept : p(q.p) { q.p = nullptr; }
	WeakPtr(const RCPtr<T>& q) noexcept : p(q) { retain(p); }
	~WeakPtr() noexcept { release(p); }

	WeakPtr& operator=(const RCPtr<T>& q) noexcept
	{
		retain(q);
		release(p);
		p = q;
		return *this;
	}

	WeakPtr& operator=(const WeakPtr& q) noexcept { return operator=(q.p); }
	WeakPtr& operator=(WeakPtr&& q) noexcept
	{
		std::swap(p, q.p);
		return *this;
	}
	WeakPtr& operator=(std::nullptr_t) noexcept
	{
		release(p);
		p = nullptr;
		return *this;
	}

#define subclass_only typename std::enable_if<std::is_base_of<T, T2>::value, int>::type = 1
#ifdef _GCC
	template<typename T2, subclass_only>
	friend class ::WeakPtr;
#else
	template<typename T2>
	friend class ::WeakPtr;
#endif

	template<typename T2, subclass_only>
	WeakPtr(const WeakPtr<T2>& q) noexcept : p(q.p)
	{
		retain(p);
	}

	template<typename T2, subclass_only>
	WeakPtr(WeakPtr<T2>&& q) noexcept : p(q.p)
	{
		q.p = nullptr;
	}

	template<typename T2, subclass_only>
	WeakPtr& operator=(const WeakPtr<T2>& q) noexcept
	{
		return operator=(q.p);
	}

	template<typename T2, subclass_only>
	WeakPtr& operator=(WeakPtr<T2>&& q) noexcept
	{
		release(p);
		p	= q.p;
		q.p = nullptr;
		return *this;
	}
#undef subclass_only

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	friend void swap(WeakPtr<T>& a, WeakPtr<T>& b) noexcept { std::swap(a.p, b.p); }

	// No direct access to the stored object!
	T* operator->() const  = delete;
	T& operator*() const   = delete;
	   operator T&() const = delete;
	   operator T*() const = delete;

	// test for non-nullptr:
	// 'true' result does not guarantee that the object is still valid!
	operator bool() const noexcept { return p != nullptr; }
};


/*	____________________________________________________________________________
	template class RCPtr is a smart pointer to objects using reference counting.
*/
template<typename T>
class RCPtr
{
	template<class TT>
	friend class RCArray;
	template<class T1, class T2>
	friend class RCHashMap;

	T* p {nullptr};

private:
	static void retain(T* p) noexcept
	{
		if (p) // sequence wc, hc:
		{
			if (T::_has_wc) ++p->_rcdata.wc;
			++p->_rcdata.hc;
		}
	}
	static void release(T* p) noexcept
	{
		if (p)
		{
			if (T::_has_wc) // sequence hc, tc:
			{
				if (--p->_rcdata.hc == 0) p->~T();
				if (--p->_rcdata.wc == 0) delete reinterpret_cast<const volatile char*>(p);
			}
			else
			{
				if (--p->_rcdata.hc == 0) delete p;
			}
		}
	}

public:
	uint refcnt() const noexcept { return p ? uint(p->_rcdata.hc) : 0; }   // hard count
	uint totalcnt() const noexcept { return p ? uint(p->_rcdata.wc) : 0; } // total = hard + weak count

	RCPtr(std::nullptr_t = nullptr) noexcept {}
	RCPtr(T* p) noexcept : p(p) { retain(p); }
	RCPtr(const RCPtr& q) noexcept : p(q.p) { retain(p); }
	RCPtr(RCPtr&& q) noexcept : p(q.p) { q.p = nullptr; }
	~RCPtr() noexcept { release(p); }

	RCPtr& operator=(RCPtr&& q) noexcept
	{
		std::swap(p, q.p);
		return *this;
	}
	RCPtr& operator=(T* q) noexcept
	{
		retain(q);
		release(p);
		p = q;
		return *this;
	}
	RCPtr& operator=(const RCPtr& q) noexcept
	{
		return operator=(q.p); //
	}
	RCPtr& operator=(std::nullptr_t) noexcept
	{
		release(p);
		p = nullptr;
		return *this;
	}

#define subclass_only typename std::enable_if<std::is_base_of<T, T2>::value, int>::type = 1
#ifdef _GCC
	template<typename T2, subclass_only>
	friend class ::RCPtr;
#else
	template<typename T2>
	friend class ::RCPtr;
#endif

	// convert a WeakPtr to RCPtr:
	template<typename T2, subclass_only>
	RCPtr(WeakPtr<T2>& q) noexcept : p(nullptr)
	{
		if (T* qp = q.p) // not null
		{
			if (qp->_rcdata.hc > 0) // has hard locks
			{
				if (T::_has_lock) rc_lock_spinlock(qp->_rcdata.lock);

				if (++qp->_rcdata.hc > 1) // got it
				{
					++qp->_rcdata.wc;
					p = qp;
				}
				else // just destroyed
				{
					// --qp->_rcdata.hc;
					qp->_rcdata.hc = 0;
				}

				if (T::_has_lock) rc_unlock_spinlock(qp->_rcdata.lock);
			}
			else q = nullptr; // clear the WeakPtr so that it no longer locks the memory
		}
	}

	template<typename T2, subclass_only>
	RCPtr(const RCPtr<T2>& q) noexcept : p(q.p)
	{
		retain(p);
	}
	template<typename T2, subclass_only>
	RCPtr(RCPtr<T2>&& q) noexcept : p(q.p)
	{
		q.p = nullptr;
	}

	template<typename T2, subclass_only>
	RCPtr& operator=(const RCPtr<T2>& q) noexcept
	{
		return operator=(q.p);
	}
	template<typename T2, subclass_only>
	RCPtr& operator=(RCPtr<T2>&& q) noexcept
	{
		release(p);
		p	= q.p;
		q.p = nullptr;
		return *this;
	}
#undef subclass_only

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	friend void swap(RCPtr<T>& a, RCPtr<T>& b) noexcept { std::swap(a.p, b.p); }

	T* operator->() const noexcept { return p; }
	   operator T*() const noexcept { return p; }
	T* ptr() const noexcept { return p; }
	T* get() const noexcept { return p; } // for compatibility with std::shared_ptr
	T& ref() const noexcept
	{
		assert(p != nullptr);
		return *p;
	}
	operator T&() const noexcept
	{
		assert(p != nullptr);
		return *p;
	}

	// prevent erroneous use of operator[] with pointer:
	T& operator[](int) const = delete;

	// test for non-nullptr
	operator bool() const volatile noexcept { return p != nullptr; }
};


// ___________________________________________________________
// convenience:
// (auto deducted type)

template<class T>
RCPtr<T> rcptr(T* o)
{
	return RCPtr<T>(o);
}

template<class T>
RCPtr<T> rcptr(const RCPtr<T>& o)
{
	return o;
}

template<class T>
RCPtr<T> rcptr(WeakPtr<T>& o)
{
	return RCPtr<T>(o);
}


// ___________________________________________________________
// relational operators:
// NOTE: it's C++ standard to compare the pointers.
//       a nullptr is less than any other pointer.

template<class T>
bool operator==(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() == b.ptr();
}
template<class T>
bool operator!=(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() != b.ptr();
}
template<class T>
bool operator>(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() > b.ptr();
}
template<class T>
bool operator<(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() < b.ptr();
}
template<class T>
bool operator>=(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() >= b.ptr();
}
template<class T>
bool operator<=(const RCPtr<T>& a, const RCPtr<T>& b) noexcept
{
	return a.ptr() <= b.ptr();
}
