/*	Copyright  (c)	Günter Woigk  2002-2005
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
	
	html glyph names <-> unicodes
*/






// html special characters:

H( "\"","Anführungszeichen oben",			"quot",		34	),
H( "&",	"kaufmännisches Und",				"amp",		38	),
H( "<",	"öffnende spitze Klammer",			"lt",		60	),
H( ">",	"schließende spitze Klammer",		"gt",		62	),

// html 3.2: ISO 8859-1 character set:

H( " ",	"Nicht umbrechendes Leerzeichen",	"nbsp",		160	),
H( "¡",	"Satzanfangs-Ausrufezeichen",		"iexcl",	161	),
H( "¢",	"Cent-Zeichen",						"cent",		162	),
H( "£",	"Pfund-Zeichen",					"pound",	163	),
H( "¤", "Währungs-Zeichen",					"curren",	164	),
H( "¥",	"Yen-Zeichen",						"yen",		165	),
H( "¦",	"durchbrochener Strich",			"brvbar",	166	),
H( "§",	"Paragraph-Zeichen",				"sect",		167	),
H( "¨",	"Pünktchen oben",					"uml",		168	),
H( "©",	"Copyright-Zeichen",				"copy",		169	),
H( "ª",	"Ordinal-Zeichen weiblich",			"ordf",		170	),
H( "«",	"Anführungszeichen links '<<'",		"laquo",	171	),
H( "¬",	"Verneinungs-Zeichen",				"not",		172	),
H( "­",	"kurzer Trennstrich",				"shy",		173	),
H( "®",	"Registriermarke-Zeichen",			"reg",		174	),
H( "¯",	"Überstrich",						"macr",		175	),
H( "°",	"Grad-Zeichen",						"deg",		176	),
H( "±",	"Plusminus-Zeichen",				"plusmn",	177	),
H( "²", "Hoch-2-Zeichen",					"sup2",		178	),
H( "³", "Hoch-3-Zeichen",					"sup3",		179	),
H( "´",	"Acute-Zeichen",					"acute",	180	),
H( "µ",	"Mikro-Zeichen",					"micro",	181	),
H( "¶",	"Absatz-Zeichen",					"para",		182	),
H( "·",	"Mittelpunkt",						"middot",	183	),
H( "¸",	"Häkchen unten",					"cedil",	184	),
H( "¹", "Hoch-1-Zeichen",					"sup1",		185	),
H( "º",	"Ordinal-Zeichen männlich",			"ordm",		186	),
H( "»",	"Anführungszeichen rechts '>>'",	"raquo",	187	),
H( "¼", "ein Viertel",						"frac14",	188	),
H( "½", "ein Halb",							"frac12",	189	),
H( "¾", "drei Viertel",						"frac34",	190	),
H( "¿",	"Satzanfangs-Fragezeichen",			"iquest",	191	),
H( "À",	"A mit Accent grave",				"Agrave",	192	),
H( "Á",	"A mit Accent acute",				"Aacute",	193	),
H( "Â",	"A mit Circumflex",					"Acirc",	194	),
H( "Ã",	"A mit Tilde",						"Atilde",	195	),
H( "Ä",	"A Umlaut",							"Auml",		196	),
H( "Å",	"A mit Ring",						"Aring",	197	),
H( "Æ",	"AE Ligatur",						"AElig",	198	),
H( "Ç",	"C mit Häkchen",					"Ccedil",	199	),

