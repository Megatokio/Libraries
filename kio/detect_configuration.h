#pragma once
// Copyright (c) 1994 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/* detect platform, compiler, processor, endianess, sizeof(types)

	platform identifier:
	_UNIX, _WINDOWS							undef or 1

	sub platform identifier:
	_LINUX, _BSD, _MINIX, _SOLARIS, _CYGWIN	undef or 1
	_FREEBSD, _OPENBSD, _NETBSD, _MACOSX	undef or 1

	identifier string:
	_platform_str							e.g. "Unix-Linux"

	compiler:
	_CLANG, _GCC, _MSVC, _IBMCPP, _MPW		undef or 1
	_compiler_str							e.g. "gcc"

	processor:
	_POWERPC, _M68K, _I386, _ALPHA, _SPARC, _I386x64, _ARM
	_processor_str							e.g. "i386"

	endianess & data alignment:
	__LITTLE_ENDIAN__, __BIG_ENDIAN__		undef or 1
	_byteorder_str							e.g. "big endian (msb first)"
	native_byteorder
	native_alignment

	data size:
	_bits_per_byte, _sizeof_char, _sizeof_short, _sizeof_int, _sizeof_long
	_sizeof_pointer, _sizeof_double
	_sizeof_float, _sizeof_long_double		if supported by cpu, else _sizeof_double
*/


// DEPRECATED:
#define _ALIGNMENT_REQUIRED    always assume: true
#define _BYTEORDER             replace with: _byteorder_str
#define _PLATFORM              replace with: _platform_str
#define _COMPILER			   replace with: _compiler_str
#define _PROCESSOR			   replace with: _processor_str
#define _MAX_ALIGNMENT         replace with: sizeof(std::max_align_t) or native_alignment depending on what you meant

/*
	========== detect compiler ========================
*/

#if defined(__clang__)
  #define _CLANG
  #define _compiler_str "clang"

#elif defined(__GNUC__) || defined(__GNUG__)
  #define _GCC
  #define _compiler_str "gcc"

#elif defined(_MSC_VER)
  #define _MSVC
  #define _compiler_str "MSVC"

#elif defined(__IBMCPP__)
  #define _IBMCPP
  #define _compiler_str "IBMCPP"

#elif defined(_MPW_C) || defined(_MPW_CPLUS)
  #define _MPW		1
  #define _compiler_str "MPW"

#else
  #error "can't detect compiler"

#endif


#if defined(__ELF__)
  #define _OBJECT_FORMAT_ELF 1
#elif defined(__MACH__)
  #define _OBJECT_FORMAT_MACHO 1
#else
  #define _OBJECT_FORMAT_COFF 1
#endif


/*
	========== detect platform ========================
*/

#if defined(__FreeBSD__) || defined(_FREEBSD)
  #define _UNIX		1
  #define _BSD		1
  #define _FREEBSD	1
  #define _platform_str "Unix-FreeBSD"

#elif defined(__NetBSD__) || defined(_NETBSD)
  #define _UNIX		1
  #define _BSD		1
  #define _NETBSD	1
  #define _platform_str "Unix-NetBSD"

#elif defined(__OpenBSD__) || defined(_OPENBSD)
  #define _UNIX		1
  #define _BSD		1
  #define _OPENBSD	1
  #define _platform_str "Unix-OpenBSD"

#elif (defined(__MACH__) && defined(__APPLE__)) || defined(_MACOSX)
  #define _UNIX		1
  #define _BSD		1
  #define _MACOSX	1
  #define _platform_str "Unix-MacOSX"

#elif defined(_BSD)
  #define _UNIX		1
  #define _platform_str "Unix-BSD"

#elif defined(__CYGWIN__)
  #define _UNIX		1
  #define _CYGWIN	1
  #define _platform_str "Unix-Cygwin"

#elif defined(__linux__) || defined(_LINUX)
  #define _UNIX		1
  #define _LINUX	1
  #define _platform_str "Unix-Linux"

#elif defined(__minix) || defined(_MINIX)
  #define _UNIX		1
  #define _MINIX	1
  #define _platform_str "Unix-Minix"

#elif defined(__sun) || defined(_SOLARIS)
  #define _UNIX		1
  #define _SOLARIS	1
  #define _platform_str "Unix-Solaris"

#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(_UNIX)
  #define _UNIX		1
  #define _platform_str "Unix-unknown"

#elif defined(_WIN64)
  #define _WINDOWS	1
  #define _platform_str "Windows64"

#elif defined(_WIN32)
  #define _WINDOWS	1
  #define _platform_str "Windows32"

