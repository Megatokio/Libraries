// Copyright (c) 2015 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Volatile Objects, Reference Counter, Weak and Locking Pointer:
	==============================================================

	This file provides thread-safe smart pointers using reference counting.
	As with all pointers, the inner workings are thread-safe, not the access to the smart pointer.
	If multiple threads can access and modify a smart pointer at the same time, then this needs locking.

	RCPtr<T>:
	pointer to objects which provide reference counters.

	WeakPtr<T>:
	pointer to objects which provide reference counters.
	unlike RCPtr this does not lock the object and is intended for observers.
	to access the pointer you must create a RCPtr from the WeakPtr.

	NVPtr<T>:
	pointer to initially volatile objects which provide lock() and unlock() function.
	In multi-threaded applications RCPtr can be used for volatile objects.
	then dereferencing the RCPtr returns a volatile object.
	this can be converted into an accessible non-volatile object by storing the ptr into a NVPtr.

	Macros to help including the required support into classes for use with RCPtrs:
	RCDATA:			with support for WeakPtr. hard ref count is only uint16.
	RCDATA_noWEAK: 	no support for WeakPtr
	RCDATA_noLOCK:	no spinlock used in ctor RCPtr(WeakPtr).
		this can be used if all observers for any such object only calls RCPtr(WeakPtr)
		on the same thread. otherwise there is a miniscule risk of a race condition.
	you can make further variants of these macros if needed.

	Options:
	#define NO_THREADS: use a not thread-safe implementation for the smart pointers.


	Description of operation
	========================

	the WeakPtr locks the memory by incrementing _rc.wc ("weak count").
	the RCPtr additionally locks the object by incrementing _rc.hc ("hard count").
	normally both counters must be atomic.
	therefore incrementing is pain-free and decrementing must test for reaching 0:
	if the RCPtr decrements _rc.hc to 0 it must destroy the object.
	if the RCPtr or WeakPtr decrements _rc.wc to 0 it must free the memory.

!!	this separation will not work with heaps which use free(ptr,size).
!!	no custom memory allocator can be used for the object.
!!	after beeing destroyed the memory is not freed until all WeakPtrs to it are cleared.

	problematic is the conversion of a WeakPtr into a RCPtr for access of the object:
	if hardcnt==0 before incrementing, then the object is not valid.
	=> but just between seeing hardcnt!=0 and ++hardcnt the hardcnt can be decremented to 0 and destroyed
	   on another thread.
	alternatively: if hardcnt==1 after incrementing, then the object is not valid.
	=> but just between ++hardcnt and testing hardcnt!=1 another thread could just do the same,
	   then one of both will see a 2 instead of 1 and believe the object is valid.

	therefore locking cannot be avoided.
	if using a test after ++hardcnt then only the conversion from WeakPtr to RCPtr must be locked.
	otherwise the RCPtr dtor would also need locking.
	this implementation actually does both tests: hardcnt!=0 => locked(++hardcnt!=1) => got it.
	the probability of blocking is extremely low or, in many use cases, even impossible,
	because this requires 2 threads to create a RCPtr from a WeakPtr at the same time.
*/


#pragma once
#include "Array.h"
#include <type_traits>
#include <utility>

#ifndef NO_THREADS
  #include <atomic>
  #include <thread>
using rc_uint32_t = std::atomic<uint32>;
using rc_uint16_t = std::atomic<uint16>;
using rc_bool_t	  = std::atomic<bool>;
#else
using rc_uint32_t = uint32;
using rc_uint16_t = uint16;
using rc_bool_t	  = bool;
#endif

struct RCDataNoWeak
{
	union
	{
		rc_uint32_t wc {0}; // total = hard refs
		rc_uint32_t hc;		// hard (locking) refs
	};
	static rc_bool_t	  lock; // n.ex.
	static constexpr bool _has_hc	= false;
	static constexpr bool _has_lock = false;

	RCDataNoWeak() noexcept = default;
	RCDataNoWeak(const RCDataNoWeak&) noexcept {}
	RCDataNoWeak(RCDataNoWeak&&) noexcept {}
	RCDataNoWeak& operator=(const RCDataNoWeak&) noexcept { return *this; }
	RCDataNoWeak& operator=(RCDataNoWeak&&) noexcept { return *this; }
};

