#pragma once
// Copyright (c) 2015 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Array.h"
#include <type_traits>

/*	Volatile Objects, Reference Counter and Locking Pointer:

	RCPtr:
	pointer to objects which implement reference counting with retain() and release().

	RCObject:
	a simple base class for Objects is in RCObject.h

	class T must provide:
		retain() const
		release() const
	if T is volatile:
		retain() volatile const
		release() volatile const
	optional:
		refcnt()
		operator[]
		operator == != > >= < <=
		print(FD&, uint indent) const			// formatted output for debugger, logfile etc.
		serialize(FD&) const throws	 or
		serialize(FD&, void*) const throws
		deserialize(FD&) throws  or
		deserialize(FD&, void*) throws
		static T* newFromFile(FD&, void*) throws;	// factory method to deserialize polymorthic objects

	RCArray:
	template Array<T> can be used with Array<RCPtr<T>>.
	A template RCArray<T> is defined for convenience.

	NVPtr:
	In multi-threaded applications RCPtr may refer to a mutable object.
	then dereferencing the RCPtr returns a mutable object.
	retain() and release() must be defined volatile and probably use locking in this case.

	A RCPtr cannot be deleted (reassigned, go out of scope) while a NVPtr exists which points to the same object.
	This results in a dead lock. The NVPtr must be deleted (reassigned, go out of scope) before.
*/

template<class T>
class RCPtr
{
	template<class TT>
	friend class RCArray;
	template<class T1, class T2>
	friend class RCHashMap;

protected:
	T* p;

private:
	void retain() const noexcept
	{
		if (p) p->retain();
	}
	void release() const noexcept
	{
		if (p) p->release();
	}

public:
	RCPtr() noexcept : p(nullptr) {}
	RCPtr(T* p) noexcept : p(p) { retain(); }
	explicit RCPtr(const RCPtr& q) noexcept : p(q.p) { retain(); }
	RCPtr(RCPtr& q) noexcept : p(q.p) { retain(); }
	RCPtr(RCPtr&& q) noexcept : p(q.p) { q.p = nullptr; }
	~RCPtr() noexcept { release(); }

