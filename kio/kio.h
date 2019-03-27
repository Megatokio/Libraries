#pragma once
/*	Copyright  (c)	Günter Woigk 1994 - 2019
                    mailto:kio@little-bat.de

    This file is free software

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    • Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    • Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
    PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
    WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
    OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
    ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Kio's universal standard c++ header.

    required files in project:
        config.h			the unix-style MY_COMPILER_HAS_THIS_OR_THAT #defines
        settings.h			project settings, must exist, may be empty

    includes minimal set of headers from my libraries/
        kio/standard_types.h"
        kio/log.h			#ifdef(LOGFILE)
        kio/abort.h
        cstrings/cstrings.h


    asserted macros:
		security level:		NDEBUG, XSAFE, XXSAFE, assert et.al.
        log level:			XLOG, XXLOG, log et.al.
        major platform:		_UNIX  or  _WINDOWS
        minor platform:		_WINDOWS  or  _LINUX  or  _BSD  or  _MINIX  or  _SOLARIS
        compiler:			_METROWERKS  or  _GCC  or  _MPW
        processor:			_POWERPC  or  _M68K  or  _I386  or  _ALPHA  or  _SPARC  or  _I386x64  or  _ARM
        byte order:			_LITTLE_ENDIAN  or  _BIG_ENDIAN  or  _PDP_ENDIAN
        data alignment:		_ALIGNMENT_REQUIRED
                            _MAX_ALIGNMENT
        data size:		 	_bits_per_byte
                            _sizeof_char
                            _sizeof_short
                            _sizeof_int
                            _sizeof_long
                            _sizeof_double
                            _sizeof_short_float			if supported by cpu, else _sizeof_double
                            _sizeof_long_float			if supported by cpu, else _sizeof_double
                            _sizeof_pointer

    other defines:
        _PLATFORM			"<platform-subplatform>"
        _COMPILER			"<compiler>"
        _PROCESSOR			"<processor>"
        _BYTEORDER			"<byteorder-info>"

        INIT_MSG(text)		print message to stderr during startup
        ON_INIT(action())	initialize module
        char const dirsep;	directory separator character	(dep. on target platform)
        char const nl;		line break character			(dep. on target platform)
        #define no_index	dummy index for empty arrays	(dep. on compiler)
*/


#define	myName				"Günter Woigk"
#define	myAddress			"Sankt Johann 6/246, 91056 Erlangen, Germany"
#define	myEmail				"kio@little-bat.de"
#define	myDomain			"k1.spdns.de"



/* ----	include "config.h" ----------------------------------------
        on unix systems "config.h" should be generated from "configure.in"
        using autoconf and should not be part of the cvs repository.

        on other systems put this file into the platform specific subfolder
        and edit this file directly. this platform specific variant should
        go into the cvs.
*/
#include "config.h"     // config.h MUST be included before ANY system header
                        // wg. #define _FILE_OFFSET_BITS 64 on 32-bit Linux
#include "settings.h"



/* ----	determine platform ----------------------------------------

        platform identifier:
        guarantee that platform is detected.
        _UNIX, _WINDOWS							undefined or 1

        sub platform identifier:
        guarantee that sub platform is detected.
        _LINUX, _BSD, _MINIX, _SOLARIS			undefined or 1
*/
#if defined(_UNIX) + defined(_WINDOWS) != 1
    #if defined(unix) || defined(__unix) || defined(__unix__) || (defined(__MACH__) && defined(__APPLE__))
        #define _UNIX 1
    #endif
    #if defined(_WIN32) || defined(_WIN64)
        #define _WINDOWS 1
    #endif
    #if defined(_UNIX) + defined(_WINDOWS) != 1
        #error platform could not be auto-detected: please set it in config.h file!
    #endif
#endif

