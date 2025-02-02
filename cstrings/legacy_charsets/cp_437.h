// Copyright (c) 2002 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*
	conversion table Code Page 437 -> UCS-2
*/


{
	/*   0 */ 0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
	/*   8 */ 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
	/*  16 */ 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
	/*  24 */ 0x0018, 0x0019, 0x001A, 0x001B, 0x001C, 0x001D, 0x001E, 0x001F,
	/*  32 */ 0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
	/*  40 */ 0x0028, 0x0029, 0x002A, 0x002B, 0x002C, 0x002D, 0x002E, 0x002F,
	/*  48 */ 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
	/*  56 */ 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E, 0x003F,
	/*  64 */ 0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
	/*  72 */ 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
	/*  80 */ 0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
	/*  88 */ 0x0058, 0x0059, 0x005A, 0x005B, 0x005C, 0x005D, 0x005E, 0x005F,
	/*  96 */ 0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
	/* 104 */ 0x0068, 0x0069, 0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F,
	/* 112 */ 0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
	/* 120 */ 0x0078, 0x0079, 0x007A, 0x007B, 0x007C, 0x007D, 0x007E, 0x007F,

	// from www.unicode.org:

	0x00c7, //LATIN CAPITAL LETTER C WITH CEDILLA
	0x00fc, //LATIN SMALL LETTER U WITH DIAERESIS
	0x00e9, //LATIN SMALL LETTER E WITH ACUTE
	0x00e2, //LATIN SMALL LETTER A WITH CIRCUMFLEX
	0x00e4, //LATIN SMALL LETTER A WITH DIAERESIS
	0x00e0, //LATIN SMALL LETTER A WITH GRAVE
	0x00e5, //LATIN SMALL LETTER A WITH RING ABOVE
	0x00e7, //LATIN SMALL LETTER C WITH CEDILLA
	0x00ea, //LATIN SMALL LETTER E WITH CIRCUMFLEX
	0x00eb, //LATIN SMALL LETTER E WITH DIAERESIS
	0x00e8, //LATIN SMALL LETTER E WITH GRAVE
	0x00ef, //LATIN SMALL LETTER I WITH DIAERESIS
	0x00ee, //LATIN SMALL LETTER I WITH CIRCUMFLEX
	0x00ec, //LATIN SMALL LETTER I WITH GRAVE
	0x00c4, //LATIN CAPITAL LETTER A WITH DIAERESIS
	0x00c5, //LATIN CAPITAL LETTER A WITH RING ABOVE

	0x00c9, //LATIN CAPITAL LETTER E WITH ACUTE
	0x00e6, //LATIN SMALL LIGATURE AE
	0x00c6, //LATIN CAPITAL LIGATURE AE
	0x00f4, //LATIN SMALL LETTER O WITH CIRCUMFLEX
	0x00f6, //LATIN SMALL LETTER O WITH DIAERESIS
	0x00f2, //LATIN SMALL LETTER O WITH GRAVE
	0x00fb, //LATIN SMALL LETTER U WITH CIRCUMFLEX
	0x00f9, //LATIN SMALL LETTER U WITH GRAVE
	0x00ff, //LATIN SMALL LETTER Y WITH DIAERESIS
	0x00d6, //LATIN CAPITAL LETTER O WITH DIAERESIS
	0x00dc, //LATIN CAPITAL LETTER U WITH DIAERESIS
	0x00a2, //CENT SIGN
	0x00a3, //POUND SIGN
	0x00a5, //YEN SIGN
	0x20a7, //PESETA SIGN
	0x0192, //LATIN SMALL LETTER F WITH HOOK

	0x00e1, //LATIN SMALL LETTER A WITH ACUTE
	0x00ed, //LATIN SMALL LETTER I WITH ACUTE
	0x00f3, //LATIN SMALL LETTER O WITH ACUTE
	0x00fa, //LATIN SMALL LETTER U WITH ACUTE
	0x00f1, //LATIN SMALL LETTER N WITH TILDE
	0x00d1, //LATIN CAPITAL LETTER N WITH TILDE
	0x00aa, //FEMININE ORDINAL INDICATOR
	0x00ba, //MASCULINE ORDINAL INDICATOR
	0x00bf, //INVERTED QUESTION MARK
	0x2310, //REVERSED NOT SIGN
	0x00ac, //NOT SIGN
	0x00bd, //VULGAR FRACTION ONE HALF
	0x00bc, //VULGAR FRACTION ONE QUARTER
	0x00a1, //INVERTED EXCLAMATION MARK
	0x00ab, //LEFT-POINTING DOUBLE ANGLE QUOTATION MARK
	0x00bb, //RIGHT-POINTING DOUBLE ANGLE QUOTATION MARK

	0x2591, //LIGHT SHADE
	0x2592, //MEDIUM SHADE
	0x2593, //DARK SHADE
	0x2502, //BOX DRAWINGS LIGHT VERTICAL
	0x2524, //BOX DRAWINGS LIGHT VERTICAL AND LEFT
	0x2561, //BOX DRAWINGS VERTICAL SINGLE AND LEFT DOUBLE
	0x2562, //BOX DRAWINGS VERTICAL DOUBLE AND LEFT SINGLE
	0x2556, //BOX DRAWINGS DOWN DOUBLE AND LEFT SINGLE
	0x2555, //BOX DRAWINGS DOWN SINGLE AND LEFT DOUBLE
	0x2563, //BOX DRAWINGS DOUBLE VERTICAL AND LEFT
	0x2551, //BOX DRAWINGS DOUBLE VERTICAL
	0x2557, //BOX DRAWINGS DOUBLE DOWN AND LEFT
	0x255d, //BOX DRAWINGS DOUBLE UP AND LEFT
	0x255c, //BOX DRAWINGS UP DOUBLE AND LEFT SINGLE
	0x255b, //BOX DRAWINGS UP SINGLE AND LEFT DOUBLE
	0x2510, //BOX DRAWINGS LIGHT DOWN AND LEFT

	0x2514, //BOX DRAWINGS LIGHT UP AND RIGHT
	0x2534, //BOX DRAWINGS LIGHT UP AND HORIZONTAL
	0x252c, //BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
	0x251c, //BOX DRAWINGS LIGHT VERTICAL AND RIGHT
	0x2500, //BOX DRAWINGS LIGHT HORIZONTAL
	0x253c, //BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
	0x255e, //BOX DRAWINGS VERTICAL SINGLE AND RIGHT DOUBLE
	0x255f, //BOX DRAWINGS VERTICAL DOUBLE AND RIGHT SINGLE
	0x255a, //BOX DRAWINGS DOUBLE UP AND RIGHT
	0x2554, //BOX DRAWINGS DOUBLE DOWN AND RIGHT
	0x2569, //BOX DRAWINGS DOUBLE UP AND HORIZONTAL
	0x2566, //BOX DRAWINGS DOUBLE DOWN AND HORIZONTAL
	0x2560, //BOX DRAWINGS DOUBLE VERTICAL AND RIGHT
	0x2550, //BOX DRAWINGS DOUBLE HORIZONTAL
	0x256c, //BOX DRAWINGS DOUBLE VERTICAL AND HORIZONTAL
	0x2567, //BOX DRAWINGS UP SINGLE AND HORIZONTAL DOUBLE

	0x2568, //BOX DRAWINGS UP DOUBLE AND HORIZONTAL SINGLE
	0x2564, //BOX DRAWINGS DOWN SINGLE AND HORIZONTAL DOUBLE
	0x2565, //BOX DRAWINGS DOWN DOUBLE AND HORIZONTAL SINGLE
	0x2559, //BOX DRAWINGS UP DOUBLE AND RIGHT SINGLE
	0x2558, //BOX DRAWINGS UP SINGLE AND RIGHT DOUBLE
	0x2552, //BOX DRAWINGS DOWN SINGLE AND RIGHT DOUBLE
	0x2553, //BOX DRAWINGS DOWN DOUBLE AND RIGHT SINGLE
	0x256b, //BOX DRAWINGS VERTICAL DOUBLE AND HORIZONTAL SINGLE
	0x256a, //BOX DRAWINGS VERTICAL SINGLE AND HORIZONTAL DOUBLE
	0x2518, //BOX DRAWINGS LIGHT UP AND LEFT
	0x250c, //BOX DRAWINGS LIGHT DOWN AND RIGHT
	0x2588, //FULL BLOCK
	0x2584, //LOWER HALF BLOCK
	0x258c, //LEFT HALF BLOCK
	0x2590, //RIGHT HALF BLOCK
	0x2580, //UPPER HALF BLOCK

	0x03b1, //GREEK SMALL LETTER ALPHA
	0x00df, //LATIN SMALL LETTER SHARP S
	0x0393, //GREEK CAPITAL LETTER GAMMA
	0x03c0, //GREEK SMALL LETTER PI
	0x03a3, //GREEK CAPITAL LETTER SIGMA
	0x03c3, //GREEK SMALL LETTER SIGMA
	0x00b5, //MICRO SIGN
	0x03c4, //GREEK SMALL LETTER TAU
	0x03a6, //GREEK CAPITAL LETTER PHI
	0x0398, //GREEK CAPITAL LETTER THETA
	0x03a9, //GREEK CAPITAL LETTER OMEGA
	0x03b4, //GREEK SMALL LETTER DELTA
	0x221e, //INFINITY
	0x03c6, //GREEK SMALL LETTER PHI
	0x03b5, //GREEK SMALL LETTER EPSILON
	0x2229, //INTERSECTION

	0x2261, //IDENTICAL TO
	0x00b1, //PLUS-MINUS SIGN
	0x2265, //GREATER-THAN OR EQUAL TO
	0x2264, //LESS-THAN OR EQUAL TO
	0x2320, //TOP HALF INTEGRAL
	0x2321, //BOTTOM HALF INTEGRAL
	0x00f7, //DIVISION SIGN
	0x2248, //ALMOST EQUAL TO
	0x00b0, //DEGREE SIGN
	0x2219, //BULLET OPERATOR
	0x00b7, //MIDDLE DOT
	0x221a, //SQUARE ROOT
	0x207f, //SUPERSCRIPT LATIN SMALL LETTER N
	0x00b2, //SUPERSCRIPT TWO
	0x25a0, //BLACK SQUARE
	0x00a0, //NO-BREAK SPACE
};
