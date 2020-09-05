#pragma once
#ifndef KIO_H		// the syntax checker has a problem with #pragma once for this file
#define KIO_H

/*	Copyright  (c)  Günter Woigk 1994 - 2020
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


	Kio's universal standard c++ header.

	required files in project:
	must exist, may be empty:
	- settings.h	project settings
*/

#ifndef __cplusplus
#error c++ ahead
#endif

#define	kioName		"Günter Woigk"
#define	kioAddress	"Erlangen, Germany"
#define	kioEmail	"kio@little-bat.de"
#define	kioDomain	"k1.spdns.de"


// test for DEBUG and RELEASE are preferred:
#ifdef DEBUG
  #if defined NDEBUG || defined RELEASE || defined FINAL
	#error ""
  #endif
#else
  #undef  NDEBUG
  #define NDEBUG
  #undef  RELEASE
  #define RELEASE
#endif


#include "auto_config.h"    // platform settings
#include "settings.h"       // project settings

#ifdef _UNIX
#include <unistd.h>
#endif

#include <cstdlib>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cctype>
#include <cassert>
#include <cstdint>
#include <climits>
#include <utility>
#include <ctime>
extern char **environ;		// was required by: Mac OSX (pre 10.5 ?)

#include "standard_types.h"


/* ==== constants ====
*/
static const bool	yes		= 1;
static const bool	no		= 0;
static const bool	on		= 1;
static const bool	off		= 0;
//static const bool	high	= 1;
//static const bool	low		= 0;
static const bool	enabled	= 1;
static const bool	disabled = 0;

static const int noerror	= 0;
static const int ok			= 0;
//static const int success	= 0;
static const int error  	= -1;

#define	kB		*0x400u
#define MB		*0x100000u
#define GB		*0x40000000ul


/* directory separator
*/
#if defined(_WINDOWS)
	static char const dirsep = '\\';
#else
	static char const dirsep = '/';
#endif


/* newline separator
	note: maybe we should support DOS a little better?
*/
static char const nl = '\n';


/* no_index
	in structs and classes the last data member can be a variable length array.
	this is handled differently by different compilers:
	either you set the index to 0 or leave index empty.
	note: current cpp compilers seem all to support '0' (though some complain)
*/
#define no_index 0


/* #defines:
*/
#define throws noexcept(false)
#define	NELEM(feld) (sizeof(feld)/sizeof((feld)[0]))	// UNSIGNED !!

#ifndef __printflike
// argument positions start at 1
// attn: member functions have an invisible first 'this' argument
// use firstvararg=0 for va_arg
#define __printflike(fmtarg,firstvararg) __attribute__((__format__(printf,fmtarg,firstvararg)))
#endif

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

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
struct on_init { on_init(void(*f)()){f();} };
	#define KITTY(X,Y) X ## Y
	#define CAT(X,Y) KITTY(X,Y)
	#define ON_INIT static on_init CAT(z,__LINE__)

// log filename during statics initialization
#ifdef DEBUG
  #define DEBUG_INIT_MSG  ON_INIT( []{logline( "%s:", strrchr("/" __FILE__, '/')+1);} );
#else
  #define DEBUG_INIT_MSG
#endif



/* ==== Code checking depending on SAFETY ====
*/
#ifdef RELEASE
  #undef  SAFETY
  #define SAFETY 0
#else
  #if not defined SAFETY || SAFETY == 0
	#undef  SAFETY
	#define SAFETY 1
  #endif
#endif
#define XSAFE  (SAFETY>=1)
#define XXSAFE (SAFETY>=2)

#if SAFETY == 0
  #define IFDEBUG(X)
  #define IFNDEBUG(X)  X
  #undef  debugstr
  #define debugstr(...)	((void)0)
  #undef  assert
  #define assert(X) ((void)0)
#else
  #define IFDEBUG(X)  X
  #define IFNDEBUG(X)
  #undef  debugstr
  #define debugstr(...)	 fprintf(stderr,__VA_ARGS__)
  #undef  assert
  #define assert(X)  do{ if (unlikely(!(X))) abort("%s:%u: assert failed: %s\n",__FILE__, __LINE__, #X); }while(0)
#endif

#if SAFETY >= 1
  #define xassert		assert
#else
  #define xassert(X)	((void)0)
#endif
#if SAFETY >= 2
  #define xxassert		assert
#else
  #define xxassert(X)	((void)0)
#endif



/* ==== Logging depending on LOGLEVEL ====
*/
#ifdef RELEASE
  #undef  LOGLEVEL
  #define LOGLEVEL 0
#else
  #ifndef LOGLEVEL
	#define LOGLEVEL 0
  #endif
#endif
#define XLOG  (LOGLEVEL>=1)
#define XXLOG (LOGLEVEL>=2)

#if LOGLEVEL>=1
  #define xlog			log
  #define xlogline		logline
  #define xlogNl		logNl
  #define xdebugstr		debugstr
  #define xlogIn		logIn
#else
  #define xlog(...)		 ((void)0)
  #define xlogline(...)  ((void)0)
  #define xlogNl(...)	 ((void)0)
  #define xdebugstr(...) ((void)0)
  #define xlogIn(...)	 ((void)0)
#endif

#if LOGLEVEL>=2
  #define xxlog			log
  #define xxlogline		logline
  #define xxlogNl		logNl
  #define xxdebugstr	debugstr
  #define xxlogIn		logIn
