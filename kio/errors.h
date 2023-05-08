// Copyright (c) 1999 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	error number and error text definitions

	To extend the error code list
	• create file in your project with more EMAC definitions
	• #define CUSTOM_ERRORS to the file name in settings.h

	normally this file is included to define the custom error numbers.
	the messages are included in kio.cpp and can be retrieved with errorstr(errno)
*/


#ifndef EMAC
  #define EMAC(A, B) A
  #include <cerrno>

  #ifndef EBAS
	#ifdef ELAST
	  #define EBAS ELAST
	#elif defined(__ELASTERROR)
 	  #define EBAS __ELASTERROR
	#else
	  #define EBAS 7400
	#endif
  #endif

typedef const char* cstr;
extern cstr			errorstr(int err) noexcept; // in kio.h: get error string for system or custom error number
inline cstr			errorstr() noexcept { return errorstr(errno); }

enum
#endif
{

	// basic errors:
	EMAC(customerror = EBAS, "custom error"),		  // kio/errors.h: custom error message
	EMAC(notanumber, "not a number"),				  // cstrings/cstrings.h:	0.0/0.0 or string is not a number
	EMAC(unexpectedfup, "utf-8 char unexpected fup"), // cstrings/cstrings.h
	EMAC(truncatedchar, "utf-8 char truncated"),	  // cstrings/cstrings.h
	EMAC(notindestcharset, "character not in destination charset"), // utf-8, cstrings/utf8
	EMAC(brokenescapecode, "broken escape code in string"),			// cstrings/utf8.cpp
	EMAC(endoffile, "end of file"),									// unix/FD.h
#define outofmemory ENOMEM
	EMAC(internalerror, "internal error"),			// kio/cdefs.h: macro IERR(),  class InternalError
	EMAC(notyetimplemented, "not yet implemented"), // kio/cdefs.h: macro TODO(),  class InternalError
	EMAC(limiterror, "size exceeds limit"),			// kio/exceptions.h: class LimitError
	EMAC(dataerror, "data error"),					// kio/exceptions.h: class DataError

	// os_utilities.cpp:
	EMAC(childterminatedbysignal, "child terminated by signal"),
	EMAC(childreturnederror, "child returned error"),
	//	EMAC( datacorrupted,			"data corrupted" ),
	EMAC(wrongfiletype, "wrong file type"),

#ifdef CUSTOM_ERRORS
  #include CUSTOM_ERRORS
#endif

#undef EMAC
};
