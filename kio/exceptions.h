#pragma once
/*	Copyright  (c)	Günter Woigk 1999 - 2020
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


	custom exceptions
*/


#include "kio/kio.h"
#include <new>
#include <exception>
class FD;


/*
	class std::exception
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
	cstr	text;		// allocated

public:
	explicit any_error (cstr msg, ...)		noexcept __printflike(2,3);
	any_error (cstr msg, va_list va)		noexcept __printflike(2,0);
	explicit any_error (int error)			noexcept :error(error), text(nullptr){}
	any_error (int error, cstr msg)			noexcept;
	~any_error ()							noexcept override;

	cstr what () const						noexcept override;

	any_error(any_error const&)				noexcept;
	any_error(any_error&&)					noexcept;
	any_error& operator= (any_error const&) = delete;
	any_error& operator= (any_error&&)		= delete;
};



// ---------------------------------------------
//			internal_error
// ---------------------------------------------

class internal_error : public any_error
{
public:
	cstr file;			// source file: const or temp;  assumed: __FILE__
	uint line;			// source line number;			assumed: __LINE__

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
public:
	limit_error (cstr where, ulong sz, ulong max)	noexcept;
};



// ---------------------------------------------
//			data_error
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
	int  fd;

public:
	file_error(cstr path, int error)			noexcept;
	file_error(cstr path, int error, cstr msg)	noexcept;
	file_error(cstr path, int fd, int error)			noexcept;
	file_error(cstr path, int fd, int error, cstr msg)	noexcept;
	file_error(const FD&, int error)			noexcept;
	file_error(const FD&, int error, cstr msg)	noexcept;

	file_error(file_error const&)				noexcept;
	file_error(file_error&&)					noexcept;
	file_error& operator= (file_error const&)	= delete;
	file_error& operator= (file_error&&)		= delete;
	~file_error ()								noexcept override;

	cstr what() const							noexcept override;
};
