H( "È",	"E mit Accent grave",				"Egrave",	200	),
H( "É",	"E mit Accent acute",				"Eacute",	201	),
H( "Ê",	"E mit Circumflex",					"Ecirc",	202	),
H( "Ë",	"E Umlaut",							"Euml",		203	),
H( "Ì",	"I mit Accent grave",				"Igrave",	204	),
H( "Í",	"I mit Accent acute",				"Iacute",	205	),
H( "Î",	"I mit Circumflex",					"Icirc",	206	),
H( "Ï",	"I Umlaut",							"Iuml",		207	),
H( "Ð", "Eth (isländisch)",					"ETH",		208	),
H( "Ñ",	"N mit Tilde",						"Ntilde",	209	),
H( "Ò",	"O mit Accent grave",				"Ograve",	210	),
H( "Ó",	"O mit Accent acute",				"Oacute",	211	),
H( "Ô",	"O mit Circumflex",					"Ocirc",	212	),
H( "Õ",	"O mit Tilde",						"Otilde",	213	),
H( "Ö",	"O Umlaut",							"Ouml",		214	),
H( "×", "Mal-Zeichen",						"times",	215	),
H( "Ø",	"O mit Schrägstrich",				"Oslash",	216	),
H( "Ù",	"U mit Accent grave",				"Ugrave",	217	),
H( "Ú",	"U mit Accent acute",				"Uacute",	218	),
H( "Û",	"U mit Circumflex",					"Ucirc",	219	),
H( "Ü",	"U Umlaut",							"Uuml",		220	),
H( "Ý",	"Y mit Accent acute",				"Yacute",	221	),
H( "Þ", "Thorn (isländisch)",				"THORN",	222	),
H( "ß",	"sz-Ligatur (scharfes s)",			"szlig",	223	),
H( "à",	"a mit Accent grave",				"agrave",	224	),
H( "á",	"a mit Accent acute",				"aacute",	225	),
H( "â",	"a mit Circumflex",					"acirc",	226	),
H( "ã",	"a mit Tilde",						"atilde",	227	),
H( "ä",	"a Umlaut",							"auml",		228	),
H( "å",	"a mit Ring",						"aring",	229	),
H( "æ",	"ae Ligatur",						"aelig",	230	),
H( "ç",	"c mit Häkchen",					"ccedil",	231	),
H( "è",	"e mit Accent grave",				"egrave",	232	),
H( "é",	"e mit Accent acute",				"eacute",	233	),
H( "ê",	"e mit Circumflex",					"ecirc",	234	),
H( "ë",	"e Umlaut",							"euml",		235	),
H( "ì",	"i mit Accent grave",				"igrave",	236	),
H( "í",	"i mit Accent acute",				"iacute",	237	),
H( "î",	"i mit Circumflex",					"icirc",	238	),
H( "ï",	"i Umlaut",							"iuml",		239	),
H( "ð", "eth (isländisch)",					"eth",		240	),
H( "ñ",	"n mit Tilde",						"ntilde",	241	),
H( "ò",	"o mit Accent grave",				"ograve",	242	),
H( "ó",	"o mit Accent acute",				"oacute",	243	),
H( "ô",	"o mit Circumflex",					"ocirc",	244	),
H( "õ",	"o mit Tilde",						"otilde",	245	),
H( "ö",	"o Umlaut",							"ouml",		246	),
H( "÷",	"Divisions-Zeichen",				"divide",	247	),
H( "ø",	"o mit Schrägstrich",				"oslash",	248	),
H( "ù",	"u mit Accent grave",				"ugrave",	249	),
H( "ú",	"u mit Accent acute",				"uacute",	250	),
H( "û",	"u mit Circumflex",					"ucirc",	251	),
H( "ü",	"u Umlaut",							"uuml",		252	),
H( "ý",	"y mit Accent acute",				"yacute",	253	),
H( "þ", "thorn (isländisch)",				"thorn",	254	),
H( "ÿ",	"y Umlaut",							"yuml",		255 ),

// html 4.0: greek letters:

H( "Α",	"Alpha groß",						"Alpha",	913	),
H( "Β",	"Beta groß",						"Beta",		914	),
H( "Γ",	"Gamma groß",						"Gamma",	915	),
H( "Δ",	"Delta groß",						"Delta",	916	),
H( "Ε",	"Epsilon groß",						"Epsilon",	917	),
H( "Ζ",	"Zeta groß",						"Zeta",		918	),
H( "Η",	"Eta groß",							"Eta",		919	),
H( "Θ",	"Theta groß",						"Theta",	920	),
H( "Ι",	"Iota groß",						"Iota",		921	),
H( "Κ",	"Kappa groß",						"Kappa",	922	),
H( "Λ",	"Lambda groß",						"Lambda",	923	),
H( "Μ",	"Mu groß",							"Mu",		924	),
H( "Ν",	"Nu groß",							"Nu",		925	),
H( "Ξ",	"Xi groß",							"Xi",		926	),
H( "Ο",	"Omicron groß",						"Omicron",	927	),
H( "Π",	"Pi groß",							"Pi",		928	),
H( "Ρ",	"Rho groß",							"Rho",		929	),
H( "Σ",	"Sigma groß",						"Sigma",	931	),
H( "Τ",	"Tau groß",							"Tau",		932	),
H( "Υ",	"Upsilon groß",						"Upsilon",	933	),
H( "Φ",	"Phi groß",							"Phi",		934	),
H( "Χ",	"Chi groß",							"Chi",		935	),
H( "Ψ",	"Psi groß",							"Psi",		936	),
H( "Ω",	"Omega groß",						"Omega",	937	),

H( "α",	"alpha klein",						"alpha",	945	),
H( "β",	"beta klein",						"beta",		946	),
H( "γ",	"gamma klein",						"gamma",	947	),
H( "δ",	"delta klein",						"delta",	948	),
H( "ε",	"epsilon klein",					"epsilon",	949	),
H( "ζ",	"zeta klein",						"zeta",		950	),
H( "η",	"eta klein",						"eta",		951	),
H( "θ",	"theta klein",						"theta",	952	),
H( "ι",	"iota klein",						"iota",		953	),
H( "κ",	"kappa klein",						"kappa",	954	),
H( "λ",	"lambda klein",						"lambda",	955	),
H( "μ",	"mu klein",							"mu",		956	),
H( "ν",	"nu klein",							"nu",		957	),
H( "ξ",	"xi klein",							"xi",		958	),
H( "ο",	"omicron klein",					"omicron",	959	),
H( "π",	"pi klein",							"pi",		960	),
H( "ρ",	"rho klein",						"rho",		961	),
H( "ς",	"sigmaf klein",						"sigmaf",	962	),
H( "σ",	"sigma klein",						"sigma",	963	),
H( "τ",	"tau klein",						"tau",		964	),
H( "υ",	"upsilon klein",					"upsilon",	965	),
H( "φ",	"phi klein",						"phi",		966	),
H( "χ",	"chi klein",						"chi",		967	),
H( "ψ",	"psi klein",						"psi",		968	),
H( "ω",	"omega klein",						"omega",	969	),
H( "ϑ",	"theta Symbol",						"thetasym",	977	),
H( "ϒ",	"upsilon mit Haken",				"upsih",	978	),
H( "ϖ",	"pi Symbol",						"piv",		982	),

