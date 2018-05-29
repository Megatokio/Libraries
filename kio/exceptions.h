/*	Copyright  (c)	Günter Woigk 1999 - 2018
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



#ifndef	EXCEPTIONS_H
#define	EXCEPTIONS_H

#include <new>
#include "kio/kio.h"
extern str usingstr( cstr, ... );		// #include "cstrings/cstrings.h"

#ifndef __cplusplus
	#error	C und nicht C++  !!
#endif



// ------------------------------------------------------------------------
//
//			exception classes
//
// ------------------------------------------------------------------------


/*
	class std::exception			// #include <exception>
	{
		public:			exception	()			noexcept{}
		virtual			~exception	()			noexcept;
		virtual cstr	what		() const	noexcept;
	};

hierarchy:
	std::exception					// #include <exception>		base class
		bad_alloc					// #include <new>			out of memory
		std::bad_typeid				// #include <typeinfo>		Falscher Objekttyp
		std::bad_cast				// #include <typeinfo>		Falscher Objekttyp bei Typumwandlung
		std::bad_exception			// #include <exception>		unexpected()

		any_error;					//							base class for own errors: includes error code
			internal_error			// #include "kio/abort.h"	ABORT, TRAP, assert, IERR, TODO
			limit_error;			//							Array<T>: array too large
			data_error;				//							data parsers, e.g. for input data from file
			file_error;				// "unix/file_utilities.h"

		std::logic_error			// #include <stdexcept>		theoretisch vermeidbare Laufzeitfehler
			std::invalid_argument	// #include <stdexcept>		stdc++ lib ((general function argument error))
			std::length_error		// #include <stdexcept>		stdc++ lib ((data exceeding allowed size))
			std::out_of_range		// #include <stdexcept>		stdc++ lib ((e.g. index))
			std::domain_error		// #include <stdexcept>		stdc++ lib ((domain of math. function))

		std::runtime_error			// #include <stdexcept>		fehlerhafte Daten zur Laufzeit
			std::range_error		// #include <stdexcept>		arith. Bereichsüberschreitung
			std::overflow_error		// #include <stdexcept>		arith. Überlauf
			std::underflow_error	// #include <stdexcept>		arith. Unterlauf
*/


// ---------------------------------------------
//			bad_alloc
// ---------------------------------------------

typedef	std::bad_alloc	bad_alloc;



// ---------------------------------------------
//			any_error
// ---------------------------------------------

class any_error : public std::exception
{
public:
	int		error;		// errno
	cstr	text;		// custom error message: const, temp or NULL

public:
	any_error (cstr msg, ...)			noexcept __printflike(2,3);
	any_error (cstr msg, va_list va)	noexcept __printflike(2,0) :error(customerror), text(usingstr(msg,va)){}
	any_error (int error)				noexcept :error(error), text(NULL){}
	any_error (int error, cstr msg)		noexcept :error(error), text(msg){}
	cstr what () const					noexcept override;
};



// ---------------------------------------------
//			internal_error
// ---------------------------------------------

class internal_error : public any_error
{
public:
	cstr file;			// source line: const or temp
	uint line;			// source line number

public:
	internal_error (cstr file, uint line)			 noexcept :any_error(internalerror),file(file),line(line){}
	internal_error (cstr file, uint line, int err)	 noexcept :any_error(err),file(file),line(line){}
	internal_error (cstr file, uint line, cstr msg)	 noexcept :any_error(internalerror,msg),file(file),line(line){}
	cstr what () const								 noexcept override;
};



// ---------------------------------------------
//			limit_error
// ---------------------------------------------

class limit_error : public any_error
{
	uint sz,max;

public:
	limit_error (cstr msg, uint sz, uint max)	noexcept :any_error(limiterror,msg),sz(sz),max(max){}
	cstr what () const							noexcept override;
};



// ---------------------------------------------
//			data_error								--> md5, backup_daemon.client
// ---------------------------------------------

class data_error : public any_error
{
public:
	 data_error	()								noexcept :any_error(dataerror){}
	 data_error	(cstr msg, ...)					noexcept __printflike(2,3);
	 data_error	(int error, cstr msg)			noexcept :any_error(error,msg){}
	 data_error	(int error)						noexcept :any_error(error){}
};



// ---------------------------------------------
//			file_error
// ---------------------------------------------

class file_error : public any_error
{
public:
	cstr filepath;
	int	 fd;

public:
	file_error(class FD&, int error)					noexcept;
	file_error(class FD&, int error, cstr msg)			noexcept;
	file_error(cstr path, int fd, int error)			noexcept :any_error(error),filepath(path),fd(fd){}
	file_error(cstr path, int fd, int error, cstr msg)	noexcept :any_error(error,msg),filepath(path),fd(fd){}
	cstr what() const									noexcept override;
};



#endif




























