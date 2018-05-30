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
*/


#define	SAFE	3
#define	LOG		1

#import	"BoxP1P2.h"


#define	SETMIN(A,B)	if (B<A) A=B
#define	SETMAX(A,B)	if (B>A) A=B

#define x1	p1.x
#define	x2	p2.x
#define	y1	p1.y
#define	y2	p2.y
#define	w	(x2-x1)
#define	h	(y2-y1)



/* ----	calculate enclosing rectangle of two rectangles ------------------------------
		returns the bounding box of two boxes
		if one of both boxes is empty, the other box is returned
		if both boxes are empty, the first box is returned
*/
Box& Box::operator+= ( cBox& q )
{
	if( q.IsNotEmpty() )
	{	
		if( this->IsNotEmpty() )
		{
			SETMIN( x1, q.x1 );
			SETMIN( y1, q.y1 );
			SETMAX( x2, q.x2 );
			SETMAX( y2, q.y2 );
		}
		else
		{
			*this = q;
		}
	}
	else
	{
		if( this->IsEmpty() ) p2 = p1;
	}

	return *this;
}


/* ----	calculate the intersection of two rectangles ----------------------------------
		P1 is always repositioned to [ max(x1,q.x1), max(y1,,q.y1) ] 
			even if the intersection is empty.
		If the intersection is empty, then an empty box with W=H=0 is returned.
*/
Box& Box::operator^= ( cBox& q ) 
{
	SETMAX( x1, q.x1 );
	SETMAX( y1, q.y1 );
	SETMIN( x2, q.x2 );
	SETMIN( y2, q.y2 );
	
	if( this->IsEmpty() ) p2 = p1;
	return *this;
}


/* ----	punch rectangle -----------------------------------------------
		box q is subtracted from this box and the bounding box of the result is returned.
		if box q is not high or not wide enough to fully cover the with or height of this box, 
		then this box is not modified.
*/
Box& Box::operator-= ( cBox& q ) 
{
	if ( q.y1<=y1 && q.y2>=y2 )
	{
		if( q.x1 <= x1 ) { SETMAX ( x1, q.x2 ); }
		if( q.x2 >= x2 ) { SETMIN ( x2, q.x1 ); }
	}
	
	if ( q.x1<=x1 && q.x2>=x2 )
	{
		if( q.y1 <= y1 ) { SETMAX ( y1, q.y2 ); }
		if( q.y2 >= y2 ) { SETMIN ( y2, q.y1 ); }
	}
	
	if( this->IsEmpty() ) p2 = p1;
	return *this;
}



