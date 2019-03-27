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

    2013-08-15	kio	modifications for Linux
*/

#include "config.h"
#include <math.h>
#include <sys/time.h>
#include <thread>
#include "cppthreads.h"

INIT_MSG

const  std::thread::id		main_thread = std::this_thread::get_id();


/* ----	PSemaphore ----------------------------------------
*/
PSemaphore::PSemaphore( cstr name, uint32 avail )
:
    name(name),
    avail(avail),
    mutex(),		// can throw std::system_error
    cond()			// can throw std::system_error
{}


PSemaphore::~PSemaphore()
{}

void PSemaphore::release( uint32 n )
{
	{
		std::lock_guard<std::mutex> l(mutex);	// std::system_error
		avail += n;
	}
	cond.notify_all(); // noexcept
}

void PSemaphore::release()
{
	{
		std::lock_guard<std::mutex> l(mutex);	// std::system_error
		avail += 1;
	}
	cond.notify_one(); // noexcept
}

void PSemaphore::request(uint32 n)
{
	std::unique_lock<std::mutex> lock(mutex);
	cond.wait(lock, [=]{return avail >= n;});
	avail -= n;
}

uint32 PSemaphore::request(uint32 n_min, uint32 n_max)
{
	std::unique_lock<std::mutex> lock(mutex);
	cond.wait(lock, [=]{return avail >= n_min;});
    uint32 n = min(avail,n_max);
    avail -= n;
    return n;
}

bool PSemaphore::tryRequest()
{
    if( avail==0 ) return false;
	std::lock_guard<std::mutex> l(mutex);
	if(avail==0) return false;
    avail -= 1;
    return true;
}

bool PSemaphore::tryRequest( double timeout )
{
	std::unique_lock<std::mutex> lock(mutex);
//	cond.wait_for(lock, std::chrono::nanoseconds(int64(timeout/1000000000.0)), [=]{return avail;});
	cond.wait_for(lock, std::chrono::duration<double>(timeout), [=]{return avail;});
	if(avail==0) return false;
    avail -= 1;
    return true;
}




/* ----	PTimer ----------------------------------------
*/

static std::condition_variable wait_cond;	// eine condition für Wait(), die niemals getriggert wird
static std::mutex			wait_lock;		// ihr Lock

void waitDelay ( double delay )
{
	std::unique_lock<std::mutex> lock(wait_lock);
//	cond.wait_for(lock, std::chrono::nanoseconds(int64(timeout/1000000000.0)), [=]{return avail;});
	wait_cond.wait_for(lock, std::chrono::duration<double>(delay), []{return false;});
}


void waitUntil ( double time )
{
	std::unique_lock<std::mutex> lock(wait_lock);
	using namespace std::chrono;
	wait_cond.wait_until(lock, time_point<system_clock,duration<double>>(duration<double>(time)), []{return false;});
}





#if 0
/* ----	Execute procedure in regular intervals ----------------------------------------
*/

struct exec_every_t
{
    bool(*fu)(void*);
    void*	arg;
    double  delay;

    exec_every_t( bool(*fu)(void*), void* arg, double delay )		:fu(fu),arg(arg),delay(delay){}
};

static void* execute_every_proc( void* data )
{
    exec_every_t* x = static_cast<exec_every_t*>(data);
    double now = ::now();
    do { waitUntil( now += x->delay ); } while( x->fu(x->arg) );
    delete x;
    return NULL;
}

pthread_t executeEvery( double delay, bool(*fu)(void*), void* arg )
{
    pthread_t thread;
    exec_every_t* spawn_data = new exec_every_t(fu,arg,delay);
    int e = pthread_create( &thread, NULL /*attr*/, execute_every_proc, spawn_data );
    if(e) abort("executeEvery: %s", strerror(e));
    return thread;
}

static void* execute_with_delay_proc( void* arg )
{
    exec_every_t* x = static_cast<exec_every_t*>(arg);
    do { waitUntil( now() + x->delay ); } while( x->fu(x->arg) );
    delete x;
    return NULL;
}

pthread_t executeWithDelay( double delay, bool(*fu)(void*), void* arg )
{
    pthread_t thread;
    exec_every_t* spawn_data = new exec_every_t(fu,arg,delay);
    int e = pthread_create( &thread, NULL /*attr*/, execute_with_delay_proc, spawn_data );
    if(e) abort("executeWithDelay: %s", strerror(e));
    return thread;
}




/* ----	Execute procedure once or with irregular intervals ----------------------------------------
*/

struct exec_at_t
{
    double(*fu)(void*);
    void*	arg;
    double  time;

    exec_at_t( double(*fu)(void*), void* arg, double time )		:fu(fu),arg(arg),time(time){}
};

static void* execute_at_proc( void* arg )
{
    exec_at_t* x = static_cast<exec_at_t*>(arg);

    for(;;)
    {
        waitUntil( x->time );
        double d = x->fu(x->arg);

        if(d<=0) break;							// exit
        if(d<x->time/2) x->time = now() + d;	// duration returned
        else		    x->time = d;			// time returned
    }
    delete x;
    return NULL;
}

pthread_t executeAt( double time, double(*fu)(void*), void* arg )
{
    pthread_t thread;
    exec_at_t* spawn_data = new exec_at_t(fu,arg,time);
    int e = pthread_create( &thread, NULL /*attr*/, execute_at_proc, spawn_data );
    if(e) abort("executeAt: %s", strerror(e));
    return thread;
}

pthread_t executeAfter( double delay, double(*fu)(void*), void* arg )
{
    return executeAt( now()+delay, fu, arg );
}

#endif













