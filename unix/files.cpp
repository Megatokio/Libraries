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

    2003-08-15 kio	fixed buggy call to ioctl() in SetBlocking() and SetAsync()
    2003-10-09 kio	FullPath(): enhanced to match broken utf-8 filenames
    2004-05-07 kio	FullPath(): added buffer overflow check for initial rpath assembly
    2004-05-08 kio	FullPath(): returned path now has a trailing "/" for a deref'ed, existing directory
    2004-06-11 kio	FullPath(): Bug: always overwrote n.ex. path components with latin1
    2007-02-14 kio	NewTempFile(), DupFile(), filecopy(), lots of new file i/o functions.
*/

#include "config.h"
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/fcntl.h>
#include <sys/mount.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <new>
#include <fnmatch.h>
#include <dirent.h>
#include <time.h>
#include <grp.h>		// BSD, most Linux
#include <sys/types.h>
#include <pwd.h>
#ifdef HAVE_SYS_VFS_H   // Linux
#include <sys/vfs.h>
#endif
#ifdef HAVE_MNTENT_H    // Linux
#include <mntent.h>
#endif
#include "kio/kio.h"
#include "Templates/Array.h"
#include "tempmem.h"
#include "MyFileInfo.h"
#include "pthreads.h"
#include "files.h"



// these shall bummer if assumptions fail:
#ifndef _LINUX
#define	S_IFMT		0170000		/* [XSI] type of file mask */
#define	S_IFIFO		0010000		/* [XSI] named pipe (fifo) */
#define	S_IFCHR		0020000		/* [XSI] character special */
#define	S_IFDIR		0040000		/* [XSI] directory */
#define	S_IFBLK		0060000		/* [XSI] block special */
#define	S_IFREG		0100000		/* [XSI] regular */
#define	S_IFLNK		0120000		/* [XSI] symbolic link */
#define	S_IFSOCK	0140000		/* [XSI] socket */
#ifdef  S_IFWHT
#define	S_IFWHT		0160000		/* whiteout */
#endif
#define	DT_UNKNOWN	 0
#define	DT_FIFO		 1
#define	DT_CHR		 2
#define	DT_DIR		 4
#define	DT_BLK		 6
#define	DT_REG		 8
#define	DT_LNK		10
#define	DT_SOCK		12
#define	DT_WHT		14
#endif


#if !defined(_POSIX_SOURCE) && !defined(_SOLARIS)
    #define M_TIME(TS) ((TS).st_mtimespec.tv_sec)
    #define A_TIME(TS) ((TS).st_atimespec.tv_sec)
    #define C_TIME(TS) ((TS).st_ctimespec.tv_sec)
#else
    #define M_TIME(TS) ((TS).st_mtime /* +(TS).st_mtimesec/1e9 */)
    #define A_TIME(TS) ((TS).st_atime /* +(TS).st_atimesec/1e9 */)
    #define C_TIME(TS) ((TS).st_ctime /* +(TS).st_ctimesec/1e9 */)
#endif



// ----------------------------------------------------------------------
//					Helper
// ----------------------------------------------------------------------


// stat() file
//
static int stat( cstr path, struct stat* buf, bool follow_symlink )
{
    return follow_symlink ? stat(path,buf) : lstat(path,buf);
}


#ifdef USE_LATIN1_PATH
/* ----	convert filename to latin-1 ------------------------- [rewritten: kio 2004-05-10]

        normally when searching a directory for a given filename, we expect both,
        the filename to search and the filenames in the directory, to have the same encoding.

        if both use utf-8 or both use the same 8-bit character set everything is fine.
        even if both use _different_ 8-bit character sets, the application can at least
        access all files, even if the filenames displayed may look weird.

        but if our application internally uses UCS2 or UCS4 encoded strings (unicode)
        then it will convert filenames to utf-8 encoded c-style strings to access the files.

        valid utf-8 strings can not express all arbitrary 8-bit character sequences.
        thus this way some 8-bit character set filenames not only display strange
        but also become completely unreachable for the application.

        latin1str() implements a quirk to find iso-latin-1 encoded filenames in a directory
        which correspond to a utf-8 encoded filename.

    howto:
        1. match your filename against all filenames in the directory of interest without quirks.
        2. if you don't find a match, try to match against latin1str(filename) as a second shot.

    returns:
        returns iso-latin-1 encoded filename or
        returns NULL if conversion did not result in a different filename
*/
cstr latin1str( cstr filename )
{
// does filename contain non-Ascii letters?
// if not, then there is no difference between utf-8 and latin-1 --> quick exit
    cptr q = filename;
    for(;;)
    {
        signed char c = *q++;
        if(c>0)  continue;
        if(c==0) return NULL;				// no non-ascii letters found
        else	 break;						// non-acii letter found
    }

// convert filename to iso-latin-1:
// if filename contains utf-8 character codes >= 256 --> give up
// if filename contains broken utf-8 itself --> give up
// if filename contains illegal overlong encoding --> give up
    while(*q) q++;							// find string end
    str file = tempstr(q-filename);			// temp string
    ptr z = file;
    q = filename;
    for(;;)
    {
        char c = *q++;
        if (c>0)  { *z++ = c; continue; }
        if (c==0) { *z=0; break; }			// conversion utf-8 -> latin-1 complete
    // 2 byte codes:	110xxxxx 10xxxxxx
    // ill. overlong:	1100000x 10xxxxxx
    // latin-1 range:	1100001x 10xxxxxx
        if ((c&0xFE)!=0xC2) return NULL;	// char code too high, ill. overlong encoding or broken utf-8
        uchar c2 = *q++ - 0x80;
        if (c2>=0x3F) return NULL;			// no fup -> broken utf-8
        *z++ = (c<<6) + c2;					// combine c1 + fup
    }

// the resulting latin-1 string may also be a valid utf-8 string again!
// if so, we must not return it:
//
// if we used it, and it actually is an ordinary utf-8 filename,
//		then we access and manipulate wrong files:
//		e.g. we try to delete all files "XY" and unexpectedly also delete all files "Z".
// if we don't use it, though it really is a latin-1 filename,
//		then it is still accessible by the application
//		though it probaly will have a weird filename
//		which even may contain exotic characters which your display font does not provide
//		therefore showing up as the 'character replacement char', frequently a square box.
//
// so using it is dangerous, not using it is just ugly.
// therefore we assert that the converted filename is invalid utf-8 and discard it otherwise.
//
// routine adopted from:
// 		static UCS4Char* ucs4_from_utf8 ( UCS4Char*z, cUTF8CharPtr a, cUTF8CharPtr e, bool latin_1 )
// 		in lib-kio/ustring/sstring.cp
// 		re-implemented to avoid dependency on class String.
    z = file;
    for (;;)
    {
        uint32 c0 = uchar(*z++);
        if ((signed char)c0>0) continue;					// ascii
        if (c0==0) return NULL;						// end of filename reached without invalid utf-8
        if ((signed char)c0 < (signed char)0xc0) return file;		// unexpected fups
        if (c0>=0xfe) return file;					// ill. codes

    // parse multi-byte character:
        uint32 n = 0;		// UCS-4 char code akku
        uint  i = 0;		// UTF-8 character size

        while( c0 & (0x80u>>(++i)) )
        {
            uint32 c1 = uchar(*z++);
            if ((signed char)c1 >= (signed char)0xc0) return file;// no fup: truncated char (end of text or non-fup)
            n = (n<<6) + (c1&0x3F);
        }

    // now: i = total number of digits
    //      n = UCS4 char code without c0 bits
        n += (c0&(0x7f>>i)) << (6*i-6);				// n := UCS4 char code

    // test for ill. overlong encoding:
        if ( n < (1u<<(i*5-4)) ) return file;		// ill. overlong encoding
    }
}
#endif


