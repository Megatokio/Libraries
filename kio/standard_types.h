#pragma once
// Copyright (c) 1999 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	standard data type definitions
	included from kio/kio.h

	integer size: one of the prominent examples, why leaving central points of
	a language specification as an option to the implementer totally sucks.
*/

#include <cctype>
#include <climits>
#include <cstdint>


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

typedef int8_t		   int8, *i8ptr;
typedef const int8	   cint8, *ci8ptr;
typedef uint8_t		   uint8, *u8ptr;
typedef const uint8	   cuint8, *cu8ptr;
typedef int16_t		   int16, *i16ptr;
typedef const int16	   cint16, *ci16ptr;
typedef uint16_t	   uint16, *u16ptr;
typedef const uint16   cuint16, *cu16ptr;
typedef int32_t		   int32, *i32ptr;
typedef const int32	   cint32, *ci32ptr;
typedef uint32_t	   uint32, *u32ptr;
typedef const uint32   cuint32, *cu32ptr;
typedef int64_t		   int64, *i64ptr;
typedef const int64	   cint64, *ci64ptr;
typedef uint64_t	   uint64, *u64ptr;
typedef const uint64   cuint64, *cu64ptr;
typedef float		   float32, *f32ptr;
typedef const float32  cfloat32, *cf32ptr;
typedef double		   float64, *f64ptr;
typedef const float64  cfloat64, *cf64ptr;
typedef long double	   float128, *f128ptr;
typedef const float128 cfloat128, *cf128ptr;
typedef long double	   longdouble;

//typedef intptr_t	size_t;
//typedef uintptr_t


// minimum width defs:
// there are also 'int_least8_t'-style typedefs in <stdint.h>

typedef unsigned char	   uchar, *ucharptr;   // ISO: min. sizeof(char)   =  undefined !
typedef signed char		   schar, *scharptr;   // ISO: min. sizeof(char)   =  undefined !
typedef unsigned short	   ushort, *ushortptr; //		min. sizeof(short)  =  undefined !
typedef unsigned int	   uint, *uintptr;	   //		min. sizeof(int)    =  16 bit    !
typedef unsigned long	   ulong, *ulongptr;   //		min. sizeof(long)   =  32 bit
typedef long long		   llong, *llongptr;   //		min. sizeof(llong)  =  64 bit
typedef unsigned long long ullong, *ullongptr;
typedef char*			   charptr;
typedef short*			   shortptr;
typedef int*			   intptr;
typedef long*			   longptr;


// character pointers and strings:

typedef char*		str;
typedef const char* cstr;

typedef char*		ptr;
typedef const char* cptr;

typedef uchar*		 uptr;
typedef const uchar* cuptr;


// cast shortenders:

#define _ptr(N)	 reinterpret_cast<ptr>(N)
#define _cptr(N) reinterpret_cast<cptr>(N)
#define _str(N)	 reinterpret_cast<str>(N)
#define _cstr(N) reinterpret_cast<cstr>(N)
