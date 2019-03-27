/*	Copyright  (c)	Günter Woigk 2002 - 2019
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
*/

#include	"Unicode.h"

INIT_MSG


// 8-bit charset conversion tables
cUCS2Table ucs2_from_mac_roman =
#include "Headers/mac_roman.h"

cUCS2Table ucs2_from_ascii_ger =
#include "Headers/ascii_ger.h"

cUCS2Table ucs2_from_rtos =
#include "Headers/rtos.h"

cUCS2Table ucs2_from_cp_437 =
#include "Headers/cp_437.h"

cUCS2Table ucs2_from_atari_st =
#include "Headers/atari_st.h"





/* ****************************************************************
					CONVERSION 8-bit CHAR <-> UCS4
**************************************************************** */


/* ----	convert UCS4 char -> 8-bit char ------------------------------------------
*/
char UCS4CharTo8Bit ( UCS4Char n, cUCS2Table t )
{
	if( !(n>>8) && n==t[n] ) return n;

	if( !(n>>16) )
	{
		for ( uint i=0; i<256; i++ ) { if (t[i]==UCS2Char(n)) return i; }
	}

	errno = notindestcharset;
	return UCS1ReplacementChar;
}















