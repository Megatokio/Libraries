// Copyright (c) 2010 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#ifndef NAMEID_H
#define NAMEID_H
#include <stdint.h>
#define M(ID, STR) ID
enum NameID : uint32_t
#endif

#ifdef M
{
	// the table *must* start with the empty string:
	M(tEMPTYSTR = 0, ""),

	// Preprocessor Tokens:

	M(tNL, "\n"),
	M(tIVAL, "<ival>"), // special id for immediate values in preprocessor
	M(tEOF, "<eof>"),	// end of source

	// Operators:

	M(tSEMIK, ";"), // opri -2		void;void   -> void
	M(tKOMMA, ","), // opri -1		value,value -> arguments

	M(tGL, "="), // opri 0		vref=value  -> void
	M(tDPGL, ":="),
	M(tADDGL, "+="),
	M(tSUBGL, "-="),
	M(tMULGL, "*="),
	M(tDIVGL, "/="),
	M(tMODGL, "%="),
	M(tPTRGL, "->"),
	M(tANDGL, "&="),
	M(tORGL, "|="),
	M(tXORGL, "^="),
	M(tSLGL, "<<="),
	M(tSRGL, ">>="),
	M(tANDANDGL, "&&="),
	M(tORORGL, "||="),

	M(tQMARK, "?"), // opri 1
	M(tCOLON, ":"),

	M(tANDAND, "&&"), // opri 2
	M(tOROR, "||"),

	M(tEQ, "=="), // opri 3
	M(tNE, "!="),
	M(tLT, "<"),
	M(tGT, ">"),
	M(tLE, "<="),
	M(tGE, ">="),
	M(tIS, "==="),
	M(tISNOT, "!=="),

	M(tADD, "+"), // opri 4
	M(tSUB, "-"),

	M(tMUL, "*"), // opri 5
	M(tDIV, "/"),
	M(tMOD, "%"),
	M(tDIVMOD, "/%"),

	M(tAND, "&"), // opri 6
	M(tOR, "|"),
	M(tXOR, "^"),

	M(tSR, ">>"), // opri 7
	M(tSL, "<<"),
	M(tMULMUL, "**"),

	M(tINCR, "++"), // opri 9
	M(tDECR, "--"),
	M(tNOT, "!"),
	M(tNOTNOT, "!!"),
	M(tCPL, "~"),
	M(tPOS, " +"),
	M(tNEG, " -"),

	// Special Characters:

	M(tRKauf, "("),
	M(tGKauf, "{"),
	M(tEKauf, "["),
	M(tRKzu, ")"),
	M(tGKzu, "}"),
	M(tEKzu, "]"),
	M(tDOT, "."),
	M(tHASH, "#"),
	M(tAT, "@"),
	M(tDOLLAR, "$"),

	// Data Types:
	// preprocessor relies on NameID sequence

	// Basic Types:
	M(tVOID, "void"),
	M(tSHORT, "short"),
	M(tUSHORT, "ushort"),
	M(tINT, "int"),
	M(tUINT, "uint"),
	M(tLONG, "long"),
	M(tULONG, "ulong"),
	M(tBYTE, "byte"),
	M(tUBYTE, "ubyte"),
	M(tCHAR, "char"),
	M(tSTR, "str"),


	// Start of 'Identifiers':

	// type IDs:
	M(tINTEGER, "basic"),
	M(tPROC, "proc"),
	M(tSTRUCT, "struct"),
	M(tARRAY, "array"),
	M(tREF, "ref"),
	M(tPTR, "ptr"),

	// ClassDescription:
	M(tOBJECT, "Object"),
	M(tGLOBALS, "Globals"),
	M(tEXCEPTION, "Exception"),
	M(tINDEX_ERROR, "IndexError"),
	M(tLIMIT_ERROR, "LimitError"),
	M(tTYPE_ERROR, "TypeError"),
	M(tFILE_ERROR, "FileError"),
	M(tDATA_ERROR, "DataError"),
	M(tSTREAM, "Stream"),
	M(tMSG, "msg"),
	M(tFD, "fd"),
	M(tFILETYPE, "filetype"),
	M(tFILEPATH, "filepath"),
	M(tFLAGS, "flags"),
	M(tENCODING, "encoding"),
	M(tSTATUS, "status"),
	M(tIBU, "ibu"),
	M(tIBU_RP, "ibu_rp"),
	M(tIBU_WP, "ibu_wp"),
	M(tIP, "ip"),

	// Preprocessor Instructions:

	M(tENDIF, "endif"),
	M(tINCLUDE, "include"),
	M(tREQUIRE, "require"),
	M(tIFDEF, "ifdef"),
	M(tIFNDEF, "ifndef"),
	M(tDEFINE, "define"),
	M(tUNDEF, "undef"),
	M(tERROR, "error"),

	// Reserved Names:

	M(tIF, "if"),
	M(tELSE, "else"),
	M(tELIF, "elif"),
	M(tASSERT, "assert"),
	M(tTYPE, "type"),
	M(tENUM, "enum"),
	M(tRETURN, "return"),
	M(tSWITCH, "switch"),
	M(tCASE, "case"),
	M(tDO, "do"),
	M(tWHILE, "while"),
	M(tUNTIL, "until"),
	M(tEXIT, "exit"),
	M(tBREAK, "break"),
	M(tDEFAULT, "default"),
	M(tEXTERN, "extern"),
	M(tSTATIC, "static"),
	M(tCONST, "const"),
	M(tMUTABLE, "mutable"),
	M(tSCOPE, "scope"),
	M(tEND, "end"),
	M(tSIZEOF, "sizeof"),
	M(tSIZE, "size"),
	M(tCOUNT, "count"),
	M(tTO, "to"),
	M(tNEXT, "next"),

	// More:

	M(tOPERATOR, "operator"),
	M(tOPCODE, "opcode"),
	M(tASM, "asm"),
	M(tINLINE, "inline"),
	M(tREVERTED, "reverted"),
	M(tGLOBAL, "global"),
	M(tNULL, "null"),


	M(total_idf_ids, "total_idf_ids")

};
#undef M


#endif
