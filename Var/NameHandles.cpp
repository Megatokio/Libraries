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
*/

#define	SAFE	2
#define	LOG 	1

#include "cstrings/cstrings.h"
#include "NameHandles.h"
DEBUG_INIT_MSG



/* Settings
*/
#ifndef NH_HASHRING_BITS
#define	NH_HASHRING_BITS	12			/* suggested value: 12  -->  HashRing[4k] */
#endif



/* ----------------------------------------------------------------------------
*/
const uint	handlebits		= sizeof(NameHandle)*8;		// size of NameHandle		((32))

const uint	hr_bits 		= NH_HASHRING_BITS;			// size of HashRing[]		((12))
const uint32	hr_size			= 1<<hr_bits;

const uint	hr_shift		= handlebits-hr_bits;		// shift for Handle <--> HashRingIndex
//const uint32 hr_lo_mask  	= hr_size-1;				// mask if shifted right (index position)
//const uint32 hr_hi_mask  	= hr_lo_mask << hr_shift;	// mask if shifted left  (handle position)

const uint	hd_bits			= handlebits - hr_bits;		// max.size of HashData[] in HashRing[].data[]		((20))
const uint32	hd_mask			= (1ul<<hd_bits)-1;



/* ----	data container for Handle/Name pair ----------------------
*/
struct HashData
{
	uint		usage;	// usage count
	uint32		hash;
	String		name;

	HashData ( ) : usage(0) { };	// creator
};



/* ----	HashRing array for Handle/Name pair arrays --------------------
		note: HashRing[i].data[j]  <=>  handle = (i<<hr_shift) + j
*/
static struct { uint size; HashData* data; } HashRing[hr_size];


/* ---- Initialization ------------------------------------------------
		Set NameHandle(emptyString) := 0
		note: initialization may be performed after we were already
			  used by other modules for their initialization.
			  All we have to do here is to make sure that the
			  emptyString is assigned to NameHandle 0.
*/
void InitHR()
{
	static int done=0; if(done) return; else done++;
	xlogIn("init HashRing[0]");
	xassert(emptyString.CalcHash()==0);
	xassert(HashRing[0].size==0);		// NH==0 already used?
// manual setup:
// NewNameHandle() calls cstring.cp which may be not yet initialized
	HashData* hd;
	HashRing[0].size = 1;
	HashRing[0].data = hd = new HashData[1];
	hd[0].usage = 1;					// lock NH and leave it locked 4ever
	hd[0].hash  = 0;
	hd[0].name  = emptyString;
};
static struct INIT_HR { INIT_HR(){InitHR();} } dummyname;





/* ===================================================================
		The Procedures
=================================================================== */



/* ----	Release existing Handle ------------------------------------
		Handle 1x entriegeln
*/
void UnlockNameHandle( NameHandle handle )
{
	if (handle==0) return;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	assert(i<HashRing[idx].size);
	xxassert(hd!=nullptr);
	assert(hd[i].usage>0);

	hd[i].usage--;							// may become 0
}


/* ----	Request existing Handle ----------------------------------------
		Handle 1x verriegeln
*/
void LockNameHandle ( NameHandle handle )
{
	if (handle==0) return;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	assert(i<HashRing[idx].size);			// bogus NH
	xxassert(hd!=nullptr);					// internal error
	assert(hd[i].usage>0);					// try to lock an old but no longer valid NH

	hd[i].usage++;
}


/* ----	Namen auslesen ------------------------------------------
*/
cString& GetNameForHandle ( NameHandle handle )
{
//	if (handle==0) return emptyString;

	uint     idx = handle >> hr_shift;		// HashRing[idx]
	HashData* hd = HashRing[idx].data;
	uint       i = handle & hd_mask;		// hd[i]

	assert(i<HashRing[idx].size);
	xxassert(hd!=nullptr);
	assert(hd[i].usage>0);

	return hd[i].name;
}


/* ----	Search for Name ------------------------------------------
		returns Handle if found
		returns 0 if name unknown  OR  name == empty string
*/
NameHandle FindNameHandle ( cString& s )
{
	if (s.Len()==0) return 0;

	uint32   hash = s.CalcHash();
	uint32    idx = hash >> hr_shift;
	uint      sz = HashRing[idx].size;  if (sz==0) return 0;		// not found
	HashData* hd = HashRing[idx].data;

	for (uint i=0;i<sz;i++)
	{
		if (hd[i].hash==hash && hd[i].usage && hd[i].name==s) return (idx<<hr_shift)+i;
	}

	return 0;		// not found
}


/* ----	Request Name ------------------------------------------
		Namen auf 'neu' prüfen
		Handle verriegeln
*/
NameHandle NewNameHandle ( cString& s )
{
	if (s.Len()==0) return 0;

	xlogIn("NewNameHandle(\"%s\")",s.CString());

#if XXXSAFE
	NameHandleCheck();
#endif

	uint32   hash = s.CalcHash();
	uint32     idx = hash >> hr_shift;
	uint      sz = HashRing[idx].size;
	HashData* hd;
	uint	   i;

	xlog("#%lu#",idx);

// first name for this slot?  =>  handle == hash
	if (sz==0)
	{
		HashRing[idx].size = 1;
		HashRing[idx].data = hd = new HashData[1];
		i = 0;
		xlog("{+}");
		goto i;		// --> HashRing[idx].data[i]
	}

// already exists?
	hd = HashRing[idx].data;	xxassert(hd!=nullptr);
	for (i=0;i<sz;i++)
	{
		if (hd[i].hash==hash && hd[i].usage && hd[i].name==s)
		{
			hd[i].usage++;
			return (idx<<hr_shift) +i;
		}
	}

// slot already in use, but string not found  =>  search free slot
	for (i=0;i<sz;i++)
	{
		if (hd[i].usage==0)
		{
			xlog("{o}");
	i:		hd[i].usage = 1;
			hd[i].hash  = hash;
			hd[i].name  = s;
			return (idx<<hr_shift) +i;
		}
	}

// no free slot  =>  grow vector
	HashData* ohd = hd;
	uint      osz = sz;

	if(XXLOG) { static int n=0; log("{*%i*}",++n); }

	HashRing[idx].size = sz = sz*4/3+1;
	HashRing[idx].data = hd = new HashData[sz];
	for (i=0; i<osz; i++) { hd[i] = ohd[i]; }
	delete[] ohd;

//	i = osz;
	goto i;		// --> HashRing[idx].data[i]
}


/* ----	cstr version -------------------------------------------------
		required wg. bug in gcc 3.3.3
*/
NameHandle NewNameHandle ( cstr s )
{
	return NewNameHandle(String(s));
}



/* ----	free unused memory ---------------------------------
		called in case of memory shortage
*/
void NameHandlesPurgeCaches ( )
{
	for (uint idx=0; idx<hr_size; idx++ )
	{
		uint      sz = HashRing[idx].size;	if(sz==0) continue;
		HashData* hd = HashRing[idx].data;

		int n=-1;
		for (uint i=0;i<sz; i++)
		{
			if (hd[i].usage) n=i;
			else hd[i].name=emptyString;
		}

		if (n==-1)
		{
			delete[] hd;
			HashRing[idx].size = 0;
			HashRing[idx].data = nullptr;
			continue;
		}

		if (uint(n)+1<sz)
		{
			// truncate vector ***TODO***
		}
	}
}


void NameHandleCheck()
{
	char bu[5000];
	memset(bu,0,sizeof(bu));

	for(uint i=0;i<hr_size;i++)
		for(uint j=0;j<HashRing[i].size;j++)
			HashRing[i].data[j].name.Check( __FILE__, __LINE__ );
}
















