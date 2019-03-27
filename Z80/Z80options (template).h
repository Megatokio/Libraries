#pragma once
/*	Copyright  (c)	Günter Woigk 2015 - 2015
  					mailto:kio@little-bat.de

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

 	Permission to use, copy, modify, distribute, and sell this software and
 	its documentation for any purpose is hereby granted without fee, provided
 	that the above copyright notice appear in all copies and that both that
 	copyright notice and this permission notice appear in supporting
 	documentation, and that the name of the copyright holder not be used
 	in advertising or publicity pertaining to distribution of the software
 	without specific, written prior permission.  The copyright holder makes no
 	representations about the suitability of this software for any purpose.
 	It is provided "as is" without express or implied warranty.

 	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 	PERFORMANCE OF THIS SOFTWARE.

	Z80 cpu options
*/


/* #####################################################################

			   options and callback functions for Z80
			   this file contains the project-specific tweaks

   ##################################################################### */



#include "settings.h"
#include "kio/kio.h"


// we don't use the emulated system's actual page size of 32 kB for the Z80's page size
// but slightly smaller pages to keep the dummy read and write pages small
// at the expense of some speed when the system is paging memory.
// The demo Rom doesn't use the paged memory anyway.
#define CPU_PAGEBITS	12		// 4 kB


// standard size for core bytes is 16 bit:
// 8 bit for the CPU plus 8 bit for options
typedef uint16 CoreByte;


#ifdef Z80_SOURCE
// if included from Z80.cpp
#endif







































