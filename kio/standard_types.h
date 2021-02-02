#pragma once
/*	Copyright  (c)	Günter Woigk 1999 - 2021
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


	standard data type definitions
	included from kio/kio.h

	integer size: one of the prominent examples, why leaving central points of
	a language specification as an option to the implementer totally sucks.
*/

#include <cctype>
#include <cstdint>
#include <climits>


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

//typedef intptr_t	size_t;
//typedef uintptr_t



// minimum width defs:
// there are also 'int_least8_t'-style typedefs in <stdint.h>

typedef	unsigned char		uchar,	*ucharptr;			// ISO: min. sizeof(char)   =  undefined !
typedef	signed char			schar,	*scharptr;			// ISO: min. sizeof(char)   =  undefined !
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

typedef	char				*ptr;
typedef char const			*cptr;

typedef	uchar				*uptr;
typedef uchar const			*cuptr;


// cast shortenders:

#define _ptr(N)    reinterpret_cast<ptr>(N)
#define _cptr(N)   reinterpret_cast<cptr>(N)
#define _str(N)    reinterpret_cast<str>(N)
#define _cstr(N)   reinterpret_cast<cstr>(N)

















