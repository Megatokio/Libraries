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
*/


#define SAFE	3
#define LOG		1

#include <fcntl.h>
#include <string.h>
#include <ctype.h>

#import "../unix/FD.h"
#import "GifReport.h"



static void LogColorTable( FD& fd, int size )
{
	uchar rgb[256*3];
	fd.read_bytes(rgb,size*3);
	Log("  Color Table");
	for( int i=0; i<size; i++ )
	{
		if(i%8==0) Log("\n    ");
		uchar* p = rgb+3*i;
		Log("%02hhX,%02hhX,%02hhX    ",p[0],p[1],p[2]);
	}
	LogNL();
}


static void LogBlock( FD& fd, uptr bu, int n, int flags=0 /* 'X' for pure Hex */ )
{
	for( int i=0;i<n;i++ ) 
	{ 
		if(i%64==0) Log("\n    ");
		uchar c=bu[i]; 
		if( flags=='X' ) Log("%02hhX",c);
		else if( c>=0x20&&c<0x7f ) Log("%c",c);
		else Log(":%02hhX",c);
	}
}

static void LogSubBlocks( FD& fd, int flags=0 /* 'X' for pure Hex */ )
{
	Log("  Sub Blocks:");
	while(1)
	{
		int n = fd.read_uchar(); if(n==0) { LogNL(); return; }
		uchar bu[256];
		fd.read_bytes(bu,n);
		LogBlock(fd,bu,n,flags);
	}
}

static void LogImageBlock(FD& fd)
{
	ushort w,h,x,y; uchar flags;
	x=fd.read_uint16_z();
	y=fd.read_uint16_z();
	w=fd.read_uint16_z();
	h=fd.read_uint16_z();
	flags=fd.read_uchar();
	bool has_local_cmap	= flags&0x80;
	bool is_interlaced	= flags&0x40;
	bool cmap_is_sorted	= flags&0x20;
	int cmap_bits		= (flags&7)+1;
	int cmap_size		= 2<<(flags&7);

	Log("Image Block\n");
	Log("  x=%hu, y=%hu, w=%hu, h=%hu\n", x,y,w,h);
	Log("  flags: $%02hhx\n",flags);
	Log("    Local color table: %s\n",has_local_cmap?"yes":"no");
	Log("    Image interlaced: %s\n",is_interlaced?"yes":"no");

	if(has_local_cmap) 
	{
		Log("    Color table is sorted: %s\n",cmap_is_sorted?"yes":"no");
		Log("    Local color table: %i bit / %i colors\n", cmap_bits, cmap_size );
		LogColorTable(fd,cmap_size); 
	}

	uchar min_bits = fd.read_uchar();
	Log("  LZW initial symbol size: %hhu bits\n",min_bits);

	// dump sub blocks	
	LogSubBlocks(fd,'X');
}

static void LogGraphicControlExtension(FD& fd)
{
	int   block_size = fd.read_uchar();
	uchar flags = fd.read_uchar();
	uint  delay_time = fd.read_uint16_z();
	uchar transp_index = fd.read_uchar();
	uchar block_end = fd.read_uchar();
	int disposal = (flags>>2)&7;	
	cstr disp[8] = 
	{ "0: don't care", "1: keep new pixels", "2: restore bgcolor", "3: restore pixels", "4", "5", "6", "7" };
	bool user_input = flags&2;
	bool transparency = flags&1;
	
	Log("Graphic Control Extension\n");
	if(block_size!=4) Log("  wrong field size: %i\n",block_size);
	if(block_end!=0)  Log("  wrong block delimiter: %i\n",block_end);
	Log("  delay time: %u msec\n", delay_time*10u);
	Log("  flags: $%02hhx\n",flags);		
	Log("    disposal: %s\n",disp[disposal] );
	Log("    wait for user input: %s\n", user_input?"yes":"no");
	Log("    use transparency: %s\n", transparency?"yes":"no");
	Log("  transparent color index: %hhu\n",transp_index);
}

static void LogCommentBlock(FD& fd)
{
	Log("Comment Block\n");
	LogSubBlocks(fd);
}

static void LogPlainTextBlock(FD& fd)
{
	Log("Plain Text Block\n");
	TODO();
}

static void LogApplicationExtension(FD& fd)
{
	Log("Application Extension\n");
	int block_size = fd.read_uchar();
	if(block_size!=11) Log("  wrong field size: %i\n",block_size);
	uchar bu[11];
	fd.read_bytes(bu,11);
	Log("  application:");
	LogBlock(fd,bu,11,0);
	LogNL();
	LogSubBlocks(fd);	
}


void LogGifFile( cstr filepath )
{
	Log( "Gif File Report for \"%s\"\n",filepath );
	
	FD fd(filepath,'r');	// throws

	try
	{
		char header[7]; header[6]=0;
		fd.read_bytes(header,6);
		if(strncmp(header,"GIF",3)!=0) throw file_error(fd,dataerror,"header!=\"GIF\"");
		Log( "Header: %s\n",header );

		ushort w,h; uchar flags, bgcolor, aspectratio;
		w=fd.read_uint16_z();
		h=fd.read_uint16_z();
		flags=fd.read_uchar();
		bgcolor=fd.read_uchar();
		aspectratio=fd.read_uchar();
		bool has_global_cmap	= flags&0x80;
		int  source_color_bits	= ((flags>>4)&0x07)+1;
		int  source_colors		= 2<<((flags>>4)&0x07);
		bool colors_sorted		= flags&0x08;
		int	 global_cmap_bits	= (flags&7)+1;
		int	 global_cmap_colors	= 2<<(flags&7);
		Log("Logical Screen Descriptor\n");
		Log("  screen width: %hu\n",w);
		Log("  screen height: %hu\n",h);
		Log("  flags: $%02hhx\n",flags);		
		Log("    Global color table: %s\n", has_global_cmap?"yes":"no" );
		Log("    Source color richness: %i bit / %i colors\n", source_color_bits, source_colors );
		Log("    Color table is sorted: %s\n", colors_sorted?"yes":"no" );
		Log("    Global color table: %i bit / %i colors\n", global_cmap_bits, global_cmap_colors );
		Log("  background color index: %hhu\n",bgcolor);
		Log("  aspect ratio: %s\n",aspectratio?(usingstr("%i/64",aspectratio+15)):"square");
		
		if( has_global_cmap ) LogColorTable(fd,global_cmap_colors);

		while(1)
		{
			uchar image_separator = fd.read_uchar();
		
			if( image_separator==0x2c )		// image separator
			{
				LogImageBlock(fd);
				continue;
			}
			if( image_separator==0x21 )		// extension
			{
				uchar label = fd.read_uchar();
				if( label==0xf9 )			// graphic control 
				{
					LogGraphicControlExtension(fd);
					continue;					
				}
				if( label==0xfe )			// comment
				{
					LogCommentBlock(fd);
					continue;
				}
				if( label==0x01 )			// plain text
				{
					LogPlainTextBlock(fd);
					continue;
				}
				if( label==0xff )			// application extension
				{
					LogApplicationExtension(fd);
					continue;
				}
				Log("Extension with unknown label: $%02hhx\n", label);
				break;
			}
			if( image_separator==0x3b )		// Trailer
			{
				Log("Trailer\n---END---\n\n");
				break;
			}
			else
			{
				Log("Block with unknown separator: $%02hhx\n", image_separator);
				break;
			}
		}

	}
	catch(any_error e)
	{
		Log( "file error: %s\n", e.what() );
	}

	fd.close_file();
}








