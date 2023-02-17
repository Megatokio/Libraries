// Copyright (c) 2017 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include <cmath>
#ifdef HAVE_SYS_TIME_H
  #include <sys/time.h>
#endif


cstr errorstr(int err) noexcept
{
	// get error string for system or custom error number

	// custom error texts:
	static const cstr ETEXT[] =
#define EMAC(A, B) B
#include "errors.h"

		if (err == 0) return "no error";
	//if (err==-1)					return "unknown error (-1)";	strerror: "Unknown error: -1"
	if (uint(err - EBAS) < NELEM(ETEXT)) return ETEXT[err - EBAS];
	else return strerror(err);
}


#if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

template<>
double now(int clock_id) noexcept // return double  ((default))
{
	// get Real Time (Wall Time) in seconds:
	//	• Real time = Wall time can jump when the clock is set by the user or adjusted by NTP etc.
	//	• An IEEE 64 bit double can store time since epoche with 1ns precision only up to 1970-04-15 05:59:59.
	//	  Since 2006-07-14 23:58:24 precision is 256ns.
	//	  Since 2043-01-25 23:56:49 precision is 512ns.

	// clock_gettime():
	// SUSv2, POSIX.1-2001.
	// Availability:
	// On POSIX systems on which these functions are available,
	// the symbol _POSIX_TIMERS is defined in <unistd.h> to a value greater than 0.
	// The symbols _POSIX_MONOTONIC_CLOCK, _POSIX_CPUTIME, _POSIX_THREAD_CPUTIME indicate that
	// CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID are available. (See also sysconf(3).)
	// macOS >= 10.12

	struct timespec tv;
	clock_gettime(clock_id, &tv);
	return double(tv.tv_sec) + double(tv.tv_nsec) * 1e-9;
}

template<>
time_t now(int clock_id) noexcept // return time_t
{
	struct timespec tv;
	clock_gettime(clock_id, &tv);
	return tv.tv_sec;
}

template<>
struct timespec now(int clock_id) noexcept // return timespec
{
	struct timespec tv;
	clock_gettime(clock_id, &tv);
	return tv;
}

template<>
struct timeval now(int clock_id) noexcept // return timeval  ((deprecated))
{
	struct timeval	tv;
	struct timespec tspec;
	clock_gettime(clock_id, &tspec);
	tv.tv_sec  = tspec.tv_sec;
	tv.tv_usec = tspec.tv_nsec / 1000;
	return tv;
}

#else // no clock_gettime:

template<>
double now(int) noexcept // return double  ((default))
{
	// get Real Time (Wall Time) in seconds

	// gettimeofday():
	// SVr4, 4.3BSD.
	// POSIX.1-2001 describes gettimeofday() but not settimeofday().
	// POSIX.1-2008 marks gettimeofday() as obsolete, recommending the use of clock_gettime(2) instead.

	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return double(tv.tv_sec) + double(tv.tv_usec) * 1e-6;
}

template<>
time_t now(int) noexcept // return time_t
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv.tv_sec;
}

template<>
struct timespec now(int) noexcept // return timespec
{
	struct timespec tv;
	struct timeval	tval;
	gettimeofday(&tval, nullptr);
	tv.tv_sec  = tval.tv_sec;
	tv.tv_nsec = tval.tv_usec * 1000;
	return tv;
}

template<>
struct timeval now(int) noexcept // return timeval  ((deprecated))
{
	struct timeval tv;
	gettimeofday(&tv, nullptr);
	return tv;
}

#endif // clock_gettime


// ----------------------------------------------------
#if defined(_UNIX)
  #include <time.h>

/*	wait delay in seconds:
*/
void waitDelay(double delay) noexcept // seconds
{
  #if 0
	// usleep():
	// 4.3BSD, POSIX.1-2001.
	// POSIX.1-2001 declares this function obsolete; use nanosleep(2) instead.
	// POSIX.1-2008 removes the specification of usleep().

	usleep(uint32(delay * 1e6));
  #endif

	// nanosleep():
	// POSIX.1-2001

	double time_fract, time_int;
	time_fract		   = modf(delay, &time_int);
	timespec wait_time = {time_t(time_int), long(time_fract * 1000000000)};
	assert(wait_time.tv_nsec < 1000000000);

	// note: multiple restarts after EINTR pile up rounding errors!
	// could be avoided by using clock_nanosleep() if present
	while (nanosleep(&wait_time, &wait_time) == -1 && errno == EINTR) {}
}