#else
  #define _platform_str "platform-unknown"

#endif


/*	Detect processor & processor characteristics
	using Sun's names

	note: the "_sizeof_*" macros could replaced with "uint(sizeof(T))"
	but then they would be not usable in preprocessor "#if" statements.

	https://sourceforge.net/p/predef/wiki/Architectures/
*/

#if defined(__i386__) || defined(__i386)
  #define _I386			 1
  #define _processor_str "i386"
  #define _bits_per_byte 8
  #define _sizeof_char	 1
  #define _sizeof_short	 2
  #define _sizeof_int	 4
  #define _sizeof_long	 4
  #define _sizeof_float	 4
  #define _sizeof_double 8
  #if defined(__linux__) || defined(__CYGWIN__)
	#define _sizeof_long_double 12 // ~ gcc option: -m96bit-long-double
  #else
	#define _sizeof_long_double 16 // ~ gcc option: -m128bit-long-double
  #endif
  #define _sizeof_pointer	  4
  #define native_alignment	  4

#elif defined(__x86_64__) || defined(_M_AMD64)
  #define _I386x64			  1
  #define _processor_str	  "amd64"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  8
  #define _sizeof_double	  8
  #define _sizeof_float		  4
  #define _sizeof_long_double 16
  #define _sizeof_pointer	  8
  #define native_alignment	  8

#elif defined(__riscv) && __riscv_xlen == 32
  #define _RV32				  1
  #define _processor_str	  "RV32"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  4
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 16
  #define _sizeof_pointer	  4
  #define native_alignment	  4

#elif defined(__riscv) && __riscv_xlen == 64
  #define _RV64				  1
  #define _processor_str	  "RV64"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  8
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 16
  #define _sizeof_pointer	  8
  #define native_alignment	  8

#elif defined(_M_ARM) || defined(__arm__)
  #define _ARM				  1
  #define _processor_str	  "ARM"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  4
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 8
  #define _sizeof_pointer	  4
  #define native_alignment	  4

#elif defined(_M_ARM64) || defined(__aarch64__)
  #define _ARM64		 1
  #define _processor_str "ARM64"
  #define _bits_per_byte 8
  #define _sizeof_char	 1
  #define _sizeof_short	 2
  #define _sizeof_int	 4
  #define _sizeof_long	 8
  #define _sizeof_float	 4
  #define _sizeof_double 8
  #ifdef _MACOSX
	#define _sizeof_long_double 8
  #else
	#define _sizeof_long_double 16
  #endif
  #define _sizeof_pointer	  8
  #define native_alignment	  8

#elif defined(__ppc__) || defined(__PPC__) || defined(__powerpc__)
  #define _POWERPC			  1
  #define _processor_str	  "PowerPC"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  4
  #define _sizeof_float		  4 /* FIXME */
  #define _sizeof_double	  8
  #define _sizeof_long_double 8 /* FIXME */
  #define _sizeof_pointer	  4
  #define native_alignment	  4

#elif defined(__m68k__)
  #define _M68K
  #define _processor_str	  "MC680x0"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  4
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 8
  #define _sizeof_pointer	  4
  #define native_alignment	  2

#elif defined(__alpha__) || defined(__ia64) || defined(__alpha) || defined(_M_ALPHA)
  #define _ALPHA			  1
  #define _processor_str	  "Alpha"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  8
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 16
  #define _sizeof_pointer	  8
  #define native_alignment	  16

#elif defined(__sparcv8) || defined(__sparc_v8__) // 32 bit cpu
  #define _SPARCV8			  1
  #define _processor_str	  "SparcV8"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  4
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 16 /* FIXME */
  #define _sizeof_pointer	  4
  #define native_alignment    4

#elif defined(__sparcv9) || defined(__sparc_v9__) // 64 bit cpu
  #define _SPARCV9			  1
  #define _processor_str	  "SparcV9"
  #define _bits_per_byte	  8
  #define _sizeof_char		  1
  #define _sizeof_short		  2
  #define _sizeof_int		  4
  #define _sizeof_long		  8
  #define _sizeof_float		  4
  #define _sizeof_double	  8
  #define _sizeof_long_double 16
  #define _sizeof_pointer	  8
  #define native_alignment    8

#else
  #error "can't detect processor"
#endif


/*
	Detect endianess of processor
	As soon as in C++20 we'll get language support for this. Yippie! I'm so glad, glad, glad...
	2025-10-12: but no #defines therefore we stick to the old way of doing things...

	https://sourceforge.net/p/predef/wiki/Endianness/
*/