struct RCDataWeakNoLock
{
	rc_uint32_t			  wc {0}; // total = hard + weak refs
	rc_uint32_t			  hc {0}; // hard (locking) refs
	static rc_bool_t	  lock;	  // n.ex.
	static constexpr bool _has_hc	= true;
	static constexpr bool _has_lock = false;

	RCDataWeakNoLock() noexcept = default;
	RCDataWeakNoLock(const RCDataWeakNoLock&) noexcept {}
	RCDataWeakNoLock(RCDataWeakNoLock&&) noexcept {}
	RCDataWeakNoLock& operator=(const RCDataWeakNoLock&) noexcept { return *this; }
	RCDataWeakNoLock& operator=(RCDataWeakNoLock&&) noexcept { return *this; }
};

struct RCDataWeakWithLock
{
	rc_uint32_t			  wc {0};	// total = hard + weak refs
	rc_uint16_t			  hc {0};	// hard (locking) refs
	rc_bool_t			  lock {0}; // lock for hc while upgrading WeakPtr
	uint8				  padding {0};
	static constexpr bool _has_hc	= true;
	static constexpr bool _has_lock = true;

	RCDataWeakWithLock() noexcept = default;
	RCDataWeakWithLock(const RCDataWeakWithLock&) noexcept {}
	RCDataWeakWithLock(RCDataWeakWithLock&&) noexcept {}
	RCDataWeakWithLock& operator=(const RCDataWeakWithLock&) noexcept { return *this; }
	RCDataWeakWithLock& operator=(RCDataWeakWithLock&&) noexcept { return *this; }
};

#define RCDATA_NOWEAK      \
  using RC = RCDataNoWeak; \
  mutable RC _rc;          \
  template<typename>       \
  friend class WeakPtr;    \
  template<typename>       \
  friend class RCPtr;
#define RCDATA_WEAK_NOLOCK     \
  using RC = RCDataWeakNoLock; \
  mutable RC _rc;              \
  template<typename>           \
  friend class WeakPtr;        \
  template<typename>           \
  friend class RCPtr;
#define RCDATA_WEAK_WITHLOCK     \
  using RC = RCDataWeakWithLock; \
  mutable RC _rc;                \
  template<typename>             \
  friend class WeakPtr;          \
  template<typename>             \
  friend class RCPtr;

#ifdef NO_WEAKPTR
  #define RCDATA RCDATA_NOWEAK
#else
  #ifdef NO_THREADS
	#define RCDATA RCDATA_WEAK_NOLOCK
  #else
	#define RCDATA RCDATA_WEAK_WITHLOCK
  #endif
#endif


inline void rc_lock_spinlock(rc_bool_t& lock) noexcept
{
#ifndef NO_THREADS
	for (uint i = 0; lock.exchange(true, std::memory_order_acquire); i++)
	{
		// we didn't get the lock:
		// this will rarely ever happen because this requires
		// 2 WeakPtrs to be converted into a RCPtr at the same time.

		// wait until we see that it's free:
		// if we block too long then yield to other thread:
		// this is even more unlikely because this means the sheduler interrupted the other thread
		// just while it executed the short piece of code between lock() and unlock()
		while (lock.load(std::memory_order_relaxed))
		{
			if (i > 20) std::this_thread::yield();
		}
	}
#endif
}

inline void rc_unlock_spinlock(rc_bool_t& lock) noexcept
{
#ifndef NO_THREADS
	lock.store(false, std::memory_order_release); //
#endif
}


/*	__________________________________________________________________________________
	template class WeakPtr stores a weak reference to a object managed by RCPtr.
	WeakPtr can only be created from RCPtr or other WeakPtr, not from a naked pointer.
	The stored pointer cannot be accessed directly.
	To access the stored pointer a RCPtr must be created from the WeakPtr.
*/
template<typename T>
class WeakPtr
{
protected:
	T* p = nullptr;

