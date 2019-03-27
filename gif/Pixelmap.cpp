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

#include "Pixelmap.h"
#include "Colormap.h"




/*	Copy Creator:
*/
void Pixelmap::init( cPixelmap& q ) throws/*bad alloc*/
{
	data = pixels = new uchar[q.width()*q.height()];
	box  = q.box;
	dy	 = width();

	for( int y=box.Y1(); y<box.Y2(); y++ )
	{
		memcpy( getPixelRow(y), q.getPixelRow(y), dy/*w*/ );
	}
}



/*	Find highest color index in image:
	returns max_index = 0 for empty Pixmaps
*/
int Pixelmap::getMaxColorIndex() const
{
	uchar max = 0;					// maximum collector
	cuptr p = getPixels();				// running pointer

	for( int j=height(); --j>=0; p+=dy )
	for( int i=width(); --i>=0; )
	{
		if( p[i]>max ) max=p[i];
	}

	return max;
}



/*	Count used colors (color indexes) in image:
	if argument max_color_index != NULL, then also store the max. color index found in the image
	returns 1 used color and sets max_index = 0 for empty Pixmaps
*/
int Pixelmap::countUsedColors( int* max_color_index ) const
{
	uchar bits[256]; memset(bits,0,256);		// flags for used colors
	cuptr p = getPixels();							// running pointer

	for( int j=height(); --j>=0; p+=dy )
	for( int i=width(); --i>=0; )
	{
		bits[p[i]] = 1;							// flag all used colors
	}

	int i = 256;
	while( i && bits[--i]==0 ) {}
	if( max_color_index ) *max_color_index = i; // store max. color index

	int n = 1;
	while( i ) { n += bits[--i]; }				// count used colors
	return n;
}




/*  Renumber colors in image to reduce color table size:
	overwrites this.data[] and cmap[] with new matching indexes/colors.
	clears transp_color if no transparent pixels are found.
*/
void Pixelmap::reduceColors( Colormap& cmap )
{
	xlogIn("Pixelmap::reduceColors(Colormap)");

	//xlogline( "old colors: %i", int(cmap.usedColors()) );
	//if( XXLOG && cmap.hasTranspColor() ) logline( "old transp: %i",int(cmap.transpColor()) );
	//xlogline( "cmap: %s",cmap.cmapStr() );

	Colormap my_cmap;				// new cmap
	uchar my_conv[256]; memset(my_conv,0xff,256);	// old color index -> new color index

	uptr p  = getPixels();		// running pointer
	int  w  = width();
	int  h  = height();
	int  dy = rowOffset(); assert(dy>=w);
	int  n  = 0;					// new cmap size

	for( int y=0; y<h; y++, p+=dy )
	for( int x=0; x<w; x++ )
	{
		int c = p[x];				// c = old color index
		int k = my_conv[c];			// k = new color index
		if( k>=n )
		{
			assert(c<cmap.usedColors());
			my_conv[c] = k = n++;
			my_cmap.addColor(cmap[c]);

			//xlogline( "  pixel[x=%i,y=%i] = %i -> %i, rgb=(%hhu,%hhu,%hhu)",
			//		x1()+x, y1()+y, c, k, cmap[c][0], cmap[c][1], cmap[c][2] );
		}
		p[x] = k;
	}

	assert( n <= cmap.usedColors() );
	assert( n == my_cmap.usedColors() );

	if( cmap.hasTranspColor() )
	{
		int my_transp = my_conv[cmap.transpColor()];
		if( my_transp<n )
		{
			my_cmap.setTranspColor( my_transp );
			xlogline( "new transp: %i", int(my_cmap.transpColor()) );
		}
		else
		{
			xlogline( "no transparent pixels!" );
		}
	}

	cmap = my_cmap;
//	xlogline( "new colors: %i", int(cmap.usedColors()) );
//	xlogline( "cmap: %s",cmap.cmapStr() );
}






