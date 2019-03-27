#pragma once
/*	Copyright  (c)	Günter Woigk 2004 - 2019
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

	pthread wrappers

	2008-05-02 kio	made PLock a subclass of pthread_mutex_t (instead having a data member of this class)
*/

#include <mutex>
#include <thread>
#include "kio/kio.h"


// convenience:
template<class T> T* NV(volatile T* o) { return const_cast<T*>(o); }
template<class T> T& NV(volatile T& o) { return const_cast<T&>(o); }


// =====================================================================
// mutex (mutually exclusive lockable lock)
// lock can only be released by lock owner

// non-recursive mutex lock:
class PLock : public std::mutex
{
public:
	void lock()		volatile { NV(this)->mutex::lock(); }
	void unlock()	volatile { NV(this)->mutex::unlock(); }
	bool trylock()	volatile { return NV(this)->mutex::try_lock(); }  // true = success
};


// recursive mutex lock:
class RPLock : public std::mutex
{
public:
	void lock()		volatile { NV(this)->mutex::lock(); }
	void unlock()	volatile { NV(this)->mutex::unlock(); }
	bool trylock()	volatile { return NV(this)->mutex::try_lock(); }  // true = success
};


// =====================================================================
// class which locks a PLock in it's ctor and
// unlocks it in it's dtor

class PLocker : public std::lock_guard<PLock>
{
public:
	PLocker(volatile PLock& lock)	:lock_guard(NV(lock)){}
};





// =====================================================================
// Semaphore
// can be released by any thread
// request / release multiple possible
// semaphore is created with initially 0 resources

class PSemaphore
{
	const cstr		name;
	volatile uint32	avail;
	std::mutex		mutex;
	std::condition_variable	cond;

                    PSemaphore	(const PSemaphore&) = delete;
    PSemaphore&     operator=   (const PSemaphore&) = delete;

public:				PSemaphore	(cstr static_name_str="", uint32 _avail = 0);
					~PSemaphore	();

	uint32			clear		()					{ return request(0,~0u); }
	void			request		(uint32 n = 1);
	uint32			request		(uint32 min, uint32 max);
	uint32			requestAll	()					{ return request(1,~0u); }
	bool			tryRequest	();					// return 1  =>  got it
	bool			tryRequest	(double timeout);	// return 1  =>  got it

	void			release		();
	void			release		(uint32 n);

	bool			isAvailable	() const			{ return avail; }
	uint32			available	() const			{ return avail; }
	cstr			getName		() const			{ return name; }
};



// Timer
extern double now				();					// kio.cpp
extern void waitDelay			(double delay);
extern void	waitUntil			(double time);		// system time


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is called exactly in the defined interval independently how long fu() runs itself.
	the created thread is returned in case the caller needs it.
*/
extern std::thread executeEvery	( double delay, bool(*fu)(void*), void* arg = NULL );


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is re-scheduled with 'delay' after fu() returns, not in exact intervals.
	the created thread is returned in case the caller needs it.
*/
extern std::thread executeWithDelay	( double delay, bool(*fu)(void*), void* arg = NULL );


/*	wait until 'time' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern std::thread executeAt	( double time,  double(*fu)(void*), void* arg = NULL );


/*	wait 'delay' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern std::thread executeAfter	( double delay, double(*fu)(void*), void* arg = NULL );



// Main thread utilities:

extern const std::thread::id	main_thread;
inline bool	 isMainThread()		{ return std::this_thread::get_id() == main_thread; }









