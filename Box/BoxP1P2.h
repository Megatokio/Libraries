#pragma once
/*	Copyright  (c)	Günter Woigk 1997 - 2007
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


	Data type definitions for 2-dimensional geometry

							checked & reworked    2007-06-14 kio !

	Cord	coordinate	= data size of coordinates
	Loc		location 	= coordinates of points
	Dist	distance 	= distance (vector) between points
	Box 	rectangle	= rectangular area between two points
*/


typedef int	   Cord;	typedef Cord const cCord;
typedef struct Loc;		typedef Loc const  cLoc;
typedef struct Dist;	typedef Dist const cDist;
typedef struct Box;		typedef Box const  cBox;






// ====	offsets, sizes, distances: struct Dist ================================

struct Dist
{
	Cord	dx,dy;

	Dist	( Cord DX=0, Cord DY=0 )	: dx(DX), dy(DY) {}

	Dist&	operator+=	( cDist& d )	{ dx+=d.dx; dy+=d.dy; return *this; }		// Dist + Dist = Dist
	Dist&	operator-=	( cDist& d )	{ dx-=d.dx; dy-=d.dy; return *this; }		// Dist - Dist = Dist
	Dist&	operator/=	( int n )		{ dx/=n;    dy/=n;	  return *this; }
	Dist&	operator*=	( int n )		{ dx*=n;    dy*=n;	  return *this; }

	bool	operator==	( cDist& q ) const	{ return dx==q.dx && dy==q.dy; }
	bool	operator!=	( cDist& q ) const	{ return dx!=q.dx || dy!=q.dy; }
};



// ====	coordinates: struct Loc =========================================

struct Loc
{
	Cord	x,y;

	Loc		( Cord X=0, Cord Y=0 )	: x(X), y(Y) {}

	Loc&	operator+=	( cDist& d )		{ x+=d.dx; y+=d.dy; return *this; }
	Loc&	operator-=	( cDist& d )		{ x-=d.dx; y-=d.dy; return *this; }

	bool	operator==	( cLoc& q ) const	{ return x==q.x && y==q.y; }
	bool	operator!=	( cLoc& q ) const	{ return x!=q.x || y!=q.y; }
};



// ----	inline functions for 'Loc' and 'Dist' -------------------------

inline Loc&		ToLoc	( Dist& d )				{ return *(Loc*)&d; }			// explicit cast
inline cLoc&	ToLoc	( cDist& d )			{ return *(cLoc*)&d; }			// explicit cast
inline Dist&	ToDist	( Loc& p )				{ return *(Dist*)&p; }			// explicit cast
inline cDist&	ToDist	( cLoc& p )				{ return *(cDist*)&p; }			// explicit cast

inline double	Ascent	( cDist& d )			{ return double(d.dx)    / double(d.dy);    }
inline double	Ascent	( cLoc& a, cLoc& e )	{ return double(e.x-a.x) / double(e.y-a.y); }

inline Dist		operator+ ( Dist d, cDist& q )	{ return d+=q; }				// Dist + Dist = Dist
inline Dist		operator- ( Dist d, cDist& q )	{ return d-=q; }				// Dist - Dist = Dist
inline Dist		operator/ ( Dist d, int n )		{ return d/=n; }
inline Dist		operator* ( Dist d, int n )		{ return d*=n; }
inline Dist		operator- ( cDist& d )			{ return Dist(-d.dx,-d.dy); }	// unary operator -
inline Dist		operator- ( cLoc& p, cLoc& q )	{ return ToDist(p)-ToDist(q); }	// Loc - Loc = Dist

inline Loc		operator+ ( Loc p, cDist& d )	{ return p+=d; }				// Loc + Dist = Loc
inline Loc		operator- ( Loc p, cDist& d )	{ return p-=d; }				// Loc - Dist = Loc



// ====	rectangles  [p1 ... [p2 ======================================

/*	Empty Box  <=>  width <= 0  ||  height <= 0

	most class Box member functions set  width = height = 0  if returning an empty box.
	but this is not strictly enforced.
*/

#define x1	p1.x
#define	x2	p2.x
#define	y1	p1.y
#define	y2	p2.y
#define	w	(x2-x1)
#define	h	(y2-y1)

struct Box
{
	Loc		p1;			// top left corner
	Loc		p2;			// bottom right corner

// accessing the members
	cLoc&	P1			( )		const		{ return p1; }
	cLoc&	P2			( )		const		{ return p2; }
	Cord	X1			( )		const		{ return x1; }
	Cord	Y1			( )		const		{ return y1; }
	Cord	X2			( )		const		{ return x2; }
	Cord	Y2			( )		const		{ return y2; }

