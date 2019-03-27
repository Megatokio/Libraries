#pragma once
/*	Copyright  (c)	Günter Woigk 1999 - 2019
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

    standard data type definitions
    included from kio/kio.h

    integer size: one of the prominent examples, why leaving central points of
    a language specification as an option to the implementer totally sucks.
*/

#include "config.h"

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#elif defined(_MPW)
#include <types.h>					// DB020812 21:02
#endif

#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/*	numeric_limits<> traits document implementation-defined aspects
    of fundamental arithmetic data types (integers and floating points).

    Standard C++:
    * integers
         bool
         char, signed char, unsigned char
         short, unsigned short
         int, unsigned
         long, unsigned long

    * floating points
         float
         double
         long double

    GNU C++ (where supported by the host C-library):
    * integer
         long long, unsigned long long
*/


/* ---- basic data type definitions ------------------------------------
*/

#ifdef HAVE_STDINT_H	// stdint.h
//						// in stdint.h are also the min. and max. values ((limits))

// exact size defs:

    typedef int8_t		int8,	*i8ptr;		typedef int8 const		cint8,	 *ci8ptr;
    typedef uint8_t		uint8,	*u8ptr;		typedef uint8 const		cuint8,  *cu8ptr;
    typedef int16_t		int16,	*i16ptr;	typedef int16 const		cint16,  *ci16ptr;
    typedef uint16_t	uint16,	*u16ptr;	typedef uint16 const	cuint16, *cu16ptr;
    typedef int32_t		int32,	*i32ptr;	typedef int32 const		cint32,	 *ci32ptr;
    typedef uint32_t	uint32,	*u32ptr;	typedef uint32 const	cuint32, *cu32ptr;
    typedef int64_t		int64,	*i64ptr;	typedef int64 const		cint64,	 *ci64ptr;
    typedef uint64_t	uint64,	*u64ptr;	typedef uint64 const	cuint64, *cu64ptr;
    typedef float		float32,*f32ptr;	typedef float32 const	cfloat32, *cf32ptr;
    typedef double		float64,*f64ptr;	typedef float64 const	cfloat64, *cf64ptr;
    typedef long double	float128,*f128ptr;	typedef float128 const	cfloat128, *cf128ptr;
    typedef long double longdouble;

//	typedef intptr_t	size_t;
//	typedef uintptr_t

#else
    #error what do i do ?
#endif


// minimum width defs:
// there are also 'int_least8_t'-style typedefs in <stdin.h>

    typedef	unsigned char		uchar,	*ucharptr;			// ISO: min. sizeof(char)   =  undefined !
    typedef	unsigned short		ushort,	*ushortptr;			//		min. sizeof(short)  =  undefined !
    typedef unsigned int		uint,	*uintptr;			//		min. sizeof(int)    =  16 bit    !
    typedef	unsigned long		ulong,	*ulongptr;			//		min. sizeof(long)   =  32 bit
    typedef	long long			llong,	*llongptr;			//		min. sizeof(llong)  =  64 bit
    typedef	unsigned long long	ullong,	*ullongptr;
    typedef	char						*charptr;
    typedef	short						*shortptr;
    typedef	int							*intptr;
    typedef	long						*longptr;


// character pointers and strings:

    typedef	char				*str;
    typedef char const			*cstr;

    typedef	char				*ptr;	// currently used for bytes and char - i hope most chars are bytes...
    typedef char const			*cptr;

    typedef	uchar				*uptr;
    typedef uchar const			*cuptr;


// Special:

    #ifndef __MACTYPES__
    typedef signed short		OSErr;
    #endif
    //typedef int32 		OSStatus;


// Classes:

    typedef class String		String;
    typedef const String		cString;


// cast shortenders:

//#define _int(N)    static_cast<int>(N)		the static casts can all use the function style cast
//#define _uint(N)   static_cast<uint>(N)		e.g. int(N)
//#define _long(N)   static_cast<long>(N)
//#define _ulong(N)  static_cast<ulong>(N)
//#define _int8(N)   static_cast<int8>(N)
//#define _uint8(N)  static_cast<uint8>(N)
//#define _int16(N)  static_cast<int16>(N)
//#define _uint16(N) static_cast<uint16>(N)
//#define _int32(N)  static_cast<int32>(N)
//#define _uint32(N) static_cast<uint32>(N)
//#define _int64(N)  static_cast<int64>(N)
//#define _uint64(N) static_cast<uint64>(N)
//#define _char(N)   static_cast<char>(N)
//#define _float(N)  static_cast<float>(N)
//#define _double(N) static_cast<double>(N)
//#define _long_double(N) static_cast<long double>(N)

#define _ptr(N)    reinterpret_cast<ptr>(N)
#define _cptr(N)   reinterpret_cast<cptr>(N)
#define _str(N)    reinterpret_cast<str>(N)
#define _cstr(N)   reinterpret_cast<cstr>(N)


// ----	constants ------------------------------------------------------------
#ifndef NULL
#define	NULL				((void *)0)
#endif
//nst bool true			=	1;
//nst bool false		=	0;
const bool	yes			=	1;
const bool	no 			=	0;
const bool	on			=	1;
const bool	off			=	0;
const bool	high		=	1;
const bool	low			=	0;
const bool	enabled		=	1;
const bool	disabled	=	0;

const OSErr	noerror		=	0;
const OSErr success		=	0;
const OSErr	error		=	-1;
#if defined(_METROWERKS)
    #define	ok				OSErr(0)	// somewhere in the system headers is an enum...
#else
    const OSErr	ok		=	0;
#endif

#define	kB					*0x400
#define MB					*0x100000
#define GB					*0x40000000















