#pragma once
// Copyright (c) 2001 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "s_type.h"
#include <sys/mount.h>


template<class T>
class Array;
class MyFileInfo;
typedef Array<MyFileInfo> MyFileInfoArray;
class MyVolumeInfo;
class MyVolumeInfoArray;


#define STDIN  0
#define STDOUT 1
#define STDERR 2


// ----------------------------------------------------------------------
//					Helper
// ----------------------------------------------------------------------


extern cstr latin1str(cstr non_utf8_str); // requires #define USE_LATIN1_PATH  in "settings.h"

extern str	  modestr(mode_t);
extern str	  effmodestr(mode_t, gid_t, uid_t);
inline s_type classify_file(mode_t mode) noexcept { return s_type(mode >> 12); }

extern cstr quick_fullpath(cstr path) noexcept; // just add workingdir or homedir
extern str	fullpath(cstr path, bool resolve_last_symlink = 1, bool auto_create_path = no);

extern void change_working_dir(cstr path);
extern cstr workingdirpath() noexcept;
extern cstr homedirpath() noexcept;
extern cstr tempdirpath();
extern cstr tempfilepath(cstr same_dir_as_file); // renameable to 'file'

extern gid_t* get_groups(uid_t user) noexcept;
extern cstr	  find_executable(cstr name_or_partial_path);


// ----------------------------------------------------------------------
//					file info
// ----------------------------------------------------------------------


extern s_type classify_file(cstr path, bool resolve_last_symlink = 1) noexcept;
inline bool	  exists_node(cstr path, bool resolve_last_symlink = 1) noexcept
{
	return classify_file(path, resolve_last_symlink) != s_none;
}
inline bool is_file(cstr path, bool resolve_last_symlink = 1) noexcept
{
	return classify_file(path, resolve_last_symlink) == s_file;
}
inline bool is_dir(cstr path, bool resolve_last_symlink = 1) noexcept
{
	return classify_file(path, resolve_last_symlink) == s_dir;
}
inline bool	  is_link(cstr path) noexcept { return classify_file(path, no) == s_link; }
inline bool	  is_tty(cstr path) noexcept { return classify_file(path) == s_tty; }
extern time_t file_mtime(cstr path, bool resolve_last_symlink = 1) noexcept; // modification time
extern time_t file_atime(cstr path, bool resolve_last_symlink = 1) noexcept; // last access time
extern time_t file_ctime(cstr path, bool resolve_last_symlink = 1) noexcept; // last status change time
extern off_t  file_size(cstr path, bool resolve_last_symlink = 1) noexcept;
extern bool	  is_writable(cstr path, bool resolve_last_symlink = 1); // for user
extern bool	  is_readable(cstr path, bool resolve_last_symlink = 1); // for user
extern bool	  is_executable(cstr path, bool resolve_last_symlink = 1, bool for_user = 0);


// ----------------------------------------------------------------------
//					create, delete, read & write file
// ----------------------------------------------------------------------


extern void create_file(cstr path, mode_t perm = 0660);
extern void create_dir(cstr path, mode_t perm = 0777, bool autocreatedirs = no);
extern void create_pipe(cstr path, mode_t perm = 0660);
extern void create_symlink(cstr linkpath, cstr destpath);
extern void create_hardlink(cstr newpath, cstr oldpath);
extern void create_hardlinked_copy(cstr newdir, cstr olddir, bool copy_dir_owner = no);

extern int set_owner_and_group(cstr path, uid_t = uid_t(-1), gid_t = gid_t(-1), bool deref_last_symlink = yes) noexcept;
extern int get_owner_and_group(cstr path, uid_t&, gid_t&, bool deref_last_symlink = yes) noexcept;

extern void delete_node(cstr path, bool resolve_last_symlink, s_type typ = s_any);
extern void delete_dir(cstr path, bool recursive = no);

extern void rename_node(cstr oldpath, cstr newpath, bool overwrite);
extern void swap_files(cstr path1, cstr path2);