#if defined(_UNIX)
    #if defined(_LINUX) + defined(_BSD) + defined(_MINIX) + defined(_SOLARIS) != 1
        #if defined(__linux__)
            #define _LINUX 1
        #endif
        #if (defined(__MACH__) && defined(__APPLE__)) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
            #define _BSD 1
        #endif
        #if defined(__minix)
            #define _MINIX 1
        #endif
        #if defined(__sun)
            #define _SOLARIS 1
        #endif
        #if defined(_LINUX) + defined(_BSD) + defined(_MINIX) + defined(_SOLARIS) != 1
            #error UNIX sub platform could not be autodetected: please set it in config.h file!
        #endif
    #endif

    #if defined(_LINUX)
        #if defined(_SUSE)
            #define _PLATFORM "Unix-Linux-SuSE"
        #elif defined(_DEBIAN)
            #define _PLATFORM "Unix-Linux-Debian"
        #elif defined(_REDHAT)
            #define _PLATFORM "Unix-Linux-Redhat"
        #else
            #define _PLATFORM "Unix-Linux"
        #endif
    #elif defined(_BSD)
        #if defined(__FreeBSD__)
            #define _FREEBSD 1
            #define _PLATFORM "Unix-BSD-FreeBSD"
        #elif defined(__OpenBSD__)
            #define _OPENBSD 1
            #define _PLATFORM "Unix-BSD-OpenBSD"
        #elif defined(__NetBSD__)
            #define _NETBSD 1
            #define _PLATFORM "Unix-BSD-NetBSD"
        #elif defined(__MACH__) && defined(__APPLE__)
            #define _MACOSX 1
            #define _PLATFORM "Unix-BSD-MacOSX"
        #else
            #define _PLATFORM "Unix-BSD"
        #endif
    #elif defined(_MINIX)
        #define _PLATFORM "Unix-Minix"
    #elif defined(_SOLARIS)
        #define _PLATFORM "Unix-Solaris"
    #else
        #define _PLATFORM "Unix"
    #endif

#elif defined(_WINDOWS)
    #define _PLATFORM "Windows"
#else
    #define _PLATFORM "platform unknown"
#endif


/* ----	determine compiler ----------------------------------
        guarantee that compiler is detected.
        _METROWERKS, _GCC, _MPW
*/
#if !defined(_METROWERKS) && !defined(_GCC) && !defined(_MPW)
    #if defined(_MPW_C) || defined(_MPW_CPLUS)
        #define _MPW			1
    #elif defined(__GNUC__) || defined(__GNUG__)
        #define _GCC			1
    #else
        #error "can't determine compiler"
    #endif
#endif
#if defined(_METROWERKS)
    #define _COMPILER "Metrowerks"
#elif defined(_GCC)
    #define _COMPILER "gcc"
#elif defined(_MPW)
    #define _COMPILER "MPW"
#endif



/* ----	basic system headers -----------------------
*/
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#else
extern long random();
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef HAVE_MACHINE_ENDIAN_H
#include <machine/endian.h>
#endif

#ifdef HAVE_ENDIAN_H
#include <endian.h>
#endif



/* ----	determine processor -----------------------------------
        guarantee that processor is detected.
        _POWERPC, _M68K, _I386, _ALPHA, _SPARC, _I386x64, _ARM
*/
#if defined(_POWERPC) + defined(_M68K) + defined(_I386) + defined(_ALPHA) + defined(_SPARC) + defined(_I386x64) + defined(_ARM) != 1
    #if defined(__i386__) || defined(__i386)
        #define	_I386		1
    #elif defined(__ppc__) || defined(__PPC__) || defined(__powerpc__)
        #define	_POWERPC	1
    #elif defined(__x86_64__)
        #define _I386x64	1
    #elif defined(__alpha__) || defined(__ia64)
        #define	_ALPHA		1
    #elif defined(__sparc__) || defined(__sparc)
        #if defined(__sparcv8)		// 32 bit cpu
            #define	_SPARC	"V8"
        #elif defined(__sparcv9)	// 64 bit cpu
            #define	_SPARC	"V9"
        #else
            #define _SPARC	1
        #endif
    #endif
    #if defined(_POWERPC) + defined(_M68K) + defined(_I386) + defined(_ALPHA) + defined(_SPARC) + defined(_I386x64) + defined(_ARM) != 1
        #error "can't determine processor type. ((or multiple processor types detected))"
    #endif
