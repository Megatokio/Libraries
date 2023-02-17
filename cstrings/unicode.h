#pragma once
// Copyright (c) 2019 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"


typedef uint8			ucs1char, *ucs1ptr, *ucs1str;
typedef const ucs1char *ucs1cptr, *ucs1cstr;
typedef uint16			ucs2char, *ucs2ptr, *ucs2str;
typedef const ucs2char *ucs2cptr, *ucs2cstr;
typedef uint32			ucs4char, *ucs4ptr, *ucs4str;
typedef const ucs4char *ucs4cptr, *ucs4cstr;


namespace unicode
{

// Unicode Character PROPERTY VALUES for Catalog Properties and Enumerated Properties
#include "unicode/properties.h"

}; // namespace unicode
