#pragma once
/*	Copyright  (c)	Günter Woigk 2007 - 2013
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


	used in vipsi.Compiler.cpp
*/

template<class T>
class Stack
{
	uint	size;
	uint	used;
	T*		liste;

	void	Resize		( );
public:
			Stack		( )					{ size=0; used=0; liste=NULL; }
			~Stack		( )					{ delete[] liste; }
	void	Push		( const T & item )	{ if(used==size) Resize(); liste[used++] = item; }
	T &		Pop			( )					{ return liste[--used]; }
	T &		Top			( )					{ return liste[used-1]; }
	T &		operator[]	( int i )			{ return liste[i]; }
	uint	Count		( )					{ return used;    }
	bool	IsEmpty		( )					{ return used==0; }
	bool	IsNotEmpty	( )					{ return used!=0; }
	T *		get_start	( )					{ return liste; }
	T *		get_end		( )					{ return liste + used; }
};



template<class T>
void Stack<T>::Resize ( )
{
	T* alt = liste; liste = new T[(size=size*3/2+10)];
	for(uint i=0;i<used;i++) liste[i] = alt[i];
	delete[] alt;
}