#endif
#if defined(_POWERPC)
    #define _PROCESSOR "PowerPC"
#elif defined(_M68K)
    #define _PROCESSOR "MC680x0"
#elif defined(_I386)
    #define _PROCESSOR "i386"
#elif defined(_ALPHA)
    #define _PROCESSOR "Alpha"
#elif defined(_SPARC)
    #define _PROCESSOR "Sparc"
#elif defined(_I386x64)
    #define _PROCESSOR "i386x64"
#elif defined(_ARM)
    #define _PROCESSOR "ARM"
#endif



/* ==== dependencies ==============================================
*/



/* ---- directory separator ------------------------
        guaranteed constant.
*/
#if defined(_WINDOWS)
    char const	dirsep	=	'\\';
#else
    char const	dirsep	=	'/';
#endif



/* ----	newline separator ------------------------------
        guaranteed constant.
        note: *MAYBE* we'll support DOS a little better?
*/
    char const	nl		=	'\n';



/* ----	no_index --------------------------------------
        guaranteed macro.
        in structs and classes the last data member can be a variable length array.
        this is handled differently by different compilers:
        either you set the index to 0 or you leave it completely out.
*/
#if defined(_GCC)
    #define		no_index		0
#elif defined(_METROWERKS)
    #define		no_index		/* ((empty)) */
#elif defined(_MPW)
    #error "please set no_index to what is supported by MPW. thanks."
#else
    #error
#endif



/* ----	processor characteristics ---------------------
        2003-11-26 kio: switch to Sun's names
        2012-08-03 kio: reworked
*/
#if defined(_SPARC)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_double			8
    #define	_sizeof_short_float		_sizeof_double

  #if _SPARC=="V8"
    #define	_sizeof_long			4
    #define	_sizeof_long_float		_sizeof_double		/* FIXME */
    #define	_sizeof_pointer			4
  #elif _SPARC=="V9"
    #define	_sizeof_long			8
    #define	_sizeof_long_float		16
    #define	_sizeof_pointer			8
  #endif

// all should be setup by /usr/include/sys/isa_defs.h
//	#define	_BIG_ENDIAN
//	#define	_ALIGNMENT_REQUIRED		1
//	#if _SPARC=="V8"
//		#define	_MAX_ALIGNMENT		8
//	#elif _SPARC=="V9"
//		#define	_MAX_ALIGNMENT		16
#endif


#if defined(_ALPHA)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_long			8
    #define	_sizeof_double			8
    #define	_sizeof_short_float		_sizeof_double
    #define	_sizeof_long_float		16
    #define	_sizeof_pointer			8

    #undef	_LITTLE_ENDIAN
    #undef	_BIG_ENDIAN
    #undef	_PDP_ENDIAN
    #undef	_MAX_ALIGNMENT
    #undef	_ALIGNMENT_REQUIRED

    #define	_LITTLE_ENDIAN			1
    #define	_MAX_ALIGNMENT			16
    #define	_ALIGNMENT_REQUIRED		1
#endif


#if defined(_I386)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_long			4
    #define	_sizeof_double			8
    #define	_sizeof_short_float		4
#if defined(_BSD)	// OSX
    #define	_sizeof_long_float		16		// ~ gcc option: -m128bit-long-double
#else // LINUX
    #define	_sizeof_long_float		12		// ~ gcc option: -m96bit-long-double
#endif
    #define	_sizeof_pointer			4

    #undef	_LITTLE_ENDIAN
    #undef	_BIG_ENDIAN
    #undef	_PDP_ENDIAN
    #undef	_MAX_ALIGNMENT
    #undef	_ALIGNMENT_REQUIRED

    #define	_LITTLE_ENDIAN			1
    #define	_MAX_ALIGNMENT			4
    #define	_ALIGNMENT_REQUIRED		0
#endif


