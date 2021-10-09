/*	Copyright  (c)	Günter Woigk 2010 - 2018
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
*/

#ifndef IDFID_H
#define IDFID_H

#ifndef M
#include "Libraries/kio/standard_types.h"
#define M_was_not_defined 1
#define M(ID,STR)	ID
  enum IdfID : uint16
  {
#endif


// predefined token ids:

	M(tEMPTYSTR=0,	"" ),			// empty string
	M(tNL,			"\n" ),
	M(tIVAL,		"<ival>" ),		// special id for immediate values in preprocessor
	M(tEOF,			"<eof>"),		// end of source

// operators:

	M(tSEMIK,	";" ),		// opri -2		void;void   -> void
	M(tKOMMA,	"," ),		// opri -1		value,value -> arguments

	M(tGL,		"="  ),		// opri 0		vref=value  -> void
	M(tDPGL,	":=" ),
	M(tADDGL,	"+=" ),
	M(tSUBGL,	"-=" ),
	M(tMULGL,	"*=" ),
	M(tDIVGL,	"/=" ),
	M(tMODGL,	"%=" ),
	M(tPTRGL,	"->" ),
	M(tANDGL,	"&=" ),
	M(tORGL,	"|=" ),
	M(tXORGL,	"^=" ),
	M(tSLGL,	"<<=" ),
	M(tSRGL,	">>=" ),
	M(tANDANDGL,"&&=" ),
	M(tORORGL,	"||=" ),

	M(tQMARK,	"?" ),		// opri 1
	M(tCOLON,	":" ),

	M(tANDAND,	"&&" ),		// opri 2
	M(tOROR,	"||" ),

	M(tEQ,		"==" ),		// opri 3
	M(tNE,		"!=" ),
	M(tLT,		"<" ),
	M(tGT,		">" ),
	M(tLE,		"<=" ),
	M(tGE,		">=" ),
//	M(tIS,		"is" ),
//	M(tISNOT,	"isnot" ),
	M(tIS,		"===" ),
	M(tISNOT,	"!==" ),

	M(tADD,		"+" ),		// opri 4
	M(tSUB,		"-" ),

	M(tMUL,		"*" ),		// opri 5
	M(tDIV,		"/" ),
	M(tMOD,		"%" ),
	M(tDIVMOD,	"/%"),

	M(tAND,		"&" ),		// opri 6
	M(tOR,		"|" ),
	M(tXOR,		"^" ),

	M(tSR,		">>" ),		// opri 7
	M(tSL,		"<<" ),
	M(tMULMUL,	"**" ),

	M(tINCR,	"++" ),		// opri 9
	M(tDECR,	"--" ),
	M(tNOT,		"!"  ),
	M(tNOTNOT,	"!!" ),
	M(tCPL,		"~"  ),
	M(tPOS,		" +" ),
	M(tNEG,		" -" ),

// special characters:

	M(tRKauf,	"(" ),		// keep position
	M(tGKauf,	"{" ),		// keep position
	M(tEKauf,	"[" ),		// keep position --> Tokenizer::tokenize()
	M(tRKzu,	")" ),
	M(tGKzu,	"}" ),
	M(tEKzu,	"]" ),
	M(tDOT,		"." ),
	M(tHASH,	"#" ),
	M(tAT,		"@" ),
	M(tDOLLAR,	"$" ),


// start of 'identifiers':

// Basic Types:
	M(tINT8,		"int8"		),	// keep order
	M(tUINT8,		"uint8"		),
	M(tINT16,		"int16"		),
	M(tUINT16,		"uint16"	),
	M(tINT32,		"int32"		),
	M(tUINT32,		"uint32"	),
	M(tINT64,		"int64"		),
	M(tUINT64,		"uint64"	),
	M(tSFLOAT,		"sfloat"	),
	M(tFLOAT,		"float"		),
	M(tLFLOAT,		"lfloat"	),
	M(tVOID,		"void"		),

// target depending alias types:
	M(tSHORT,		"short"		),
	M(tUSHORT,		"ushort"	),
	M(tINT,			"int"		),
	M(tUINT,		"uint"		),
	M(tLONG,		"long"		),
	M(tULONG,		"ulong"		),
	M(tBYTE,		"byte"		),
	M(tUBYTE,		"ubyte"		),
	M(tCHAR,		"char"		),
	M(tSTR,			"str"		),

// type IDs:
	//tENUM
	M(tINTEGER,		"basic"		),
	M(tPROC,		"proc"		),
	M(tSTRUCT,		"struct"	),
	M(tARRAY,		"array"		),
	M(tREF,			"ref"		),
	M(tPTR,			"ptr"		),
	M(tRANGE,		"range"		),		// array + start + end

	//M(tOBJECT_t,	"Object"	),
	//M(tSIGNAL,  	"signal"	),
	//M(tMUTEX_t,  	"Mutex"		),
	//M(tSEMA_t,  	"Sema"		),
	//M(tIRPT_t,  	"Irpt"		),
	//M(tTHREAD_t,	"Thread"	),



// preprocessor instructions:

	M(tENDIF,	"endif" ),
	M(tINCLUDE,	"include" ),
	M(tREQUIRE,	"require" ),
	M(tIFDEF,	"ifdef" ),
	M(tIFNDEF,	"ifndef" ),
	M(tDEFINE,	"define" ),
	M(tUNDEF,	"undef" ),
	M(tERROR,	"error" ),

// reserved names:

	M(tIF,		"if" 	),				// <-- also preprocessor
	M(tELSE,	"else" 	),				// <-- also preprocessor
	M(tELIF,	"elif" 	),				// <-- also preprocessor
	M(tASSERT,	"assert" ),				// <-- also preprocessor
	M(tTYPE,	"type" 	),
	M(tENUM,	"enum"	),
	M(tRETURN,	"return" ),
	M(tSWITCH,	"switch" ),
	M(tCASE,	"case" 	),
	M(tDO,		"do" 	),
	M(tWHILE,	"while" ),
	M(tUNTIL,	"until" ),
	M(tEXIT,	"exit" 	),
	M(tBREAK,	"break" ),
	M(tDEFAULT,	"default" ),
	M(tEXTERN,	"extern" ),
	M(tSTATIC,	"static" ),
	M(tCONST,	"const" ),
	M(tMUTABLE,	"mutable" ),
	M(tSCOPE,	"scope" ),
	M(tEND,		"end" 	),
	M(tSIZEOF,	"sizeof" ),
	M(tSIZE,	"size" 	),
	M(tCOUNT,	"count" ),
	M(tTO,		"to" 	),
	M(tNEXT,	"next" 	),

// more:

	M(t__FILE__,	"__FILE__" ),		// preprocessor
	M(t__LINE__,	"__LINE__" ),		// preprocessor
	M(t__DATE__,	"__DATE__" ),		// preprocessor
	M(t__TIME__,	"__TIME__" ),		// preprocessor
	M(t__B2B,		"__b2b"),
	M(t__BPB,		"__bpb"),
	M(t__BPC,		"__bpc"),
	M(t__BPS,		"__bps"),
	M(t__BPI,		"__bpi"),
	M(t__BPL,		"__bpl"),
	M(t__BPF,		"__bpf"),
	M(t__BPSF,		"__bpsf"),
	M(t__BPLF,		"__bplf"),
	M(t__BPP,		"__bpp"),
	M(t__BPR,		"__bpr"),
	M(t__OPTIMIZE,	"__optimize"),
	M(t__TARGET,	"__target"),
	M(t__BYTE_ORDER,	"__byte_order"),
	M(t__LITTLE_ENDIAN,	"__little_endian"),
	M(t__BIG_ENDIAN,	"__big_endian"),
	M(tDEFINED,		"defined"	), 		// preprocessor

	M(tOPERATOR,	"operator" 	),
	M(tOPCODE,		"opcode" 	),
	M(tASM,			"asm" 		),
	M(tINLINE,		"inline" 	),
	M(tREVERTED,	"reverted" 	),
	M(tGLOBAL,		"global" 	),
	//M(tIMPLICIT,	"implicit"	),
	//M(tSLOT,		"slot" 		),
	M(tNULL,		"null" 		),


// instructions:

	M(tNEW,			"new"		),
	M(tALLOC,		"alloc"		),
	//M(tDEALLOC,  	"dealloc"	),
	//M(tREALLOC,  	"realloc"	),
	M(tRETAIN,		"retain"	),
	//M(tCOPYOF,  	"copyof"	),
	//M(tDELETE,  	"delete"	),
	M(tDISPOSE,		"dispose"	),
	M(tREFCNT,		"refcnt"	),
	M(tIS_SHARED,	"is_shared"	),
	M(tINIT,		"init"		),
	M(tCOPY,		"copy"		),
	M(tCOPYQIZIN,	"copyqizin"	),
	M(tCOPYRANGE,	"copyrange"	),
	M(tKILL,		"kill"		),
	//M(tCONNECT,  	"connect"	),		unten bei Sockets definiert
	//M(tDISCONNECT,  "disconnect"),
	//M(tSEND,		"send"		),
	M(tSHRINK,		"shrink"	),
	M(tSHRINKSTRARRAY,"shrinkstrarray"	),
	M(tGROW,		"grow"		),
	M(tRESIZE,		"resize"	),


// opcodes:

	M(tPEEK,		"peek"		),
	M(tPEEK_NZ,		"peek_nz"	),
	//M(tPEEK_CLEAR,  "peek_clear"),
	M(tPEEKSIGNAL,	"peeksignal"),
	M(tPOKE,		"poke"		),
	M(tSWAPWITHVAR,	"swapwithvar" ),
	M(tSWAP,		"swap"		),
	M(tNOP,			"nop"		),
	M(tIN,			"in"		),
	M(tOUT,			"out"		),
	M(tCAST,		"cast"		),
	M(tTOR,			"tor"		),
	M(tDUP2R,		"dup2r"		),
	M(tFROMR,		"fromr"		),
	M(tPEEKR,		"peekr"		),
	M(tPUSHR,		"pushr"		),
	M(tDROPR,		"dropr"		),
	M(tBOOL,		"bool"		),
	M(tJP,			"jp"		),
	M(tJP_Z,		"jp_z"		),
	M(tJP_NZ,		"jp_nz"		),
	M(tAND0,		"and0"		),
	M(tOR1,			"or1"		),
	M(tFORALLITEMS,	"forallitems"),
	M(tFORRANGE,	"forrange"	),
	M(tPPPEEK,		"pppeek"	),
	M(tMMPEEK,		"mmpeek"	),
	M(tPEEKPP,		"peekpp"	),
	M(tPEEKMM,		"peekmm"	),
	M(tATINDEX,		"atindex"	),
	M(tDROP,		"drop"		),
	M(tNIP,			"nip"		),
	M(tDUP,			"dup"		),
	M(tDUP2,		"dup2"		),
	M(tDUP2_MIDSTR,	"dup2_midstr"),
	//M(tDUP_IF_NZ,	"dup_if_nz"	),
	M(tSUPER,		"super"		),
	M(tNULLPTR,		"nullptr"	),

	M(tPUSH_CONTEXT0,	"push_context0" ),
	M(tPUSH_CONTEXT1,	"push_context1" ),
	M(tPUSH_CONTEXT2,	"push_context2" ),
	M(tPUSH_CONTEXT3,	"push_context3" ),
	M(tPOP_CONTEXT0,	"pop_context0"  ),
	M(tPOP_CONTEXT1,	"pop_context1"  ),
	M(tPOP_CONTEXT2,	"pop_context2"  ),
	M(tPOP_CONTEXT3,	"pop_context3"  ),

//	M(tCATSTR,		"catstr"	),
//	M(tCATCHAR,		"catchar"	),
	M(tADDARRAY,	"addarray"	),
	M(tADDITEM,		"additem"	),
	M(tAPPENDARRAY,	"appendarray"),
	M(tAPPENDITEM,	"appenditem"),
//	M(tADDSTRARRAY,		"addstrarray"	),
//	M(tADDSTRITEM,		"addstritem"	),
//	M(tAPPENDSTRARRAY,	"appendstrarray"),
//	M(tAPPENDSTRITEM,	"appendstritem" ),
	M(tEQSTR,		"eqstr"		),
	M(tNESTR,		"nestr"		),
	M(tGESTR,		"gestr"		),
	M(tLESTR,		"lestr"		),
	M(tGTSTR,		"gtstr"		),
	M(tLTSTR,		"ltstr"		),
	M(tFIRST,		"first"		),
	M(tLAST,		"last"		),

	M(tGETCHAR,		"getchar"	),
	M(tPUTCHAR,		"putchar"	),
	M(tPUTBYTE,		"putbyte"	),
	M(tPUTSTR,		"putstr"	),
	M(tPUTNUM,		"putnum"	),
	M(tLOGCHAR,		"logchar"	),
	M(tLOGSTR,		"logstr"	),
	M(tLOGNUM,		"lognum"	),

	M(tSPACESTR,	"spacestr"	),
	M(tSUBSTR,		"substr"	),
	M(tMIDSTR,		"midstr"	),
	M(tLEFTSTR,		"leftstr"	),
	M(tRIGHTSTR,	"rightstr"	),
	M(tNUMSTR,		"numstr"	),
	M(tERRORSTR,	"errorstr"	),
	M(tMIN,			"min"		),
	M(tMAX,			"max"		),
	M(tNOW,			"now"		),
	M(tTIME,		"time"		),
	M(tTIMESTR,		"timestr"	),
	M(tTIMEVAL,		"timeval"	),
	M(tSELF,		"self"		),
	M(tFREEZE,		"freeze"	),
	M(tVFREE,		"vfree"		),
	M(tRFREE,		"rfree"		),
	M(tTIMER,		"timer"		),
	M(tWAIT,		"wait"		),
	M(tBUSYWAIT,	"busywait"	),
	M(tSHEDULE,		"shedule"	),
	M(tRUN,			"run"		),
	M(tSETPRIO,		"setprio"	),
	M(tSUSPEND,		"suspend"	),
	M(tRESUME,		"resume"	),
	M(tTERMI,		"termi"		),
	M(tCLEAR,		"clear"		),
	M(tREQUEST,		"request"	),
	M(tRELEASE,		"release"	),
	M(tTRYREQUEST,	"tryrequest"),
	M(tLOCK,		"lock"		),
	M(tUNLOCK,		"unlock"	),
	M(tTRYLOCK,		"trylock"	),
	M(tTRIGGER,		"trigger"	),
	M(tSTDIN,		"stdin"		),
	M(tSTDOUT,		"stdout"	),
	M(tSTDERR,		"stderr"	),
	M(tEI,			"ei"		),
	M(tDI,			"di"		),
	M(tA,			"a"			),
	M(tB,			"b"			),
	M(tC,			"c"			),
	M(tI,			"i"			),

//	M(tRANDOM,		"random"	),
//	M(tSORT,		"sort"		),
//	M(tRSORT,		"rsort"		),
//	M(tSHUFFLE,		"shuffle"	),
//	M(tREVERT,		"revert"	),
//	M(tROL,			"rol"		),
//	M(tROR,			"ror"		),
//	M(tFIND,		"find"		),
//	M(tRFIND,		"rfind"		),
//	M(tJOIN,		"join"		),
//	M(tSPLIT,		"split"		),
//	M(tMSBIT,		"msbit"		),
//	M(tMSBIT0,		"msbit0"	),
//	M(tNAN,			"NaN"		),
//	M(tINF_M,		"inf_m"		),
//	M(tZERO,		"zero"		),

//	M(tESCAPE,		"escape"	),
//	M(tUNESCAPE,	"unescape"	),
//	M(tENTAB,		"entab"		),
//	M(tDETAB,		"detab"		),
//	M(tTOUTF8,		"toutf8"	),
//	M(tTOUCS1,		"toucs1"	),
//	M(tTOUCS2,		"toucs2"	),
//	M(tTOUCS4,		"toucs4"	),
//	M(tTOUPPER,		"toupper"	),
//	M(tTOLOWER,		"tolower"	),
//	M(tURLENCODE,	"urlencode"	),
//	M(tURLDECODE,	"urldecode"	),
//
//	M(tABS,			"abs"		),
//	M(tSIGN,		"sign"		),
//	M(tROUND,		"round"		),
//	M(tFLOOR,		"floor"		),
//	M(tCEIL,		"ceil"		),
//	M(tINTEG,		"integ"		),
//	M(tFRACT,		"fract"		),
//	M(tSIN,			"sin"		),
//	M(tCOS,			"cos"		),
//	M(tTAN,			"tan"		),
//	M(tASIN,		"asin"		),
//	M(tACOS,		"acos"		),
//	M(tATAN,		"atan"		),
//	M(tSINH,		"sinh"		),
//	M(tCOSH,		"cosh"		),
//	M(tEXP10,		"exp10"		),
//	M(tLOG10,		"log10"		),
//	M(tEXP2,		"exp2"		),
//	M(tLOG2,		"log2"		),
//	M(tEXPE,		"expe"		),
//	M(tLOGE,		"loge"		),
//	M(tEXP,			"exp"		),
//	M(tLOG,			"log"		),
//	M(tSQRT,		"sqrt"		),
//	M(tTANH,		"tanh"		),
//	M(tPOW,			"pow"		),
//	M(tMANT,		"mant"		),
//	M(tFREXP,		"frexp"		),
//	M(tLO,			"lo"		),
//	M(tHI,			"hi"		),
//	M(tLOHI,		"lohi"		),
//	M(tHILO,		"hilo"		),
//	M(tIS_LETTER,	"is_letter" ),
//	M(tIS_BIN_DIGIT,"is_bin_digit" ),
//	M(tIS_HEX_DIGIT,"is_hex_digit" ),
//	M(tIS_DEC_DIGIT,"is_dec_digit" ),
//	M(tDIGIT_VALUE,	"digit_value" ),
//	M(tMEMCPY,		"memcpy"	),
//	M(tRMEMCPY,		"rmemcpy"	),
//	M(tMEMMOVE,		"memmove"	),
//	M(tMEMCLR,		"memclr"	),
//	M(tMEMSET,		"memset"	),
//	M(tCHARSTR,		"charstr"	),
//	M(tHEXSTR,		"hexstr"	),
//	M(tBINSTR,		"binstr"	),
//	M(tNUMVAL,		"numval"	),
//	M(tINF,			"inf"		),
//	M(tNAN,			"nan"		),
//	M(tNUM_CORES,	"num_cores" ),
//	M(t_RANGETOEND,	"helper_range_to_end"),
//	M(tINFO_START_OF_ARRAY_ITEMS,"info: start_of_array_items"),
//	M(tINFO_END_OF_ARRAY_ITEMS,  "info: end_of_array_items"),
//	M(tSELECT,		"select"	),
//	M(tSYSTEMTIME,	"systemtime"),
//	M(tDESELECT,	"deselect"	),
//	M(tABORT,		"abort"		),
//	M(tTODO,		"todo"		),
//	M(tOOMEM,		"oomem"		),
//	M(tPANIC,		"panic"		),
//	M(tTRON,		"tron"		),
//	M(tTROFF,		"troff"		),
//
//	M(tFD,			"FD"		),
//	M(tOPEN,		"open"		),
//	M(tCLOSE,		"close"		),
//	M(tFLEN,		"flen"		),
//	M(tFPOS,		"fpos"		),
//	M(tSEEK,		"seek"		),
//	M(tTRUNCATE,	"truncate"	),
//	M(tREAD,		"read"		),
//	M(tWRITE,		"write"		),
//	M(tPRINT,		"print"		),
//	M(tSOCKET,		"socket"	),
//	M(tCONNECT,		"connect"	),
//	M(tACCEPT,		"accept"	),
//	M(tSTREAM_t,	"Stream"	),
//	M(tSTREAMS,		"streams"	),
//	M(tENV,			"env"		),
//	M(tARGS,		"args"		),
//
//	M(tMATCH,		"match"		),
//	M(tFULLPATH,	"fullpath"	),
//	M(tNODETYPE,	"nodetype"	),
//	M(tFILEINFO,	"fileinfo"	),
//	M(tREADFILE,	"readfile"	),
//	M(tREADLINK,	"readlink"	),
//	M(tREADDIR,		"readdir"	),
//	M(tCREATEFILE,	"createfile"),
//	M(tCREATELINK,	"createlink"),
//	M(tCREATEDIR,	"createdir"	),
//	M(tCREATEPIPE,	"createpipe"),
//	M(tRMVFILE,		"rmvfile"	),
//	M(tRMVLINK,		"rmvlink"	),
//	M(tRMVDIR,		"rmvdir"	),
//	M(tRENAMEFILE,	"renamefile"),
//	M(tSWAPFILES,	"swapfiles"	),
//	M(tTEMPFILE,	"tempfile"	),

//	M(tLOCAL,		"local"		),
//	M(tINET,		"inet"		),
//	M(tTCP,			"tcp"		),
//	M(tUDP,			"udp"		),
//	M(t_STREAM,		"stream"	),
//	M(tPACKET,		"packet"	),
//	M(tDATAGRAM,	"datagram"	),

//	M(tCSTRARRAY,	"4int8AEcAE¢"),		// cstr[]
//	M(tFILEINFO_t,	"FileInfo"	),		// class FileInfo

	// Z80 names:
//		M(tNZ,	"nz"	),
//		M(tZ,	"z"		),
//		M(tNC,	"nc"	),
//	//	M(tC,	"c"		),
//		M(tPO,	"po"	),
//		M(tPE,	"pe"	),
//		M(tP,	"p"		),
//		M(tM,	"m"		),
//	//	M(tB,	"b"		),
//	//	M(tC,	"c"		),
//		M(tD,	"d"		),
//		M(tE,	"e"		),
//		M(tH,	"h"		),
//		M(tL,	"l"		),
//	//	M(tA,	"a"		),
//		M(tF,	"f"		),
//		M(tXH,	"xh"	),
//		M(tXL,	"xl"	),
//		M(tYH,	"yh"	),
//		M(tYL,	"yl"	),
//	//	M(tI,	"i"		),
//		M(tR,	"r"		),
//		M(tBC,	"bc"	),
//		M(tDE,	"de"	),
//		M(tHL,	"hl"	),
//		M(tSP,	"sp"	),
//		M(tIX,	"ix"	),
//		M(tIY,	"iy"	),
//		M(tAF,	"af"	),
//	//	M(tNOP,	"nop"	),
//		M(tLD ,	"ld"	),
//		M(tINC,	"inc"	),
//		M(tDEC,	"dec"	),
//		M(tRLCA,"rlca"	),
//		M(tEX ,	"ex"	),
//		M(tRET,	"ret"	),
//		M(tPOP,	"pop"	),
//	//	M(tJP ,	"jp"	),
//		M(tRRCA,"rrca"	),
//		M(tDJNZ,"djnz"	),
//		M(tRLA,	"rla"	),
//		M(tJR ,	"jr"	),
//		M(tRRA,	"rra"	),
//		M(tDAA,	"daa"	),
//	//	M(tCPL,	"cpl"	),
//		M(tHALT,"halt"	),
//		M(tSCF,	"scf"	),
//		M(tCCF,	"ccf"	),
//	//	M(tIN ,	"in"	),
//	//	M(tOUT,	"out"	),
//	//	M(tNEG,	"neg"	),
//		M(tRETN,"retn"	),
//		M(tIM ,	"im"	),
//		M(tRETI,"reti"	),
//		M(tRRD,	"rrd"	),
//		M(tRLD,	"rld"	),
//		M(tCALL,"call"	),
//		M(tPUSH,"push"	),
//		M(tRST,	"rst"	),
//		M(tEXX,	"exx"	),
//	//	M(tDI ,	"di"	),
//	//	M(tEI ,	"ei"	),
//	//	M(tADD,	"add"	),
//		M(tADC,	"adc"	),
//	//	M(tSUB,	"sub"	),
//		M(tSBC,	"sbc"	),
//	//	M(tAND,	"and"	),
//	//	M(tXOR,	"xor"	),
//	//	M(tOR ,	"or"	),
//		M(tCP ,	"cp"	),
//		M(tRLC,	"rlc"	),
//		M(tRRC,	"rrc"	),
//		M(tRL,	"rl"	),
//		M(tRR,	"rr"	),
//		M(tSLA,	"sla"	),
//		M(tSRA,	"sra"	),
//		M(tSLL,	"sll"	),
//		M(tSRL,	"srl"	),
//		M(tLDI,	"ldi"	),
//		M(tCPI,	"cpi"	),
//		M(tINI,	"ini"	),
//		M(tOUTI,"outi"	),
//		M(tLDD,	"ldd"	),
//		M(tCPD,	"cpd"	),
//		M(tIND,	"ind"	),
//		M(tOUTD,"outd"	),
//		M(tLDIR,"ldir"	),
//		M(tCPIR,"cpir"	),
//		M(tINIR,"inir"	),
//		M(tOTIR,"otir"	),
//		M(tLDDR,"lddr"	),
//		M(tCPDR,"cpdr"	),
//		M(tINDR,"indr"	),
//		M(tOTDR,"otdr"	),
//		M(tBIT,	"bit"	),
//		M(tRES,	"res"	),
//		M(tSET,	"set"	),

//		M(tXIX,	"(hl)"	),
//		M(tXBC,	"(bc)"	),
//		M(tXDE,	"(de)"	),
//		M(tXIX,	"(ix+N)"),
//		M(tXIY,	"(iy+N)"),
//		M(tN,	"N"		),
//		M(tXN,	"(N)"	),


//	M(tYELLOW_LED, "yellow_led"	),
//	M(tRED_LED,	   "red_led"	),
//	M(tBOTH_LEDS,  "both_leds"	),
//	M(tNO_LED,	   "no_led"		),
//	M(tPERCENT,		"%%"		),			// in z80 source parser
//	M(tOPERATOR,    "operator"	),
//	M(tIDENTIFIER,  "identifier"),

	M(total_idf_ids, "total_idf_ids")


#ifdef M_was_not_defined
#undef M_was_not_defined
#undef M
  };
#endif


#endif // idf_id_h