	Cord 	Width		( ) 	const		{ return w; }
	Cord	Height		( ) 	const		{ return h; }
	Dist	Size		( ) 	const 		{ return p2 - p1; }
	Cord	HCenter		( )		const		{ return (x1+x2)/2; }
	Cord	VCenter		( )		const		{ return (y1+y2)/2; }
	Loc		Center		( )		const		{ return Loc(HCenter(),VCenter()); }

	void	SetWidth	( Cord W )			{ x2 = x1 + W; }
	void	SetHeight	( Cord H )			{ y2 = y1 + H; }
	void	SetSize		( cDist& SZ )		{ p2 = p1 + SZ; }
	void	SetSize		( Cord W, Cord H )	{ x2 = x1 + W; y2 = y1 + H; }

// original creators
	Box		( )									: p1(0,0), p2(0,0) {}
	Box		( cLoc& P1, cLoc& P2 )				: p1(P1),  p2(P2) {}
	Box		( cLoc& P1, cDist& SZ )				: p1(P1),  p2(P1+d) {}
	Box		( cDist& SZ )						: p1(0,0), p2(d.dx,d.dy) {}
	Box		( Cord X, Cord Y, Cord W, Cord H )	: p1(X,Y), p2(X+W,Y+H) {}	// caveat: not Box(x1,y1,x2,y2)
	Box		( Cord W, Cord H )					: p1(0,0), p2(W,H) {}

// compare:
bool	 	operator==	( cBox& q ) const		{ return (p1==q.p1) && (p2==q.p2); }
bool		operator!=	( cBox& q ) const		{ return (p1!=q.p1) || (p2!=q.p2); }

// geometrical operators:
Box&		operator+=	( cBox& );				// Enclosing rectangle
Box&		operator-= 	( cBox& );				// Punch
Box& 		operator^= 	( cBox& );				// Intersection

// grow / shrink box at all 4 corners
Box&		Grow		( Cord W )				{ x1-=W; x2+=W; y1-=W; y2+=W; return *this; }
Box&		Grow		( Cord W, Cord H )		{ x1-=W; x2+=W; y1-=H; y2+=H; return *this; }
Box&		Grow		( cDist& D )			{ return Grow(D.dx,D.dy); }

// grow / shrink box by moving bottom/right corner
Box&		Resize		( cDist& SZ )			{ p2 = p1+SZ; return *this; }
Box& 		Resize		( Cord W, Cord H )		{ x2=x1+W; y2=y1+H; return *this; }

// move box
Box&		Move 		( Cord DX, Cord DY )	{ x1+=DX; x2+=DX; y1+=DY; y2+=DY; return *this; }
Box&		Move 		( cDist& D )			{ p1+=D; p2+=D; return *this; }
Box&		operator+=	( cDist& D )			{ return Move(D); }
Box&		operator-=	( cDist& D )			{ return Move(-D.dx,-D.dy); }

// test for enclosure and intersection:
bool		Encloses	( Cord x, Cord y ) const{ return x>=x1 && x<x2 && y>=y1 && y<y2; }	// location
bool		Encloses	( cLoc& p )	const		{ return Encloses ( p.x, p.y ); }					// location
bool		Encloses	( cBox& i )	const		{ return i.x1>=x1&&i.y1>=y1&&i.x2<=x2&&i.y2<=y2; }	// entire box
bool		Intersect 	( cBox& q ) const		{ return x1<q.x2&&q.x1<x2&&y1<q.y2&&q.y1<y2; }

bool		IsEmpty		( ) const				{ return x1>=x2 || y1>=y2; }
bool		IsNotEmpty	( ) const				{ return x1<x2  && y1<y2;  }
};


inline Box operator+ ( Box q, cDist& d )	{ return q+=d; }			// shift box
inline Box operator- ( Box q, cDist& d )	{ return q-=d; }			// shift box
inline Box operator+ ( Box q, cBox& b2 )	{ return q+=b2; }			// enclosing rectangle
inline Box operator- ( Box q, cBox& b2 )	{ return q-=b2; }			// punch
inline Box operator^ ( Box q, cBox& b2 )	{ return q^=b2; }			// intersection


/*	calculate box on alert position inside other box
*/
Box AlertBox ( cBox& bbox, cDist& size )
{
	return Box ( Loc( bbox.x1+(bbox.Width()-size.dx)/2, bbox.y1+20+(bbox.Height()-size.dy-20)/3 ), size );
}


#undef	x1
#undef	x2
#undef	y1
#undef	y2
#undef	w
#undef 	h












