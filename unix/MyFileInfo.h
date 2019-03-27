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

#include "s_type.h"
#include "../Templates/Array.h"


class MyFileInfo				// war: FileInfo; wg. name collission with Finder...
{
		char const*	name;		// 1st!	directories have no trailing "/"

public:
		mode_t		type;
		mode_t		perm;
		uid_t		uid;
		gid_t		gid;
		off_t		length;		// file length or files in sub directory			int64  (Posix)
		time_t		ctime,		// t_time last modification to meta data			time_t == long
					mtime,		// t_time last modification
					atime;		// t_time last access
		uint		links;		// number of hard links

		MyFileInfo	()								{ memset(this,0,sizeof(*this)); }
		~MyFileInfo	()								{ delete[]name; }
		MyFileInfo	(MyFileInfo const& q)			{ memcpy(this,&q,sizeof(q)); name = newcopy(name); }
		MyFileInfo& operator=(MyFileInfo const& q) 	{ if(this!=&q) { delete[]name; memcpy(this,&q,sizeof(q)); name=newcopy(name);} return *this; }

		bool		is_dir()						const	{ return type==s_dir;  }
		bool		is_file()						const	{ return type==s_file; }
		bool		is_link()						const	{ return type==s_link; }
		bool		is_none()						const	{ return type==s_none; }
		bool		is_dir_file_or_link()			const	{ return ((1<<s_file)|(1<<s_dir)|(1<<s_link)) & (1<<type); }
		cstr		fname()							const	{ return name; }
		void		set_fname(cstr s)						{ s = newcopy(s); delete[] name; name = s; }
		bool		has_hardlinks()					const	{ return links>1; }
		bool 		operator>(const MyFileInfo& b) 	const	{ return strcmp(fname(),b.fname())>0; }		// for sorting MyFileInfo[]
};


typedef Array<MyFileInfo> MyFileInfoArray;











