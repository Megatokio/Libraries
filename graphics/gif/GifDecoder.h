#pragma once
// Copyright (c) 2007 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	The LZW encoder and decoder were written
	by Gershon Elber and Eric S. Raymond
	as part of the GifLib package.

	To my best knowing the copyright never extended on the decoder.
*/

#include "kio/kio.h"
#include "unix/FD.h"


#define LZ_MAX_CODE 4095 /* Largest 12 bit code */
#define LZ_BITS		12

#define FLUSH_OUTPUT 4096 /* Impossible code = flush */
#define FIRST_CODE	 4097 /* Impossible code = first */
#define NO_SUCH_CODE 4098 /* Impossible code = empty */

#define IMAGE_LOADING  0 /* file_state = processing */
#define IMAGE_SAVING   0 /* file_state = processing */
#define IMAGE_COMPLETE 1 /* finished reading or writing */


class GifDecoder
{
	FD	fd;
	int depth, clear_code, eof_code, running_code, running_bits, max_code_plus_one, prev_code, stack_ptr, shift_state,
		file_state, position, bufsize;
	uint32 shift_data, pixel_count;
	uchar  buf[256];
	uchar  stack[LZ_MAX_CODE + 1];
	uchar  suffix[LZ_MAX_CODE + 1];
	uint   prefix[LZ_MAX_CODE + 1];

	int	  read_gif_code();
	uchar read_gif_byte();

public:
	GifDecoder() {}
	~GifDecoder() {}
	//	void	Initialize			( int fd );
	void Initialize(cstr fpath);
	void ReadScanline(uchar* scanline, int length);
	void Finish();
};
