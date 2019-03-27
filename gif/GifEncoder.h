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

	To my best knowing the copyright on the algorithm has expired.
*/

#include "../kio/kio.h"
#include "../unix/FD.h"
#include "Pixelmap.h"
#include "Colormap.h"


#define HT_SIZE         8192    /* 13 bit hash table size */
#define HT_KEY_MASK     0x1FFF  /* 13 bit key mask */

#define LZ_MAX_CODE     4095    /* Largest 12 bit code */
#define LZ_BITS         12

#define FLUSH_OUTPUT    4096    /* Impossible code = flush */
#define FIRST_CODE      4097    /* Impossible code = first */
#define NO_SUCH_CODE    4098    /* Impossible code = empty */

#define IMAGE_LOADING   0       /* file_state = processing */
#define IMAGE_SAVING    0       /* file_state = processing */
#define IMAGE_COMPLETE  1       /* finished reading or writing */

#define LOHI(N) uint8(N), uint8((N)>>8)
#define THF	noexcept(false)			// THF


/* misc. block flags:
*/
enum	// Screen Descriptor:
{
	has_global_cmap	 = 1<<7,
	glob_cmap_sorted = 1<<3,
	img_cmap_bits	 = 1<<4,	// bits 4-6
	glob_cmap_bits	 = 1<<0		// bits 0-2
};
enum	// Sub Image Descriptor:
{
	has_local_cmap	 = 1<<7,
	loc_cmap_sorted	 = 1<<5,
	rows_interlaced	 = 1<<6,
	loc_cmap_bits	 = 1<<0		// bits 0-2
};
enum	// Graphic Control Block:  ((Gif89a++))
{
	no_disposal		 = 0<<2,	// don't restore: do as you like..
	keep_new_pixels  = 1<<2,	// don't restore: keep new pixels
	restore_bgcolor  = 2<<2,	// restore to background color
	restore_pixels   = 3<<2,	// restore previous pixels
	no_transparency	 = 0x00,
	has_transparency = 0x01
};


class GifEncoder
{
	FD			fd;
	Colormap	global_cmap;		// if not used, then global_cmap.used = 0

	int			depth, clear_code, eof_code, running_code, running_bits,
				max_code_plus_one, current_code, shift_state,
				file_state, position, bufsize;
	uint32		shift_data;
	uint8		buf[256];

	uint32		hash_table[HT_SIZE];
	void		clear_hash_table();
	void		add_hash_key	(uint32 key, int code);
	int			lookup_hash_key (uint32 key);

