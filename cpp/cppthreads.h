#pragma once
// Copyright (c) 2004 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <condition_variable>
#include <mutex>
#include <thread>


// =====================================================================
// mutex (mutually exclusive lockable lock)
// lock can only be released by lock owner

// non-recursive mutex lock:
class PLock : public std::mutex
{
	static constexpr PLock* NV(volatile PLock* p) { return const_cast<PLock*>(p); }

public:
	void lock() volatile { NV(this)->mutex::lock(); }
	void unlock() volatile { NV(this)->mutex::unlock(); }
	bool trylock() volatile { return NV(this)->mutex::try_lock(); } // true = success
};


// recursive mutex lock:
class RPLock : public std::recursive_mutex
{
	static constexpr RPLock* NV(volatile RPLock* p) { return const_cast<RPLock*>(p); }

public:
	void lock() volatile { NV(this)->recursive_mutex::lock(); }
	void unlock() volatile { NV(this)->recursive_mutex::unlock(); }
	bool trylock() volatile { return NV(this)->recursive_mutex::try_lock(); } // true = success
};


// =====================================================================
// class which locks a mutex, PLock or similar in it's ctor and
// unlocks it in it's dtor

template<typename MUTEX>
class PLocker : public std::lock_guard<MUTEX>
{
public:
	PLocker(volatile MUTEX& lock) : std::lock_guard<MUTEX>(const_cast<MUTEX&>(lock)) {}
	PLocker(volatile MUTEX* lock) : std::lock_guard<MUTEX>(*const_cast<MUTEX*>(lock)) {}
};


// =====================================================================
// Semaphore
// can be released by any thread
// request / release multiple possible
// semaphore is created with initially 0 resources

class PSemaphore
{
	const cstr				name;
	volatile uint32			avail;
	std::mutex				mutex;
	std::condition_variable cond;

	PSemaphore(const PSemaphore&)			 = delete;
	PSemaphore& operator=(const PSemaphore&) = delete;

public:
	PSemaphore(cstr static_name_str = "", uint32 _avail = 0);
	~PSemaphore();

	uint32 clear() { return request(0, ~0u); }
	void   request(uint32 n = 1);
	uint32 request(uint32 min, uint32 max);
	uint32 requestAll() { return request(1, ~0u); }
	bool   tryRequest();			   // return 1  =>  got it
	bool   tryRequest(double timeout); // return 1  =>  got it

	void release();
	void release(uint32 n);

	bool   isAvailable() const { return avail; }
	uint32 available() const { return avail; }
	cstr   getName() const { return name; }
};


// Timer  --> kio.cpp
//extern double now				();					// kio.cpp
//extern void waitDelay			(double delay);
//extern void	waitUntil			(double time);		// system time


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is called exactly in the defined interval independently how long fu() runs itself.
	the created thread is returned in case the caller needs it.
*/
extern std::thread executeEvery(double delay, bool (*fu)(void*), void* arg = nullptr);


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is re-scheduled with 'delay' after fu() returns, not in exact intervals.
	the created thread is returned in case the caller needs it.
*/
extern std::thread executeWithDelay(double delay, bool (*fu)(void*), void* arg = nullptr);


/*	wait until 'time' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern std::thread executeAt(double time, double (*fu)(void*), void* arg = nullptr);


/*	wait 'delay' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern std::thread executeAfter(double delay, double (*fu)(void*), void* arg = nullptr);


// Main thread utilities:

extern const std::thread::id main_thread;
inline bool					 isMainThread() { return std::this_thread::get_id() == main_thread; }
