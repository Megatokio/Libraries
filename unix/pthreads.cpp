// Copyright (c) 2004 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <math.h>
#ifdef HAVE_SYS_TIME_H
  #include <sys/time.h>
#endif
#include "pthreads.h"

DEBUG_INIT_MSG

const pthread_t main_thread = pthread_self();

static pthread_mutexattr_t mutex_attr[3];
#if 0
static pthread_cond_t		wait_cond;	// eine condition für Wait(), die niemals getriggert wird
static pthread_mutex_t		wait_lock;	// ihr Lock
#endif

static bool initialized = 0;
static void setup(void)
{
	xlogIn("pthread: setup()");
	int e = 0;
	for (int i = 0; i < 3; i++) e |= pthread_mutexattr_init(mutex_attr + i);

	e |= pthread_mutexattr_settype(mutex_attr + PLock::normal, PTHREAD_MUTEX_NORMAL);
	e |= pthread_mutexattr_settype(mutex_attr + PLock::recursive, PTHREAD_MUTEX_RECURSIVE);
	e |= pthread_mutexattr_settype(mutex_attr + PLock::errorcheck, PTHREAD_MUTEX_ERRORCHECK);

#if 0
	e |= pthread_mutex_init( &wait_lock, nullptr );
	e |= pthread_cond_init( &wait_cond, nullptr );
#endif

	assert(!e);
	initialized = yes;
}

#if 0
	static pthread_once_t once_control = PTHREAD_ONCE_INIT;
	inline void init() { int e = pthread_once(&once_control,setup); XXXTRAP(e); }
#else
// normalerweise würde ON_INIT(setup) genügen. Aber:
// Es kann sein, dass ein statisches/globales Objekt z.B. ein PLock mit PLock(Attr) erzeugt
// und dass dieses Objekt vor unserem ON_INIT initialisiert wird. Dann sind die mutex_attr[] etc.
// noch nicht initialisiert.
// Da das nur in der Boot-Phase vorkommen kann und wir da (hoffentlich) noch nicht multithreaded sind,
// ist eine Prüfung mit pthread_once() wohl übertrieben und ein einfaches boolean Flag reicht.
inline void init()
{
	if (!initialized) setup();
} // called in PLock(Attr) und waitDelay()
ON_INIT(init);
#endif


#if 0
/* ----	Utilities -------------------------------------
*/
double now ( )		// moved to LogFile.cpp
{
	struct timeval tv;
	gettimeofday ( &tv, nullptr );
	return tv.tv_sec + tv.tv_usec * 1e-6;
}
#endif


/* ----	PLock -----------------------------------------
*/
PLock::PLock(Attr a)
{
	init();
	assert(a < NELEM(mutex_attr));
	IFDEBUG(int e =) pthread_mutex_init(&mutex, mutex_attr + a);
	assert(!e);
}


