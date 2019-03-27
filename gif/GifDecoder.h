#pragma once
/*	Copyright  (c)	Günter Woigk 2007 - 2019
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

	The LZW encoder and decoder were written
	by Gershon Elber and Eric S. Raymond
	as part of the GifLib package.

	To my best knowing the copyright never extended on the decoder.
*/

#include "../kio/kio.h"
#include "../unix/FD.h"


#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define LZ_BITS         12

#define FLUSH_OUTPUT    4096    /* Impossible code = flush */
#define FIRST_CODE      4097    /* Impossible code = first */
#define NO_SUCH_CODE    4098    /* Impossible code = empty */

#define IMAGE_LOADING   0       /* file_state = processing */
#define IMAGE_SAVING    0       /* file_state = processing */
#define IMAGE_COMPLETE  1       /* finished reading or writing */


class GifDecoder
{
	FD		fd;
	int		depth, clear_code, eof_code, running_code, running_bits,
			max_code_plus_one, prev_code, stack_ptr, shift_state,
			file_state, position, bufsize;
    uint32	shift_data, pixel_count;
    uchar	buf[256];
    uchar	stack[LZ_MAX_CODE+1];
    uchar	suffix[LZ_MAX_CODE+1];
    uint	prefix[LZ_MAX_CODE+1];

	int		read_gif_code		();
	uchar	read_gif_byte		();

public:
			GifDecoder()		{}
			~GifDecoder()		{}
//	void	Initialize			( int fd );
	void	Initialize			( cstr fpath );
	void	ReadScanline		( uchar* scanline, int length );
	void 	Finish				();
};













