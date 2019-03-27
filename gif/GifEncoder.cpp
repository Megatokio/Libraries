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
	as part of the GifLib package
	and released under the MIT license.

	To my best knowing the copyright on the algorithm has expired.
*/

#include "fcntl.h"
#include "unistd.h"
#include "GifEncoder.h"


// gif file creator:
// must be defined somewhere, e.g. in main.cpp:
//
extern cstr	appl_name;				// "zxsp"
extern cstr	appl_version_str;		// "0.6.6"



// ######################################################################################
//							hash table for lzw encoder
// ######################################################################################


/*	The 32 bits of a hash table entry contain two parts: the key and the code.
	The code is 12 bits since the algorithm is limited to 12 bits.
	The key is a 12 bit prefix code + 8 bit new char = 20 bits.
*/
#define HT_GET_KEY(x)	((x) >> 12)
#define HT_GET_CODE(x)	((x) & 0x0FFF)
#define HT_PUT_KEY(x)	((x) << 12)
#define HT_PUT_CODE(x)	((x) & 0x0FFF)


/*	Generate a hash key from the given unique key.
	The given key is assumed to be 20 bits as follows:
		lower 8 bits are the new postfix character,
		upper 12 bits are the prefix code.
*/
inline int gif_hash_key( uint32 key )
{
	return ((key >> 12) ^ key) & HT_KEY_MASK;
}


/*	Clear the hash_table to an empty state.
*/
inline void GifEncoder::clear_hash_table( )
{
	for( int i=0; i<HT_SIZE; i++ ) { hash_table[i] = 0xFFFFFFFFL; }
}


/*	Insert a new item into the hash_table.
	The data is assumed to be new.
*/
inline void GifEncoder::add_hash_key( uint32 key, int code )
{
	int hkey = gif_hash_key(key);
	while (HT_GET_KEY(hash_table[hkey]) != 0xFFFFFL) { hkey = (hkey + 1) & HT_KEY_MASK; }
	hash_table[hkey] = HT_PUT_KEY(key) | HT_PUT_CODE(code);
}


/*	Test whether a key exists in the hash_table:
	returns its code or -1.
 */
inline int GifEncoder::lookup_hash_key( uint32 key )
{
	int hkey = gif_hash_key(key);
	uint32 htkey;

	while ((htkey = HT_GET_KEY(hash_table[hkey])) != 0xFFFFFL)
	{
		if( key == htkey ) return HT_GET_CODE(hash_table[hkey]);	// exists
		hkey = (hkey + 1) & HT_KEY_MASK;
	}
	return -1;	// does not exist
}




// ######################################################################################
//								lzw encoder
// ######################################################################################


/*	Write a byte to a Gif file.

	This function is aware of Gif block structure and buffers
	chars until 255 can be written, writing the size byte first.
	If FLUSH_OUTPUT is the char to be written, the buffer is
	written and an empty block appended.
*/
void GifEncoder::write_gif_byte( int ch ) THF
{
	if( file_state == IMAGE_COMPLETE ) return;

	assert(bufsize<=255);

	if( ch == FLUSH_OUTPUT )
	{
		if(bufsize)
		{
			fd.write_uchar(bufsize);
			fd.write_bytes(buf, bufsize);
			bufsize = 0;
		}
		/* write an empty block to mark end of data */
		fd.write_uchar(0);
		file_state = IMAGE_COMPLETE;
	}
	else
	{
		if( bufsize==255 )
		{
			/* write this buffer to the file */
			fd.write_uchar(bufsize);
			fd.write_bytes(buf, bufsize);
			bufsize = 0;
		}
		buf[bufsize++] = ch;
	}
}


/*	Write a Gif code word to the output file.

	This function packages code words up into whole bytes
	before writing them. It uses the encoder to store
	codes until enough can be packaged into a whole byte.
*/
void GifEncoder::write_gif_code( int code ) THF
{
	if( code == FLUSH_OUTPUT )
	{
		// write all remaining data:
		while (shift_state > 0)
		{
			write_gif_byte(shift_data & 0xff);
			shift_data >>= 8;
			shift_state -= 8;
		}
		shift_state = 0;
		write_gif_byte(FLUSH_OUTPUT);
	}
	else
	{
		shift_data  |= uint32(code) << shift_state;
		shift_state += running_bits;

		while( shift_state >= 8 )
		{
			// write full bytes:
			write_gif_byte(shift_data & 0xff);
			shift_data >>= 8;
			shift_state -= 8;
		}
	}

	// If code can't fit into running_bits bits, raise its size.
	// Note: codes above 4095 are for signalling.
	if( running_code >= max_code_plus_one && code <= 4095 )
	{
    		max_code_plus_one = 1 << ++running_bits;
	}
}


