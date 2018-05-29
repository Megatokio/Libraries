/*	Copyright  (c)	Günter Woigk 2007 - 2012
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

#define SAFE 3

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "GifDecoder.h"
//#include "unix/file_utilities.h"
#include "../unix/FD.h"
INIT_MSG


/*	Initialize decoder for first/another image
*/
void GifDecoder::Initialize(cstr filepath)
{
	fd.open_file_r(filepath);

	depth				= fd.read_uint8();	/* lzw_min */
	clear_code			= 1 << depth;
	eof_code			= clear_code + 1;
	running_code		= eof_code + 1;
	running_bits		= depth + 1;
	max_code_plus_one	= 1 << running_bits;
	prev_code			= NO_SUCH_CODE;
	stack_ptr			= 0;
	shift_state			= 0;
	file_state			= IMAGE_LOADING;
	position			= 0;
	bufsize				= 0;
	shift_data			= 0;
	pixel_count			= 0;
//	buf[0]				= 0;
	memset(buf,0,sizeof(buf));			// nötig?
    memset(stack,0,sizeof(stack));		// nötig?
    memset(suffix,0,sizeof(suffix));	// nötig?

	for( int i=0; i<=LZ_MAX_CODE; i++ ) { prefix[i] = NO_SUCH_CODE; }
}


/*	Read to end of image, including the zero block.
*/
void GifDecoder::Finish()
{
	while( bufsize!=0 ) 
	{
		bufsize = fd.read_uchar();
		if( bufsize==0 ) break; 
		fd.read_bytes(buf,bufsize);
	}
	file_state = IMAGE_COMPLETE; 
}


/*	Read the next byte from a Gif file.
	This function is aware of the block-nature of Gif files,
	and will automatically skip to the next block to find
	a new byte to read, or return 0 if there is no next block.
*/
uchar GifDecoder::read_gif_byte()
{
	if( file_state == IMAGE_COMPLETE ) return 0;

	if( position == bufsize )		// internal buffer now empty?
	{								// => read the next block  
		bufsize = fd.read_uchar();
		if( bufsize==0 ) 
		{
			file_state = IMAGE_COMPLETE;
			return 0;
		}
		fd.read_bytes(buf,bufsize);
		position = 0;				// where to get chars 
	}
	return buf[position++];
}



/*
 *  Read the next Gif code word from the file.
 *
 *  This function looks in the decoder to find out how many
 *  bits to read, and uses a buffer in the decoder to remember
 *  bits from the last byte input.
 */
int GifDecoder::read_gif_code()
{
	static int code_masks[] = 
	{ 0x0000, 0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f,
	  0x00ff, 0x01ff, 0x03ff, 0x07ff, 0x0fff };

	while( shift_state < running_bits )
	{
		// Need more bytes from input file for next code: 
		uchar next_byte = read_gif_byte();
		shift_data |= ((ulong) next_byte) << shift_state;
		shift_state += 8;
	}

	int code = shift_data & code_masks[running_bits];

	shift_data >>= running_bits;
	shift_state -= running_bits;

	//	If code cannot fit into running_bits bits, we must raise its size.
	//	Note: codes above 4095 are used for signalling. 
	if( ++running_code > max_code_plus_one && running_bits < LZ_BITS )
	{
		max_code_plus_one <<= 1;
		running_bits++;
	}

	return code;
}


/*	Routine to trace the prefix-linked-list until we get
	a prefix which is a pixel value (less than clear_code).
	Returns that pixel value. 
	If the picture is defective, we might loop here forever,
	so we limit the loops to the maximum possible if the
	picture is okay, i.e. LZ_MAX_CODE times.
*/
inline int trace_prefix( uint* prefix, int code, int clear_code )
{
	int i = 0;
	while( code>clear_code && i++ <= LZ_MAX_CODE ) { code = prefix[code]; }
	return code;
}