#if defined(_I386x64)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_long			8
    #define	_sizeof_double			8
    #define	_sizeof_short_float		4
    #define	_sizeof_long_float		16
    #define	_sizeof_pointer			8

    #undef	_LITTLE_ENDIAN
    #undef	_BIG_ENDIAN
    #undef	_PDP_ENDIAN
    #undef	_MAX_ALIGNMENT
    #undef	_ALIGNMENT_REQUIRED

    #define	_LITTLE_ENDIAN			1
    #define	_MAX_ALIGNMENT			8
    #define	_ALIGNMENT_REQUIRED		0
#endif


#if defined(_POWERPC)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_long			4
    #define	_sizeof_double			8
    #define	_sizeof_short_float		4				/* FIXME */
    #define	_sizeof_long_float		_sizeof_double	/* FIXME */
    #define	_sizeof_pointer			4

    #undef	_LITTLE_ENDIAN
    #undef	_BIG_ENDIAN
    #undef	_PDP_ENDIAN
    #undef	_MAX_ALIGNMENT
    #undef	_ALIGNMENT_REQUIRED

    #define	_BIG_ENDIAN				1
    #define	_MAX_ALIGNMENT			4
    #define	_ALIGNMENT_REQUIRED		0
    // note: missaligned access to float/double is handled _very_slowly_ on G3/G4
#endif


#if defined(_ARM)
    #define _bits_per_byte			8
    #define _sizeof_char			1
    #define	_sizeof_short			2
    #define	_sizeof_int				4
    #define	_sizeof_long			4
    #define	_sizeof_double			8
    #define	_sizeof_short_float		4
    #define	_sizeof_long_float		8
    #define	_sizeof_pointer			4

    #undef	_LITTLE_ENDIAN
    #undef	_BIG_ENDIAN
    #undef	_PDP_ENDIAN
    #undef	_MAX_ALIGNMENT
    #undef	_ALIGNMENT_REQUIRED

    #define	_LITTLE_ENDIAN			1
    #define	_MAX_ALIGNMENT			4
    #define	_ALIGNMENT_REQUIRED		0		// since ARMv4
#endif


#if defined(_M68K)
    #undef	_BIG_ENDIAN
    #define	_BIG_ENDIAN				1
    #error FIXME
#endif


#if defined(_PDP)
    #undef	_PDP_ENDIAN
    #define	_PDP_ENDIAN				1
    #error FIXME
#endif


static_assert(sizeof(char)   == _sizeof_char,    "Size is not correct");
static_assert(sizeof(short)  == _sizeof_short,   "Size is not correct");
static_assert(sizeof(int)    == _sizeof_int,     "Size is not correct");
static_assert(sizeof(long)   == _sizeof_long,    "Size is not correct");
static_assert(sizeof(void*)  == _sizeof_pointer, "Size is not correct");
static_assert(sizeof(double) == _sizeof_double,   "Size is not correct");
static_assert(sizeof(float)  == _sizeof_short_float, "Size is not correct");
static_assert(sizeof(long double) == _sizeof_long_float, "Size is not correct");

// check size of off_t is 64 bit:
// if this fails, then probably #define _FILE_OFFSET_BITS 64 is missing in config.h
// or kio.h (and thus config.h) is included too late (after some after system header)
// e.g. kio.h must be included before <sys/types.h>
static_assert(sizeof(off_t)==8,"sizeof(off_t) wrong!");




/* ----	byte order ---------------------------------
        guarantee that byte order is detected.
        _BIG_ENDIAN  	e.g. M68k	dc.l '4321' = dc.b '4','3','2','1'	(ok.)
        _LITTLE_ENDIAN	e.g. i386	dc.l '4321' = dc.b '1','2','3','4'	(bad!)
        _PDP_ENDIAN  	e.g. PDP	dc.l '4321' = dc.b '3','4','1','2'	(brain dead!)
*/
#if defined(BYTE_ORDER) && ( defined(_LITTLE_ENDIAN) + defined(_BIG_ENDIAN) + defined(_PDP_ENDIAN) == 0 )
    #if defined(BIG_ENDIAN) && BYTE_ORDER==BIG_ENDIAN
        #define			_BIG_ENDIAN		1
    #elif defined(LITTLE_ENDIAN) && BYTE_ORDER==LITTLE_ENDIAN
        #define			_LITTLE_ENDIAN	1
    #elif defined(PDP_ENDIAN) && BYTE_ORDER==PDP_ENDIAN
        #define			_PDP_ENDIAN		1
    #endif
