#pragma once
// Copyright (c) 2007 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "graphics/geometry.h"
#include "Colormap.h"


class Pixelmap;
class Colormap;
typedef Pixelmap const cPixelmap;

class Pixelmap
{
	using Box = geometry::Rect<int>;
	using cBox = const Box;
	using Loc = geometry::Point<int>;
	using cLoc = const Loc;
	using Dist = geometry::Dist<int>;
	using cDist = const Dist;
	using Size = geometry::Size<int>;
	using cSize = const Size;

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
	int			x1				() const						{ return box.left(); }
	int			y1				() const 						{ return box.top(); }
	int			x2				() const						{ return box.right(); }
	int			y2				() const 						{ return box.bottom(); }
	int			width			() const						{ return box.width(); }
	int			height			() const						{ return box.height(); }
	int			rowOffset		() const						{ return dy; }          // was: DY()
	cLoc&		p1				() const						{ return box.topLeft(); }
	Dist		getSize			() const						{ return box.size(); }

	bool		isEmpty			() const						{ return box.isEmpty(); }
	bool		isNotEmpty		() const						{ return !box.isEmpty(); }

// Reposition frame:
// This affects which pixels are enclosed.
// Pixel coordinates are not affected!
	void		setX			( int x )						{ box.setLeft(x); }
	void		setY			( int y )						{ box.setTop(y); }
	void		setP1			( cLoc& p )						{ box.setTopLeft(p); }
	void		setP1			( int x, int y )				{ box.setTopLeft(Loc(x,y)); }
	void		setP2			( cLoc& p )						{ box.setBottomRight(p); }
	void		setP2			( int x, int y )				{ box.setBottomRight(Loc(x,y)); }
	void		setW			( int w )						{ box.setWidth(w); }
	void		setH			( int h )						{ box.setHeight(h); }
	void		setSize			( cDist& d )					{ box.setSize(d); }
	void		setSize			( int w, int h )				{ box.setSize(Size(w,h)); }
	void		setFrame		( cBox& b )						{ box = b; }
	void		setFrame		( int x, int y, int w, int h )	{ box = Box(x,y,w,h); }
	void		setFrame		( cLoc& p1, cLoc& p2 )			{ box = Box(p1,p2); }

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
	void		offsetAddresses	( int dx, int dy )			{ offsetAddresses(Dist(dx,dy)); }
	void		offsetAddresses	( cDist& d )				{ box.move(d); pixels -= d.dx+d.dy*dy; }
	void		setAddressForP1	( cLoc& p )					{ offsetAddresses(p-box.p1); }

// Color tools:
	int			getMaxColorIndex() const;
	int			countUsedColors	( int* max_color_index=nullptr ) const;
	void		reduceColors	( Colormap& cmap );

// Special resizing:
	void		setToDiff		( cPixelmap& new_pixmap, int transp_index = Colormap::unset ) __attribute__((deprecated));
	void		reduceToDiff	( cPixelmap& old_pixmap, int transp_index = Colormap::unset );
	void		shrinkToRect	( cBox& new_box )			{ box.intersectWith(new_box); }
	void		cropBackground	( int bgcolor );

	void		clear			( int color );
};














