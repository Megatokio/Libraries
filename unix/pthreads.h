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

#include <pthread.h>
#include "kio/kio.h"


// =====================================================================
// simple lock
// lock can only be released by lock owner

class PLock
{
	pthread_mutex_t mutex;

            PLock       (const PLock&) = delete;
    PLock&  operator=   (const PLock&) = delete;

public:
	enum Attr   // for usage with PLock(Attr)
	{
		normal		= PTHREAD_MUTEX_NORMAL,			// no usage error checking
		recursive	= PTHREAD_MUTEX_RECURSIVE,		// recursive locking allowed
		errorcheck	= PTHREAD_MUTEX_ERRORCHECK		// w/ usage error checking
	};

			PLock	()			{ IFDEBUG( int e = ) pthread_mutex_init(&mutex,NULL); assert(!e); }
			PLock	(Attr);
			~PLock	()          { IFDEBUG( int e = ) pthread_mutex_destroy(&mutex); 	assert(!e); }

	void	lock	() volatile noexcept { IFDEBUG( int e = ) pthread_mutex_lock(const_cast<pthread_mutex_t*>(&mutex)); assert(!e); }
	void	unlock	() volatile noexcept { IFDEBUG( int e = ) pthread_mutex_unlock(const_cast<pthread_mutex_t*>(&mutex));assert(!e);}
	bool	trylock	() volatile noexcept { return pthread_mutex_trylock(const_cast<pthread_mutex_t*>(&mutex))==0; }	// 1 = success
};


class RPLock : public PLock		// convenience class: recursive mutex lock
{
public:		RPLock	( )				: PLock( recursive ) { }
};


// =====================================================================
// class which locks a PLock in it's creator and
// unlocks it in it's destructor

class PLocker
{
	volatile PLock*	lock;
public:
				PLocker		( volatile PLock* p )	{ (lock=p)->lock(); }
				PLocker		( volatile PLock& p )	{ (lock=&p)->lock(); }
				~PLocker	()						{  lock->unlock();  }
};


// =====================================================================
// Semaphore
// can be released by any thread
// request / release multiple possible
// semaphore is created with initially 0 resources

class PSemaphore
{
	cstr			name;
	volatile uint32	avail;
	pthread_mutex_t	mutex;
	pthread_cond_t	cond;

                    PSemaphore	(PSemaphore const&);    // prohibit
    PSemaphore&     operator=   (PSemaphore const&);    // prohibit

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
extern double now				();					// was: currentTime()

extern void waitDelay			( double delay );
extern void	waitUntil			( double time );


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is called exactly in the defined interval independently how long fu() runs itself.
	the created thread is returned in case the caller needs it.
*/
extern pthread_t executeEvery		( double delay, bool(*fu)(void*), void* arg = NULL );


/*	in a loop, wait 'delay' and execute function fu() until fu() returns false.
	'arg' is passed to fu() as argument and must be persistent until fu() reads it.
	fu() is re-scheduled with 'delay' after fu() returns, not in exact intervals.
	the created thread is returned in case the caller needs it.
*/
extern pthread_t executeWithDelay 	( double delay, bool(*fu)(void*), void* arg = NULL );


/*	wait until 'time' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern pthread_t executeAt          ( double time,  double(*fu)(void*), void* arg = NULL );


/*	wait 'delay' and then execute fu().
	if fu() returns a value ≤ 0.0 then finished.
	else wait until the returned time or wait the returned delay (auto-detected)
	and call fu() again until it returns ≤ 0.0.
*/
extern pthread_t executeAfter       ( double delay, double(*fu)(void*), void* arg = NULL );



// Main thread utilities:

extern const pthread_t main_thread;
inline bool	 isMainThread()		{ return pthread_equal(pthread_self(),main_thread); }
inline void  assertMainThread()	{ assert(isMainThread()); }