	static void retain(T* p) noexcept
	{
		if (p) ++p->_rc.wc;
	}
	static void release(T* p) noexcept
	{
		if (p && --p->_rc.wc == 0) delete reinterpret_cast<const volatile char*>(p);
	}

public:
	uint refcnt() const noexcept { return p ? uint(p->_rc.hc) : 0; }   // hard count
	uint totalcnt() const noexcept { return p ? uint(p->_rc.wc) : 0; } // total = hard + weak count

	WeakPtr(std::nullptr_t = nullptr) noexcept : p(nullptr) {}
	explicit WeakPtr(const WeakPtr& q) noexcept : p(q.p) { retain(p); }
	WeakPtr(WeakPtr& q) noexcept : p(q.p) { retain(p); }
	WeakPtr(WeakPtr&& q) noexcept : p(q.p) { q.p = nullptr; }
	~WeakPtr() noexcept { release(p); }

	WeakPtr(T* q) noexcept : p(q) { retain(p); }
	WeakPtr& operator=(T* q) noexcept
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
	WeakPtr(WeakPtr<T2>& q) noexcept : p(q.p)
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

	// prevent erroneous use of operator[] with pointer:
	T& operator[](uint32) const = delete;

	// test for non-nullptr:
	// 'true' result does not guarantee that the object is still valid!
	operator bool() const noexcept { return p != nullptr; }
};

struct Test1
{
	RCDATA_WEAK_WITHLOCK
	void t1() { WeakPtr<Test1> wp; }
	void t2() { WeakPtr<volatile Test1> wp; }
};

static WeakPtr<Test1> _foo1;
static_assert(sizeof(Test1) == 8, "");
static_assert(sizeof(WeakPtr<Test1>) == 8, "");

struct Test12
{
	RCDATA_WEAK_NOLOCK
	void t() { WeakPtr<Test12> wp; }
};

static WeakPtr<Test1> _foo12;
static_assert(sizeof(Test12) == 8, "");
static_assert(sizeof(WeakPtr<Test1>) == 8, "");


/*	__________________________________________________________________________________
	template class RCPtr is a smart pointer to a object using reference counting.
*/
template<typename T>
class RCPtr
{
	template<class TT>
	friend class RCArray;
	template<class T1, class T2>
	friend class RCHashMap;

	static constexpr bool has_hc   = T::RC::_has_hc;
	static constexpr bool has_lock = T::RC::_has_lock;

	T* p = nullptr;

private:
	static void retain(T* p) noexcept
	{
		if (p) // sequence wc, hc:
		{
			++p->_rc.wc;
			if (has_hc) { ++p->_rc.hc; }
		}
	}
	static void release(T* p) noexcept
	{
		if (p)
		{
			if (has_hc) // sequence hc, tc:
			{
				if (--p->_rc.hc == 0) p->~T();
				if (--p->_rc.wc == 0) delete reinterpret_cast<const volatile char*>(p);
			}
			else
			{
				if (--p->_rc.wc == 0) delete p;
			}
		}
	}

public:
	uint refcnt() const noexcept { return p ? uint(p->_rc.hc) : 0; }   // hard count
	uint totalcnt() const noexcept { return p ? uint(p->_rc.wc) : 0; } // total = hard + weak count

	RCPtr(std::nullptr_t = nullptr) noexcept {}
	RCPtr(T* p) noexcept : p(p) { retain(p); }
	explicit RCPtr(const RCPtr& q) noexcept : p(q.p) { retain(p); }
	RCPtr(RCPtr& q) noexcept : p(q.p) { retain(p); }
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

	// convert a WeakPtr into a RCPtr:
	template<typename T2, subclass_only>
	RCPtr(const WeakPtr<T2>& q) noexcept : p(nullptr)
	{
		T* qp = q.p;
		if (qp && qp->_rc.hc != 0)
		{
			if (has_lock) rc_lock_spinlock(qp->_rc.lock);

			if (++qp->_rc.hc == 1) // just destroyed
			{
				--qp->_rc.hc = 0;
			}
			else // got it
			{
				++qp->_rc.wc;
				p = qp;
			}

			if (has_lock) rc_unlock_spinlock(qp->_rc.lock);
		}
	}

	template<typename T2, subclass_only>
	RCPtr(RCPtr<T2>& q) noexcept : p(q.p)
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
	T& operator*() const noexcept
	{
		assert(p != nullptr);
		return *p;
	}
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
	operator T*() const noexcept { return p; }

