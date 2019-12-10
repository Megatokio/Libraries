#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "Array.h"


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
	template<class TT> friend class RCArray;
	template<class T1,class T2> friend class RCHashMap;

protected:
	T*		p;

	void	retain () const				noexcept { if(p) p->retain(); }
	void	release () const			noexcept { if(p) p->release(); }

public:
	RCPtr ()							noexcept :p(nullptr){}
	RCPtr (T* p)						noexcept :p(p)   { retain(); }
	explicit RCPtr (RCPtr const& q)  	noexcept :p(q.p) { retain(); }
	RCPtr (RCPtr& q)					noexcept :p(q.p) { retain(); }
	RCPtr (RCPtr&& q)					noexcept :p(q.p) { q.p = nullptr; }
	~RCPtr ()							noexcept { release(); }

	RCPtr&	operator= (RCPtr&& q)		noexcept { std::swap(p,q.p); return *this; }
	RCPtr&	operator= (RCPtr const& q) 	noexcept { q.retain(); release(); p = q.p; return *this; }
	RCPtr&	operator= (T* q)      		noexcept { if(q) q->retain(); release(); p = q; return *this; }

	// factory method
	// not needed because RCPtr is final and can't be subclassed. (but the RCObject can.)
	//static RCPtr newFromFile (FD& fd, void* data=nullptr)	throws { return std::move(RCPtr(fd.read_uint8()?T::newFromFile(fd,data):nullptr)); }

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap (RCPtr<T>& a, RCPtr<T>& b) noexcept { std::swap(a.p,b.p); }

	T*		operator-> () const			noexcept { return p; }
	T&		operator* () const			noexcept { return *p; }
	T*		ptr () const				noexcept { return p; }
	T&		ref () const				noexcept { assert(p!=nullptr); return *p; }

	operator T& () const				noexcept { assert(p!=nullptr); return *p; }
	operator T* () const				noexcept { return p; }

	uint	refcnt () const				noexcept { return p ? p->refcnt() : 0; }

	// prevent erroneous use of operator[] with pointer:
	T& operator[] (uint32) const = delete;

	// for convenience, these are also declared volatile:
	// the result is unreliable and must be checked again after locking.
	bool	isNotNull () volatile const	noexcept { return p != nullptr; }
	bool	isNull () volatile const	noexcept { return p == nullptr; }
	bool	is	  (T const* b ) volatile const	noexcept { return p == b; }
	bool	isnot (T const* b ) volatile const	noexcept { return p != b; }
	operator bool () volatile const		noexcept { return p != nullptr; }

	void print (FD&, cstr indent) const throws;
	void serialize (FD&, void* data = nullptr) const throws;
	void deserialize (FD&, void* data = nullptr) throws;
};

#if 0
#include "template_helpers.h"
#include "RCObject.h"
static_assert(kio::has_oper_star<RCPtr<RCObject>>::value,"");
static_assert(kio::has_oper_star<const RCPtr<const RCObject>>::value,"");
#endif


// _____________________________________________________________________________________________________________
// relational operators:
// NOTE: it's C++ standard to compare the pointers.
//       a nullptr is less than any other pointer.

template<class T> bool operator== (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() == b.ptr(); }
template<class T> bool operator!= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() != b.ptr(); }
template<class T> bool operator>  (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() >  b.ptr(); }
template<class T> bool operator<  (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() <  b.ptr(); }
template<class T> bool operator>= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() >= b.ptr(); }
template<class T> bool operator<= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a.ptr() <= b.ptr(); }

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
cstr tostr (const RCPtr<T>& p)
{
	// return 1-line description of object for debugging and logging:
	return p ? tostr(*p) : "nullptr";
}

// ____ print() ____

template<class T>
inline typename std::enable_if<kio::has_print<T>::value,void>::type
print (FD& fd, const RCPtr<T>& p, cstr indent) throws
{
	// used if T has member function T::print(FD&,indent)

	if (p) p->print(fd,indent);
	else fd.write_fmt("%snullptr\n",indent);
}

