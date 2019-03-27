#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#include "../kio/kio.h"
#include "../unix/FD.h"
#ifdef USE_THREADS
#include "../cpp/cppthreads.h"
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
	mutable uint cnt;

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
	RCObject ()								noexcept :cnt(0){}
	explicit RCObject (RCObject const&)		noexcept :cnt(0){}
	RCObject (RCObject&&)					noexcept :cnt(0){}
	virtual	~RCObject ()					noexcept { assert(cnt==0); }

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









