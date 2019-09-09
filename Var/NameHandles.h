/*	Copyright  (c)	Günter Woigk   2004 - 2019
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


	----------------------------------------------------------------------------

			Name Handles

	----------------------------------------------------------------------------


	Dieses Modul speichert beliebige Texte bzw. Namen und gibt dafür ein Handle zurück.
	Die Handles für gleiche Texte sind gleich.
	Zwei Texte können auf Gleichheit getestet werden, indem man ihre Handles vergleicht.

	Gedachtes Einsatzgebiet: Speicherung von Variablennamen.

	Vorteil:
	+ Beschleunigt Variablen-Suche, weil nur die Handles verglichen werden müssen.

	Die Handles sind immer 32 Bit signifikant.

	Das Handle für leere Strings ist 0.
		Handle 0 wird nie wirklich verriegelt oder freigegeben.
		Für leere Namen muss also Request/Release nicht aufgerufen werden - kann aber.
*/


#ifndef NameHandles_h
#define	NameHandles_h

#include	"VString/String.h"


typedef uint32 NameHandle;



//	Speichere einen Namen und erhalte ein Handle.
//	Ist der Name schon bekannt, bekommt man das alte Handle.
//	Das Handle wird einmal mit Lock() verriegelt.
NameHandle	NewNameHandle		( cString& name );
NameHandle	NewNameHandle		( cstr name );			// <-- required wg. bug in gcc 3.3.3


//	Suche in der NameHandle Datenbank nach einem Namen.
//	Liefert Handle oder 0.
//	Achtung: 0 ist auch ein gültiges Handle, und zwar für leere Namen!
//	Das gefundene Handle wird nicht verriegelt.
NameHandle	FindNameHandle		( cString& name );


//	Das Handle einmal verriegeln bzw. entriegeln.
//	Sobald die Aufrufe für Lock() und Unlock() sich entsprechen, wird das Handle ungültig.
void 		LockNameHandle		( NameHandle );
void		UnlockNameHandle	( NameHandle );


//	Ermittle den Namen zu einem Handle.
cString&	GetNameForHandle	( NameHandle );


//	Free up unused space
void NameHandlesPurgeCaches ( );

void NameHandleCheck();

#endif