extern void read_dir(cstr path, MyFileInfoArray&, bool resolve_symlinks); // Attn.: no trailing "/" for dir names!
extern str	read_link(cstr path);
extern void read_file(cstr path, Array<str>& a, uint32 maxsize = 1 << 28);
inline void read_file(cstr path, Array<cstr>& a, uint32 maxsize = 1 << 28)
{
	read_file(path, reinterpret_cast<Array<str>&>(a), maxsize);
}
extern void read_file(cstr path, class StrArray& a, uint32 maxsize = 1 << 28);

extern void write_file(cstr path, cptr data, uint32 len, int flags = 'w' /*overwrite*/, mode_t = 0660);
extern void write_file(cstr path, Array<str>& a, int flags = 'w' /*overwrite*/, mode_t = 0660);

extern void copy_file(cstr qpath, cstr zpath, bool overwrite);


// File mode groups
// for individual bits see stat.h
enum {
	READABLE   = 0444, // readable by owner, group & others
	WRITABLE   = 0222, // writable by owner, group & others
	EXECUTABLE = 0111, // executable/listable by owner, group & others
	OWNER	   = 0700,
	GROUP	   = 0070,
	OTHERS	   = 0007,
	NOBODY	   = 0000
};

#if defined(_LINUX) || defined(_OPENBSD)
extern int set_file_permissions(cstr path, mode_t who, mode_t perm);
#else
extern int set_file_permissions(cstr path, mode_t who, mode_t perm, bool resolve_last_symlink = yes);
#endif
inline int set_file_readable(cstr path, mode_t who = OWNER | GROUP)
{
	return set_file_permissions(path, who, READABLE);
}
inline int set_file_writable(cstr path, mode_t who = OWNER | GROUP)
{
	return set_file_permissions(path, who, WRITABLE);
}
inline int set_file_executable(cstr path, mode_t who = OWNER | GROUP)
{
	return set_file_permissions(path, who, EXECUTABLE);
}


// ----------------------------------------------------------------------
//					volume info
// ----------------------------------------------------------------------


enum VolumeFlag { vol_wprot = 1, vol_ejectable = 2, vol_mounted = 4 };

extern uint	  get_volume_flags(cstr path); // BSD
extern uint64 get_volume_free(cstr path);  // BSD

class MyVolumeInfo
{
public:
	bool valid;

	// info from struct statfs:
	uint32 blocksize;	  // fs.f_bsize			fundamental file system block size
	uint32 bestiosize;	  // fs.f_iosize			optimal transfer block size
	uint64 totalblocks;	  // fs.f_blocks			total data blocks in file system
	uint64 blocksfree_su; // fs.f_bfree			free blocks in fs
	uint64 blocksfree;	  // fs.f_bavail			free blocks avail to non-superuser
	uint64 totalfiles;	  // fs.f_files			total file nodes in file system
	uint64 filesfree;	  // fs.f_ffree			free file nodes in fs
#ifndef __CYGWIN__
	fsid_t fsid; // fs.f_fsid 			file system id 					???
#endif
	uid_t  owner;	   // fs.f_owner			user that mounted the filesystem
	uint   fstype;	   // fs.f_type			type of filesystem				number
	uint32 flags;	   // fs.f_flags			copy of mount exported flags: 	e.g. MNT_RDONLY
	uint32 fs_subtype; // fs.f_fssubtype		fs sub-type (flavor)
	cstr   fstypename; // fs.f_fstypename[MFSTYPENAMELEN]	fs type name
	cstr   mountpoint; // fs.f_mntonname[MAXPATHLEN]		directory on which mounted
	cstr   volumename; // fs.f_mntfromname[MAXPATHLEN]		mounted filesystem

public:
	MyVolumeInfo(struct statfs& fs);	 // BSD etc.
	MyVolumeInfo(struct mntent* mntent); // Linux
	~MyVolumeInfo();

	uint64 totalbytes() const { return blocksize * totalblocks; }	  // number: blocksize * blocks
	uint64 bytesfree() const { return blocksize * blocksfree; }		  // number: blocksize * blocksfree
	uint64 bytesfree_su() const { return blocksize * blocksfree_su; } // number: blocksize * blocksfree_su
};


#include "MyFileInfo.h"

class MyVolumeInfoArray : public Array<MyVolumeInfo*>
{
public:
	MyVolumeInfoArray();
};