/*	The LZ decompression routine:
	Call this function once per scanline to fill in a picture.
*/
void GifDecoder::ReadScanline ( uchar* line, int length )
{
	int	stack_ptr	= this->stack_ptr;
    int	eof_code	= this->eof_code;
    int	clear_code	= this->clear_code;
	int	prev_code	= this->prev_code;
	int	current_code;
	int	current_prefix;

    int i = 0;
	while( stack_ptr!=0 && i<length ) { line[i++] = stack[--stack_ptr]; }	// Pop the stack 

    while( i<length )
    {
		current_code = this->read_gif_code();

		if( current_code == eof_code )			// unexpected EOF 
		{		   
		   if( i!=length-1 || this->pixel_count!=0 ) return;
		   i++; continue;
		}
		
		if( current_code == clear_code )		// reset prefix table etc.
		{	    
			for( int j=0; j<=LZ_MAX_CODE; j++ ) { prefix[j] = NO_SUCH_CODE; }
			running_code = eof_code + 1;
			this->running_bits = this->depth + 1;
			this->max_code_plus_one = 1 << this->running_bits;
			prev_code = this->prev_code = NO_SUCH_CODE;
			continue;
		}

		// Regular code - if in pixel range simply add it to output pixel stream,
		// otherwise trace code-linked-list until the prefix is in pixel range. 
		if( current_code < clear_code ) 
		{
			line[i++] = current_code;			// Simple case. 
		}
		else 
		{
			//	This code needs to be traced:
			//	trace the linked list until the prefix is a pixel, while pushing the suffix pixels 
			//	on to the stack. If finished, pop the stack to output the pixel values.
			if( (current_code<0) || (current_code>LZ_MAX_CODE) ) return;		// image broken
			if( prefix[current_code] == NO_SUCH_CODE ) 
			{
				// Only allowed if current_code is exactly the running code:
				// In that case current_code = XXXCode,
				// current_code or the prefix code is the last code and 
				// the suffix char is exactly the prefix of last code! 
				if( current_code != running_code-2 ) return;		// image broken 
				current_prefix = prev_code;
				suffix[running_code-2] = stack[stack_ptr++] = trace_prefix(prefix,prev_code,clear_code);
			}
			else
			{
				current_prefix = current_code;
			}

			// Now (if picture is okay) we should get no NO_SUCH_CODE during the trace.
			// As we might loop forever (if picture defect) we count the number of loops we trace 
			// and stop if we get LZ_MAX_CODE. Obviously we cannot loop more than that. 
			int j = 0;
			while( j++ <= LZ_MAX_CODE && current_prefix > clear_code && current_prefix <= LZ_MAX_CODE )
			{
				stack[stack_ptr++] = suffix[current_prefix];
				current_prefix = prefix[current_prefix];
			}
			if( j >= LZ_MAX_CODE || current_prefix > LZ_MAX_CODE ) return;	// image broken

			// Push the last character on stack:
			stack[stack_ptr++] = current_prefix;	

			// Now pop the entire stack into output: 
			while( stack_ptr != 0 && i < length ) { line[i++] = stack[--stack_ptr]; }
		}
		
		if( prev_code != NO_SUCH_CODE ) 
		{
			if( (running_code < 2) || (running_code > LZ_MAX_CODE+2) ) return; // image broken
			prefix[running_code-2] = prev_code;

			if( current_code == running_code-2 ) 
			{
				// Only allowed if current_code is exactly the running code:
				// In that case current_code = XXXCode,
				// current_code or the prefix code is the last code and the suffix char 
				//	is exactly the prefix of the last code! 
				suffix[running_code-2] = trace_prefix(prefix,prev_code,clear_code);
			}
			else 
			{
				suffix[running_code-2] = trace_prefix(prefix,current_code,clear_code);
			}
		}
		prev_code = current_code;
    }

    this->prev_code = prev_code;
    this->stack_ptr = stack_ptr;
}










