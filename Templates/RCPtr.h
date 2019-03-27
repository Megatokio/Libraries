#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
  					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "../kio/kio.h"
#include "../unix/FD.h"
#include "template_helpers.h"


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
		serialize(FD&) const throws				// serialize
		deserialize(FD&) throws					// deserialize
		static T* restore(FD&, void*) throws;	// deserialize

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
	//static RCPtr restore	(FD& fd)	throws	 { return std::move(RCPtr(fd.read_uint8()?T::restore():nullptr)); }

	// see https://stackoverflow.com/questions/11562/how-to-overload-stdswap
	static void swap (RCPtr<T>& a, RCPtr<T>& b) noexcept { std::swap(a.p,b.p); }

	T*		operator-> () const			noexcept { return p; }
	T&		operator* () const			noexcept { return *p; }
	T*		ptr () const				noexcept { return p; }
	T&		ref () const				noexcept { assert(p!=nullptr); return *p; }

	operator T& () const				noexcept { assert(p!=nullptr); return *p; }
	operator T* () const				noexcept { return p; }

	uint	refcnt () const				noexcept { return p ? p->refcnt() : 0; }

	// operator[]:
	// NOTE: it's C++ standard to use (*p)[] as if pointer wrapper classes actually were just bare pointers
	//       e.g. in unique_ptr<> and shared_ptr<>.
	//       This is almost never useful for arbitrarily positioned objects.
	// => operator[] calls the object's operator[]!
	template<typename TT> TT operator[] (uint i) const { assert(p!=nullptr); return (*p)[i]; }

	// for convenience, these are also declared volatile:
	// the result is unreliable and must be checked again after locking.
	bool	isNotNull () volatile const	noexcept __attribute__((deprecated)); // use is(0);
	bool	isNull () volatile const	noexcept __attribute__((deprecated)); // use isnot(0);
	bool	is	  (T const* b ) volatile const	noexcept { return p == b; }
	bool	isnot (T const* b ) volatile const	noexcept { return p != b; }
	operator bool () volatile const		noexcept { return p != nullptr; }

	void	print		(FD&, cstr indent) const throws;
	void	serialize	(FD&) const		throws;
	void	deserialize	(FD&)			throws;

	// ____ internal template support: ____
	template<class U> static inline typename std::enable_if<kio::has_print<U>::value,void>::type
	print( FD& fd, U const& object, cstr indent ) throws { object.print(fd,indent); }

	template<class U> static inline typename std::enable_if<!kio::has_print<U>::value,void>::type
	print( FD& fd, U const& object, cstr indent ) throws { fd.write_fmt("%s%s\n",indent,tostr(object)); }
};


// convenience subclasses for Array and HashMap:
template<typename T> class Array;
template<class T> class RCArray : public Array<RCPtr<T>> {};

template<class KEY, class ITEM> class HashMap;
template<class KEY, class T> class RCHashMap : public HashMap<KEY,RCPtr<T>> {};


// relational operators:
// NOTE: it's C++ standard to compare the pointers as if pointer wrapper classes actually were just bare pointers
//       e.g. in unique_ptr<> and shared_ptr<>.
//       This is rarely useful for arbitrarily positioned objects.
// => relational operators compare the objects!
//    a nullptr is less than any object.

template<class T> bool operator== (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a == *b : a.is(b); }
template<class T> bool operator!= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a != *b :!a.is(b); }
template<class T> bool operator>  (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a >  *b : a; }
template<class T> bool operator<  (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a <  *b : b; }
template<class T> bool operator>= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a >= *b :!b; }
template<class T> bool operator<= (RCPtr<T> const& a, RCPtr<T> const& b) noexcept { return a&&b ? *a <= *b :!a; }

// CAVEAT: don't pass in a 'new T(…)' directly because it won't be destroyed!
template<class T> bool operator== (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a == *b : a.is(b); }
template<class T> bool operator!= (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a != *b :!a.is(b); }
template<class T> bool operator>  (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a >  *b : a; }
template<class T> bool operator<  (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a <  *b : b; }
template<class T> bool operator>= (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a >= *b :!b; }
template<class T> bool operator<= (RCPtr<T> const& a, T const* b) noexcept { return a&&b ? *a <= *b :!a; }

// CAVEAT: don't pass in a 'new T(…)' directly because it won't be destroyed!
template<class T> bool operator== (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a == *b : b.is(a); }
template<class T> bool operator!= (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a != *b :!b.is(a); }
template<class T> bool operator>  (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a >  *b : a; }
template<class T> bool operator<  (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a <  *b : b; }
template<class T> bool operator>= (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a >= *b :!b; }
template<class T> bool operator<= (T const* a, RCPtr<T> const& b) noexcept { return a&&b ? *a <= *b :!a; }


template<typename T>
cstr tostr (RCPtr<T> const& p)
{
	// return 1-line description of object for debugging and logging:
	return p ? tostr(*p) : "nullptr";
}

template<class T>
void RCPtr<T>::print (FD& fd, cstr indent) const throws
{
	// print description of object for debugging and logging:
	// uses T::print() if defined else tostr(T)
	// note: implementation of two versions of RCPtr::print() for types T with and without trait "has_print"
	//       cannot be done for template class member function RCPtr::print()
	//       a conditional function template can only be applied on a static function: --> static RCPtr::print()
	if (p) RCPtr::print(fd,*p,indent);
	else fd.write_fmt("%snullptr\n",indent);
}

template<class T>
void RCPtr<T>::serialize (FD& fd) const throws
{
	fd.write_uint8(p!=nullptr);
	if (p) p->serialize(fd);
}

template<class T>
void RCPtr<T>::deserialize (FD& fd) throws
{
	// use factory method of T: may return T or subclass
	*this = fd.read_uint8() ? T::restore(fd) : nullptr;
}

