#endif

#if defined(__BIG_ENDIAN__) && !defined(_BIG_ENDIAN)
    #error Big Endian: fix me!
#endif

#if defined(__LITTLE_ENDIAN__) && !defined(_LITTLE_ENDIAN)
    #error Little Endian: fix me!
#endif

#if defined(_LITTLE_ENDIAN) + defined(_BIG_ENDIAN) + defined(_PDP_ENDIAN) != 1
    #error "can't detect byte order  ((or multiple byte orders detected))"
#endif


//enum ByteOrder
//{
//	MsbFirst, BigEndian=MsbFirst,    NetworkByteOrder=MsbFirst,
//	LsbFirst, LittleEndian=LsbFirst, IntelByteOrder=LsbFirst,
//	PdpEndian
//};


#if defined(_LITTLE_ENDIAN)
    #define native_byteorder LittleEndian
    #define _BYTEORDER "little endian (lsb first)"
#elif defined(_BIG_ENDIAN)
    #define native_byteorder BigEndian
    #define _BYTEORDER "big endian (msb first)"
#elif defined(_PDP_ENDIAN)
    #define native_byteorder PdpEndian
    #define _BYTEORDER "pdp endian (lsb first + msw first)"
#endif


/* ----	verify that all processor characteristics are set --------------------
*/
#if !defined(_ALIGNMENT_REQUIRED) || !defined(_MAX_ALIGNMENT)
    #error "missing CPU characteristics. fixme!"
#endif



/* ---- define __printflike(fmtarg, firstvararg) ----------------
        this is how it is defined on my MacOS
*/
#ifndef __printflike
#define __printflike(A,B) __attribute__((__format__(printf,A,B)))
#endif



/* ----	LOGGING and SAFETY ---------------------------
*/

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

#ifdef NDEBUG
  #undef  LOGLEVEL
  #define LOGLEVEL 0
  #undef  SAFETY
  #define SAFETY   0
#else
  #ifndef SAFETY
	#define SAFETY   1
  #endif
  #ifndef LOGLEVEL
	#define LOGLEVEL 0
  #endif
#endif


#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)

#if SAFETY==0
  #undef  NDEBUG
  #define NDEBUG
  #define IFDEBUG(X)
  #define IFNDEBUG(X)	X
  #undef  debugstr
  #define debugstr(...)	((void)0)
  #undef  assert
  #define assert(X) ((void)0)
#else
  #undef  NDEBUG
  #define IFDEBUG(X)	X
  #define IFNDEBUG(X)
  #undef  debugstr
  #define debugstr(FMT,...)	vfprintf(stderr,FMT,__VA_ARGS__)
  #undef  assert
  #define assert(X)		do{ if(unlikely(!(X))) abort("%s:%u: assert failed: %s\n",__FILE__, __LINE__, #X); }while(0)
#endif

#define XSAFE  (SAFETY>=1)
#define XXSAFE (SAFETY>=2)

#if SAFETY>=1
  #define xassert		assert
#else
  #define xassert(X)	((void)0)
#endif
#if SAFETY>=2
  #define xxassert		assert
#else
  #define xxassert(X)	((void)0)
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

//	Test Macros:
//	must not be called from a function registered with atexit()
#ifndef NDEBUG
  #define IERR()		abort("%s line %u: INTERNAL ERROR",__FILE__,__LINE__)
  #define TODO()		abort("%s line %u: TODO",__FILE__,__LINE__)
#else
  #define IERR()		throw internal_error(__FILE__, __LINE__,internalerror)
  #define TODO()		throw internal_error(__FILE__,__LINE__,notyetimplemented)
#endif

//	abort application with exit(2)
//	must not be called from a function registered with atexit()!
extern void abort( cstr format, va_list )	__attribute__((__noreturn__)) __printflike(1,0);
extern void abort( cstr formatstring, ... )	__attribute__((__noreturn__)) __printflike(1,2);
extern void abort( int error_number )		__attribute__((__noreturn__));