/* ----	separate filename from path ---------------------------------------
        returned string points into passed path argument  ((guaranteed!))
        returned string is empty if path ended with '/'
*/
cstr filename_from_path( cstr path ) noexcept
{
    assert(path);
    cstr p = strrchr(path,'/');
    return p ? p+1 : path;
}


/* ----	separate filename extension from path -----------------------------
        returned string points into passed path argument  ((guaranteed!))
        returned strings includes '.'
        if path has no valid extension, then the returned string points to chr(0) at end of path
*/
cstr extension_from_path( cstr path ) noexcept
{
    path = filename_from_path(path);
    cstr dot = strrchr(path,'.');
    cstr spc = strrchr(path,' ');
    if( dot>spc ) return dot; else return strchr(path,0);
}


/* ----	separate filename basename from path ------------------------------
        returns string in cstring pool
*/
cstr basename_from_path( cstr path ) noexcept
{
    path = filename_from_path(path);
    return substr( path, extension_from_path(path) );
}


/* ----	separate directory path from path ------------------------------
        returns string in cstring pool
        returned string includes final '/'
        returns "./" instead of "" if passed path contains no '/'
*/
cstr directory_from_path( cstr path ) noexcept
{
    path = substr( path, filename_from_path(path) );
    return *path ? path : "./";
}


/* ----	separate filename or last directory from path ---------------------
        returned string points into passed path argument  ((guaranteed!))
*/
cstr last_component_from_path( cstr path ) noexcept
{
    assert(path);
    cstr p = strchr(path,0) -2;
    while( p>=path && *p!='/' ) { p--; }
    return p>=path ? p+1 : path;
}


// make string from mode bits
//
str modestr( mode_t mode )
{
    str s = binstr(mode," ---------"," rwxrwxrwx");
    s[0] = mode&S_ISUID ? 'u' : mode&S_ISGID ? 'g' : '-';
    return s;
}


// make string from mode bits
// TODO: move this into vipsi! and vicci?
//
str effmodestr( mode_t mode, gid_t file_grp, uid_t file_usr )
{
    gid_t g = getegid();
    uid_t u = geteuid();

// mix bits to 'other' bits:
    if (g==file_grp) mode |= (mode>>3);
    if (u==file_usr) mode |= (mode>>6);
    if (u==0)        mode |= (mode>>6);		// prüfen: ist root==0?

    str s = binstr(mode," ---"," rwx");
    s[0] = mode&S_ISUID ? 'u' : mode&S_ISGID ? 'g' : '-';
    return s;
}


cstr quick_fullpath( cstr path )
{
    if(path[0]=='~' && path[1]=='/') { cstr h=homedirpath(); if(h) path = catstr(h,path+1); }
    if(path[0]!='/')				 { path = catstr(workingdirpath(),"/",path); }
    return path;
}