void waitUntil(double time, int clk) noexcept { waitDelay(time - now(clk)); }


// ----------------------------------------------------
#elif defined(_POSIX_THREADS) // in <unistd.h>
  #include <pthread.h>

static pthread_cond_t  wait_cond; // eine condition für Wait(), die niemals getriggert wird
static pthread_mutex_t wait_lock; // ihr Lock
ON_INIT([]() {
	int e = pthread_mutex_init(&wait_lock, nullptr) | pthread_cond_init(&wait_cond, nullptr);
	assert(!e);
});

void waitUntil(double time) // TODO: rework to use one of the various clocks
{
	double time_fract, time_int;
	time_fract		   = modf(time, &time_int);
	timespec wait_time = {time_t(time_int), long(time_fract * 1000000000)};
	assert(wait_time.tv_nsec < 1000000000);

	int e = pthread_mutex_lock(&wait_lock);
	if (!e) e = pthread_cond_timedwait(&wait_cond, &wait_lock, &wait_time);
	if (e == ETIMEDOUT) e = 0;
	if (!e) e = pthread_mutex_unlock(&wait_lock);
	if (!e) return;

	abort("waitUntil: %s", strerror(e));
}

void waitDelay(double delay) // TODO: rework to use one of the various clocks
{
	waitUntil(now() + delay);
}

// ----------------------------------------------------
#elif defined(_POSIX_C_SOURCE)

  #include <sys/select.h>

void waitDelay(double delay) noexcept // TODO: rework to use one of the various clocks
{
	double delay_fract, delay_int;
	delay_fract = modf(delay, &delay_int);
	assert(int32(delay_fract * 1000000) < 1000000);

	struct timeval t;
	t.tv_sec  = long(delay_int);
	t.tv_usec = int32(delay_fract * 1000000);
	select(0, nullptr, nullptr, nullptr, &t); // <-- may return with EINTR and may or may not modify struct timeval
}

void waitUntil(double time) // seconds since epoche
{
	waitDelay(time - now()); // TODO: rework to use one of the various clocks
}

// ----------------------------------------------------
#elif __cplusplus >= 201401
  #include <chrono>
  #include <thread>

/*	wait delay in seconds:
*/
void waitDelay(double delay) // seconds
{							 // TODO: rework to use one of the various clocks
	using namespace std::chrono;
	if (delay > 0) std::this_thread::sleep_for(nanoseconds(int64(delay * 1e9)));
}

void waitUntil(double time) // seconds since epoche
{							// TODO: rework to use one of the various clocks
	using namespace std::chrono;
	static const system_clock::time_point epoche = system_clock::from_time_t(0);
	std::this_thread::sleep_until(epoche + nanoseconds(int64(time * 1e9)));
}
#endif


#ifndef LOGFILE
// ****************************************************
//				Logging to stderr
// ****************************************************

void logline(cstr format, va_list va) { vfprintf(stderr, catstr(format, "\n"), va); }

void log(cstr format, va_list va) { vfprintf(stderr, format, va); }

void logNl() { fprintf(stderr, "\n"); }

void logline(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	logline(format, va);
	va_end(va);
}

void log(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	log(format, va);
	va_end(va);
}

static uint indent; // message indentation		TODO

// log line and add indentation for the following lines
// the indentation will be undone by the d'tor
// For use with macro LogIn(...)
//
LogIndent::LogIndent(cstr format, ...)
{
	va_list va;
	va_start(va, format);
	logline(format, va);
	indent += 2;
	va_end(va);
}

LogIndent::~LogIndent() { indent -= 2; }


// ****************************************************
//	print error in case of emergeny:
//	- in an atexit() registered function
// ****************************************************

  #define ABORTED 2

void abort(cstr fmt, va_list va) // __attribute__((__noreturn__));
{
	if (lastchar(fmt) != '\n') fmt = catstr(fmt, "\n");
	vfprintf(stderr, fmt, va);
	fprintf(stderr, "aborted.\n");
	exit(ABORTED);
}

void abort(cstr fmt, ...) // __attribute__((__noreturn__));
{
	va_list va;
	va_start(va, fmt);
	abort(fmt, va);
	//va_end(va);
}

void abort(int error) // __attribute__((__noreturn__));
{
	abort("%s", strerror(error));
}

#endif
