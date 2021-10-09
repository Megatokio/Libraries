#pragma once
// Copyright (c) 2001 - 2021 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "kio/kio.h"
#include "s_type.h"
#include <sys/mount.h>


#define TPL template<class T>
#define SOT sizeof(T)
#define THF	noexcept(false)	// FileError
#define EXT extern
#define INL inline
template <class T> class Array;
class MyFileInfo;
typedef Array<MyFileInfo> MyFileInfoArray;
class MyVolumeInfo;
class MyVolumeInfoArray;


#define STDIN	0
#define STDOUT	1
#define STDERR	2



// ----------------------------------------------------------------------
//					Helper
// ----------------------------------------------------------------------


EXT	cstr	latin1str   			(cstr non_utf8_str);	// requires #define USE_LATIN1_PATH  in "settings.h"

EXT	cstr	filename_from_path		(cstr path) noexcept;	// "…/name.ext" --> "name.ext"	"…/" -> ""
EXT	cstr	extension_from_path		(cstr path) noexcept;	// "….ext"		--> ".ext"		"…"  -> ""
EXT	cstr	basename_from_path		(cstr path) noexcept;	// "…/name.ext"	--> "name"
EXT	cstr	directory_from_path		(cstr path) noexcept;	// "path/…"		--> "path/"		"…"	 -> "./"
EXT	cstr	parent_directory_from_path(cstr path) noexcept;	// "path/name/"	--> "path/"		"path/name"	--> "path/"		"…"	 -> "./"
EXT	cstr	last_component_from_path(cstr path) noexcept;	// "…/name.ext"	--> "name.ext"	"…/dir/" -> "dir/"

EXT	str		modestr					(mode_t);
EXT	str		effmodestr				(mode_t, gid_t, uid_t);
INL	s_type	classify_file			(mode_t mode) noexcept				{ return s_type(mode>>12); }

EXT	cstr	quick_fullpath			(cstr path) noexcept;				// just add workingdir or homedir
EXT	str		fullpath				(cstr path, bool resolve_last_symlink=1, bool auto_create_path=no);

EXT	void	change_working_dir		(cstr path);
EXT	cstr	workingdirpath			() noexcept;
EXT	cstr	homedirpath				() noexcept;
EXT	cstr	tempdirpath				();
EXT	cstr	tempfilepath			(cstr same_dir_as_file);			// renameable to 'file'

EXT	gid_t*	get_groups				(uid_t user) noexcept;
EXT	cstr	find_executable			(cstr name_or_partial_path);


// ----------------------------------------------------------------------
//					file info
// ----------------------------------------------------------------------


EXT	s_type	classify_file	(cstr path, bool resolve_last_symlink=1) noexcept;
INL	bool	exists_node		(cstr path, bool resolve_last_symlink=1) noexcept	{ return classify_file(path,resolve_last_symlink)!=s_none; }
INL	bool	is_file			(cstr path, bool resolve_last_symlink=1) noexcept	{ return classify_file(path,resolve_last_symlink)==s_file; }
INL	bool	is_dir			(cstr path, bool resolve_last_symlink=1) noexcept	{ return classify_file(path,resolve_last_symlink)==s_dir; }
INL	bool	is_link			(cstr path) noexcept								{ return classify_file(path,no)==s_link; }
INL	bool	is_tty			(cstr path) noexcept								{ return classify_file(path)==s_tty; }
EXT	time_t	file_mtime		(cstr path, bool resolve_last_symlink=1) noexcept;	// modification time
EXT	time_t	file_atime		(cstr path, bool resolve_last_symlink=1) noexcept;	// last access time
EXT	time_t	file_ctime		(cstr path, bool resolve_last_symlink=1) noexcept;	// last status change time
EXT	off_t	file_size		(cstr path, bool resolve_last_symlink=1) noexcept;
EXT	bool	is_writable		(cstr path, bool resolve_last_symlink=1);	// for user
EXT	bool	is_readable		(cstr path, bool resolve_last_symlink=1);	// for user
EXT	bool	is_executable	(cstr path, bool resolve_last_symlink=1, bool for_user=0);



// ----------------------------------------------------------------------
//					create, delete, read & write file
// ----------------------------------------------------------------------


EXT	void	create_file		(cstr path, mode_t perm=0660)							THF;
EXT	void	create_dir		(cstr path, mode_t perm=0777, bool autocreatedirs=no)	THF;
EXT	void	create_pipe		(cstr path, mode_t perm=0660)							THF;
EXT	void	create_symlink	(cstr linkpath, cstr destpath)							THF;
EXT	void	create_hardlink	(cstr newpath, cstr oldpath)							THF;
EXT	void 	create_hardlinked_copy(cstr newdir, cstr olddir, bool copy_dir_owner=no) THF;

EXT int		set_owner_and_group(cstr path, uid_t=uid_t(-1), gid_t=gid_t(-1), bool deref_last_symlink=yes) noexcept;
EXT int		get_owner_and_group(cstr path, uid_t&, gid_t&, bool deref_last_symlink=yes) noexcept;

