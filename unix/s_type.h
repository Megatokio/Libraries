#pragma once
// Copyright (c) 2001 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"

#if defined(HAVE_SYS_DIRENT_H)
  #include <sys/dirent.h>
#elif defined(HAVE_DIRENT_H)
  #include <dirent.h>
#else
  #define DT_UNKNOWN 0
  #define DT_FIFO	 1
  #define DT_CHR	 2
  #define DT_DIR	 4
  #define DT_BLK	 6
  #define DT_REG	 8
  #define DT_LNK	 10
  #define DT_SOCK	 12
  #define DT_WHT	 14
#endif


enum s_type // range: 0 .. 15
{
	s_none	= DT_UNKNOWN,
	s_unkn	= DT_UNKNOWN, // 0
	s_any	= 0xFFFF,	  // 0	used for selecting files by type
	s_file	= DT_REG,
	s_dir	= DT_DIR,
	s_tty	= DT_CHR,
	s_pipe	= DT_FIFO,
	s_sock	= DT_SOCK,
	s_block = DT_BLK,
	s_link	= DT_LNK,
#ifdef DT_WHT
	s_erased = DT_WHT,
#endif
};
