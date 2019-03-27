/*	Copyright  (c)	Günter Woigk 1999 - 2019
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


	error number and error text definitions
	this file is included in "kio/kio.h", "kio/kio.cpp" and "kio/exceptions.cpp"

	extending the error code list:
	file "custom_errors.h"
	custom application error messages
	file must exist but may be empty
*/


#ifndef EMAC
 #define NO_EMAC
 #define EMAC(A,B)	A
 enum {
#endif

#ifndef EBAS
 #define EBAS 4711
#endif


// basic errors:
	EMAC( customerror=EBAS, "custom error"				),	// kio/errors.h: custom error message
	EMAC( notanumber,		"not a number"				),	// cstrings/cstrings.h:	0.0/0.0 or string is not a number
	EMAC( unexpectedfup,	"utf-8 char started with fup"),	// cstrings/cstrings.h
	EMAC( truncatedchar,	"utf-8 char truncated"		),	// cstrings/cstrings.h
	EMAC( notindestcharset,	"character not in destination charset" ), // utf-8, cstrings/utf8
	EMAC( brokenescapecode,	"broken escape code in string"),// cstrings/utf8.cpp
	EMAC( endoffile,		"end of file"				),	// unix/FD.h
#define   outofmemory		ENOMEM							// kio/errors.h: macro OMEM(), <=> std::bad_alloc : exception
	EMAC( internalerror,	"internal error"			),	// kio/errors.h: macro NIMP(),  class internal_error
	EMAC( notyetimplemented,"not yet implemented"		),	// kio/errors.h: macro TODO(),  class internal_error
	EMAC( limiterror,		"size exceeds limit"		),	// kio/errors.h: class limit_error : internal_error
//	EMAC( anyerror,			"unspecified error"			),	// kio/errors.h:
	EMAC( dataerror,		"data error"				),	// kio/errors.h:

// os_utilities.cpp:
	EMAC( childterminatedbysignal,	"child terminated by signal" ),
	EMAC( childreturnederror,		"child returned error" ),
	EMAC( datacorrupted,			"data corrupted" ),
	EMAC( wrongfiletype,			"wrong file type" ),

#ifdef CUSTOM_ERRORS
 #include CUSTOM_ERRORS
#endif

#undef EMAC

#ifdef NO_EMAC
 #undef NO_EMAC
 };
#endif



