/* ---- real path ----------------------------------

        resolve partial paths
        resolve "//" and "./" and "../"
        resolve ~
        resolve symbolic links
        resolve non-utf-8 filenames

        returns temp str in tempmem pool
        errno: cleared or set to error

    on success:
        if the input path ends with "/", then the returned path always refers to a directory.
        if the returned path refers (not) to a directory it always ends (not) on a "/".

    on error:
        ENOTDIR		encountered non-directory:		path -> node (file, symlink, ...)
        ELOOP		infinite symlink loop:			path -> symlink
        ENOENT		last component does not exist	path = path suitable for creation.
*/
str fullpath( cstr _path, bool resolve_last_symlink, bool auto_create_path )
{
    errno = noerror;

    // sanity:
    if (!_path||!*_path) { errno=ENOTDIR; return emptystr; }
    str path = dupstr(_path);

    // path starts with '~'  -->  prepend home directory
    if( path[0]=='~' && (path[1]==0||path[1]=='/') )
    {
        cstr home = getenv("HOME");
        if(home) path = catstr(home,path+1);
    }

    // not an absolute path  -->  prepend local directory
    if( path[0]!='/' )
    {
        char bu[MAXPATHLEN];
        cstr wd = getcwd(bu,MAXPATHLEN);
        if(!wd) { if(errno==ENOENT) errno=ENOTDIR; return path; }	// failed. errno set
        assert(wd[0]=='/');
        path = lastchar(wd)!='/' ? catstr(wd,"/",path) : catstr(wd,path);
    }

    int lc = 33;		// link counter -> detect recursion
    int i  = 0;			// path[i] = "/";  path[0..i] = checked
    int j;				// path[j] = next "/"
    ptr p;				// p -> next "/"
    char c;				// c = *p

// loop over existing directories:

    for(;;)
    {
        p = strchr(path+i+1,'/');

        if(p)		// path component:		check for // ./ and ../
        {
            j = int(p-path);		// j=index of next '/'

        // discard "//"
            if(j==i+1) { memmove(p,p+1,strlen(p)); continue; }

        // discard "./"
            if(j==i+2 && path[i+1]=='.') { memmove(p-1,p+1,strlen(p)); continue; }

        // "../" -> backup one directory
            if( j==i+3 && path[i+1]=='.' && path[i+2]=='.' )
            {
                if(i==0) { errno=ENOTDIR; return path; }	// for security reasons: fail!
                do{i--;}while(path[i]!='/');
                memmove(path+i,p,strlen(p)+1); continue;
            }
        }
        else		// final component:		check for . and ..
        {
            p = strchr(path+i,0);
            j = int(p-path);
            if(j==i+1) return path;							// path ends on "/"
            if(j==i+2 && path[i+1]=='.') { path[i+1]=0; return path; }	// path ended on "/."
            if(j==i+3 && path[i+1]=='.' && path[i+2]=='.')	// path ended on "/.."
            {
                if(i==0) { errno=ENOTDIR; return path; }	// for security reasons: fail!
                do{i--;}while(path[i]!='/');
                path[i+1]=0; return path;
            }
        }

    // temporarily truncate path at "/" (or final 0):
        c = *p;		// c = 0 or '/'
        *p = 0;

    // does path component exist?
        struct stat statdata;
        errno = ok;
        lstat(path,&statdata);

        if(errno)
        {
            if(errno!=ENOENT) return path;			// other error

#ifdef USE_LATIN1_PATH
            cstr l1path = Latin1(path+i);			// try iso-latin-1
            if (l1path)								// second chance:
            {
                path[i]=0; l1path = catstr(path,l1path); path[i]='/';
                errno=ok; lstat(l1path,&statdata);	// success? => use latin1 path
                if(errno==ok) { *p=c; path = catstr(l1path,p); continue; }
            }
            if(errno==ENOENT)
#endif
            {
                if(c==0) return path;				// final component
                if(!auto_create_path) { errno=ENOTDIR; return path; }
                errno=ok; mkdir(path,0777);
                if(errno) { if(errno==ENOENT) errno=ENOTDIR; return path; }
                i=j; *p=c; continue;
            }
        }

    // handle directory:
        if(S_ISDIR(statdata.st_mode)) { if(c) { i=j; *p=c; continue; } else { return catstr(path,"/"); } }

    // handle link:
        if(S_ISLNK(statdata.st_mode))
        {
            if(c==0 && !resolve_last_symlink) return path;	// path resolves to symlink
            if (--lc==0) { errno=ELOOP; return path; }		// too many recursions
            char bu[MAXPATHLEN+1];
            int n = int(readlink(path, bu, MAXPATHLEN));
            if (n<0) { if(errno==ENOENT) errno=ENOTDIR; return path; }	// error
            bu[n]=0; *p=c;
            if(bu[0]=='/') { path=catstr(bu,p); i=0; continue; }
            else { path[i+1]=0; path=catstr(path,bu,p); continue; }
        }

    // handle regular file ((or pipe et.al.))
        if(c) errno=ENOTDIR;
        return path;
    }
}


/* ----	find executable ---------------------
	returns the best guess … does not guarantee that the returned path is valid or executable
	returns NULL if nothing found
*/
cstr find_executable( cstr name )
{
	cstr s = quick_fullpath(name);			// full or partial path?
	if(is_file(s) && is_executable(s)) return s;

	if(!strchr(name,'/'))					// just the name?
	{										// -> search in $PATH
		Array<str> ss;
		split(ss, getenv("PATH"), ':');
		for(uint i=0; i<ss.count(); i++)
		{
			cstr s = catstr(ss[i],"/",name);
			if(is_file(s) && is_executable(s)) return s;
		}
	}

	return NULL;							// not found
}


/* ----	working directory --------------------------------------
*/
void change_working_dir( cstr path )
{
    if(!path||!*path)       { errno=ENOTDIR; return; }
//	if(lastchar(path)!='/') { errno=ENOTDIR; return; }
    if(chdir(path)) return;     // errno set
    path = fullpath(path);
    if(errno==ok) {int r = chdir(path); (void)r; }	// will probably fail again
}

// get working directory or NULL
cstr workingdirpath()
{
	char s[MAXPATHLEN+1];
	s[MAXPATHLEN] = 0;
	return dupstr( getcwd(s,MAXPATHLEN) );
}

// get user home directory or NULL
cstr homedirpath()
{
    return getenv("HOME");
}

