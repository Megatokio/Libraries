#pragma once
/*	Copyright  (c)	Günter Woigk 2001 - 2019
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
*/

#include "config.h"

#if defined(HAVE_SYS_DIRENT_H)
    #include <sys/dirent.h>
#elif defined(HAVE_DIRENT_H)
    #include <dirent.h>
#else
    #define	DT_UNKNOWN	 0
    #define	DT_FIFO		 1
    #define	DT_CHR		 2
    #define	DT_DIR		 4
    #define	DT_BLK		 6
    #define	DT_REG		 8
    #define	DT_LNK		10
    #define	DT_SOCK		12
    #define	DT_WHT		14
#endif


enum s_type		// range: 0 .. 15
{
        s_none	= DT_UNKNOWN,
        s_unkn	= DT_UNKNOWN,		// 0
        s_any	= 0xFFFF,			// 0	used for selecting files by type
        s_file	= DT_REG,
        s_dir	= DT_DIR,
        s_tty	= DT_CHR,
        s_pipe	= DT_FIFO,
        s_sock	= DT_SOCK,
        s_block	= DT_BLK,
        s_link	= DT_LNK,
    #ifdef DT_WHT
        s_erased= DT_WHT,
    #endif
};
















