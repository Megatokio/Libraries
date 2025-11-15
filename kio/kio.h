#pragma once
// Copyright (c) 1994 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Kio's universal standard c++ header.

	ATTN: wg. _FILE_OFFSET_BITS: this file must be included before any system header.

	required files in project:
	must exist, may be empty:
	- settings.h	project settings
*/

#ifndef KIO_H // the syntax checker has a problem with #pragma once for this file
  #define KIO_H

  #ifndef __cplusplus
	#error c++ ahead
  #endif

  #define kioName	 "Günter Woigk"
  #define kioAddress "Erlangen, Germany"
  #define kioEmail	 "kio@little-bat.de"
  #define kioDomain	 "k1.spdns.de"


  #include "auto_config.h" // platform settings
  #include "settings.h"	   // project settings

  #ifdef _UNIX
	#include <unistd.h>
  #endif

  #include <cassert>
  #include <cctype>
  #include <cerrno>
  #include <climits>
  #include <cstdarg>
  #include <cstdint>
  #include <cstdio>
  #include <cstdlib>
  #include <cstring>
  #include <ctime>
  #include <utility>
extern char** environ; // was required by: Mac OSX (pre 10.5 ?)

  #include "cdefs.h"
  #include "standard_types.h"


/* ==== constants ====
*/
static const bool yes = 1;
static const bool no  = 0;
static const bool on  = 1;
static const bool off = 0;
//static const bool	high	= 1;
//static const bool	low		= 0;
static const bool enabled  = 1;
static const bool disabled = 0;

static const int noerror = 0;
static const int ok		 = 0;
//static const int success	= 0;
static const int error = -1;

  #define kB *0x400u
  #define MB *0x100000u
  #define GB *0x40000000ul


/* directory separator
*/
  #if defined(_WINDOWS)
static const char dirsep = '\\';
  #else
static const char dirsep = '/';
  #endif


/* newline separator
*/
static const char nl = '\n';


/*	initialization at start
	• during statics initialization
	• only once
	• you must assert yourself that the module/class is not used
	  by other modules/classes before it is initialized.
	  else use an 'initialized' flag in constructors et.al.
	  or use pthread pthread_once_t.
	• use:  -->  ON_INIT(function_to_call);
	• use:  -->  ON_INIT([]{...});
	• note: because the brackets are not part of the macro, the body can extend over multiple lines.
	  e.g.: -->  ON_INIT([]{
					init_foo();
					init_bar();
				 });
*/
struct on_init
{
	on_init(void (*f)()) { f(); }
};
  #define ON_INIT static on_init CAT(z, __LINE__)

// log filename during statics initialization
  #ifdef DEBUG
	#define DEBUG_INIT_MSG ON_INIT([] { logline("%s:", strrchr("/" __FILE__, '/') + 1); });
  #else
	#define DEBUG_INIT_MSG
  #endif


/* ==== Code checking depending on SAFETY ====
*/
  #ifdef SAFETY
	#error "macro SAFETY no longer supported"
  #endif


/* ==== functions ====
*/

/*	abort application with exit(2)
	must not be called from a function registered with atexit()!
	defined in log_to_xxx.cpp
*/
extern void abort(cstr format, va_list) __noreturn __printflike(1, 0);
extern void abort(cstr format, ...) __noreturn __printflike(1, 2);


/*	get current wall time in seconds since epoche,
	a monotonic time or
	the per thread/process execution time
	for possible more clocks see time.h
	defined in kio/kio.cpp

	CLOCK_REALTIME = wall time
	The symbols _POSIX_MONOTONIC_CLOCK, _POSIX_CPUTIME, _POSIX_THREAD_CPUTIME indicate that
	CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID are available. (See also sysconf(3).)
*/
  #if defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0
template<typename T = double>
extern T	now(int = CLOCK_REALTIME) noexcept;
extern void waitUntil(double time, int = CLOCK_REALTIME) noexcept; // default = Wall time
  #else															   // only wall time:
template<typename T = double>
extern T	now(int = 0) noexcept;
extern void waitUntil(double time, int clock = 0) noexcept;
  #endif
template<>
double now(int) noexcept; // available specialization: double (template default)
template<>
time_t now(int) noexcept; // available specialization: time_t (int64)
template<>
struct timespec now(int) noexcept; // available specialization: struct timespec
template<>
struct timeval now(int) noexcept; // available specialization: struct timeval ((deprecated))

extern void waitDelay(double seconds) noexcept; // realtime (monotonic)


/*	basic maths
*/
template<class T, class T2>
inline constexpr T min(T a, T2 b) noexcept
{
	return a < b ? a : b;
}

template<class T, class T2>
inline constexpr T max(T a, T2 b) noexcept
{
	return a > b ? a : b;
}
template<class T>
inline constexpr int sign(T a)
{
	return int(a > 0) - int(a < 0);
}
//template <class T> inline T abs ( T a )				{ return a<0 ? -a : a; }
template<class T>
inline constexpr T minmax(T a, T n, T e)
{
	return n <= a ? a : n >= e ? e : n;
}
template<class T>
inline constexpr void limit(T a, T& n, T e)
{
	if (n < a) n = a;
	else if (n > e) n = e;
}

namespace kio
{
// there's a global swap() in some STL file and Qt includes it
template<class T>
inline void swap(T& a, T& b)
{
	T c = std::move(a);
	a	= std::move(b);
	b	= std::move(c);
}

// there's a global abs() in some /usr/inlude/c++/ file and Qt includes it on Linux
template<class T>
inline T abs(T a)
{
	return a < 0 ? -a : a;
}
} // namespace kio

/* revert bytes in buffer
*/
inline void revert_bytes(void* p, uint sz)
{
	ptr a = ptr(p);
	ptr e = a + (sz - 1);
	while (a < e) { std::swap(*a++, *e--); }
}


/*	other standard headers
*/
  #include "cstrings/cstrings.h"
  #include "errors.h"
  #include "exceptions.h"
  #include "peekpoke.h"
  #include "unix/log.h"


/*	check size of off_t is 64 bit:
	if this fails, then probably #define _FILE_OFFSET_BITS 64 is missing in auto_config.h
	or kio.h (and thus auto_config.h) is included too late (after some system header)
	e.g. kio.h must be included before <sys/types.h>
*/
static_assert(sizeof(off_t) == 8, "sizeof(off_t) wrong!");


/* copy&paste templates:

#define BITMASK(i,n)	((0xFFFFFFFF<<(i)) ^ (0xFFFFFFFF<<((i)+(n))))	 // mask to select bits ]i+n .. i]
#define RMASK(n)		(~(0xFFFFFFFF<<(n)))							 // mask to select n bits from the right
#define LMASK(i)		(0xFFFFFFFF<<(i))								 // mask to select all but the i bits from right
double random(double r)	{ return ldexp(random() * r, -31); }			 // #include <cmath>
uint   random(uint n)	{ return (uint32(n) * uint16(random())) >> 16; } // 16 bit random number in range [0 ... [n
*/


#endif