	// prevent erroneous use of operator[] with pointer:
	T& operator[](uint32) const = delete;

	// test for non-nullptr
	operator bool() const noexcept { return p != nullptr; }
};

// ______________________________
struct Test2
{
	RCDATA
	void t1() { RCPtr<Test2> wp; }
	void t2() { RCPtr<volatile Test2> wp; }
};
struct Test3
{
	RCDATA_NOWEAK
	void t1() { RCPtr<Test3> wp; }
	void t2() { RCPtr<volatile Test3> wp; }
};

static RCPtr<Test2> _foo2;
static_assert(sizeof(Test2) == 8, "");
static_assert(sizeof(RCPtr<Test2>) == 8, "");


template<typename T>
cstr tostr(const WeakPtr<T>& p)
{
	// return 1-line description of object for debugging and logging:
	return p ? tostr(*p) : "nullptr";
}

template<typename T>
cstr tostr(const RCPtr<T>& p)
{
	// return 1-line description of object for debugging and logging:
	return p ? tostr(*p) : "nullptr";
}


// _____________________________________________________________________________________________________________
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


// _____________________________________________________________________________________________________________
// convenience base classes:

class RCBase // non-virtual
{
	RCDATA
public:
	~RCBase() {}
	uint refcnt() const noexcept { return _rc.hc; }
};
class RCBaseNoWeak // non-virtual
{
	RCDATA_NOWEAK
public:
	~RCBaseNoWeak() {}
	uint refcnt() const noexcept { return _rc.hc; }
};


class RCObject // virtual
{
	RCDATA
public:
	virtual ~RCObject() {}
	uint refcnt() const noexcept { return _rc.hc; }
};

class RCObjectNoWeak // virtual
{
	RCDATA_NOWEAK
public:
	virtual ~RCObjectNoWeak() {}
	uint refcnt() const noexcept { return _rc.hc; }
};


// _____________________________________________________________________________________________________________
// NVPtr locking pointer:

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
	using vT = volatile T;

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
	NVPtr(vT* p) : p(NV(p)) { lock(); }
	//NVPtr(vT& q) : p(NV(&q)) { lock(); }
	NVPtr(vT* p, int nsec) : p(p && NV(p)->trylock(nsec) ? NV(p) : nullptr) {}
	~NVPtr() { unlock(); }

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
// (auto deduced type)

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


// _____________________________________________________________________________________________________________
// convenience subclasses for Array and HashMap:

template<class T>
class RCArray : public Array<RCPtr<T>>
{
protected:
	using Array<RCPtr<T>>::cnt;
	using Array<RCPtr<T>>::data;

public:
	explicit RCArray() throws {}
	explicit RCArray(uint cnt, uint max = 0) throws : Array<RCPtr<T>>(cnt, max) {}

	uint indexof(const T* item) const noexcept // find first occurance or return ~0u
	{										   // compares object addresses (pointers)
		for (uint i = 0; i < cnt; i++)
		{
			if (data[i].p == item) return i;
		}
		return ~0u;
	}

	bool contains(const T* item) const noexcept { return indexof(item) != ~0u; }
	void removeitem(const T* item, bool fast = 0) noexcept
	{
		uint i = indexof(item);
		if (i != ~0u) remove(i, fast);
	}

	void remove(const T* item, bool fast = 0) noexcept { removeitem(item, fast); }
	void remove(uint idx, bool fast = 0) noexcept { this->removeat(idx, fast); }

	using Array<RCPtr<T>>::append;
	RCPtr<T>& append(T* q) throws { return RCArray<T>::append(RCPtr<T>(q)); }

	using Array<RCPtr<T>>::operator<<;
	RCArray&			   operator<<(T* q) throws
	{
		append(q);
		return *this;
	}
};


// TODO: HashMap.h must be included first
template<class KEY, class ITEM>
class HashMap;
template<class KEY, class T>
class RCHashMap : public HashMap<KEY, RCPtr<T>>
{
public:
	// default: for up to 1024 items without resizing
	explicit RCHashMap(uint max = 1 << 10) throws : HashMap<KEY, RCPtr<T>>(max) {}
};