// get temp directory or NULL			2016-01-27
cstr tempdirpath()
{
	// ISO/IEC 9945 (POSIX):
	//	The path supplied by the first environment variable found in the list
	//	TMPDIR, TMP, TEMP, TEMPDIR. If none of these are found, "/tmp".
	static cstr envnames[] = { "TMPDIR", "TMP", "TEMP", "TEMPDIR" };

    for(uint i=0;i<7;i++)
    {
		cstr tmpdir = i<4 ? getenv(envnames[i])		// one of the well-known env variables
					: i<5 ? "/tmp"					// last chance for a temp dir
					: i<6 ? homedirpath()			// last chance for any dir
					:       workingdirpath();		// even try to use the cwd…
		if(tmpdir && is_dir(tmpdir) && is_writable(tmpdir)) return tmpdir;
    }

	xlogline("tempdirpath(): no tempdir found!");
	return NULL;
}


/* ----	provide a temp file name --------------
        temp file must be renameable to 'file'.
*/
cstr tempfilepath( cstr file )
{
    assert( file!=NULL );

    if(file[0]!='/') file=quick_fullpath(file);

    for(uint n=0;;n++)
    {
		cstr zfile = catstr(file,".temp",tostr(n));
		if(exists_node(zfile,no)) continue;
		else return zfile;
    }
}




// ----------------------------------------------------------------------
//					file info
// ----------------------------------------------------------------------


s_type classify_file ( cstr path, bool follow_symlink )
{
    if (!path) path="";		// --> ENOENT
    struct stat fs;
    if( stat(path,&fs,follow_symlink) ) return s_none;		// errno set!
    else return classify_file(fs.st_mode);
}


/* ---- modification date of file ------------------
*/
time_t file_mtime( cstr path, bool follow_symlink )
{
    struct stat fs;
    stat(path,&fs,follow_symlink);
    return M_TIME(fs);
}


/* ---- last access date of file ------------------
*/
time_t file_atime( cstr path, bool follow_symlink )
{
    struct stat fs;
    stat(path,&fs,follow_symlink);
    return A_TIME(fs);
}


/* ---- last status change time of file ------------------
*/
time_t file_ctime( cstr path, bool follow_symlink )
{
    struct stat fs;
    stat(path,&fs,follow_symlink);
    return C_TIME(fs);
}


off_t file_size( cstr path, bool follow_symlink )
{
    struct stat fs;
    if (!path||!*path) return -1;
    if (stat(path,&fs,follow_symlink)) return -1;	// error
    return fs.st_size;
}


/*	get list of groups for user uid				2015-01-24	kio
	on error:   returns NULL
	on success: returns 0-terminated list of groups allocated with malloc()
	sets errno
*/
gid_t* get_groups(uid_t uid)
{
//	pwd.h:
//	int getpwuid_r( uid_t uid, struct passwd *pwd, char *buf, size_t buflen, struct passwd **result);
//	struct passwd {			 /* LINUX! BSD has more fields! */
//    char   *pw_name;       /* username */
//    char   *pw_passwd;     /* user password */
//    uid_t   pw_uid;        /* user ID */
//    gid_t   pw_gid;        /* group ID */
//    char   *pw_gecos;      /* user information */
//    char   *pw_dir;        /* home directory */
//    char   *pw_shell;      /* shell program */
//	};

	struct passwd pwd, *pwdptr;
	int size = 400;
a:	int err; char bu[size];
	do { err = getpwuid_r(uid,&pwd,bu,size,&pwdptr); } while(err==EINTR);
	if(err==ERANGE) { if(size>8000) { errno=ERANGE; return NULL; } else { size*=2; goto a; } }
	if(err) return NULL;
	if(pwdptr==NULL) { errno=ENOENT; return NULL; }	// zu dieser UID gibt es keinen Eintrag

//	grp.h:
//	int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups);

	int ngroups = 50;
	gid_t* groups = NULL;
	do { free(groups); groups = (gid_t*)malloc((ngroups+1)*sizeof(int)); }
#if _MACOSX
	while( getgrouplist(pwd.pw_name, pwd.pw_gid, (int*)groups, &ngroups) == -1 );
	static_assert(sizeof(gid_t)==sizeof(int),"sizeof(gid_t)==sizeof(int)");
#else	// _LINUX, _BSD
	while( getgrouplist(pwd.pw_name, pwd.pw_gid, groups, &ngroups) == -1 );
#endif
	groups[ngroups] = 0;
	errno = ok;
	return groups;
}


static pthread_mutex_t is_in_group_mutex;
ON_INIT([]{ IFDEBUG( int e = ) pthread_mutex_init(&is_in_group_mutex, NULL); assert(e==0); });


/*	test whether a user is in the specified group				2015-01-24	kio
	the groups list for that user is cached for some seconds
	the returned result is not 100% reliable
*/
static bool is_in_group(uid_t uid, gid_t gid)
{
	bool f = yes;
	IFDEBUG( int e = ) pthread_mutex_lock(&is_in_group_mutex);	assert(!e);

	static uid_t  s_uid  = 0;		// cached user
	static gid_t* s_gids = NULL;	// cached groups[]
	static time_t s_when = 0;		// when groups[] was retrieved

	if(uid!=s_uid || s_gids==NULL || time(NULL)>=s_when+5)
	{
		free(s_gids);
		s_gids = NULL;
		s_uid  = uid;
		s_when = time(NULL);
		s_gids = get_groups(uid); if(s_gids==NULL) goto x0;
	}

	for(gid_t* p=s_gids; *p; p++) { if(*p==gid) goto x1; }
	goto x0;

x0:	f = no;
x1:	IFDEBUG( e = ) pthread_mutex_unlock(&is_in_group_mutex);		assert(!e);
	return f;
}