///* ----	PSema ------------------------------------------
//*/
//PSema::PSema ( )
//{
//	int e;
//	sema = 0;		// sema<=0 -> free; sema>0 -> locked
//	//who = 0;		// id of blocking thread
//	name = "";
//	if ( (e = pthread_mutex_init( &mutex, nullptr )) ) goto x;
//	if ( (e = pthread_cond_init(  &cond, nullptr )) ) goto x;
//	return;
//
//x:	Abort("PSema:PSema::",errorstr(e),"\n");
//}
//
//
//PSema::PSema ( cstr s )
//{
//	int e;
//	sema = 0;		// sema<=0 -> free; sema>0 -> locked
//	//who = 0;		// id of blocking thread
//	name = s;
//	if ( (e = pthread_mutex_init( &mutex, nullptr )) ) goto x;
//	if ( (e = pthread_cond_init(  &cond, nullptr )) ) goto x;
//	return;
//
//x:	Abort("PSema:PSema:",catstr(name,":"),errorstr(e));
//}
//
//
//PSema::~PSema()
//{
//	int e;
//	if ( (e = pthread_mutex_destroy( &mutex )) ) goto x;
//	if ( (e = pthread_cond_destroy ( &cond )) ) goto x;
//	return;
//
//x:	Abort("PSema:~PSema:",catstr(name,":"),errorstr(e));
//}
//
//
//void PSema::lock()
//{
//	int e;
//	if ( (e = pthread_mutex_lock( &mutex )) ) goto x;
//
//	// check predicate - go to sleep - recheck predicate on awakening
//		while ( sema>0 ) if ( (e = pthread_cond_wait( &cond, &mutex )) ) goto x;
//		sema++; who=pthread_self();
//
//	if ( (e = pthread_mutex_unlock( &mutex )) ) goto x;
//	return;
//
//x:	Abort("PSema:lock:",catstr(name,":"),errorstr(e));
//}
//
//
//void PSema::unlock()
//{
//	int e;
//	if ( (e = pthread_mutex_lock( &mutex )) ) goto x;
//
//	sema -= sema>0;
//	if ( sema<=0 ) if ( (e = pthread_cond_signal( &cond )) ) goto x;
//
//	if ( (e = pthread_mutex_unlock( &mutex )) ) goto x;
//	return;
//
//x:	Abort("PSema:Unlock:",catstr(name,":"),errorstr(e));
//}
//
//
//bool PSema::tryLock()
//{
//	if ( sema>0 ) return no;
//
//	int  e;
//	bool f;
//	if ( (e = pthread_mutex_lock( &mutex )) ) goto x;
//
//		f = sema<=0;
//		if (f) { sema++; who=pthread_self(); }
//
//	if ( (e = pthread_mutex_unlock( &mutex )) ) goto x;
//	return f;
//
//x:	Abort("PSema:TryLock:",catstr(name,":"),errorstr(e));
//	return 0;	// dead code
//}
//
//
//bool PSema::tryLock ( double timeout )
//{
//	timeout += now();
//	double time_fract, time_int; time_fract = modf(timeout,&time_int);
//	timespec time = { time_t(time_int), long(time_fract*1000000000) };
//	XXXTRAP( time.tv_nsec==1000000000 );
//
//	int  e;
//	bool f;
//	if ( (e = pthread_mutex_lock( &mutex )) ) goto x;
//
//	// check predicate - go to sleep - recheck predicate on awakening
//		while ( sema>0 )
//		{
//			e = pthread_cond_timedwait( &cond, &mutex, &time );
//			if (e) { if (e==ETIMEDOUT) break; else goto x; }
//		}
//
//		f = sema<=0;
//		if (f) { sema++; who=pthread_self(); }
//
//	if ( (e = pthread_mutex_unlock( &mutex )) ) goto x;
//	return f;
//
//x:	Abort("PSema:TryLock(timeout):",catstr(name,":"),errorstr(e));
//	return 0;	// dead code
//}


/* ----	PSemaphore ----------------------------------------
*/
PSemaphore::PSemaphore(cstr name, uint32 avail) : name(name), avail(avail)
{
	int e = pthread_mutex_init(&mutex, nullptr);
	if (!e) e = pthread_cond_init(&cond, nullptr);

	if (e) abort("new PSemaphore: %s", strerror(e));
}


PSemaphore::~PSemaphore()
{
	int e = pthread_mutex_destroy(&mutex);
	if (!e) e = pthread_cond_destroy(&cond);

	if (e) abort("~PSemaphore: %s", strerror(e));
}


void PSemaphore::release(uint32 n) noexcept
{
	int e = pthread_mutex_lock(&mutex);
	avail += n;
	if (!e) e = pthread_mutex_unlock(&mutex);
	if (!e) e = pthread_cond_broadcast(&cond);

	if (e) abort("PSemaphore.release(n): %s", strerror(e));
}


void PSemaphore::release() noexcept
{
	int e = pthread_mutex_lock(&mutex);
	avail += 1;
	if (!e) e = pthread_mutex_unlock(&mutex);
	if (!e) e = pthread_cond_signal(&cond);

	if (e) abort("PSemaphore.release: %s", strerror(e));
}


void PSemaphore::request(uint32 n) noexcept
{
	int e = pthread_mutex_lock(&mutex);
	// check predicate - go to sleep - recheck predicate on awakening
	while (!e && avail < n) { e = pthread_cond_wait(&cond, &mutex); }
	avail -= n;
	if (!e) e = pthread_mutex_unlock(&mutex);

	if (e) abort("PSemaphore.request: %s", strerror(e));
}


uint32 PSemaphore::request(uint32 n_min, uint32 n_max) noexcept
{
	int e = pthread_mutex_lock(&mutex);
	// check predicate - go to sleep - recheck predicate on awakening
	while (!e && avail < n_min) { e = pthread_cond_wait(&cond, &mutex); }
	uint32 n = min(avail, n_max);
	avail -= n;
	if (!e) e = pthread_mutex_unlock(&mutex);
	if (e) abort("PSemaphore.requestAll: %s", strerror(e));
	return n;
}


bool PSemaphore::tryRequest() noexcept
{
	if (avail == 0) return no;
	int	 e = pthread_mutex_lock(&mutex);
	bool f = avail > 0;
	avail -= f;
	if (!e) e = pthread_mutex_unlock(&mutex);
	if (!e) return f;

	abort("PSemaphore.tryRequest: %s", strerror(e));
}


