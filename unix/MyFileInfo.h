#pragma once
// Copyright (c) 2001 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Templates/Array.h"
#include "s_type.h"


class MyFileInfo // war: FileInfo; wg. name collission with Finder...
{
	cstr name; // 1st!	directories have no trailing "/"

public:
	mode_t type;
	mode_t perm;
	uid_t  uid;
	gid_t  gid;
	off_t  length; // file length or files in sub directory			int64  (Posix)
	time_t ctime,  // t_time last modification to meta data			time_t == long
		mtime,	   // t_time last modification
		atime;	   // t_time last access
	uint links;	   // number of hard links

	MyFileInfo() { memset(this, 0, sizeof(*this)); }
	~MyFileInfo() { delete[] name; }
	MyFileInfo(const MyFileInfo& q)
	{
		memcpy(this, &q, sizeof(q));
		name = newcopy(name);
	}
	MyFileInfo(MyFileInfo&& q)
	{
		memcpy(this, &q, sizeof(q));
		q.name = nullptr;
	}
	MyFileInfo& operator=(const MyFileInfo& q)
	{
		if (this != &q)
		{
			delete[] name;
			memcpy(this, &q, sizeof(q));
			name = newcopy(name);
		}
		return *this;
	}
	MyFileInfo& operator=(MyFileInfo&& q)
	{
		assert(this != &q);
		delete[] name;
		memcpy(this, &q, sizeof(q));
		q.name = nullptr;
		return *this;
	}

	bool is_dir() const { return type == s_dir; }
	bool is_file() const { return type == s_file; }
	bool is_link() const { return type == s_link; }
	bool is_none() const { return type == s_none; }
	bool is_dir_file_or_link() const { return ((1 << s_file) | (1 << s_dir) | (1 << s_link)) & (1 << type); }
	cstr fname() const { return name; }
	void set_fname(cstr s)
	{
		s = newcopy(s);
		delete[] name;
		name = s;
	}
	bool has_hardlinks() const { return links > 1; }
	bool operator>(const MyFileInfo& b) const { return strcmp(fname(), b.fname()) > 0; } // for sorting MyFileInfo[]
};


typedef Array<MyFileInfo> MyFileInfoArray;
