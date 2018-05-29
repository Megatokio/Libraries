/*	Copyright  (c)	Günter Woigk 2002 - 2005
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


#define SAFE	3
#define	LOG		1

#include "Unicode.h"
#include "TESTING/compress/freeze.h"

INIT_MSG//("<init-free>");



/*	Unvollständig enumerierte Blocks in UnicodeData.txt:

3400;<CJK Ideograph Extension A, First>;Lo;0;L;;;;;N;;;;;
4DB5;<CJK Ideograph Extension A, Last>;Lo;0;L;;;;;N;;;;;
4E00;<CJK Ideograph, First>;Lo;0;L;;;;;N;;;;;
9FBB;<CJK Ideograph, Last>;Lo;0;L;;;;;N;;;;;
AC00;<Hangul Syllable, First>;Lo;0;L;;;;;N;;;;;
D7A3;<Hangul Syllable, Last>;Lo;0;L;;;;;N;;;;;
D800;<Non Private Use High Surrogate, First>;Cs;0;L;;;;;N;;;;;
DB7F;<Non Private Use High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
DB80;<Private Use High Surrogate, First>;Cs;0;L;;;;;N;;;;;
DBFF;<Private Use High Surrogate, Last>;Cs;0;L;;;;;N;;;;;
DC00;<Low Surrogate, First>;Cs;0;L;;;;;N;;;;;
DFFF;<Low Surrogate, Last>;Cs;0;L;;;;;N;;;;;
E000;<Private Use, First>;Co;0;L;;;;;N;;;;;
F8FF;<Private Use, Last>;Co;0;L;;;;;N;;;;;

20000;<CJK Ideograph Extension B, First>;Lo;0;L;;;;;N;;;;;
2A6D6;<CJK Ideograph Extension B, Last>;Lo;0;L;;;;;N;;;;;
F0000;<Plane 15 Private Use, First>;Co;0;L;;;;;N;;;;;
FFFFD;<Plane 15 Private Use, Last>;Co;0;L;;;;;N;;;;;
100000;<Plane 16 Private Use, First>;Co;0;L;;;;;N;;;;;
10FFFD;<Plane 16 Private Use, Last>;Co;0;L;;;;;N;;;;;
*/


/* =====================================================================
	Character Names
	
	Character Names and an array of pointers from UCS2 code to names
	are compressed and only decompressed if a character name is actually retrieved.
	memory footprint is ~86 kB compressed, uncompressed ~450 kB.
*/	

struct UCS2_Range { UCS2Char Start; ushort Count; };
struct UCS4_Range { UCS4Char Start; ushort Count; };

static UCS2_Range const UCS2CodeRanges[U_CODEPOINTRANGES_UCS2] = 
#include "Includes/CodePointRanges.h"

static UCS4_Range const UCS4CodeRanges[U_CODEPOINTRANGES_UCS4] = 
#include "Includes/CodePointRanges_UCS4.h"

static uchar const CompressedNames[U_NAMEARRAY_COMPRESSED] =
#include "Includes/Names.h"

static ptr	  DecompressedNames	= NULL;		// char DecompressedNames[450000]
static cstr*  NamePtr			= NULL;		// cstr NamePtr[16000] -> DecompressedNames
static ushort* UCS2NameBase		= NULL;		// UCS2Range base index in NamePtr[16000]
static ushort* UCS4NameBase		= NULL;		// UCS4Range base index in NamePtr[16000]


/*	decompress name array and setup pointer arrays
	TODO: multi-tasking lock to prevent double work or access before work completed
*/