	void		write_gif_code	(int) THF;
	void		write_gif_byte  (int) THF;


public:
                GifEncoder      ()			{}
                ~GifEncoder     ()			{ /*fd.close_file();*/ }


/*	open file and write gif file header
*/
	void    openFile			(cstr path)     THF;
//	void    setFile             (int fd)		{ this->fd = fd; }
	bool    imageInProgress     () const		{ return fd.is_valid(); }

/*	Clear global cmap and write logical screen descriptor:
	aspect ratio: If the value of the field is not 0,
	this approximation of the aspect ratio is computed based on the formula:
			Aspect Ratio = (pixel_width / pixel_height + 15) / 64
*/
	void writeScreenDescriptor ( uint16 w, uint16 h, uint16 colors, uint8 aspect=0 ) THF
	{
		assert( colors>0 );
		global_cmap.purgeCmap();
		uint8 flags = uint8((reqBits(colors)-1)<<4);
		uint8 bu[] = { LOHI(w), LOHI(h), flags, 0/*bgcolor*/, aspect };
		fd.write_bytes( bu, sizeof(bu) );
	}

/*	Set global cmap, write logical screen descriptor and global cmap:
*/
	void writeScreenDescriptor ( uint16 w, uint16 h, Colormap const& cmap, uint8 aspect=0 ) THF
	{
		assert( cmap.usedColors()>0 );
		global_cmap = cmap;
		uint8 bits  = cmap.usedBits() -1;
		uint8 flags = has_global_cmap + (bits<<4) + (bits<<0);
		uint8 bu[] = { LOHI(w), LOHI(h), flags, 0/*bgcolor*/, aspect };
		fd.write_bytes( bu, sizeof(bu) );
		writeColormap(cmap);
	}

/*	FinishImage()  (if not yet done),
	write comment block with creator message,
	write trailer byte and close file
*/
	void closeFile			() THF;


/*	Compression of image pixel stream:
*/
	void startImage			( int bits_per_pixel ) THF;
	void writePixelRow		( cuptr pixel, uint w ) THF;
	void writePixelRect		( cuptr pixel, uint w, uint h, uint dy=0 ) THF;
	void finishImage		() THF;
	void writePixelmap		( int bits_per_pixel, Pixelmap const& pixel ) THF;


/*	Write different kinds of blocks:
*/
	void writeGif87aHeader	() THF		{ fd.write_bytes( "GIF87a", 6 ); }
	void writeGif89aHeader	() THF		{ fd.write_bytes( "GIF89a", 6 ); }
	void writeGifTrailer	() THF		{ fd.write_uchar( 0x3B ); }


/*	Write descriptor for following sub image:
*/
	void writeImageDescriptor ( uint16 x, uint16 y, uint16 w, uint16 h, uint8 flags=0 ) THF
	{
		xlogline("writeImageDescriptor: x=%i, y=%i, w=%i, h=%i",x,y,w,h);
		uint8 bu[] = { 0x2c, LOHI(x), LOHI(y), LOHI(w), LOHI(h), flags };
		fd.write_bytes(bu,sizeof(bu));
	}
	void writeImageDescriptor ( Pixelmap const& pm, uint8 flags=0 )	{ writeImageDescriptor( pm.x1(), pm.y1(), pm.width(), pm.height(), flags ); }
	void writeImageDescriptor ( Pixelmap const* pm, uint8 flags=0 )	{ writeImageDescriptor( *pm, flags ); }
	void writeImageDescriptor ( Pixelmap const* pm, Colormap const* cmap )
	{
		assert( cmap->cmapSize()>=2 );
		writeImageDescriptor( *pm, has_local_cmap + cmap->cmapBits()-1 );
		writeColormap( cmap );
	}


/*	Write colormap
	Colormap size must be 2^n colors (1≤n≤8)
	((not tested. will silently write nothing for cmap_size==0.))
*/
	void writeColormap ( Colormap const& cmap ) THF	{ fd.write_bytes( cmap.getCmap(), cmap.cmapByteSize() ); }
	void writeColormap ( Colormap const* cmap ) THF	{ writeColormap(*cmap); }


/*	Enable looping animations.								GIF89a++
*/
	void writeLoopingAnimationExtension ( uint16 max_loops = 0 ) THF
	{
		uint8 bu[] = { 0x21, 0xff, 11, 'N','E','T','S','C','A','P','E','2','.','0',
					   3/*sub block size*/, 1, LOHI(max_loops), 0 };
		fd.write_bytes( bu, sizeof(bu) );
	}


/*	Write Comment Block										GIF89a++
*/
	void writeCommentBlock ( cstr comment ) THF
	{
		uint n = min(255u, uint(strlen(comment)));
		uint8 bu[] = {0x21,0xfe, uint8(n)}; fd.write_bytes(bu,3);
		fd.write_bytes(comment,n+1);
	}


/*	Write Graphic Control Block:							GIF89++
	Animation delay and transparent color.
*/
	void writeGraphicControlBlock ( uint16 delay/*1/100sec*/, int transp_index = Colormap::unset ) THF
	{
		uint8 flags = keep_new_pixels + (transp_index!=Colormap::unset);
		uint8 bu[] = { 0x21, 0xf9, 4, flags, LOHI(delay), uint8(transp_index), 0 };
		fd.write_bytes( bu, sizeof(bu) );
	}


/*	Write sub image to gif file
	use global_cmap or include a local table
*/
	void writeImage ( Pixelmap* pm, cColormap* cm ) THF;
	void writeImage ( Pixelmap& pm, cColormap& cm ) THF	{ writeImage(&pm,&cm); }
	void writeImage ( Pixelmap& pm )               THF    { writeImage(&pm,&global_cmap); }
};

