template<class T>
inline typename std::enable_if<!kio::has_print<T>::value,void>::type
print (FD& fd, const RCPtr<T>& p, cstr indent) throws
{
	// used if T has no member function T::print(FD&,indent)

	if (p) fd.write_fmt("%s%s\n",indent,tostr(p));
	else fd.write_fmt("%snullptr\n",indent);
}

template<class T>
void RCPtr<T>::print (FD& fd, cstr indent) const throws
{
	// print description of object for debugging and logging
	// uses one of the above inline print() functions

	::print(fd,*this,indent);
}

// ____ serialize() ____

template <typename T>
typename std::enable_if<kio::has_serialize<T>::value && !kio::has_serialize_w_data<T,void*>::value,void>::type
/*void*/ serialize (FD& fd, const RCPtr<T>& p, void*) throws
{
	// used if type T has member function T::serialize(FD&)

	fd.write_uint8(p!=nullptr);
	if (p) p->serialize(fd);
}

template <typename T>
typename std::enable_if<kio::has_serialize_w_data<T,void*>::value,void>::type
/*void*/ serialize (FD& fd, const RCPtr<T>& p, void* data) throws
{
	// used if type T has member function T::serialize(FD&,void*)

	fd.write_uint8(p!=nullptr);
	if (p) p->serialize(fd,data);
}

template<typename T>
void RCPtr<T>::serialize (FD& fd, void* data) const throws
{
	// this template will find the above serialize(FD&,RCPtr<T>&, void*)
	// for type T which implement T::serialize(FD&) and
	// for type T which implement T::serialize(FD&,void*)

	::serialize(fd, *this, data);
}

// ____ deserialize() ____

template <typename T>
typename std::enable_if<kio::has_deserialize<T>::value && !kio::has_deserialize_w_data<T,void*>::value,void>::type
/*void*/ deserialize (FD& fd, RCPtr<T>& p, void*) throws
{
	// used if type T has member function T::deserialize(FD&)

	// actually use factory method of T which may return T or subclass
	p = fd.read_uint8() ? T::newFromFile(fd) : nullptr;
}

template <typename T>
typename std::enable_if<kio::has_deserialize_w_data<T,void*>::value,void>::type
/*void*/ deserialize (FD& fd, RCPtr<T>& p, void* data) throws
{
	// used if type T has member function T::deserialize(FD&,void*)

	// actually use factory method of T which may return T or subclass
	p = fd.read_uint8() ? T::newFromFile(fd,data) : nullptr;
}

template<typename T>
void RCPtr<T>::deserialize (FD& fd, void* data) throws
{
	// this template will find the above deserialize(FD&,RCPtr<T>&, void*)
	// for type T which implement T::deserialize(FD&) and
	// for type T which implement T::deserialize(FD&,void*)

	::deserialize(fd, *this, data);
}


// _____________________________________________________________________________________________________________

// convenience subclasses for Array and HashMap:
template<typename T> class Array;
template<class T> class RCArray : public Array<RCPtr<T>>
{
protected:
	using Array<RCPtr<T>>::cnt;
	using Array<RCPtr<T>>::data;

public:
	uint indexof  (const T* item) const	noexcept		// find first occurance or return ~0u
	{													// compares object addresses (pointers)
		for (uint i=0; i<cnt; i++) { if (data[i].p == item) return i; }
		return ~0u;
	}

	bool contains (const T* item) const	noexcept		{ return indexof(item) != ~0u; }
	void removeitem (const T* item, bool fast=0) noexcept { uint i = indexof(item); if (i != ~0u) remove(i,fast); }

//	uint indexof  (const RCPtr<T>& item) const noexcept			{ return indexof(item.p); }
//	bool contains (const RCPtr<T>& item) const	noexcept		{ return contains(item.p); }
//	void removeitem (const RCPtr<T>& item, bool fast=0) noexcept { removeitem(item.p, fast); }

	void remove (const T* item, bool fast=0) noexcept { removeitem(item,fast); }
	void remove (uint idx, bool fast=0)	noexcept { this->removeat(idx,fast); }
};

template<class KEY, class ITEM> class HashMap;
template<class KEY, class T> class RCHashMap : public HashMap<KEY,RCPtr<T>> {};






























