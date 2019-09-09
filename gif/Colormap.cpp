/*	Copyright  (c)	Günter Woigk 2007 - 2019
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

#include "Colormap.h"



int
Colormap::addColor ( cComp q[3] ) throws/*bad alloc*/
{
	int n=used_colors++;
	if(n==cmapSize()) growCmap(n+1);
	memcpy(array+n*3,q,3);
	return n;
}


int
Colormap::condAddColor ( cComp q[3] ) throws/*bad alloc*/
{
	int i = findColor(q);
	return i==not_found ? addColor(q) : i;
}


int
Colormap::condAddColor ( Comp r, Comp g, Comp b ) throws/*bad alloc*/
{
	int i = findColor(r,g,b);
	return i==not_found ? addColor(r,g,b) : i;
}


void
Colormap::shrinkCmap ( int n ) throws/*bad alloc*/
{
	n=valid_count(n);
	Cmap::Shrink(n*3);
	if( used_colors>n ) used_colors = n;
	if( transp>=used_colors ) transp = unset;
}


void
Colormap::shrinkColors ( int n ) throws/*bad alloc*/
{
	int m=valid_count(n);
	Cmap::Shrink(m*3);
	if( used_colors>n ) used_colors=n;
	if( transp>=used_colors ) transp=unset;
}


int
Colormap::findColor ( Comp r, Comp g, Comp b ) const
{
	cComp* p0 = getCmap();
	cComp* p  = p0 + cmapByteSize();

	while( p>p0 )
	{
		p-=3;
		if( p[0]==r && p[1]==g && p[2]==b ) { int i = int(p-p0)/3; if( i!=transp ) return i; }
	}

	return not_found;
}


cstr
Colormap::cmapStr() const
{
	str s = tempstr( usedColors()*12+30+2 );
	ptr p = s;
	cComp* q = getCmap();

	p += sprintf(p,"max/used=%i(%i)/%i(%i), cmap: ", cmapSize(),cmapBits(),usedColors(),usedBits() );

	for( int i=0; i<usedColors(); i++ )
	{
		p += sprintf(p,"%hhu,%hhu,%hhu%s ",q[0],q[1],q[2],i==transp?"*":"");
		q += 3;
	}
	*--p = 0;
	return s;
}