/*	Set this Pixmap to the difference which converts this old Pixmap into the new Pixmap.
	-	the Pixmap size is shrinked to the bounding box of all changed pixels
	-	unchanged pixels are set to the transparent color, except if transp_color<0 ((use Colormap::unset))
	intended for gif animation
*/
void Pixelmap::setToDiff ( cPixelmap& neu, int transp_color )
{
	box ^= neu.box;

	int h  = height();
	int w  = width();
	if( w==0 || h==0 ) return;
	int x1 = this->x1();
	int y1 = this->y1();

	uptr  zp = getPixelPtr( x1,y1 );		// -> top left pixel in box
	cuptr np = neu.getPixelPtr( x1,y1 );

	while( h>0 )	// skip identical top rows:
	{
		int x; for( x=0; x<w; x++ ) { if( zp[x]!=np[x] ) break; }
		if( x<w ) break; else { h--; y1++; zp+=dy; np+=neu.dy; }
	}

	zp += (h-1)*dy;							// -> bottom left pixel in box
	np += (h-1)*neu.dy;

	while( h>0 )	// skip identical bottom rows:
	{
		int x; for( x=0; x<w; x++ ) { if( zp[x]!=np[x] ) break; }
		if( x<w ) break; else { h--; zp-=dy; np-=neu.dy; }
	}

	zp -= (h-1)*dy;							// -> top left pixel in box
	np -= (h-1)*neu.dy;

	while( w>0 )	// skip identical left columns:
	{
		int y; for( y=0; y<h; y++ ) { if( zp[y*dy]!=np[y*neu.dy] ) break; }
		if( y<h ) break; else { w--; x1++; np++; zp++; }
	}

	zp += w-1;
	np += w-1;

	while( w>0 )	// skip identical right columns:
	{
		int y; for( y=0; y<h; y++ ) { if( zp[y*dy]!=np[y*neu.dy] ) break; }
		if( y<h ) break; else { w--; zp--; np--; }
	}

	zp -= w-1;
	np -= w-1;

	setP1(x1,y1);
	setSize(w,h);

// my bounding box x,y,w,h is the bounding box for all changes.
// replace unchanged pixels with transparent color:

	if( transp_color>=0 )
		for( int y=0; y<h; y++, zp+=dy, np+=neu.dy )
			for( int x=0; x<w; x++ )
				zp[x] = zp[x]==np[x] ? transp_color : np[x];
	else
		for( int y=0; y<h; y++, zp+=dy, np+=neu.dy )
			for( int x=0; x<w; x++ )
				zp[x] = np[x];
}


/*	Reduce this Pixmap to the difference which converts the old Pixmap into this Pixmap.
	-	the Pixmap size is shrinked to the bounding box of all changed pixels
	-	unchanged pixels are set to the transparent color, except if transp_color<0 ((use Colormap::unset))
	intended for gif animation
*/
void Pixelmap::reduceToDiff ( cPixelmap& old, int transp_color )
{
	box ^= old.box;

	int h  = height();
	int w  = width();
	if( w==0 || h==0 ) return;
	int x1 = this->x1();
	int y1 = this->y1();

	uptr  zp = getPixelPtr( x1,y1 );		// -> top left pixel in box
	cuptr np = old.getPixelPtr( x1,y1 );

	while( h>0 )	// skip identical top rows:
	{
		int x; for( x=0; x<w; x++ ) { if( zp[x]!=np[x] ) break; }
		if( x<w ) break; else { h--; y1++; zp+=dy; np+=old.dy; }
	}

	zp += (h-1)*dy;							// -> bottom left pixel in box
	np += (h-1)*old.dy;

	while( h>0 )	// skip identical bottom rows:
	{
		int x; for( x=0; x<w; x++ ) { if( zp[x]!=np[x] ) break; }
		if( x<w ) break; else { h--; zp-=dy; np-=old.dy; }
	}

	zp -= (h-1)*dy;							// -> top left pixel in box
	np -= (h-1)*old.dy;

	while( w>0 )	// skip identical left columns:
	{
		int y; for( y=0; y<h; y++ ) { if( zp[y*dy]!=np[y*old.dy] ) break; }
		if( y<h ) break; else { w--; x1++; np++; zp++; }
	}

	zp += w-1;
	np += w-1;

	while( w>0 )	// skip identical right columns:
	{
		int y; for( y=0; y<h; y++ ) { if( zp[y*dy]!=np[y*old.dy] ) break; }
		if( y<h ) break; else { w--; zp--; np--; }
	}

	zp -= w-1;
	np -= w-1;

	setP1(x1,y1);
	setSize(w,h);

// my bounding box x,y,w,h is the bounding box for all changes.
// replace unchanged pixels with transparent color:

	if( transp_color>=0 )
		for( int y=0; y<h; y++, zp+=dy, np+=old.dy )
			for( int x=0; x<w; x++ )
				if(zp[x]==np[x]) zp[x] = transp_color;
}



void Pixelmap::clear ( int color )
{
	uptr p = getPixels();		// p-> first == top/left pixel
	int  w = width();
	int  h = height();
	uptr e = p + h * dy;

	while( p<e ) { memset(p,color,w); p+=dy; }
}























