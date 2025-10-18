// Copyright (c) 2025 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "RCPtr.h"


/*	base classes to be used instead of directly using RCDATA.	
*/

class RCBase
{
	RCDATA
};

class RCObject
{
	RCDATA
public:
	virtual ~RCObject() = default;
};