// html 4.0 named technical symbols

H( "⌈",	"links oben",						"lceil",	8968	),
H( "⌉",	"rechts oben",						"rceil",	8969	),
H( "⌊",	"links unten",						"lfloor",	8970	),
H( "⌋",	"rechts unten",						"rfloor",	8971	),
H( "〈",	"spitze Klammer links",				"lang",		9001	),
H( "〉",	"spitze Klammer rechts",			"rang",		9002	),

// html 4.0: named arrow symbols

H( "←",	"Pfeil links",						"larr",		8592	),
H( "↑",	"Pfeil oben",						"uarr",		8593	),
H( "→",	"Pfeil rechts",						"rarr",		8594	),
H( "↓",	"Pfeil unten",						"darr",		8595	),
H( "↔",	"Pfeil links/rechts",				"harr",		8596	),
H( "↵",	"Pfeil unten-Knick-links",			"crarr",	8629	),
H( "⇐",	"Doppelpfeil links",				"lArr",		8656	),
H( "⇑",	"Doppelpfeil oben",					"uArr",		8657	),
H( "⇒",	"Doppelpfeil rechts",				"rArr",		8658	),
H( "⇓",	"Doppelpfeil unten",				"dArr",		8659	),
H( "⇔",	"Doppelpfeil links/rechts",			"hArr",		8660	),

// html 4.0: named misc. symbols

H( "•",	"Bullet-Zeichen",					"bull",		8226	),
H( "…",	"Horizontale Ellipse",				"hellip",	8230	),
H( "′",	"Minutenzeichen",					"prime",	8242	),
H( "‾",	"Überstrich",						"oline",	8254	),
H( "⁄",	"Bruchstrich",						"frasl",	8260	),
H( "℘",	"Weierstrass p",					"weierp",	8472	),
H( "ℑ",	"Zeichen für 'imaginär'",			"image",	8465	),
H( "ℜ",	"Zeichen für 'real'",				"real",		8476	),
H( "™",	"Trademark-Zeichen",				"trade",	8482	),
H( "€",	"Euro-Zeichen",						"euro",		8364	),
H( "ℵ",	"Alef-Symbol",						"alefsym",	8501	),
H( "♠",	"Pik-Zeichen",						"spades",	9824	),
H( "♣",	"Kreuz-Zeichen",					"clubs",	9827	),
H( "♥",	"Herz-Zeichen",						"hearts",	9829	),
H( "♦",	"Karo-Zeichen",						"diams",	9830	),

// html 4.0: named punctuation

H( " ",	"Leerzeichen Breite n",				"ensp",		8194	),
H( " ",	"Leerzeichen Breite m",				"emsp",		8195	),
H( " ",	"Schmales Leerzeichen",				"thinsp",	8201	),
H( "",	"null breiter Nichtverbinder",		"zwnj",		8204	),	// ***TODO***
H( "",	"null breiter Verbinder",			"zwj",		8205	),	// ***TODO***
H( "",	"links-nach-rechts-Zeichen",		"lrm",		8206	),	// ***TODO***
H( "",	"rechts-nach-links-Zeichen",		"rlm",		8207	),	// ***TODO***
H( "–",	"Gedankenstrich Breite n",			"ndash",	8211	),
H( "—",	"Gedankenstrich Breite m",			"mdash",	8212	),
H( "‘",	"einfaches Anf.zeichen links",		"lsquo",	8216	),
H( "’",	"einfaches Anf.zeichen rechts",		"rsquo",	8217	),
H( "‚",	"einfaches low-9-Zeichen",			"sbquo",	8218	),
H( "“",	"doppeltes Anf.zeichen links",		"ldquo",	8220	),
H( "”",	"doppeltes Anf.zeichen rechts",		"rdquo",	8221	),
H( "„",	"doppeltes low-9-Zeichen rechts",	"bdquo",	8222	),
H( "†",	"Kreuz",							"dagger",	8224	),
H( "‡",	"Doppelkreuz",						"Dagger",	8225	),
H( "‰",	"zu tausend",						"permil",	8240	),
H( "‹",	"Anführungszeichen links '<'",		"lsaquo",	8249	),
H( "›",	"Anführungszeichen rechts '>'",		"rsaquo",	8250	),




