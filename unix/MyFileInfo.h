#pragma once
/*	Copyright  (c)	Günter Woigk 2001 - 2019
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

#include "s_type.h"
#include "Templates/Array.h"


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