/*	Initialise the lzw encoder
	in: cmap depth (bits).
*/
void GifEncoder::startImage( int cmap_bits ) THF
{
	xlogIn("GifEncoder:StartImage(bits)");

	assert( fd.is_valid() );				// file must already be opened with CreateFile()
	if(cmap_bits<2) cmap_bits = 2;	// min. table size is 2 bit for algorithm reasons

	depth				= cmap_bits;
	file_state			= IMAGE_SAVING;
	position			= 0;
	bufsize				= 0;
	buf[0]				= 0;
	clear_code			= 1 << cmap_bits;
	eof_code			= clear_code + 1;
	running_code		= eof_code + 1;
	running_bits		= cmap_bits + 1;
	max_code_plus_one	= 1 << running_bits;
	current_code		= FIRST_CODE;
	shift_state			= 0;
	shift_data			= 0;

	fd.write_uchar( cmap_bits );	// Write the lzw minimum code size
	clear_hash_table();				// Clear hash table
	write_gif_code( clear_code );	// output Clear code
}


/*	Write one scanline of pixels out to the gif file,
	compressing that line using lzw into a series of codes.
*/
void GifEncoder::writePixelRow( cuptr pixel, uint length ) THF
{
	xxlogIn("GifEncoder:WritePixelRow(px,n)");

	int   new_code;
    uint32 new_key;
    uchar pixval;
	uint  i = 0;

	int current_code = this->current_code == FIRST_CODE ? pixel[i++] : this->current_code;

    while( i < length )
    {
		pixval = pixel[i++]; // Fetch next pixel from stream

		// Form a new unique key to search hash table for the code
		// Combines current_code as prefix string with pixval as postfix char
		new_key = (uint32(current_code) << 8) + pixval;
		if( (new_code = lookup_hash_key(new_key)) >= 0 )
		{
			// This key is already there, or the string is old,
			// so simply take new code as current_code
			current_code = new_code;
		}
		else
		{
			// Put it in hash table, output the prefix code,
			// and make current_code equal to pixval
			this->write_gif_code( current_code );
			current_code = pixval;

			// If the hash_table if full, send a clear first
			// then clear the hash table:
			if (this->running_code >= LZ_MAX_CODE)
			{
				this->write_gif_code(this->clear_code);
				this->running_code = this->eof_code + 1;
				this->running_bits = this->depth + 1;
				this->max_code_plus_one = 1 << this->running_bits;
				clear_hash_table();
			}
			else
			{
				// Put this unique key with its relative code in hash table
				add_hash_key( new_key, this->running_code++ );
			}
		}
    }

    // Preserve the current state of the compressor:
    this->current_code = current_code;
}


/* Write a block of pixels to the gif file
*/
void GifEncoder::writePixelRect ( cuptr pixel, uint w, uint h, uint dy ) THF
{
	xlogIn("GifEncoder:WritePixelRect(px,w,h,dy)");

	assert( int16(dy) >= int16(w) );
	assert( w<0x8000 && h<0x8000 );

	while(h--)
	{
		writePixelRow( pixel, w );
		pixel += dy;
	}
}


/* Flush last bits to file
*/
void GifEncoder::finishImage( ) THF
{
	xlogIn("GifEncoder:FinishImage()");

	write_gif_code(current_code);
	write_gif_code(eof_code);
	write_gif_code(FLUSH_OUTPUT);
}


/* Start image, write image, finish image
*/
void GifEncoder::writePixelmap ( int bits_per_pixel, cPixelmap& pm ) THF
{
	xlogIn("GifEncoder:WritePixelmap(bits,pmap)");

	startImage( bits_per_pixel );
	writePixelRect( pm.getPixels(), pm.width(), pm.height(), pm.rowOffset() );
	finishImage();
}


/*	Open gif file for writing
	and write gif header
	file must not exist				2009-06-12
*/
void GifEncoder::openFile( cstr path ) THF
{
	xlogIn("GifEncoder:openFile()");
	assert( !fd.is_valid() );

	fd.open_file_n(path); 	// throws

	writeGif89aHeader();
}


/*	Write Comment Block with creator message
	Write Trailer byte
	Close file
*/
void GifEncoder::closeFile() THF
{
	xlogIn("GifEncoder:CloseFile()");
	assert(fd.is_valid());

	if(file_state!=IMAGE_COMPLETE) finishImage();
//	writeCommentBlock( usingstr("Made with %s%s",appl_name,appl_version_str) );
	writeGifTrailer();
	fd.close_file();
}


/*	Write sub image block to gif file
	use the global color table or include a local table
	The image block consists of:
	- Image Descriptor
	- optional local color table
	- lzw compressed pixel data
*/
void GifEncoder::writeImage (Pixelmap* pmap, cColormap *cmap ) THF
{
	xlogIn("GifEncoder:WriteImage(pmap,cmap)");

	assert( cmap->usedColors()>0 );

	if( cmap==&global_cmap )	// TODO: or cmap==global_cmap[to sizeof(cmap)]
	{
		writeImageDescriptor( pmap );
	}
	else
	{
		writeImageDescriptor( pmap, cmap );
	}

// write pixels:
	writePixelmap( cmap->usedBits(), *pmap );
}


