EXT	void	delete_node		(cstr path, bool resolve_last_symlink, s_type typ=s_any)THF;
EXT	void	delete_dir		(cstr path, bool recursive=no)							THF;

EXT	void	rename_node		(cstr oldpath, cstr newpath, bool overwrite)			THF;
EXT	void	swap_files		(cstr path1, cstr path2)								THF;

EXT	void	read_dir		(cstr path, MyFileInfoArray&, bool resolve_symlinks)	THF;	// Attn.: no trailing "/" for dir names!
EXT	str		read_link		(cstr path);
EXT	void 	read_file		(cstr path, Array<str>& a, uint32 maxsize=1<<28) 		THF;
INL	void 	read_file		(cstr path, Array<cstr>& a, uint32 maxsize=1<<28) 		THF	{ read_file(path,reinterpret_cast<Array<str>&>(a),maxsize); }
EXT	void 	read_file		(cstr path, class StrArray& a, uint32 maxsize=1<<28) 	THF;

EXT	void	write_file		(cstr path, cptr data, uint32 len, int flags='w'/*overwrite*/, mode_t=0660)	THF;
EXT	void 	write_file		(cstr path, Array<str>& a, int flags='w'/*overwrite*/, mode_t=0660)			THF;

EXT	void	copy_file		(cstr qpath, cstr zpath, bool overwrite) 				THF;


// File mode groups
// for individual bits see stat.h
enum
{
	READABLE	= 0444,		// readable by owner, group & others
	WRITABLE	= 0222,		// writable by owner, group & others
	EXECUTABLE	= 0111,		// executable/listable by owner, group & others
	OWNER		= 0700,
	GROUP		= 0070,
	OTHERS		= 0007,
	NOBODY		= 0000
};

#if defined(_LINUX) || defined(_OPENBSD)
EXT	int	set_file_permissions(cstr path, mode_t who, mode_t perm);
#else
EXT	int	set_file_permissions(cstr path, mode_t who, mode_t perm, bool resolve_last_symlink=yes);
#endif
INL	int	set_file_readable	(cstr path, mode_t who = OWNER|GROUP) { return set_file_permissions(path,who,READABLE); }
INL	int	set_file_writable	(cstr path, mode_t who = OWNER|GROUP) { return set_file_permissions(path,who,WRITABLE); }
INL	int	set_file_executable	(cstr path, mode_t who = OWNER|GROUP) { return set_file_permissions(path,who,EXECUTABLE); }



// ----------------------------------------------------------------------
//					volume info
// ----------------------------------------------------------------------


enum VolumeFlag
{
	vol_wprot	  = 1,
	vol_ejectable = 2,
	vol_mounted	  = 4
};

EXT	uint	get_volume_flags  	(cstr path);			// BSD
EXT	uint64	get_volume_free		(cstr path);			// BSD

class MyVolumeInfo
{
public:
	bool	valid;

// info from struct statfs:
	uint32	blocksize;		// fs.f_bsize			fundamental file system block size
	uint32	bestiosize;		// fs.f_iosize			optimal transfer block size
	uint64	totalblocks;	// fs.f_blocks			total data blocks in file system
	uint64	blocksfree_su;	// fs.f_bfree			free blocks in fs
	uint64	blocksfree;		// fs.f_bavail			free blocks avail to non-superuser
	uint64	totalfiles;		// fs.f_files			total file nodes in file system
	uint64	filesfree;		// fs.f_ffree			free file nodes in fs
	fsid_t	fsid;			// fs.f_fsid 			file system id 					???
	uid_t	owner;			// fs.f_owner			user that mounted the filesystem
	uint	fstype;			// fs.f_type			type of filesystem				number
	uint32	flags;			// fs.f_flags			copy of mount exported flags: 	e.g. MNT_RDONLY
	uint32	fs_subtype;		// fs.f_fssubtype		fs sub-type (flavor)
	cstr	fstypename;		// fs.f_fstypename[MFSTYPENAMELEN]	fs type name
	cstr	mountpoint;		// fs.f_mntonname[MAXPATHLEN]		directory on which mounted
	cstr	volumename;		// fs.f_mntfromname[MAXPATHLEN]		mounted filesystem

public:		MyVolumeInfo	(struct statfs& fs);        // BSD etc.
			MyVolumeInfo    (struct mntent* mntent);    // Linux
			~MyVolumeInfo	();

	uint64	totalbytes		()	const 	{ return blocksize * totalblocks; }		// number: blocksize * blocks
	uint64	bytesfree		()	const 	{ return blocksize * blocksfree; }		// number: blocksize * blocksfree
	uint64	bytesfree_su	()	const 	{ return blocksize * blocksfree_su; }	// number: blocksize * blocksfree_su
};


#include "MyFileInfo.h"

class MyVolumeInfoArray : public Array<MyVolumeInfo*>
{
public:		MyVolumeInfoArray();
};




#undef TPL
#undef SOT
//#undef EXT
//#undef INL