/*	test whether a file is writable for the current user				2015-01-24	kio reworked
	args = the file's permissions, group and owner
	current user = geteuid()
	the returned result is not 100% reliable
*/
inline bool is_writable( mode_t mode, gid_t gid, uid_t uid)
{
    // File mode
    // Read, write, execute/search by owner
    // #define	S_IRWXU		0000700		/* [XSI] RWX mask for owner */
    // #define	S_IRUSR		0000400		/* [XSI] R for owner */
    // #define	S_IWUSR		0000200		/* [XSI] W for owner */
    // #define	S_IXUSR		0000100		/* [XSI] X for owner */
    // Read, write, execute/search by group
    // #define	S_IRWXG		0000070		/* [XSI] RWX mask for group */
    // #define	S_IRGRP		0000040		/* [XSI] R for group */
    // #define	S_IWGRP		0000020		/* [XSI] W for group */
    // #define	S_IXGRP		0000010		/* [XSI] X for group */
    // Read, write, execute/search by others
    // #define	S_IRWXO		0000007		/* [XSI] RWX mask for other */
    // #define	S_IROTH		0000004		/* [XSI] R for other */
    // #define	S_IWOTH		0000002		/* [XSI] W for other */
    // #define	S_IXOTH		0000001		/* [XSI] X for other */

	//	Note: Linux:
	//	if user == file's owner then *only* check user permissiones else
	//	if user in file's group then *only* check group permissions else
	//	check world permissions

	uid_t euid = geteuid();
	return euid==uid ? mode & S_IWUSR : getegid()==gid || is_in_group(euid,gid) ? mode & S_IWGRP : mode & S_IWOTH;
}

/*	test whether the file is writable for the current user
	current user = geteuid()
	the returned result is not 100% reliable
*/
bool is_writable( cstr path, bool resolve_last_symlink )
{
    struct stat data;
    if( stat(path, &data, resolve_last_symlink) ) return no;	// error
    else return is_writable(data.st_mode, data.st_gid, data.st_uid);
}

/*	test whether a file is readable for the current user				2015-01-24	kio reworked
	args = the file's permissions, group and owner
	current user = geteuid()
	the returned result is not 100% reliable
*/
inline bool is_readable( mode_t mode, gid_t gid, uid_t uid)
{
	uid_t euid = geteuid();
	return euid==uid ? mode & S_IRUSR : getegid()==gid || is_in_group(euid,gid) ? mode & S_IRGRP : mode & S_IROTH;
}

/*	test whether the file is readable for the current user
	current user = geteuid()
	the returned result is not 100% reliable
*/
bool is_readable( cstr path, bool resolve_last_symlink )
{
    struct stat data;
    if( stat(path, &data, resolve_last_symlink) ) return no;	// error
    else return is_readable(data.st_mode, data.st_gid, data.st_uid);
}

/*	test whether a file is executable for the current user				2015-01-24	kio reworked
	args = the file's permissions, group and owner
	current user = geteuid()
	the returned result is not 100% reliable
*/
inline bool is_executable( mode_t mode, gid_t gid, uid_t uid)
{
	uid_t euid = geteuid();
	return euid==uid ? mode & S_IXUSR : getegid()==gid || is_in_group(euid,gid) ? mode & S_IXGRP : mode & S_IXOTH;
}

/*	test whether the file is executable									2015-01-24	kio added flag for_user
	NOTE: the default is 'for anybody' and not 'for the current user'
		  as used in is_writable() or is_readable()
	for_user = 0: (default)
		return whether executable flag is set for any of user, group or world
	for_user = 1:
		current user = geteuid()
		the returned result is not 100% reliable
*/
bool is_executable( cstr path, bool resolve_last_symlink, bool for_user )
{
    struct stat data;
    if( stat(path, &data, resolve_last_symlink) ) return no;	// error
    if(for_user) return data.st_mode & (S_IXOTH|S_IXGRP|S_IXUSR);
    return is_executable(data.st_mode, data.st_gid, data.st_uid);
}


// ----------------------------------------------------------------------
//					create, delete, read & write file
// ----------------------------------------------------------------------


void create_file(cstr path, int mode) THF
{
    FD(path,O_CREAT,mode);
}


/* ----	create directory -------------------------------------
        in:	 mode = filemode
        throws on error:
             ENOTDIR	encountered non-directory
             ELOOP		infinite symlink loop
*/
void create_dir( cstr path, int mode, bool autocreatedirs ) THF
{
    while(path&&lastchar(path)=='/') path = substr(path,strchr(path,0)-1);

    path = fullpath(path,yes/*follow_symlink*/,autocreatedirs);

    if(errno==ok)						// node exists
    {
        if(is_dir(path,yes)) return;	// dir already exists
        else errno = ENOTDIR;			// exists, but not a dir
    }
    else if(errno==ENOENT)				// final path component does not exist
    {
        if(mkdir(path,mode)==0) return;	// create dir or
    }

    throw file_error(path,errno, "create dir");
}


/* ----	create pipe -------------------------------------
        in:	 mode = filemode
        throws on error:
             EEXIST		Directory already exists
             ENOTDIR	encountered non-directory
             ELOOP		infinite symlink loop
*/
void create_pipe( cstr path, int mode ) THF
{
    path = fullpath(path,yes/*follow_symlink*/);

    if(errno==ok || errno==ENOENT)
    {
        if(mkfifo(path,mode)==0) return; // ok
    }

    throw file_error(path,errno,"create pipe");
}


