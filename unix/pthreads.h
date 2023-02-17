#pragma once
// Copyright (c) 2004 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	pthread wrappers
*/

#include "kio/kio.h"
#include <pthread.h>


// pthread_t is int or ptr on some systems
static const pthread_t NO_THREAD = pthread_t(0);


// =====================================================================
// simple lock
// lock can only be released by lock owner

class PLock
{
	pthread_mutex_t mutex;

	PLock(const PLock&)			   = delete;
	PLock& operator=(const PLock&) = delete;

public:
	enum Attr // for usage with PLock(Attr)
	{
		normal	   = PTHREAD_MUTEX_NORMAL,	  // no usage error checking
		recursive  = PTHREAD_MUTEX_RECURSIVE, // recursive locking allowed
		errorcheck = PTHREAD_MUTEX_ERRORCHECK // w/ usage error checking
	};

	PLock()
	{
		IFDEBUG(int e =) pthread_mutex_init(&mutex, nullptr);
		assert(!e);
	}
	PLock(Attr);
	~PLock()
	{
		IFDEBUG(int e =) pthread_mutex_destroy(&mutex);
		assert(!e);
	}

	void lock() volatile noexcept
	{
		IFDEBUG(int e =) pthread_mutex_lock(const_cast<pthread_mutex_t*>(&mutex));
		assert(!e);
	}
	void unlock() volatile noexcept
	{
		IFDEBUG(int e =) pthread_mutex_unlock(const_cast<pthread_mutex_t*>(&mutex));
		assert(!e);
	}
	bool trylock() volatile noexcept
	{
		return pthread_mutex_trylock(const_cast<pthread_mutex_t*>(&mutex)) == 0;
	} // 1 = success
};


class RPLock : public PLock // convenience class: recursive mutex lock
{
public:
	RPLock() : PLock(recursive) {}
};


// =====================================================================
// class which locks a PLock in it's creator and
// unlocks it in it's destructor

class PLocker
{
	volatile PLock* lock;

public:
	PLocker(volatile PLock* p) { (lock = p)->lock(); }
	PLocker(volatile PLock& p) { (lock = &p)->lock(); }
	~PLocker() { lock->unlock(); }
};


// =====================================================================
// Semaphore
// can be released by any thread
// request / release multiple possible
// semaphore is created with initially 0 resources

class PSemaphore
{
	cstr			name;
	volatile uint32 avail;
	pthread_mutex_t mutex;
	pthread_cond_t	cond;

	PSemaphore(const PSemaphore&)			 = delete;
	PSemaphore& operator=(const PSemaphore&) = delete;

public:
	PSemaphore(cstr static_name_str = "", uint32 _avail = 0);
	~PSemaphore();

	uint32 clear() noexcept { return request(0, ~0u); }
	void   request(uint32 n = 1) noexcept;
	uint32 request(uint32 min, uint32 max) noexcept;
	uint32 requestAll() noexcept { return request(1, ~0u); }
	bool   tryRequest() noexcept;				// return 1  =>  got it
	bool   tryRequest(double timeout) noexcept; // return 1  =>  got it

	void release() noexcept;
	void release(uint32 n) noexcept;

	bool   isAvailable() const noexcept { return avail; }
	uint32 available() const noexcept { return avail; }
	cstr   getName() const noexcept { return name; }
};


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is called exactly in the defined interval independently how long fu() runs itself.
	the created thread is returned in case the caller needs it.
*/
extern pthread_t executeEvery(double delay, bool (*fu)(void*), void* arg = nullptr);


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is re-scheduled with 'delay' after fu() returns, not in exact intervals.
	the created thread is returned in case the caller needs it.
*/
extern pthread_t executeWithDelay(double delay, bool (*fu)(void*), void* arg = nullptr);


/*	wait until 'time' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern pthread_t executeAt(double time, double (*fu)(void*), void* arg = nullptr);


/*	wait 'delay' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern pthread_t executeAfter(double delay, double (*fu)(void*), void* arg = nullptr);


// Main thread utilities:

extern const pthread_t main_thread;
inline bool			   isMainThread() { return pthread_equal(pthread_self(), main_thread); }
inline void			   assertMainThread() { assert(isMainThread()); }
