// Copyright (c) 2022 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"
#include <memory>

namespace kio
{

using SIZE = uint; // byte count for read/write/erase
#if defined DEVICES_LARGE_FILE_SUPPORT && DEVICES_LARGE_FILE_SUPPORT
using ADDR = uint64;
#else
using ADDR = uint32; // uint32/64: disk size, file size, file position. exFAT needs uint64
#endif
using LBA = uint32; // up to 2TB

using Error = cstr;

class FileSystem;
using FileSystemPtr = std::shared_ptr<FileSystem>;
class Directory;
using DirectoryPtr = std::shared_ptr<Directory>;
class File;
using FilePtr = std::shared_ptr<File>;


constexpr char NOT_WRITABLE[]		   = "not writable";
constexpr char NOT_READABLE[]		   = "not readable";
constexpr char NOT_ERASABLE[]		   = "not erasable";
constexpr char END_OF_FILE[]		   = "end of file";
constexpr char TIMEOUT[]			   = "timeout";
constexpr char INVALID_ARGUMENT[]	   = "invalid argument";	  // ioctl()
constexpr char DEVICE_NOT_RESPONDING[] = "Device not responding"; // block devices
constexpr char HARD_WRITE_ERROR[]	   = "Hard write error";	  // block devices
constexpr char HARD_READ_ERROR[]	   = "Hard read error";		  // block devices
constexpr char FILE_NOT_FOUND[]		   = "File not found";
constexpr char DIRECTORY_NOT_FOUND[]   = "Directory not found";
constexpr char UNKNOWN_FILESYSTEM[]	   = "unknown file system";		  // mount(), makeFS()
constexpr char UNKNOWN_DEVICE[]		   = "unknown device";			  // mount()
constexpr char DEVICE_IN_USE[]		   = "device in use";			  // mount(), new FileSystem()
constexpr char NO_MOUNTPOINT_FREE[]	   = "no mountpoint free";		  // mount()
constexpr char NO_WORKING_DEVICE[]	   = "no current working device"; //
constexpr char NAME_TOO_LONG[]		   = "name too long";			  // mount()
constexpr char REGULAR_FILE[]		   = "not a directory";			  // remove()


enum FileOpenMode : uint8 {
	READ	  = 1 + 16, // Open for reading, implies EXIST
	WRITE	  = 2 + 32, // Open for writing, implies TRUNCATE
	READWRITE = 3,		// Open for reading and writing
	APPEND	  = 4 + 2,	// Open for writing at end of file
	NEW		  = 8,		// flag: file must be new
	EXIST	  = 16,		// flag: file must exist
	TRUNCATE  = 32,		// flag: truncate existing file
};

inline constexpr FileOpenMode operator|(FileOpenMode a, FileOpenMode b) { return FileOpenMode(a | ~~b); }
inline constexpr FileOpenMode operator+(FileOpenMode a, FileOpenMode b) { return FileOpenMode(a | ~~b); }
inline constexpr FileOpenMode operator-(FileOpenMode a, FileOpenMode b) { return FileOpenMode(a & ~b); }


enum FileType : uint8 {
	NoFile		  = 0,
	RegularFile	  = 1,
	DirectoryFile = 2,
};

enum FileMode : uint8 {
	WriteProtected = 0x1, // Fat: Read-only
	Hidden		   = 0x2, // Fat: Hidden
	SystemFile	   = 0x4, // Fat: System
	Modified	   = 0x8, // Fat: Archive
};


struct DateTime
{
	uint8 year	 = 0; // 1970 = 0
	uint8 month	 = 0; // Jan = 0
	uint8 day	 = 0; // 1st = 0
	uint8 hour	 = 0;
	uint8 minute = 0;
	uint8 second = 0;

	constexpr DateTime() = default;
	constexpr DateTime(uint8 y, uint8 mo, uint8 d, uint8 h, uint8 mi, uint8 s) noexcept :
		year(y),
		month(mo),
		day(d),
		hour(h),
		minute(mi),
		second(s)
	{}
};


struct FileInfo
{
	cstr	 fname = nullptr; // tempstr
	SIZE	 fsize;
	DateTime mtime;
	FileType ftype;
	FileMode fmode;