static void decompress_names() throw(bad_alloc)
{
	XXXTRAP(DecompressedNames!=0);

// alloc memory	
	DecompressedNames = new char[U_NAMEARRAY_SIZE];
	NamePtr = new cstr[U_CODEPOINTS_UCS2+U_CODEPOINTS_UCS4];
	UCS2NameBase = new ushort[U_CODEPOINTRANGES_UCS2];
	UCS4NameBase = new ushort[U_CODEPOINTRANGES_UCS4];

// decompress names
	size_t sz = Melt( CompressedNames, U_NAMEARRAY_COMPRESSED, (uptr)DecompressedNames, U_NAMEARRAY_SIZE );
	XXXTRAP(errno);
	XXXTRAP(sz!=U_NAMEARRAY_SIZE);

// setup UCSxNameBase[] base offsets UCSxCodeRanges[i] -> NamePtr[j]
// setup cstr NamePtr[] -> char DecompressedNames[]
// replace $$$$ in character names with character code

	ptr name_ptr = DecompressedNames;			// name_ptr -> DecompressedNames
	int name_idx = 0;							// name_idx = NamePtr[name_idx]

	for( int i=0; i<U_CODEPOINTRANGES_UCS2; i++ )
	{											// i = UCS2CodeRanges[i]  &  UCS2NameBase[i] 
		int a =	UCS2CodeRanges[i].Start;		// a = Range.StartCode
		int e = a + UCS2CodeRanges[i].Count;	// e = Range.EndCode
		UCS2NameBase[i] = name_idx;
		
		for( ; a<e; a++ )
		{
			NamePtr[name_idx++] = name_ptr;
			ptr z = strchr(name_ptr,'$');
			if(z) { char const h[] = "0123456789ABCDEF"; for(int d=4,n=a;d;n>>=4) z[--d] = h[n&0x0f]; }
			name_ptr = strchr(name_ptr,0)+1;
		}
	}
	XXXTRAP( name_idx != U_CODEPOINTS_UCS2 );

	for( int i=0; i<U_CODEPOINTRANGES_UCS4; i++ )
	{
		int n = UCS4CodeRanges[i].Count;	// n = Range.Count
		UCS4NameBase[i] = name_idx;
		
		while( n-- )
		{
			NamePtr[name_idx++] = name_ptr;
			name_ptr = strchr(name_ptr,0)+1;
		}
	}
	XXXTRAP( name_idx != U_CODEPOINTS_UCS2+U_CODEPOINTS_UCS4 );
	XXXTRAP( name_ptr != DecompressedNames + U_NAMEARRAY_SIZE );
}


inline int FindUCS2 ( UCS2Char n, int e )
{ int a=0; do { int i=(a+e)/2; if( n<UCS2CodeRanges[i].Start ) e=i; else a=i; } while (a<e-1); return a; }

inline int FindUCS4 ( UCS4Char n, int e )
{ int a=0; do { int i=(a+e)/2; if( n<UCS4CodeRanges[i].Start ) e=i; else a=i; } while (a<e-1); return a; }


cstr UCS1CharCharacterName ( UCS1Char n )	
{
	if(!DecompressedNames) decompress_names(); 
	return NamePtr[n];					// Codepoints $0000 - $0250 sind durchgehend besetzt -> Range[0]
}

cstr UCS2CharCharacterName ( UCS2Char n ) 
{
	if(!DecompressedNames) decompress_names(); 

	if( n>>8 )
	{
		int i = FindUCS2(n,U_CODEPOINTRANGES_UCS2);
		n -= UCS2CodeRanges[i].Start;		// n = offset im Range
		if( n >= UCS2CodeRanges[i].Count ) 
		{
			cstr r = "<no unicode character>";
		// Test special ranges
			if( n<0x3400 ) {} else	  if( n<=0x4DB5 ) r = "<CJK Ideograph Extension A>"; else		// [3400 … 4DB5]
			if( n<0x4E00 ) {} else	  if( n<=0x9FBB ) r = "<CJK Ideograph>"; else					// [4E00 … 9FBB]
			if( n<0xAC00 ) {} else	  if( n<=0xD7A3 ) r = "<Hangul Syllable>"; else					// [AC00 … D7A3]
			if( n<0xD800 ) {} else	  if( n<=0xDB7F ) r = "<Non Private Use High Surrogate>"; else	// [D800 … DB7F]
		/*	if( n<0xDB80 ) {} else */ if( n<=0xDBFF ) r = "<Private Use High Surrogate>"; else		// [DB80 … DBFF]
		/*	if( n<0xDC00 ) {} else */ if( n<=0xDFFF ) r = "<Low Surrogate>"; else					// [DC00 … DFFF]
		/*	if( n<0xE000 ) {} else */ if( n<=0xF8FF ) r = "<Private Use>";							// [E000 … F8FF]
			return r;
		}
		n += UCS2NameBase[i]; 
	}
	return NamePtr[n]; 
}

cstr UCS4CharCharacterName ( UCS4Char n )
{
	if( n>>16 ) return UCS2CharCharacterName(n);
	
	if(!DecompressedNames) decompress_names(); 

	int i = FindUCS4(n,U_CODEPOINTRANGES_UCS4);
	n -= UCS4CodeRanges[i].Start;		// n = offset in Range
	if( n >= UCS4CodeRanges[i].Count )	// not within Range
	{
		cstr r = "<no unicode character>";
	// Test special ranges
		if( n<0x020000 ) {} else if( n<=0x02A6D6 ) r = "<CJK Ideograph Extension B>"; else	// [020000 … 02A6D6]
		if( n<0x0F0000 ) {} else if( n<=0x0FFFFD ) r = "<Plane 15 Private Use>";	  else	// [0F0000 … 0FFFFD]
		if( n<0x100000 ) {} else if( n<=0x10FFFD ) r = "<Plane 16 Private Use>"; 			// [100000 … 10FFFD]		
		return r;
	}
	n += UCS4NameBase[i]; 
	return NamePtr[n]; 
}





















