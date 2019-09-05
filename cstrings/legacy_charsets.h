#pragma once
/*	Copyright  (c)	Günter Woigk 2002 - 2019
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

#include "Libraries/kio/kio.h"
#include "Libraries/cstrings/cstrings.h"


/*
	convert ucs4char <-> legacy 8bit char
*/

typedef uint8  ucs1char;
typedef uint16 ucs2char;
typedef uint32 ucs4char;
typedef const ucs2char ucs2table[256]; // map legacy char -> ucs2char

extern ucs2table legacy_charset_mac_roman;
extern ucs2table legacy_charset_ascii_ger;
extern ucs2table legacy_charset_rtos;
extern ucs2table legacy_charset_cp_437;
extern ucs2table legacy_charset_atari_st;


namespace ucs2 {

inline ucs2char from_8bit	 (char c, ucs2table t) noexcept { return t[uchar(c)]; }
inline ucs2char from_Latin1	 (char c) noexcept { return uchar(c); }
inline ucs2char from_Ascii	 (char c) noexcept { return c & 0x7f; }
inline ucs2char from_MacRoman(char c) noexcept { return legacy_charset_mac_roman[uchar(c)]; }
inline ucs2char from_AsciiGer(char c) noexcept { return legacy_charset_ascii_ger[uchar(c)]; }
inline ucs2char from_Rtos	 (char c) noexcept { return legacy_charset_rtos[uchar(c)]; }
inline ucs2char from_Cp437	 (char c) noexcept { return legacy_charset_cp_437[uchar(c)]; }
inline ucs2char from_AtariST (char c) noexcept { return legacy_charset_atari_st[uchar(c)]; }

static inline char _replacementchar() noexcept { errno=notindestcharset; return '?'; }

// to_xxx functions set errno: notindestcharset
extern char to_8bit		(ucs2char c, ucs2table t) noexcept;
inline char to_Latin1	(ucs2char c) noexcept { return c <= 255 ? char(c) : _replacementchar(); }
inline char to_Ascii	(ucs2char c) noexcept { return c <= 127 ? char(c) : _replacementchar(); }
inline char to_AsciiGer	(ucs2char c) noexcept { return (c|0x20)<='z' && c!='@' ? char(c) : to_8bit(c,legacy_charset_ascii_ger); }
inline char to_Rtos		(ucs2char c) noexcept { return (c|0x20)<='z' && c!='@' ? char(c) : to_8bit(c,legacy_charset_rtos); }
inline char to_MacRoman	(ucs2char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_mac_roman); }
inline char to_Cp437	(ucs2char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_cp_437); }
inline char to_AtariST	(ucs2char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_atari_st); }

}; // namespace


namespace ucs4 {

inline ucs4char from_8bit	 (char c, ucs2table t) noexcept { return t[uchar(c)]; }
inline ucs4char from_Latin1	 (char c) noexcept { return uchar(c); }
inline ucs4char from_Ascii	 (char c) noexcept { return c & 0x7f; }
inline ucs4char from_MacRoman(char c) noexcept { return legacy_charset_mac_roman[uchar(c)]; }
inline ucs4char from_AsciiGer(char c) noexcept { return legacy_charset_ascii_ger[uchar(c)]; }
inline ucs4char from_Rtos	 (char c) noexcept { return legacy_charset_rtos[uchar(c)]; }
inline ucs4char from_Cp437	 (char c) noexcept { return legacy_charset_cp_437[uchar(c)]; }
inline ucs4char from_AtariST (char c) noexcept { return legacy_charset_atari_st[uchar(c)]; }

static inline char _replacementchar() noexcept { errno=notindestcharset; return '?'; }

// to_xxx functions set errno: notindestcharset
extern char to_8bit		(ucs4char c, ucs2table t) noexcept;
inline char to_Latin1	(ucs4char c) noexcept { return c <= 255 ? char(c) : _replacementchar(); }
inline char to_Ascii	(ucs4char c) noexcept { return c <= 127 ? char(c) : _replacementchar(); }
inline char to_AsciiGer	(ucs4char c) noexcept { return (c|0x20)<='z' && c!='@' ? char(c) : to_8bit(c,legacy_charset_ascii_ger); }
inline char to_Rtos		(ucs4char c) noexcept { return (c|0x20)<='z' && c!='@' ? char(c) : to_8bit(c,legacy_charset_rtos); }
inline char to_MacRoman	(ucs4char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_mac_roman); }
inline char to_Cp437	(ucs4char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_cp_437); }
inline char to_AtariST	(ucs4char c) noexcept { return c <= 127 ? char(c) : to_8bit(c,legacy_charset_atari_st); }

}; // namespace