	constexpr	   operator bool() const noexcept { return fname != nullptr; }
	constexpr bool isaFile() const noexcept { return fname && ftype == FileType::RegularFile; }
	constexpr bool isaDir() const noexcept { return fname && ftype == FileType::DirectoryFile; }

	FileInfo() noexcept = default;

	FileInfo(cstr name, uint fsize, const DateTime& mtime, FileType ftype, FileMode fmode) :
		fname(name),
		fsize(fsize),
		mtime(mtime),
		ftype(ftype),
		fmode(fmode)
	{}
	template<typename T>
	explicit FileInfo(const T&);
};


// IoCtl enums
// match FatFS ioctl() function codes
struct IoCtl
{
	enum Cmd {
		/* Generic command (Used by FatFs) */
		CTRL_SYNC		 = 0, // Complete pending write process (needed at FF_FS_READONLY == 0)
		GET_SECTOR_COUNT = 1, // Get media size (needed at FF_USE_MKFS == 1)
		GET_SECTOR_SIZE	 = 2, // 1 << ss_write // Get sector size (needed at FF_MAX_SS != FF_MIN_SS)
		GET_BLOCK_SIZE	 = 3, // 1 << ss_erase // Get erase block size (needed at FF_USE_MKFS == 1)
		CTRL_TRIM = 4, // Tell device that the data on the block of sectors is no longer used (used if FF_USE_TRIM == 1)

		/* Generic command (Not used by FatFs) */
		CTRL_POWER	= 5, // Get/Set power status
		CTRL_LOCK	= 6, // Lock/Unlock media removal
		CTRL_EJECT	= 7, // Eject media
		CTRL_FORMAT = 8, // Create physical format on the media

		/* MMC/SDC specific ioctl command */
		MMC_GET_TYPE   = 10, // Get card type
		MMC_GET_CSD	   = 11, // Get CSD
		MMC_GET_CID	   = 12, // Get CID
		MMC_GET_OCR	   = 13, // Get OCR
		MMC_GET_SDSTAT = 14, // Get SD status
		ISDIO_READ	   = 55, // Read data form SD iSDIO register
		ISDIO_WRITE	   = 56, // Write data to SD iSDIO register
		ISDIO_MRITE	   = 57, // Masked write data to SD iSDIO register

		/* ATA/CF specific ioctl command */
		ATA_GET_REV	  = 20, // Get F/W revision
		ATA_GET_MODEL = 21, // Get model name
		ATA_GET_SN	  = 22, // Get serial number

		// alias, others:
		FLUSH_OUT	  = CTRL_SYNC,	 // wait & send all pending outputs
		ERASE_SECTORS = CTRL_TRIM,	 // mark sectors as unused: start,count, and format if needed by device
		ERASE_DISK	  = CTRL_FORMAT, // create file system after formatting whole disk, if needed by device

		FLUSH_IN   = 9,	 // flush & discard pending buffered inputs
		CTRL_RESET = 80, // reset internal state, discard pending input and output, keep connected
		CTRL_CONNECT,	 // connect to hardware, load removable disk
		CTRL_DISCONNECT, // disconnect from hardware, unload removable disk
	};

	enum Arg {
		NONE = 0,
		SIZE = sizeof(kio::SIZE),
		ADDR = sizeof(kio::ADDR),
		LBA	 = sizeof(kio::LBA),
	};

	uint cmd  : 16;
	uint arg1 : 8;
	uint arg2 : 8;

	IoCtl(Cmd cmd, Arg a1 = NONE, Arg a2 = NONE) : cmd(cmd), arg1(a1), arg2(a2) {}
};


// Device Flags:
enum Flags : uint8 {
	readable	 = 1,	 //
	writable	 = 2,	 //
	overwritable = 4,	 // can overwrite old data without formatting. else eventually 0xff->anyvalue->0x00.
	removable	 = 8,	 // SDCard
	dont_close	 = 0x10, // don't close file in dtor (StdFile)
	partition	 = 0x20, // hint for mkfs: needs partitioning like a HD
	EOF_PENDING	 = 0x40, // File::read()
	APPEND_MODE	 = 0x80, // File
	readwrite	 = readable | writable | overwritable,
};
inline Flags operator|(Flags a, Flags b) { return Flags(uint(a) | uint(b)); }
inline Flags operator&(Flags a, int b) { return Flags(uint(a) & uint(b)); }


} // namespace kio
