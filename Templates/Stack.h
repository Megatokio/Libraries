#pragma once
/*	Copyright  (c)	Günter Woigk 2007 - 2013
					mailto:kio@little-bat.de

	This file is free software

 	This program is distributed in the hope that it will be useful,
 	but WITHOUT ANY WARRANTY; without even the implied warranty of
 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	• Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	• Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
	OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
	OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
	ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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