#if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) && !defined(__PDP_ENDIAN__)
  #
  #if defined(__BYTE_ORDER__)
	#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	  #define __LITTLE_ENDIAN__ 1
	#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	  #define __BIG_ENDIAN__ 1
	#elif __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__
	  #define __PDP_ENDIAN__ 1
	#endif
	#
  #elif defined(__FreeBSD__) || defined(__NetBSD__)
	#include <sys/endian.h>
	#if defined(_BYTE_ORDER)
	  #if _BYTE_ORDER == _LITTLE_ENDIAN
		#define __LITTLE_ENDIAN__ 1
	  #elif _BYTE_ORDER == _BIG_ENDIAN
		#define __BIG_ENDIAN__ 1
	  #elif _BYTE_ORDER == _PDP_ENDIAN
		#define __PDP_ENDIAN__ 1
	  #endif
	#endif
	#
  #elif defined(_WIN32)
	#define __LITTLE_ENDIAN__ 1
	#
  #elif defined(__sun__)
	#include <sys/isa_defs.h>
	#if defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
	  #define __LITTLE_ENDIAN__ 1
	#elif defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
	  #define __BIG_ENDIAN__ 1
	#endif
  #endif
#endif

  #if !defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__) && !defined(__PDP_ENDIAN__)
	#include <endian.h>
	#if defined(BYTE_ORDER)
	  #if BYTE_ORDER == LITTLE_ENDIAN
		#define __LITTLE_ENDIAN__ 1
	  #elif BYTE_ORDER == BIG_ENDIAN
		#define __BIG_ENDIAN__ 1
	  #elif defined(BYTE_ORDER) && BYTE_ORDER == PDP_ENDIAN
		#define __PDP_ENDIAN__ 1
	  #endif
	#elif defined(__BYTE_ORDER)
	  #if __BYTE_ORDER == __LITTLE_ENDIAN
		#define __LITTLE_ENDIAN__ 1
	  #elif __BYTE_ORDER == __BIG_ENDIAN
		#define __BIG_ENDIAN__ 1
	  #elif __BYTE_ORDER == __PDP_ENDIAN
		#define __PDP_ENDIAN__ 1
	  #endif
	#elif defined(_BYTE_ORDER)
	  #if _BYTE_ORDER == _LITTLE_ENDIAN
		#define __LITTLE_ENDIAN__ 1
	  #elif _BYTE_ORDER == _BIG_ENDIAN
		#define __BIG_ENDIAN__ 1
	  #elif _BYTE_ORDER == _PDP_ENDIAN
		#define __PDP_ENDIAN__ 1
	  #endif
	#endif
  #endif

#if defined(__LITTLE_ENDIAN__) + defined(__BIG_ENDIAN__) + defined(__PDP_ENDIAN__) != 1
  #error "unable to detect endianess"
#endif

enum ByteOrder {
	msb_first,
	big_endian		  = msb_first,
	network_byteorder = msb_first,
	byteorder_hilo	  = msb_first,
	lsb_first,
	little_endian   = lsb_first,
	intel_byteorder = lsb_first,
	byteorder_lohi  = lsb_first,
	pdp_endian,
	unknown_byteorder
};

#if defined __LITTLE_ENDIAN__
  #define native_byteorder little_endian
  #define _byteorder_str "little endian (lsb first)"
#elif defined __BIG_ENDIAN__
  #define native_byteorder big_endian
  #define _byteorder_str "big endian (msb first)"
#elif defined __PDP_ENDIAN__
  #define native_byteorder pdp_endian
  #define _byteorder_str "pdp endian (msw first + lsb first)"
#else
  #define native_byteorder unknown_byteorder
  #define _byteorder_str "unknown endianess"
#endif


static_assert(sizeof(char) == _sizeof_char, "Size is not correct");
static_assert(sizeof(short) == _sizeof_short, "Size is not correct");
static_assert(sizeof(int) == _sizeof_int, "Size is not correct");
static_assert(sizeof(long) == _sizeof_long, "Size is not correct");
static_assert(sizeof(void*) == _sizeof_pointer, "Size is not correct");
static_assert(sizeof(double) == _sizeof_double, "Size is not correct");
static_assert(sizeof(float) == _sizeof_float, "Size is not correct");
static_assert(sizeof(long double) == _sizeof_long_double, "Size is not correct");
static_assert(static_cast<unsigned char>(0xffffffff) == (1 << _bits_per_byte) - 1, "Size of char is not correct");

