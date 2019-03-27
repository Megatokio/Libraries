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
*/

#include "../kio/kio.h"
#include "BoxP1SZ.h"
#include "Colormap.h"


class Pixelmap;
class Colormap;
typedef Pixelmap const cPixelmap;

class Pixelmap
{
protected:

	void		kill()				{ delete[] data; data=nullptr; }
	void		init(cPixelmap& q)	noexcept(false); // bad_alloc

	Box			box;			// frame: x,y,w,h
	int			dy;				// address offset per row
	uptr		data;			// allocated memory for delete[]
	uptr		pixels;			// -> pixel[0,0]


public:

	Pixelmap	()							: box(),		dy(0),   data(nullptr),               pixels(nullptr)              {}
	Pixelmap	( cBox& bbox )				: box(bbox),    dy(width()), data(new uchar[width()*height()]), pixels(data-x1()-y1()*dy) {}
	Pixelmap	( int w,int h )				: box(w,h),     dy(w),   data(new uchar[w*h]),     pixels(data)              {}
	Pixelmap	( int x,int y,int w,int h )	: box(x,y,w,h), dy(w),   data(new uchar[w*h]),     pixels(data-x-y*w)        {}

	Pixelmap	( uptr px, int x,int y, int w,int h, int z=0 )	: box(x,y,w,h), dy(z?z:w), data(px), pixels(px-x-y*dy) {}
	Pixelmap	( uptr px,              int w,int h, int z=0 )	: box(w,h),     dy(z?z:w), data(px), pixels(px) {}

	Pixelmap	( uptr px, cBox& bx,			int z=0 )		: box(bx),   dy(z?z:width()),data(px),pixels(px-x1()-y1()*dy) {}
	Pixelmap	( uptr px, cLoc& p1, cLoc& p2,  int z=0 )		: box(p1,p2),dy(z?z:width()),data(px),pixels(px-p1.x-p1.y*dy) {}
	Pixelmap	( uptr px, cLoc& p1, cDist& sz, int z=0 )		: box(p1,sz),dy(z?z:width()),data(px),pixels(px-p1.x-p1.y*dy) {}
	Pixelmap	( uptr px,           cDist& sz, int z=0 )		: box(sz),   dy(z?z:width()),data(px),pixels(px) {}

				~Pixelmap		()								{ kill(); }
				Pixelmap		( cPixelmap& q )				{ init(q); }
	Pixelmap&	operator=		( cPixelmap& q )				{ if(this!=&q) { kill(); init(q); } return *this; }

	cBox&		getBox			() const						{ return box; }         // was: Frame()
	int			x1				() const						{ return box.X1(); }
	int			y1				() const 						{ return box.Y1(); }
	int			x2				() const						{ return box.X2(); }
	int			y2				() const 						{ return box.Y2(); }
	int			width			() const						{ return box.Width(); }
	int			height			() const						{ return box.Height(); }
	int			rowOffset		() const						{ return dy; }          // was: DY()
	cLoc&		p1				() const						{ return box.P1(); }
	Dist		getSize			() const						{ return box.Size(); }

	bool		isEmpty			() const						{ return box.IsEmpty(); }
	bool		isNotEmpty		() const						{ return box.IsNotEmpty(); }

// Reposition frame:
// This affects which pixels are enclosed.
// Pixel coordinates are not affected!
	void		setX			( int x )						{ box.SetX1(x); }
	void		setY			( int y )						{ box.SetY1(y); }
	void		setP1			( cLoc& p )						{ box.SetP1(p); }
	void		setP1			( int x, int y )				{ box.SetP1(x,y); }
	void		setP2			( cLoc& p )						{ box.SetP2(p); }
	void		setP2			( int x, int y )				{ box.SetP2(x,y); }
	void		setW			( int w )						{ box.SetWidth(w); }
	void		setH			( int h )						{ box.SetHeight(h); }
	void		setSize			( cDist& d )					{ box.SetSize(d); }
	void		setSize			( int w, int h )				{ box.SetSize(w,h); }
	void		setFrame		( cBox& b )						{ box = b; }
	void		setFrame		( int x, int y, int w, int h )	{ box.SetP1(x,y); box.SetSize(w,h); }
	void		setFrame		( cLoc& p1, cLoc& p2 )			{ box.p1=p1; box.sz=p2-p1; }

	void		set_data_ptr	( uptr p )						{ data = p; }		// use with care...
	void		set_pixel_ptr	( uptr p )						{ pixels = p; }		// caveat: check data, p1 and p2!
	void		set_dy			( int z )						{ dy = z; }			// caveat: check pixels, p1 and p2!

// ptr -> pixel[x1,y1]:
	uptr		getPixels		()								{ return getPixelPtr(p1()); }
	cuptr		getPixels		() const						{ return getPixelPtr(p1()); }

// ptr -> first pixel of row:
	uptr		getPixelRow		( int y )						{ return getPixelPtr(x1(),y); }
	cuptr		getPixelRow		( int y ) const					{ return getPixelPtr(x1(),y); }

// ptr -> pixel:
	uptr		getPixelPtr		( int x, int y )				{ return pixels +x   +   y*dy;   }
	cuptr		getPixelPtr		( int x, int y ) const			{ return pixels +x   +   y*dy;   }

	uptr		getPixelPtr		( cLoc& p )						{ return pixels +p.x + p.y*dy;   }
	cuptr		getPixelPtr		( cLoc& p ) const				{ return pixels +p.x + p.y*dy;   }

	uchar&		getPixel		( int x, int y )				{ return pixels[ x   +   y*dy ]; }
	uchar		getPixel		( int x, int y ) const			{ return pixels[ x   +   y*dy ]; }

	uptr		getData			()								{ return data; }
	cuptr		getData			()	const						{ return data; }


// Member Functions:

// Change the pixel coordinate system.
// Thereafter all pixel coordinates, including the frame rect coordinates are shifted!
// This does not reposition the frame rect: it does not affect which pixels are enclosed.
	void		offsetAddresses	( int DX, int DY )			{ box.Move(DX,DY); pixels -= DX+DY*dy; }
	void		offsetAddresses	( cDist& d )				{ box.Move(d);     pixels -= d.dx+d.dy*dy; }
	void		setAddressForP1	( cLoc& p )					{ offsetAddresses(p-box.p1); }

// Color tools:
	int			getMaxColorIndex() const;
	int			countUsedColors	( int* max_color_index=nullptr ) const;
	void		reduceColors	( Colormap& cmap );

// Special resizing:
	void		setToDiff		( cPixelmap& new_pixmap, int transp_index = Colormap::unset ) __attribute__((deprecated));
	void		reduceToDiff	( cPixelmap& old_pixmap, int transp_index = Colormap::unset );
	void		shrinkToRect	( cBox& new_box )			{ box ^= new_box; }
	void		cropBackground	( int bgcolor );

	void		clear			( int color );
};