/* ---- create symbolic link ------------------------------------------
        create a symbolic link to 'dest' at position 'path'
        overwrites old link, if present
        in:	 path, dest
        throws on error
*/
void create_link( cstr path, cstr dest ) THF
{
    path = fullpath(path,no);
    if(errno && errno!=ENOENT) goto x;

    if(errno==ok)	// node exists?
    {
        struct stat fs;
        if( lstat(path,&fs)==0 && S_ISLNK(fs.st_mode) ) if(unlink(path)) goto x;
    }

    if(symlink(dest,path)==0)	// may fail if node still exists (e.g. dir or file)
        return;	// ok

x:	throw file_error(path,errno, "create symlink");
}


/* ---- create hard link ------------------------------------------
        create another hard link to 'dest' at position 'path'
        overwrites old link, if present
        in:	 path, dest
        throws on error
*/
void create_hardlink( cstr zpath, cstr qpath ) THF
{
    zpath = fullpath(zpath,no);
    if(errno==ok) errno = EEXIST;						// node exists?
    if(errno==ENOENT && link(qpath,zpath)==0) return;	// ok

    throw file_error(zpath,errno,"create hardlink");
}


/*	create copy of directory tree
        files are hard linked
        symlinks are copied
        directories are copied
        other node types are ignored
*/
void create_hardlinked_copy(char const* zpath, char const* qpath) THF
{
    TempMemPool z;
    MyFileInfoArray dir;

    try
    {
        if(lastchar(zpath)!='/') zpath = catstr(zpath,"/");
        if(lastchar(qpath)!='/') qpath = catstr(qpath,"/");

        read_dir(qpath,dir,no);

        create_dir(zpath);

        for(uint i=0;i<dir.count();i++)
        {
            MyFileInfo& fi = dir[i];
            cstr q = catstr(qpath,fi.fname());
            cstr z = catstr(zpath,fi.fname());
            if(fi.is_dir())  { create_hardlinked_copy(z,q); continue; }
            if(fi.is_file()) { create_hardlink(z,q); continue; }
            if(fi.is_link()) { create_link(z,read_link(q)); }
            // else: ignore
        }
    }
    catch(file_error& e)
    {
        logline("create_hardlinked_copy dir size = %u, dir = %s", dir.count(),qpath);
        throw std::move(e);
    }
}


/* ----	delete file or folder -----------------------------------------
*/
void delete_node( cstr path, bool follow_symlink, s_type typ ) THF
{
    path = fullpath(path,follow_symlink);
    if(errno) goto x;

    if(typ!=s_any)
    {
        s_type t = classify_file(path,no);
        if(t==s_none) goto x;
        if(t!=typ) { errno=wrongfiletype; goto x; }
    }
    if(remove(path)==0) return;

x:	throw file_error(path,errno,"delete node");
}


// Helper:
// remove directory and all contents
// does not follow symlinks
// returns: 0 = ok,    errno void
// 	  	   -1 = error, errno set
//	uses tempmem
//
static int remove_tree( cstr dpath )
{
    DIR* dir = opendir(dpath);
    if (!dir) return -1;

    struct dirent* direntry;
    int rval = 0;
    if(lastchar(dpath)!='/') dpath = catstr(dpath,"/");

    while( (direntry=readdir(dir)) )
    {
        cstr fname = direntry->d_name;
        cptr p = fname; if(*p++=='.') { if(p[*p=='.']==0) continue; }	// skip "." and ".."

        struct stat fstat;
        cstr fpath = catstr(dpath,fname);
        if( lstat(fpath,&fstat) ) { rval=-1; continue; }
        rval |= S_ISDIR(fstat.st_mode) ? remove_tree(fpath) : remove(fpath);
    }
    closedir(dir);
    if(rval==0) rval = rmdir(dpath);
    return rval;
}


/* ----	delete directory / tree -----------------------------------------
        throws on error
*/
void delete_dir( cstr path, bool fulltree ) THF
{
    path = fullpath(path,yes/*follow_symlink*/);
    if(errno) goto x;

    if(path[0]=='/' && path[1]==0) { errno=EPERM; goto x; }

    if(rmdir(path)==0 || (errno==ENOTEMPTY && fulltree && remove_tree(path)==0) ) return; // ok

x:	throw file_error(path,errno,"delete dir");
}


/* ----	rename link, file or folder -----------------------------------------
        auto-creates newpath
        if oldpath is a symlink, then the link is renamed
        if newpath exists
            it will be atomically replaced
            if it is a dir, then it must be empty
            if it is a symlink, then the link will be overwritten
*/
void rename_node( cstr oldpath, cstr newpath, bool overwrite ) THF
{
    oldpath = fullpath(oldpath,no/*!follow_symlink*/);
    if(errno) goto x;

    newpath = fullpath(newpath,no/*!follow_symlink*/,/*create_path*/yes);
    if(errno==ok && !overwrite) { errno=EEXIST; goto x; }

    if(errno==ok || errno==ENOENT)
        if(rename(oldpath,newpath)==0) return;			// renames files, dirs and symlinks!

x:	throw file_error(oldpath,errno,usingstr("rename to \"%s\"",newpath));
}


/* ----	swap two files ------------------------------------------
        throws on error
*/
void swap_files( cstr path1, cstr path2 ) THF
{
    path1 = fullpath(path1,1);	if(errno) goto x;
    path2 = fullpath(path2,1);	if(errno) goto x;

    {	cstr zpath = tempfilepath(path2); assert(errno==ok);		// must work
        rename(path1,zpath);		if(errno) goto x;				//failed
        rename(path2,path1);		if(errno) { int e=errno; rename(zpath,path1); errno=e; goto x; }	// failed
        rename(zpath,path2);    	assert(errno==ok);			// must work
        return; // ok
    }

x:	throw file_error(path1,errno, usingstr("swap with file \"%s\"",path2));
}


