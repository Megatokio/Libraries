#pragma once
/*	Copyright  (c)	Günter Woigk 2001 - 2019
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

#include	<time.h>
#include	"kio/kio.h"


/* ----	system info ----------------------
*/
extern	cstr	hostName		();
extern	uint	numCPUs			();
extern	time_t	bootTime		();

extern	void	sysLoad			(double load[3]);			// ranges: 1, 5, and 15 minutes
extern	size_t 	memoryUsage		(bool resident = true);
extern	double	cpuLoad			();

extern	time_t	intCurrentTime	();
extern	double	now				();		// was: floatCurrentTime
inline	time_t	upTime			()		{ return intCurrentTime()-bootTime(); }

inline	uid_t	getUID			( )		{ return getuid(); }
inline	gid_t	getGID			( )		{ return getgid(); }
inline	uid_t	getEffUID		( )		{ return geteuid(); }
inline	gid_t	getEffGID		( )		{ return getegid(); }
extern	cstr	getUser			( );
extern	cstr	getEffUser		( );


extern	str		execCmd			( /* cmd = argv[0] */ str const argv[], str const envv[]=NULL );
extern	str		execCmd			( cstr cmd, ... /* argv[1] ... NULL */ );










