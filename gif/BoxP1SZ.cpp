/*	Copyright  (c)	Günter Woigk 1997 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "BoxP1SZ.h"


#define x1	p1.x
#define	y1	p1.y
#define	w	sz.dx
#define	h	sz.dy

#define	SETMIN(A,B)	if (B<A) A=B
#define	SETMAX(A,B)	if (B>A) A=B



/* ----	calculate enclosing rectangle of two rectangles ------------------------------
		returns the bounding box of two boxes
		if one of both boxes is empty, the other box is returned
		if both boxes are empty, the first box is returned
*/
Box& Box::operator+= ( cBox& q )
{
	if( q.IsNotEmpty() )
	{
		if( IsNotEmpty() )
		{
			Cord x2=x1+w, y2=y1+h;

			SETMIN( x1, q.x1 );
			SETMIN( y1, q.y1 );
			SETMAX( x2, q.x1+q.w );
			SETMAX( y2, q.y1+q.h );

			w=x2-x1; h=y2-y1;
		}
		else
		{
			*this = q;
		}
	}
	else
	{
		if( IsEmpty() ) w=h=0;
	}

	return *this;
}


/* ----	calculate the intersection of two rectangles ----------------------------------
		P1 is always repositioned to [ max(x1,q.x1), max(y1,,q.y1) ]
			even if the intersection is empty.
		If the intersection is empty, then an empty box with w=h=0 is returned.
*/
Box& Box::operator^= ( cBox& q )
{
	Cord x2=x1+w, y2=y1+h;

	SETMAX( x1, q.x1 );
	SETMAX( y1, q.y1 );
	SETMIN( x2, q.x1+q.w );
	SETMIN( y2, q.y1+q.h );

	w=x2-x1; h=y2-y1;

	if( IsEmpty() ) w=h=0;
	return *this;
}


/* ----	punch rectangle -----------------------------------------------
		box q is subtracted from this box and the bounding box of the result is returned.
		if box q is not high or not wide enough to fully cover the with or height of this box,
		then this box is not modified.
*/
Box& Box::operator-= ( cBox& q )
{
	Cord x2=x1+w, y2=y1+h, qx2=q.x1+q.w, qy2=q.y1+q.h;

	if ( q.y1 <= y1 && qy2 >= y2 )
	{
		if( q.x1 <= x1  ) { SETMAX ( x1,  qx2 ); }
		if( qx2 >= x2 ) { SETMIN ( x2, q.x1 ); }
	}

	if ( q.x1 <= x1 && qx2 >= x2 )
	{
		if( q.y1 <= y1  ) { SETMAX ( y1,  qy2 ); }
		if( qy2 >= y2 ) { SETMIN ( y2, q.y1 ); }
	}

	w=x2-x1; h=y2-y1;

	if( IsEmpty() ) w=h=0;
	return *this;
}