/* ----	read directory into vector variable -------------------
        path must refer to a directory
        or an optional pattern may be appended
        throws on error accessing the dir itself
        names of sub directories in this directory have no trailing '/'
*/
void read_dir( cstr path, MyFileInfoArray& v, bool resolve_symlinks ) THF
{
    v.purge();
    path = fullpath(path,yes);
    if(errno && errno!=ENOENT)
    {
        x: throw file_error(path,errno, "read dir");
    }

    cstr pattern = NULL;
    if (lastchar(path)!='/')
    {
        assert(!is_dir(path));				// note: sets sporadic errors
        pattern = filename_from_path(path);
        path    = directory_from_path(path);
    }

    DIR* dir = opendir(path);	  				// note: sets sporadic errors
    if(!dir) goto x; 							// error

    uint pathlen = uint(strlen(path));
    str filepath = tempstr(pathlen+1024);
    memcpy(filepath,path,pathlen);

    for(;;)
    {
        errno = ok;
        dirent* direntry = readdir(dir);
        if(direntry==NULL) break;

        cstr filename = direntry->d_name;
        if(filename[0]=='.' && ( filename[1]==0 || (filename[1]=='.' && filename[2]==0) )) continue;	// "." or ".."
        if(pattern && fnmatch(pattern,filename,FNM_NOESCAPE)) continue;									// pattern does not match
        uint filenamelen = uint(strlen(filename));
        if(filenamelen>1024) continue;				// fail safe		((cannot happen on DARWIN machines))

        memcpy(filepath+pathlen,filename,filenamelen+1);
        struct stat filestat;
        if(lstat(filepath,&filestat)) continue; 	// skip on error

        if(S_ISLNK(filestat.st_mode) && resolve_symlinks)
        {
            if(stat(filepath,&filestat))			// get info about derefed file
                lstat(filepath,&filestat);			// on error (dead link) fallback on link itself
        }

//		if(S_ISDIR(filestat.st_mode)) filename = catstr(filename,"/");

        v.grow();
        MyFileInfo& w = v[v.count()-1];

        w.set_fname(filename);
        w.length = filestat.st_size;
        w.type   = uint16(filestat.st_mode) >> 12;
        w.perm   = filestat.st_mode & 0x0FFF;
        w.uid    = filestat.st_uid;
        w.gid    = filestat.st_gid;
        w.mtime  = M_TIME(filestat);
        w.atime  = A_TIME(filestat);
        w.ctime  = C_TIME(filestat);
        w.links	 = filestat.st_nlink;
    }

    closedir(dir);
}


/* ----	get link target ----------------------------------------
        returns NULL on error and errno set
*/
str read_link( cstr path )
{
    path = fullpath(path,no,no);
    if(errno) return NULL;

    if(is_link(path))
    {
        char bu[MAXPATHLEN];
        int n = (int)readlink( path, bu, MAXPATHLEN );
        if(n>=0) { str s=tempstr(n); memcpy(s,bu,n); return s; }		// filesystem encoding
    }
    else errno = EINVAL;
    return NULL;
}


void read_file(cstr path, Array<str>& a, uint32 maxsize) THF
{
    FD fd(path,'r');
    fd.read_file(a,maxsize);
}

void read_file(cstr path, StrArray& a, uint32 maxsize) THF
{
    FD fd(path,'r');
    fd.read_file(a,maxsize);
}


/* ----	create regular file ---------------------------------------
        in: flags = 'n'=new, 'w'=write or 'a'=append or combination of O_WRONLY, O_CREAT, O_APPEND etc.
*/
void write_file( cstr path, cptr data, uint32 cnt, int flags, int mode ) THF
{
    FD fd(path,flags,mode);
    fd.write_bytes(data,cnt);
    fd.close_file();
}

void write_file(cstr path, Array<str>& a, int flags, int mode) THF
{
    FD fd(path,flags,mode);
    fd.write_file(a);
    fd.close_file();
}


void copy_file(cstr qpath, cstr zpath, bool overwrite) THF
{
    FD q(qpath,'r');
    FD z(zpath, overwrite ? 'w' : 'n');
    copy(q,z,q.file_size());
}


#ifdef _LINUX
int set_file_permissions(cstr path, mode_t who, mode_t what)
{
    path = quick_fullpath(path);

    struct stat fs;
    int err = stat(path,&fs);
    if(err) return errno;

    mode_t mode = fs.st_mode & ~what;
    mode |= who & what;

    if(mode==fs.st_mode) return ok;
    err = chmod(path,mode);
    return err ? errno : ok;
}
#else
int set_file_permissions(cstr path, mode_t who, mode_t what, bool deref_last_symlink)
{
    path = quick_fullpath(path);

    struct stat fs;
    int err = deref_last_symlink ? stat(path,&fs) : lstat(path,&fs);
    if(err) return errno;

    mode_t mode = fs.st_mode & ~what;
    mode |= who & what;

    if(mode==fs.st_mode) return ok;
    err = deref_last_symlink ? chmod(path,mode) : lchmod(path,mode);
    return err ? errno : ok;
}
#endif



// ----------------------------------------------------------------------
//					volume info
// ----------------------------------------------------------------------


#ifndef _LINUX

uint get_volume_flags( cstr path )
{
    struct statfs fs;
    int n = statfs(fullpath(path,1,0),&fs);
    if(n) return vol_wprot;						// error: errno set

    uint rval = 0;
    if(  fs.f_flags & MNT_RDONLY							  ) rval |= vol_wprot;
#ifdef MNT_NODEV
    if( (fs.f_flags & MNT_NOSUID) && (fs.f_flags & MNT_NODEV) ) rval |= vol_ejectable;
#endif
    if(  fs.f_blocks>0										  ) rval |= vol_mounted;
    errno = noerror;
    return rval;
}