#else
  #define xxlog(...)	  ((void)0)
  #define xxlogline(...)  ((void)0)
  #define xxlogNl(...)	  ((void)0)
  #define xxdebugstr(...) ((void)0)
  #define xxlogIn(...)	  ((void)0)
#endif

// in log.cpp or kio.cpp:
typedef char const *cstr;
extern void logline(cstr, ...)		__printflike(1,2);
extern void logline(cstr, va_list)  __printflike(1,0);
extern void log(cstr, ...)			__printflike(1,2);
extern void log(cstr, va_list)		__printflike(1,0);
extern void logNl();

// indent logging for the lifetime of a function:
struct LogIndent { LogIndent(cstr fmt, ...) __printflike(2,3); ~LogIndent(); };
#define logIn  LogIndent _z_log_ident	// usage:  logIn("format/message", ...)



/* ==== functions ====
*/

/*	abort application with exit(2)
	must not be called from a function registered with atexit()!
	defined in kio.cpp
*/
extern void abort( cstr format, va_list ) __attribute__((__noreturn__)) __printflike(1,0);
extern void abort( cstr format, ... )	  __attribute__((__noreturn__)) __printflike(1,2);
extern void abort( int error_number )	  __attribute__((__noreturn__));

#ifdef NDEBUG
	// catchable in final code:
	#define IERR()	throw InternalError(__FILE__, __LINE__,internalerror)
	#define TODO()	throw InternalError(__FILE__,__LINE__,notyetimplemented)
#else
	// fail hard if debugging: (--> set breakpoint in abort() in kio.h)
	#define IERR()	abort("%s line %u: INTERNAL ERROR",__FILE__,__LINE__)
	#define TODO()	abort("%s line %u: TODO",__FILE__,__LINE__)
#endif


/*	get current time in seconds since epoche
	defined in kio/kio.cpp
*/
template<typename T = double> extern T now() noexcept;
template<> double now() noexcept;						// available specialization: double (template default)
template<> time_t now() noexcept;						// available specialization: time_t (int64)
template<> struct timespec now() noexcept;				// available specialization: struct timespec
template<> struct timeval now() noexcept;				// available specialization: struct timeval ((deprecated))

extern void waitDelay (double seconds);
extern void waitUntil (double seconds_since_epoche);



/*	basic maths
*/
template <class T> inline T min (T a, T b)				{ return a<b ? a : b; }
template <class T> inline T max (T a, T b)				{ return a>b ? a : b; }
template <class T> inline int sign ( T a )				{ return int(a>0) - int(a<0); }
//template <class T> inline T abs ( T a )				{ return a<0 ? -a : a; }
template <class T> inline T	minmax ( T a, T n, T e )	{ return n<=a ? a : n>=e ? e : n;  }
template <class T> inline void limit ( T a, T& n, T e )	{ if (n<a) n=a; else if (n>e) n=e; }

namespace kio
{
	// there's a global swap() in some STL file and Qt includes it
	template <class T> inline void swap ( T& a, T& b )	{ T c=std::move(a); a=std::move(b); b=std::move(c); }

	// there's a global abs() in some /usr/inlude/c++/ file and Qt includes it on Linux
	template <class T> inline T	abs ( T a )				{ return a<0 ? -a : a; }
}

/* revert bytes in buffer
*/
inline void revert_bytes(void* p, uint sz)
{
	ptr a = ptr(p);
	ptr e = a+(sz-1);
	while (a<e) { std::swap(*a++,*e--); }
}


//	helper for refactoring:
//
enum Foo{foo};
#define	LOL  fprintf(stderr,"LOL> \"%s\" - %d\n",__FILE__,__LINE__);


/*	other standard headers
*/
#include "errors.h"
#include "exceptions.h"
#include "cstrings/cstrings.h"
#include "peekpoke.h"
#ifdef LOGFILE
#include "unix/log.h"
#endif




/*	check size of off_t is 64 bit:
	if this fails, then probably #define _FILE_OFFSET_BITS 64 is missing in auto_config.h
	or kio.h (and thus auto_config.h) is included too late (after some system header)
	e.g. kio.h must be included before <sys/types.h>
*/
static_assert(sizeof(off_t)==8,"sizeof(off_t) wrong!");



/* copy&paste templates:

#define BITMASK(i,n)	((0xFFFFFFFF<<(i)) ^ (0xFFFFFFFF<<((i)+(n))))	 // mask to select bits ]i+n .. i]
#define RMASK(n)		(~(0xFFFFFFFF<<(n)))							 // mask to select n bits from the right
#define LMASK(i)		(0xFFFFFFFF<<(i))								 // mask to select all but the i bits from right
double random(double r)	{ return ldexp(random() * r, -31); }			 // #include <cmath>
uint   random(uint n)	{ return (uint32(n) * uint16(random())) >> 16; } // 16 bit random number in range [0 ... [n
*/


#if 0
#if __cplusplus >= 201700
#define FALLTHROUGH [[fallthrough]]
#elif defined(_GCC)
// oder [[gnu::fallthrough]] in c++11 und c++14
#define FALLTHROUGH __attribute__((fallthrough))
#elif defined(_CLANG)
#define FALLTHROUGH [[clang::fallthrough]]
#else
#define FALLTHROUGH
#endif
#endif

// this is the most portable way to define a FALLTHROUGH annotation
// only disadvantage: you must not write a ';' after it
#undef FALLTHROUGH
#define FALLTHROUGH goto CAT(label,__LINE__); CAT(label,__LINE__):

#endif























