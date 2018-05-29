/*
******************************************************************************

UNICODE, INC. LICENSE AGREEMENT - DATA FILES AND SOFTWARE

Unicode Data Files include all data files under the directories
http://www.unicode.org/Public/ and http://www.unicode.org/reports/.
Unicode Software includes any source code under the directories
http://www.unicode.org/Public/ and http://www.unicode.org/reports/.

NOTICE TO USER: Carefully read the following legal agreement. BY DOWNLOADING,
INSTALLING, COPYING OR OTHERWISE USING UNICODE INC.'S DATA FILES ("DATA FILES"),
AND/OR SOFTWARE ("SOFTWARE"), YOU UNEQUIVOCALLY ACCEPT, AND AGREE TO BE BOUND
BY, ALL OF THE TERMS AND CONDITIONS OF THIS AGREEMENT. IF YOU DO NOT AGREE,
DO NOT DOWNLOAD, INSTALL, COPY, DISTRIBUTE OR USE THE DATA FILES OR SOFTWARE.

COPYRIGHT AND PERMISSION NOTICE

Copyright (C) 2003-2004, Unicode, Inc. and International Business Machines Corporation.
All Rights Reserved.
Distributed under the Terms of Use in http://www.unicode.org/copyright.html.

Permission is hereby granted, free of charge, to any person obtaining a copy of
the Unicode data files and associated documentation (the "Data Files") or
Unicode software and associated documentation (the "Software") to deal in the
Data Files or Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, and/or sell copies of
the Data Files or Software, and to permit persons to whom the Data Files or
Software are furnished to do so, provided that (a) the above copyright notice(s)
and this permission notice appear in all copies of the Data Files or Software,
(b) both the above copyright notice(s) and this permission notice appear in
associated documentation, and (c) there is clear notice in each modified Data
File or in the Software as well as in the documentation associated with the Data
File(s) or Software that the data or software has been modified.

THE DATA FILES AND SOFTWARE ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF THIRD
PARTY RIGHTS. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR HOLDERS INCLUDED IN THIS
NOTICE BE LIABLE FOR ANY CLAIM, OR ANY SPECIAL INDIRECT OR CONSEQUENTIAL
DAMAGES, OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THE DATA FILES OR
SOFTWARE.

Except as contained in this notice, the name of a copyright holder shall not be
used in advertising or otherwise to promote the sale, use or other dealings in
these Data Files or Software without prior written authorization of the
copyright holder.

******************************************************************************
*   file name:  scsumini.c
*   encoding:   US-ASCII
*   tab size:   8 (not used)
*   indentation:4
*
*   created on: 2003jun24
*   created by: Markus W. Scherer
*
*   Minimal SCSU encoder.
*   This SCSU encoder is sufficient and small but not very efficient.
*   While it could be used as is, it is intended to show that a simple, small
*   yet functioning SCSU encoder is feasible.
*
*   This file will by itself compile on many platforms into a standalone
*   command-line program. A user can type Unicode code points and the program
*   will output the SCSU bytes from the encoder.
*
*   Features:
*   - Encoder state: 4 bits (boolean mode and 3-bit window number)
*   - Uses very small set of SCSU features; does not define dynamic windows
*     but uses predefined ones.
*   - Generates conformant SCSU output.
*   - Generates XML-suitable output.
*   - Encodes U+FEFF as recommended for SCSU signatures.
*   - Encodes US-ASCII and ISO-8859-1 text as itself.
*   - Encodes scripts in compact form for which SCSU has predefined dynamic windows.
*   - Encodes many texts with no more bytes than either UTF-8 or UTF-16.
*   - Encodes some texts with more bytes than UTF-8/16, especially texts in
*     many small scripts:
*     This encoder will use Unicode mode for them (2 bytes per character)
*     and for each space character switch to single byte mode, emit the space,
*     and switch back to Unicode mode. (3 bytes per space)
*/

#include <stdio.h>

/* Type definitions, usually from a system header. */

/** 32-bit signed integer (need at least a 22-bit signed integer here) */
typedef long int32_t;


/** Forward declaration of a byte output function. (Takes an int to avoid casts.) */
extern void out(int byte);


/* SCSU command byte values */
enum {
    SQ0=0x01, /* Quote from window pair 0 */
    SQU=0x0E, /* Quote a single Unicode character */
    SCU=0x0F, /* Change to Unicode mode */
    SC0=0x10, /* Select window 0 */

    UC0=0xE0, /* Select window 0 */
    UQU=0xF0  /* Quote a single Unicode character */
};

/* constant offsets for the 8 static and 8 dynamic windows */
static const int32_t offsets[16]={
    /* initial offsets for the 8 dynamic (sliding) windows */
    0x0080, /* Latin-1 */
    0x00C0, /* Latin Extended A */
    0x0400, /* Cyrillic */
    0x0600, /* Arabic */
    0x0900, /* Devanagari */
    0x3040, /* Hiragana */
    0x30A0, /* Katakana */
    0xFF00, /* Fullwidth ASCII */

    /* offsets for the 8 static windows */
    0x0000, /* ASCII for quoted tags */
    0x0080, /* Latin - 1 Supplement (for access to punctuation) */
    0x0100, /* Latin Extended-A */
    0x0300, /* Combining Diacritical Marks */
    0x2000, /* General Punctuation */
    0x2080, /* Currency Symbols */
    0x2100, /* Letterlike Symbols and Number Forms */
    0x3000  /* CJK Symbols and punctuation */
};