uint64 get_volume_free( cstr path )
{
    struct statfs fs;
    int n = statfs(fullpath(path,1,0),&fs);
    if(n) return 0;		// errno set
    errno = noerror;
    return (uint64)fs.f_bsize * (uint64)fs.f_bavail;
}
#endif



MyVolumeInfo::~MyVolumeInfo()
{
	delete[] fstypename;
	delete[] mountpoint;
	delete[] volumename;
}


#if defined(_BSD)
MyVolumeInfo::MyVolumeInfo(struct statfs& fs)
:
    valid(no),
    fstypename(NULL),
    mountpoint(NULL),
    volumename(NULL)
{
    if( fs.f_files+fs.f_ffree==0 ) return;			// special: volfs, free automount slots

    cstr mpath = fs.f_mntonname;					// directory on which mounted
    cstr mdev  = fs.f_mntfromname;					// mounted filesystem

    if(eq(mpath,"/dev")) return;					// devfs, fdesc
    if(strchr(mdev,'/')==NULL) return;				// "none", "usbdevfs", etc.

// info from struct statfs:
    this->blocksize 	= fs.f_bsize;
    this->bestiosize 	= fs.f_iosize;
    this->totalblocks	= fs.f_blocks;
    this->blocksfree_su = fs.f_bfree;
    this->blocksfree 	= fs.f_bavail;
    this->totalfiles 	= fs.f_files;
    this->filesfree 	= fs.f_ffree;
    this->fsid 			= fs.f_fsid;
    this->owner 		= fs.f_owner;
    this->fstype 		= fs.f_type;
    this->flags			= fs.f_flags;
#if __DARWIN_64_BIT_INO_T
    this->fs_subtype	= fs.f_fssubtype;
#else
    this->fs_subtype	= 0;						// e.g. FreeBSD: has no f_fssubtype
#endif
    this->fstypename	= newcopy(fs.f_fstypename);
    this->mountpoint	= newcopy(fs.f_mntonname);
    this->volumename	= newcopy(fs.f_mntfromname);

    this->valid 		= yes;
}


MyVolumeInfoArray::MyVolumeInfoArray()
{
    struct statfs* info;
    int n = getmntinfo(&info, 1?MNT_WAIT:MNT_NOWAIT);

    for(int i=0;i<n;i++)
    {
        MyVolumeInfo* vi = new MyVolumeInfo(info[i]);
        if(vi->valid) append(vi);
        else delete vi;
    }
}
#endif	// _BSD


#ifdef _LINUX
MyVolumeInfoArray::MyVolumeInfoArray()
{
    FILE* file = setmntent("/etc/mtab", "r");

    while(!feof(file) && !ferror(file))
    {
        struct mntent* mntent = getmntent(file);
        if(mntent==NULL) break;

        MyVolumeInfo* vi = new MyVolumeInfo(mntent);
        if(vi->valid) append(vi);
    }

    endmntent(file);
}

MyVolumeInfo::MyVolumeInfo(struct mntent* mntent)
:
    valid(no),
    fstypename(NULL),
    mountpoint(NULL),
    volumename(NULL)
{
    cstr mpath = mntent->mnt_dir;					// directory on which mounted
    cstr mdev  = mntent->mnt_fsname;                // mounted filesystem

    if(eq(mpath,"/dev")) return;					// devfs, fdesc
    if(strchr(mdev,'/')==NULL) return;				// "none", "usbdevfs", etc.

    struct statfs info;
    if(statfs(mntent->mnt_dir,&info)) return;

//  mntent:
//        char* mnt_fsname;		/* Device or server for filesystem.  */
//        char* mnt_dir;		/* Directory mounted on.  */
//        char* mnt_type;		/* Type of filesystem: ufs, nfs, etc.  */
//        char* mnt_opts;		/* Comma-separated options for fs.  */
//        int   mnt_freq;       /* Dump frequency (in days).  */
//        int   mnt_passno;		/* Pass number for `fsck'.  */

//  statfs:
//    __fsword_t    f_type;     /* type of file system (see linux.die.net) */
//    __fsword_t    f_bsize;    /* optimal transfer block size */
//    __fsblkcnt_t  f_blocks;   /* total data blocks in file system */
//    __fsblkcnt_t  f_bfree;    /* free blocks in fs */
//    __fsblkcnt_t  f_bavail;   /* free blocks available to unprivileged user */
//    __fsfilcnt_t  f_files;    /* total file nodes in file system */
//    __fsfilcnt_t  f_ffree;    /* free file nodes in fs */
//    __fsid_t      f_fsid;     /* file system id */
//    __fsword_t    f_namelen;  /* maximum length of filenames */
//    __fsword_t    f_frsize;   /* fragment size (since Linux 2.6) */
//    __fsword_t    f_flags;
//    __fsword_t    f_spare[4];

    if( info.f_files+info.f_ffree==0 ) return;			// special: volfs, free automount slots

// info from struct statfs:
    this->blocksize 	= info.f_bsize;             // fundamental device block size
    this->bestiosize 	= info.f_bsize;             // best i/o block size
    this->totalblocks	= info.f_blocks;
    this->blocksfree_su = info.f_bfree;
    this->blocksfree 	= info.f_bavail;
    this->totalfiles 	= info.f_files;
    this->filesfree 	= info.f_ffree;
    this->fsid 			= info.f_fsid;
    this->owner 		= 0; // info.f_owner;       // user that mounted the filesystem
    this->fstype 		= info.f_type;
    this->flags			= info.f_flags;
    this->fs_subtype	= 0; // info.f_fssubtype;   // fs sub-type (flavor)
    this->fstypename	= newcopy(mntent->mnt_type);
    this->mountpoint	= newcopy(mpath);
    this->volumename	= newcopy(mdev);

    this->valid 		= yes;
}
#endif	// _LINUX























































