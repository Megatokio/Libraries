// Copyright (c) 1997 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#define SAFE 3
#define LOG	 1

#import "BoxP1SZ.h"


#define x1 p1.x
#define y1 p1.y
#define w  sz.dx
#define h  sz.dy

#define SETMIN(A, B) \
  if (B < A) A = B
#define SETMAX(A, B) \
  if (B > A) A = B


/* ----	calculate enclosing rectangle of two rectangles ------------------------------
		returns the bounding box of two boxes
		if one of both boxes is empty, the other box is returned
		if both boxes are empty, the first box is returned
*/
Box& Box::operator+=(cBox& q)
{
	if (q.IsNotEmpty())
	{
		if (IsNotEmpty())
		{
			Cord x2 = x1 + w, y2 = y1 + h;

			SETMIN(x1, q.x1);
			SETMIN(y1, q.y1);
			SETMAX(x2, q.x1 + q.w);
			SETMAX(y2, q.y1 + q.h);

			w = x2 - x1;
			h = y2 - y1;
		}
		else { *this = q; }
	}
	else
	{
		if (IsEmpty()) w = h = 0;
	}

	return *this;
}


/* ----	calculate the intersection of two rectangles ----------------------------------
		P1 is always repositioned to [ max(x1,q.x1), max(y1,,q.y1) ]
			even if the intersection is empty.
		If the intersection is empty, then an empty box with w=h=0 is returned.
*/
Box& Box::operator^=(cBox& q)
{
	Cord x2 = x1 + w, y2 = y1 + h;

	SETMAX(x1, q.x1);
	SETMAX(y1, q.y1);
	SETMIN(x2, q.x1 + q.w);
	SETMIN(y2, q.y1 + q.h);

	w = x2 - x1;
	h = y2 - y1;

	if (IsEmpty()) w = h = 0;
	return *this;
}


/* ----	punch rectangle -----------------------------------------------
		box q is subtracted from this box and the bounding box of the result is returned.
		if box q is not high or not wide enough to fully cover the with or height of this box,
		then this box is not modified.
*/
Box& Box::operator-=(cBox& q)
{
	Cord x2 = x1 + w, y2 = y1 + h, qx2 = q.x1 + q.w, qy2 = q.y1 + q.h;

	if (q.y1 <= y1 && qy2 >= y2)
	{
		if (q.x1 <= x1) { SETMAX(x1, qx2); }
		if (qx2 >= x2) { SETMIN(x2, q.x1); }
	}

	if (q.x1 <= x1 && qx2 >= x2)
	{
		if (q.y1 <= y1) { SETMAX(y1, qy2); }
		if (qy2 >= y2) { SETMIN(y2, q.y1); }
	}

	w = x2 - x1;
	h = y2 - y1;

	if (IsEmpty()) w = h = 0;
	return *this;
}
