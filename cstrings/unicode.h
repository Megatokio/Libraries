#pragma once
/*	Copyright  (c)	Günter Woigk 2019 - 2020
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice and this permission notice appear in
	supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#include "kio/kio.h"


typedef uint8  ucs1char, *ucs1ptr, *ucs1str; typedef const ucs1char *ucs1cptr, *ucs1cstr;
typedef uint16 ucs2char, *ucs2ptr, *ucs2str; typedef const ucs2char *ucs2cptr, *ucs2cstr;
typedef uint32 ucs4char, *ucs4ptr, *ucs4str; typedef const ucs4char *ucs4cptr, *ucs4cstr;


namespace unicode
{

// Unicode Character PROPERTY VALUES for Catalog Properties and Enumerated Properties
#include "unicode/properties.h"

}; // namespace