bool PSemaphore::tryRequest(double timeout) noexcept
{
	timeout += now();
	double time_fract, time_int;
	time_fract	  = modf(timeout, &time_int);
	timespec time = {time_t(time_int), long(time_fract * 1000000000)};
	assert(time.tv_nsec < 1000000000);

	int e = pthread_mutex_lock(&mutex);
	// check predicate - go to sleep - recheck predicate on awakening
	while (!e && !avail) { e = pthread_cond_timedwait(&cond, &mutex, &time); }
	if (e == ETIMEDOUT) e = 0;
	bool f = avail > 0;
	avail -= f;
	if (!e) e = pthread_mutex_unlock(&mutex);
	if (!e) return f;

	abort("PSemaphore.tryRequest(timeout): %s", strerror(e));
}


/* ----	PTimer ----------------------------------------
		now in kio.cpp
*/
#if 0
void waitUntil ( double time )
{
	init();
	double time_fract, time_int; time_fract = modf(time,&time_int);
	timespec wait_time = { time_t(time_int), long(time_fract*1000000000) };
	assert( wait_time.tv_nsec<1000000000 );

	int e = pthread_mutex_lock(&wait_lock);
	if (!e) e = pthread_cond_timedwait(&wait_cond, &wait_lock, &wait_time);
	if (e==ETIMEDOUT) e = 0;
	if (!e) e = pthread_mutex_unlock(&wait_lock);
	if (!e) return;

	abort("waitUntil: %s", strerror(e));
}
#endif

#if 0
void waitDelay ( double delay )
{
	waitUntil( now()+delay );
}
#endif


/* ----	Execute procedure in regular intervals ----------------------------------------
*/

struct exec_every_t
{
	bool (*fu)(void*);
	void*  arg;
	double delay;

	exec_every_t(bool (*fu)(void*), void* arg, double delay) : fu(fu), arg(arg), delay(delay) {}
};

static void* execute_every_proc(void* data)
{
	exec_every_t* x	  = reinterpret_cast<exec_every_t*>(data);
	double		  now = ::now();
	do {
		waitUntil(now += x->delay);
	}
	while (x->fu(x->arg));
	delete x;
	return nullptr;
}

pthread_t executeEvery(double delay, bool (*fu)(void*), void* arg)
{
	pthread_t	  thread;
	exec_every_t* spawn_data = new exec_every_t(fu, arg, delay);
	int			  e			 = pthread_create(&thread, nullptr /*attr*/, execute_every_proc, spawn_data);
	if (e) abort("executeEvery: %s", strerror(e));
	return thread;
}

static void* execute_with_delay_proc(void* arg)
{
	exec_every_t* x = reinterpret_cast<exec_every_t*>(arg);
	do {
		waitUntil(now() + x->delay);
	}
	while (x->fu(x->arg));
	delete x;
	return nullptr;
}

pthread_t executeWithDelay(double delay, bool (*fu)(void*), void* arg)
{
	pthread_t	  thread;
	exec_every_t* spawn_data = new exec_every_t(fu, arg, delay);
	int			  e			 = pthread_create(&thread, nullptr /*attr*/, execute_with_delay_proc, spawn_data);
	if (e) abort("executeWithDelay: %s", strerror(e));
	return thread;
}


/* ----	Execute procedure once or with irregular intervals ----------------------------------------
*/

struct exec_at_t
{
	double (*fu)(void*);
	void*  arg;
	double time;

	exec_at_t(double (*fu)(void*), void* arg, double time) : fu(fu), arg(arg), time(time) {}
};

static void* execute_at_proc(void* arg)
{
	exec_at_t* x = reinterpret_cast<exec_at_t*>(arg);

	for (;;)
	{
		waitUntil(x->time);
		double d = x->fu(x->arg);

		if (d <= 0) break;						  // exit
		if (d < x->time / 2) x->time = now() + d; // duration returned
		else x->time = d;						  // time returned
	}
	delete x;
	return nullptr;
}

pthread_t executeAt(double time, double (*fu)(void*), void* arg)
{
	pthread_t  thread;
	exec_at_t* spawn_data = new exec_at_t(fu, arg, time);
	int		   e		  = pthread_create(&thread, nullptr /*attr*/, execute_at_proc, spawn_data);
	if (e) abort("executeAt: %s", strerror(e));
	return thread;
}

pthread_t executeAfter(double delay, double (*fu)(void*), void* arg) { return executeAt(now() + delay, fu, arg); }