	RCPtr& operator=(RCPtr&& q) noexcept
	{
		std::swap(p, q.p);
		return *this;
	}
	RCPtr& operator=(const RCPtr& q) noexcept
	{
		q.retain();
		release();
		p = q.p;
		return *this;
	}
	RCPtr& operator=(T* q) noexcept
	{
		if (q) q->retain();
		release();
		p = q;
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
	template<typename T2, subclass_only>
	RCPtr& operator=(RCPtr<T2>&& q) noexcept
	{
		q.retain();
		release();
		p = q.ptr();
		return *this;
	}
	template<typename T2, subclass_only>
	RCPtr& operator=(const RCPtr<T2>& q) noexcept
	{
		q.retain();
		release();
		p = q.ptr();
		return *this;
	}
	template<typename T2, subclass_only>
	RCPtr(RCPtr<T2>& q) noexcept : p(q.ptr())
	{
		retain();
	}
	template<typename T2, subclass_only>
	RCPtr(RCPtr<T2>&& q) noexcept : p(q.ptr())
	{
		retain();
	}
#undef subclass_only

	// factory method
	// not needed because RCPtr is final and can't be subclassed. (but the RCObject can.)
	//static RCPtr newFromFile (FD& fd, void* data=nullptr)	throws { return std::move(RCPtr(fd.read_uint8()?T::newFromFile(fd,data):nullptr)); }

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap(RCPtr<T>& a, RCPtr<T>& b) noexcept { std::swap(a.p, b.p); }

	T* operator->() const noexcept { return p; }
	T& operator*() const noexcept { return *p; }
	T* ptr() const noexcept { return p; }
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

	uint refcnt() const noexcept { return p ? p->refcnt() : 0; }

	// prevent erroneous use of operator[] with pointer:
	T& operator[](uint32) const = delete;

	// for convenience, these are also declared volatile:
	// the result is unreliable and must be checked again after locking.
	bool isNotNull() const volatile noexcept { return p != nullptr; }
	bool isNull() const volatile noexcept { return p == nullptr; }
	bool is(const T* b) const volatile noexcept { return p == b; }
	bool isnot(const T* b) const volatile noexcept { return p != b; }
		 operator bool() const volatile noexcept { return p != nullptr; }

	void print(FD&, cstr indent) const throws;
	void serialize(FD&, void* data = nullptr) const throws;
	void deserialize(FD&, void* data = nullptr) throws;
};

#if 0
  #include "RCObject.h"
  #include "template_helpers.h"
static_assert(kio::has_oper_star<RCPtr<RCObject>>::value,"");
static_assert(kio::has_oper_star<const RCPtr<const RCObject>>::value,"");
#endif


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

#if 0
// prevent auto propagation from T* to RCPtr<T>:
// seemingly auto propagation in the other direction this is already done by 'operator T* ()' which is fine :-)

template<class T> bool operator== (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() == b; }
template<class T> bool operator!= (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() != b; }
template<class T> bool operator>  (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() >  b; }
template<class T> bool operator<  (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() <  b; }
template<class T> bool operator>= (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() >= b; }
template<class T> bool operator<= (RCPtr<T> const& a, T const* b) noexcept { return a.ptr() <= b; }

template<class T> bool operator== (T const* a, RCPtr<T> const& b) noexcept { return a == b.ptr(); }
template<class T> bool operator!= (T const* a, RCPtr<T> const& b) noexcept { return a != b.ptr(); }
template<class T> bool operator>  (T const* a, RCPtr<T> const& b) noexcept { return a >  b.ptr(); }
template<class T> bool operator<  (T const* a, RCPtr<T> const& b) noexcept { return a <  b.ptr(); }
template<class T> bool operator>= (T const* a, RCPtr<T> const& b) noexcept { return a >= b.ptr(); }
template<class T> bool operator<= (T const* a, RCPtr<T> const& b) noexcept { return a <= b.ptr(); }
#endif

// _____________________________________________________________________________________________________________
// it seems impossible to specialize a class template's member function for a group of types with common traits.
// therefore functionality is extracted into a global function which can templated and overloaded as needed.
// https://jguegant.github.io/blogs/tech/sfinae-introduction.html

template<typename T>
cstr tostr(const RCPtr<T>& p)
{
	// return 1-line description of object for debugging and logging:
	return p ? tostr(*p) : "nullptr";
}

// ____ print() ____

template<class T>
inline typename std::enable_if<kio::has_print<T>::value, void>::type
print(FD& fd, const RCPtr<T>& p, cstr indent) throws
{
	// used if T has member function T::print(FD&,indent)

	if (p) p->print(fd, indent);
	else fd.write_fmt("%snullptr\n", indent);
}

template<class T>
inline typename std::enable_if<!kio::has_print<T>::value, void>::type
print(FD& fd, const RCPtr<T>& p, cstr indent) throws
{
	// used if T has no member function T::print(FD&,indent)

	if (p) fd.write_fmt("%s%s\n", indent, tostr(p));
	else fd.write_fmt("%snullptr\n", indent);
}

template<class T>
void RCPtr<T>::print(FD& fd, cstr indent) const throws
{
	// print description of object for debugging and logging
	// uses one of the above inline print() functions

	::print(fd, *this, indent);
}

// ____ serialize() ____

template<typename T>
typename std::enable_if<kio::has_serialize<T>::value && !kio::has_serialize_w_data<T, void*>::value, void>::type
/*void*/
serialize(FD& fd, const RCPtr<T>& p, void*) throws
{
	// used if type T has member function T::serialize(FD&)

	fd.write_uint8(p != nullptr);
	if (p) p->serialize(fd);
}

template<typename T>
typename std::enable_if<kio::has_serialize_w_data<T, void*>::value, void>::type
/*void*/
serialize(FD& fd, const RCPtr<T>& p, void* data) throws
{
	// used if type T has member function T::serialize(FD&,void*)

	fd.write_uint8(p != nullptr);
	if (p) p->serialize(fd, data);
}

template<typename T>
void RCPtr<T>::serialize(FD& fd, void* data) const throws
{
	// this template will find the above serialize(FD&,RCPtr<T>&, void*)
	// for type T which implement T::serialize(FD&) and
	// for type T which implement T::serialize(FD&,void*)

	::serialize(fd, *this, data);
}

// ____ deserialize() ____

template<typename T>
typename std::enable_if<kio::has_deserialize<T>::value && !kio::has_deserialize_w_data<T, void*>::value, void>::type
/*void*/
deserialize(FD& fd, RCPtr<T>& p, void*) throws
{
	// used if type T has member function T::deserialize(FD&)

	// actually use factory method of T which may return T or subclass
	p = fd.read_uint8() ? T::newFromFile(fd) : nullptr;
}

template<typename T>
typename std::enable_if<kio::has_deserialize_w_data<T, void*>::value, void>::type
/*void*/
deserialize(FD& fd, RCPtr<T>& p, void* data) throws
{
	// used if type T has member function T::deserialize(FD&,void*)

	// actually use factory method of T which may return T or subclass
	p = fd.read_uint8() ? T::newFromFile(fd, data) : nullptr;
}

template<typename T>
void RCPtr<T>::deserialize(FD& fd, void* data) throws
{
	// this template will find the above deserialize(FD&,RCPtr<T>&, void*)
	// for type T which implement T::deserialize(FD&) and
	// for type T which implement T::deserialize(FD&,void*)

	::deserialize(fd, *this, data);
}


// _____________________________________________________________________________________________________________

// convenience subclasses for Array and HashMap:
template<typename T>
class Array;
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

	//	uint indexof  (const RCPtr<T>& item) const noexcept			{ return indexof(item.p); }
	//	bool contains (const RCPtr<T>& item) const	noexcept		{ return contains(item.p); }
	//	void removeitem (const RCPtr<T>& item, bool fast=0) noexcept { removeitem(item.p, fast); }

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
