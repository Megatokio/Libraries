#pragma once
// Copyright (c) 2019 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


//	this file is included by kio.h
//	ATTN: wg. _FILE_OFFSET_BITS: this file must be included before any system header.


// system info:
#define HAVE_LONG_FILE_NAMES 1 // file names longer than 14 characters supported
#define STDC_HEADERS		 1 // have the ANSI C header files
//#define _LARGE_FILES		1	// for large files, on AIX-style hosts
#define _FILE_OFFSET_BITS 64 // Number of bits in a file offset --> defines sizeof off_t on 32 bit Linux
//#define _POSIX_1_SOURCE	2	// if the system does not provide POSIX.1 features except with this define
//#define _POSIX_SOURCE		1	// if needed for 'stat' and other things to work
#define _GNU_SOURCE 1 // enable GNU extensions


#include "detect_configuration.h"


#if defined(_UNIX)
  #define HAVE_UNISTD_H 1 // <unistd.h>

  #define HAVE_ASSERT_H	  1 // <assert.h>
  #define HAVE_CTYPE_H	  1 // <ctype.h>
  #define HAVE_ERRNO_H	  1 // <errno.h>
  #define HAVE_FCNTL_H	  1 // <fcntl.h>
  #define HAVE_FNMATCH_H  1 // <fnmatch.h>
  #define HAVE_INTTYPES_H 1 // <inttypes.h>
  #define HAVE_LIBM		  1 // 'm' library (-lm)
  #define HAVE_LIMITS_H	  1 // <limits.h>
  #define HAVE_MEMORY_H	  1 // <memory.h>
  #define HAVE_NETDB_H	  1 // <netdb.h>
  #define HAVE_STDARG_H	  1 // <stdarg.h>
  #define HAVE_STDINT_H	  1 // <stdint.h>
  #define HAVE_STDIO_H	  1 // <stdio.h>
  #define HAVE_STDLIB_H	  1 // <stdlib.h>
  #define HAVE_STRINGS_H  1 // <strings.h>
  #define HAVE_STRING_H	  1 // <string.h>
  #define HAVE_TIME_H	  1 // <time.h>

  #define HAVE_SYS_STAT_H	 1 // <sys/stat.h>
  #define HAVE_SYS_TYPES_H	 1 // <sys/types.h>
  #define HAVE_SYS_UTSNAME_H 1 // <sys/utsname.h>
  #define HAVE_SYS_WAIT_H	 1 // <sys/wait.h>
  #define HAVE_SYS_TIME_H	 1 // <sys/time.h>
#endif


#if defined(_LINUX)
  #define HAVE_SYS_ISA_DEFS_H	// <sys/isa_defs.h>
  #define HAVE_DIRENT_H		  1 // <dirent.h>
  #define HAVE_SYS_VFS_H	  1 // <sys/vfs.h>
  #define HAVE_MNTENT_H		  1 // <mntent.h>
#endif

#if defined(_OPENBSD)
  #define HAVE_MACHINE_VMPARAM_H 1 // <machine/vmparam.h>	maybe _MACOSX
  #define HAVE_DIRENT_H			 1 // <dirent.h>			--> 2021 bug#7

#elif defined(_BSD)
  #define HAVE_MACHINE_VMPARAM_H 1 // <machine/vmparam.h>	maybe _MACOSX
  #define HAVE_SYS_DIRENT_H		 1 // <sys/dirent.h>
#endif

#if defined(_CYGWIN)
  #define HAVE_DIRENT_H	 1 // <dirent.h>
  #define HAVE_SYS_VFS_H 1 // <sys/vfs.h>
#endif

#if defined _MACOSX
//#define HAVE_SYS_LOADAVG_H  // <sys/loadavg.h>
  #define HAVE_DIRENT_H		1 // <dirent.h>
  #define HAVE_SYS_SYSCTL_H 1 // <sys/sysctl.h>
#endif