// get current time:
extern double	now();		// in kio/kio.cpp




//	new() / delete() library to use
//
//	void*		operator new	    ( size_t );
//	void*		operator new[]		( size_t );
//	inline void*operator new    	( size_t, void* p )	{ return p; }
//	void		operator delete 	( void* );
//	void 		operator delete[]	( void* );


//	initialization at start
//	• during statics initialization
//	• only once
//	• you must assert yourself that the module/class is not used
//	  by other modules/classes before it is initialized.
//	  else use a 'virgin' test in constructors et.al.
//	  or use pthread pthread_once_t.
//	• use:  -->  ON_INIT(function_to_call);
//	• use:  -->  ON_INIT([]{...});
//	• note: because the brackets are not part of the macro, the body can extend over multiple lines.
//	  e.g.: -->  ON_INIT([]{
//					init_foo();
//					init_bar();
//				 });
//
struct on_init { on_init(void(*f)()){f();} };
	#define KITTY(X,Y) X ## Y
	#define CAT(X,Y) KITTY(X,Y)
	#define ON_INIT static on_init CAT(z,__LINE__)


// log filename during statics initialization
#if XLOG
  #define INIT_MSG  ON_INIT( []{logline( "%s:", strrchr("/" __FILE__, '/')+1);} );
#else
  #define INIT_MSG
#endif


#define VIR	virtual
#define	INL	inline
#define EXT extern
#define throws noexcept(false)


// define custom error numbers:
// the messages are included in log.cpp
enum
{
#define	 EBAS		0x7400		// base of free range for own error numbers
#define  EMAC(A,B)	A
#include "error_emacs.h"
};

extern cstr errorstr(int err);	// get error string for system or custom error number
inline cstr errorstr() { return errorstr(errno); }


//	other standard headers
//
#include "standard_types.h"
#include "exceptions.h"
#include "peekpoke.h"
#ifdef LOGFILE
#include "../unix/log.h"
#endif
#include "cstrings/cstrings.h"


//	basic maths
//
template <class T> INL int	sign   ( T a )				{ return int(a>0) - int(a<0); }
//template <class T> INL T  abs    ( T a )				{ return a<0 ? -a : a; }
template <class T> INL T	min    ( T a, T b )			{ return a<b ? a : b; }
template <class T> INL T	max    ( T a, T b )			{ return a>b ? a : b; }
template <class T> INL T	minmax ( T a, T n, T e )	{ return n<=a ? a : n>=e ? e : n;  }
template <class T> INL void	limit  ( T a, T&n, T e )	{ if(n<a) n=a; else if(n>e) n=e; }


// 	num elements in array:
//
#define	NELEM(feld)		(sizeof(feld)/sizeof((feld)[0]))	// UNSIGNED !!


//	for refactoring:
//
enum Foo{foo};


/* copy&paste templates:
#define BITMASK(i,n)	((0xFFFFFFFF<<(i)) ^ (0xFFFFFFFF<<((i)+(n))))		// mask to select bits ]i+n .. i]
#define RMASK(n)		(~(0xFFFFFFFF<<(n)))								// mask to select n bits from the right
#define LMASK(i)		(0xFFFFFFFF<<(i))									// mask to select all but the i bits from right
#define	LOL				fprintf(stderr,"LOL> \"%s\" - %d\n",__FILE__,__LINE__);
double random(double r)	{ return ldexp(random() * r, -31); }				// #include <math.h>
uint   random(uint n)	{ return (uint32(n) * uint16(random())) >> 16; }	// 16 bit random number in range [0 ... [n
*/


namespace kio
{
    // there's a global swap() in some STL file and Qt includes it
    template <class T> INL void	swap   ( T& a, T& b )		{ T c=a; a=b; b=c; }

    // there's a global abs() in some /usr/inlude/c++/ file and Qt includes it on Linux
    template <class T> INL T	abs    ( T a )				{ return a<0 ? -a : a; }
}



























