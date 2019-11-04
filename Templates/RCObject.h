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

#include "kio/kio.h"
#include "unix/FD.h"
#ifdef USE_THREADS
#include "cpp/cppthreads.h"
#endif


/*	Simple base class for objects with reference counting
	for use with RCPtr and NVPtr.

	RCPtr and RCObject are used for automatical live-keeping and destruction of objects.
	Once an RCObject pointer is stored in an RCPtr, it may no longer be delete'd.
	instead it will be deleted when the last RCPtr pointing to it is destroyed or assigned another pointer.

	NVPtr and RCObject are used for locking volatile Objects in multi-threaded applications.
	best used in conjunction with RCPtrs.

	An own implementation, if you cannot use this base class, must implement retain() and release().

	notes:

	if RCPtr<T>::deserialize(FD&) is used,
	then class T (or a base class) must provide a static factory method T* restore(FD&).

	if App uses threads, then
	• all objects which can be accessed by multiple threads must be declared volatile.
	• this is especially true for all global variables!
	• actually ALL variables which can be accessed by multiple threads must be declared volatile,
	  though this does not help for basic types. (the compiler ignores this.)
	• to access non-volatile member functions in volatile objects, the object must be locked, then it can be cast to
	  it's non-volatile counterpart and the member function can be called, and finally it must be unlocked.
	  this is best done by storing it in a temporary/local NVPtr.
*/


class RCObject
{
	template<class T> friend class RCPtr;
	template<class T> friend class NVPtr;

protected:
	mutable uint cnt = 0;

	void	retain () const 				noexcept { ++cnt; }
	void	release () const				noexcept { if (--cnt == 0) delete this; }

#ifdef USE_THREADS
	mutable PLock plock;
	void	lock ()    volatile const		noexcept { plock.lock(); }
	void	unlock ()  volatile const		noexcept { plock.unlock(); }
	void	retain ()  volatile const		noexcept { lock(); ++cnt; unlock(); }
	void	release () volatile const		noexcept { lock(); if(--cnt==0) delete this; else unlock(); }
#endif


public:
	RCObject ()								noexcept {}
	explicit RCObject (RCObject const&)		noexcept {}
	RCObject (RCObject&&)					noexcept {}
	virtual	~RCObject ()					noexcept { if (unlikely(cnt!=0)) abort("~RCObject(): cnt=%i\n",cnt); }

	RCObject& operator= (RCObject const&)	noexcept { return *this; }
	RCObject& operator= (RCObject&&)		noexcept { return *this; }

	uint	refcnt () const					noexcept { return cnt; }

// implement if required:
	static	RCObject* restore (FD&)			throws;		// factory method: may return this or any subclass
	virtual	void print (FD&, cstr /*indent*/) const throws {}
	virtual	void serialize (FD&) const		throws {}
	virtual	void deserialize (FD&)			throws {}
};

template<typename T>
cstr tostr (RCObject const&)
{
	// return 1-line description of object for debugging and logging:
	return "RCObject";
}









