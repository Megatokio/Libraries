/*	Copyright  (c)	Günter Woigk   2004 - 2019
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
void		LockNameHandle		( NameHandle );
void		UnlockNameHandle	( NameHandle );


//	Ermittle den Namen zu einem Handle.
cString&	GetNameForHandle	( NameHandle );


//	Free up unused space
void NameHandlesPurgeCaches ( );

void NameHandleCheck();

#endif