static /* inline */ char isInWindow(int32_t offset, int32_t c) {
    return (char)(offset<=c && c<=(offset+0x7f));
}

/* get the index of the static/dynamic window that contains c; -1 if none */
static int getWindow(int32_t c) {
    int i;

    for(i=0; i<16; ++i) {
        if(isInWindow(offsets[i], c)) {
            return i;
        }
    }
    return -1;
}

struct SCSUMiniState {
    char isUnicodeMode;
    char window;
};
typedef struct SCSUMiniState SCSUMiniState;

/**
 * Function for minimal SCSU encoding.
 * Calls out(byte) to output SCSU bytes.
 *
 * @param c Input Unicode code point or UTF-16 code unit.
 *        The output will be the same whether the function is called with
 *        a sequence of code points or an equivalent sequence of UTF-16 code
 *        units, or an equivalent mixture of them.
 *        It must be 0<=c<=0x10ffff.
 * @param pState Pointer to SCSUMiniState struct.
 *        Indicates whether the encoder is currently in Unicode mode,
 *        and which is the current dynamic window for single-byte mode.
 *        All fields must initially be set to 0 (single-byte mode, window 0).
 */
extern void SCSUMiniEncode(int32_t c, SCSUMiniState *pState) {
    char window; /* dynamic window 0..7 */
    int w;       /* result of getWindow(), -1..7 */

    /*
     * Check arguments:
     * - Is c a valid code point? (0<=c<=0x10ffff)
     * - Is pState a valid pointer to a state struct with valid values?
     *   (isUnicodeMode=0..1  and window=0..7)
     */
    if( c<0 || c>0x10ffff ||
        pState==NULL || (pState->isUnicodeMode&~1)!=0 || (pState->window&~7)!=0
    ) {
        /* illegal argument error */
        return;
    }

    if(c>0xffff) {
        /* encode a supplementary code point as a surrogate pair */
        SCSUMiniEncode(0xd7c0+(c>>10), pState);
        SCSUMiniEncode(0xdc00+(c&0x3ff), pState);
        return;
    }

    window=pState->window;
    if(!pState->isUnicodeMode) {
        /* single-byte mode */
        if(c<0x20) {
            /*
             * Encode C0 control code:
             * Check the code point against the bit mask 0010 0110 0000 0001
             * which contains 1-bits at the bit positions corresponding to
             * code points 0D 0A 09 00 (CR LF TAB NUL)
             * which are encoded directly.
             * All other C0 control codes are quoted with SQ0.
             */
            if(c<=0xf && ((1<<c)&0x2601)==0) {
                out(SQ0);
            }
            out(c);
        } else if(c<=0x7f) {
            /* encode US-ASCII directly */
            out(c);
        } else if(isInWindow(offsets[window], c)) {
            /* use the current dynamic window */
            out(0x80+(c-offsets[window]));
        } else if((w=getWindow(c))>=0) {
            if(w<=7) {
                /* switch to a dynamic window */
                out(SC0+w);
                out(0x80+(c-offsets[w]));
                pState->window=window=(char)w;
            } else {
                /* quote from a static window */
                out(SQ0+(w-8));
                out(c-offsets[w]);
            }
        } else if(c==0xfeff) {
            /* encode the signature character U+FEFF with SQU */
            out(SQU);
            out(0xfe);
            out(0xff);
        } else {
            /* switch to Unicode mode */
            out(SCU);
            pState->isUnicodeMode=1;
            SCSUMiniEncode(c, pState);
        }
    } else {
        /* Unicode mode */
        if(c<=0x7f) {
            /* US-ASCII: switch to single-byte mode with the previous dynamic window */
            pState->isUnicodeMode=0;
            out(UC0+window);
            SCSUMiniEncode(c, pState);
        } else if((w=getWindow(c))>=0 && w<=7) {
            /* switch to single-byte mode with a matching dynamic window */
            out(UC0+w);
            pState->window=window=(char)w;
            pState->isUnicodeMode=0;
            SCSUMiniEncode(c, pState);
        } else {
            if(0xe000<=c && c<=0xf2ff) {
                out(UQU);
            }
            out(c>>8);
            out(c);
        }
    }
}

/* driver for sample code */
extern void out(int byte) {
    printf("--> %02X\n", byte&0xff);
}

extern int main(int argc, const char *argv[]) {
    int32_t c;
    SCSUMiniState state={ 0, 0 }; /* initial state */

    puts("Type hexadecimal Unicode code point values, one per line, for SCSU encoding:");

    while(1==scanf("%lX", &c)) {
        SCSUMiniEncode(c, &state);
    }
    return 0;
}
