/*	Copyright  (c)	Günter Woigk 2018 - 2019
					mailto:kio@little-bat.de

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	Permission to use, copy, modify, distribute, and sell this software and
	its documentation for any purpose is hereby granted without fee, provided
	that the above copyright notice appear in all copies and that both that
	copyright notice and this permission notice appear in supporting
	documentation, and that the name of the copyright holder not be used
	in advertising or publicity pertaining to distribution of the software
	without specific, written prior permission.  The copyright holder makes no
	representations about the suitability of this software for any purpose.
	It is provided "as is" without express or implied warranty.

	THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
	INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
	EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
	CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
	DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
	TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
	PERFORMANCE OF THIS SOFTWARE.
*/

#undef NDEBUG
#define SAFETY 2
#define LOGLEVEL 1
#include "Libraries/kio/kio.h"
#undef  assert
#define assert(X) do{ if(X){}else{throw internal_error(__FILE__, __LINE__, "FAILED: " #X);} }while(0)
#include "Libraries/unix/FD.h"
#include "cstrings.h"
#include "ucs1.h"
#include "utf8.h"
#include "unix/tempmem.h"
#include "Libraries/Templates/Array.h"
#include "Libraries/kio/util/defines.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}



void test_cstrings(uint& num_tests, uint& num_errors)
{
	logIn("test cstrings");

	TRY
		assert(eq(emptystr,""));
	END

	TRY
		assert(is_space(' '));
		char c=0; do { assert(is_space(c) == (c<=' ' && c>0)); } while(++c);
	END

	TRY
		assert(is_uppercase('A'));
		char c=0; do { assert(is_uppercase(c) == (c>='A' && c<='Z')); } while(++c);
	END

	TRY
		assert( is_lowercase('a'));
		char c=0; do { assert(is_lowercase(c) == (c>='a' && c<='z')); } while(++c);
	END

	TRY
		assert( is_letter('a'));
		char c=0; do { assert(is_letter(c) == (to_upper(c)>='A' && to_upper(c)<='Z')); } while(++c);
	END

	TRY
		assert(to_lower('A')=='a');
		char c=0; do { assert((c>='A' && c<='Z') ? to_lower(c)==c+32 : to_lower(c)==c); } while(++c);
	END

	TRY
		assert(to_upper('a')=='A');
		char c=0; do { assert((c>='a' && c<='z') ? to_upper(c)==c-32 : to_upper(c)==c); } while(++c);
	END

	TRY
		assert( is_bin_digit('0'));
		char c=0; do { assert(is_bin_digit(c) == (c=='0'||c=='1')); } while(++c);
	END

	TRY
		assert(is_oct_digit('0'));
		char c=0; do { assert(is_oct_digit(c) == (c>='0'&&c<='7')); } while(++c);
	END

	TRY
		assert(is_dec_digit('0'));
		char c=0; do { assert(is_dec_digit(c) == (c>='0'&&c<='9')); } while(++c);
	END

	TRY
		assert(is_hex_digit('0'));
		char c=0; do { assert(is_hex_digit(c) == ((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f'))); } while(++c);
	END

	TRY
		for(char c=77;++c!=77;)
		{
			assert(is_bin_digit(c) != no_bin_digit(c));
			assert(is_dec_digit(c) != no_dec_digit(c));
			assert(is_hex_digit(c) != no_hex_digit(c));
			assert(is_oct_digit(c) != no_oct_digit(c));
		}
	END

	TRY
		assert(digit_val(('0')==0));
	END

	TRY
		assert(digit_value(('0')==0));
		assert(digit_value(('9')==9));
		assert(digit_value(('A')==10));
		assert(digit_value(('a')==10));
		assert(digit_value(('Z')==35));
		assert(digit_value(('z')==35));
		assert(digit_value(('0'-1)>=36));
		assert(digit_value(('9'+1)>=36));
		assert(digit_value(('a'-1)>=36));
		assert(digit_value(('z'+1)>=36));
		assert(digit_value(('A'-1)>=36));
		assert(digit_value(('Z'+1)>=36));
		char c=0; do { assert((digit_value(c)>=36) == (no_dec_digit(c) && !is_letter(c))); } while(++c);
	END

	TRY
		assert(hexchar(0)=='0');
		assert(hexchar(9)=='9');
		assert(hexchar(10)=='A');
		assert(hexchar(15)=='F');
		assert(hexchar(16)=='0');
	END

	TRY
		assert(strLen("123")==3);
	END

	TRY
		assert(lt("123","23"));
	END

	TRY
		assert(gt("234","1234"));
	END

	TRY
		assert(gt_tolower("big","Apple"));
	END

	TRY
		assert(eq("123","123"));
	END

	TRY
		assert(ne("123","124"));
	END

	TRY
		char s[] = "asdfghJKlJKL";
		assert(find(s,"JK")==s+6);
	END

	TRY
		char s[] = "asdfghJKlJKL";
		assert(rfind(s,"JK")==s+9);
	END

	TRY
		assert(startswith("Asdfg","Asd"));
	END

	TRY
		assert(endswith("Asdfg","fg"));
	END

	TRY
		assert(contains("Asdfg","sd"));
	END

	TRY
		assert(isupperstr("ANTON_01;"));
	END

	TRY
		assert(islowerstr("anton_01;"));
	END

	TRY
		str s = newstr(20);
		assert(s[20]==0);
		delete[] s;
	END

	TRY
		str s = newcopy("123");
		assert(eq(s,"123"));
		delete[] s;
	END

	TRY
		TempMemPool z;
		str s1 = tempstr(20); assert(s1[20]==0);
		str s2 = tempstr(1);  assert(s2 == s1-2);	// not required but expected
	END

	TRY
		TempMemPool z;
		str s1 = tempstr(20); assert(s1[20]==0);
		TempMemPool z2;
		str s2 = xtempstr(1); assert(s2 == s1-2);	// not required but expected
	END

	TRY
		assert(eq(spacestr(5,'x'),"xxxxx"));
	END

	TRY
		assert(eq(spaces(5),"     "));
		assert(spaces(5) == spaces(10)+5);	// not required but expected: short strings are in static const
	END

	TRY
		assert(eq(whitestr("\tFoo Bar\n"),"\t       \n"));
	END

	TRY
		static const char s1[4] = "123";
		assert(eq(s1,dupstr(s1)));
		assert(s1 != dupstr(s1));
	END

	TRY
		static const char s1[4] = "123";
		char* s2;
		{
			TempMemPool z;
			s2 = dupstr(s1);
			s2 = xdupstr(s2);
			purgeTempMem();
			(void) spacestr(66);
		}
		assert(eq(s1,s2));
		assert(s1 != s2);
	END

	TRY
		static const cstr s = "12345678";
		assert(eq(substr(s+2,s+5),"345"));
	END

	TRY
		assert(eq(mulstr("123",4),"123123123123"));
	END

	TRY
		assert(eq(catstr("123","45"),"12345"));
		assert(eq(catstr("A","n","ton"),"Anton"));
		assert(eq(catstr("A","n","ton","ov"),"Antonov"));
		assert(eq(catstr("A","n","ton","ov","_"),"Antonov_"));
		assert(eq(catstr("A","n","ton","ov"," ","78"),"Antonov 78"));
	END

	TRY
		assert(eq(midstr("Antonov",2,3),"ton"));
		assert(eq(midstr("Antonov",2),"tonov"));
	END

	TRY
		assert(eq(leftstr("Antonov",3),"Ant"));
	END

	TRY
		assert(eq(rightstr("Antonov",3),"nov"));
	END

	TRY
		assert(lastchar("123")=='3');
	END

	TRY
		str s = dupstr("AntoNov 123"); toupper(s);
		assert(eq(s,"ANTONOV 123"));
	END

	TRY
		str s = dupstr("AntoNov 123"); tolower(s);
		assert(eq(s,"antonov 123"));
	END

	TRY
		assert(eq(upperstr("AnToNoV 123;"),"ANTONOV 123;"));
	END

	TRY
		assert(eq(lowerstr("AnToNoV 123;"),"antonov 123;"));
	END

	TRY
		assert(eq(replacedstr("Beet",'e','o'),"Boot"));
	END

	TRY
		assert(eq(escapedstr("123\tabc\n"),"123\\tabc\\n"));
	END

	TRY
		assert(eq(unescapedstr("123\\tabc\\n"),"123\tabc\n"));
	END

	TRY
		assert(eq(quotedstr("123\tabc\n"),"\"123\\tabc\\n\""));
	END

	TRY
		assert(eq(unquotedstr("\"123\\tabc\\n\""),"123\tabc\n"));
	END

	TRY
		assert(eq(tohtmlstr("a<int&>\n"),"a&lt;int&amp;&gt;<br>"));
	END

	TRY
		assert(eq(fromhtmlstr("a&lt;int&amp;&gt;<br>"),"a<int&>\n"));
		assert(eq(fromhtmlstr("foo<br>a&lt;int&amp;&gt;<br>"),"foo\na<int&>\n"));
	END

	TRY
		static const uchar s[] = {'a',0xC4,0xFF,0};
		assert(strlen(toutf8str(cstr(s)))==5);
		assert(eq(toutf8str(cstr(s)),"aÄÿ"));
	END

	TRY
		static const uchar s[] = {'a',0xC4,0xFF,0};
		assert(eq(fromutf8str("aÄÿ"),cstr(s)));
	END

	TRY
		static const uchar s[] = {'a',0xC4,0xFF,' ','\n',0};
		assert(eq(unhexstr("61C4ff200A"),cstr(s)));
	END

	//TRY
	//	str uuencodedstr(cstr)
	//END

	//TRY
	//	str uudecodedstr(cstr)
	//END

	TRY
		assert(eq(base64str("Die Fo01[]"),"RGllIEZvMDFbXQ=="));
		assert(eq(base64str("oDie Fo01[]"),"b0RpZSBGbzAxW10="));
		assert(eq(base64str("o Die Fo01[]"),"byBEaWUgRm8wMVtd"));
	END

	TRY
		assert(eq(unbase64str("byBEaWUgRm8wMVtd"),"o Die Fo01[]"));
		assert(eq(unbase64str("b0RpZSBGbzAxW10="),"oDie Fo01[]"));
		assert(eq(unbase64str("RGllIEZvMDFbXQ=="),"Die Fo01[]"));
	END

	TRY
		assert(eq(croppedstr("\t hallo\n"),"hallo"));
	END

	TRY
		assert(eq(detabstr(" \thallo\t\n",4),"    hallo   \n"));
	END

	TRY
		assert(eq(usingstr("foo%s","bar"),"foobar"));
	END

	TRY
		assert(eq(tostr(1.5f),"1.5"));
		assert(eq(tostr(1.5),"1.5"));
		assert(eq(tostr(1.5L),"1.5"));
		assert(eq(tostr(uint64(12345)),"12345"));
		assert(eq(tostr(int64(-12345)),"-12345"));
		assert(eq(tostr(uint32(12345)),"12345"));
		assert(eq(tostr(int32(-12345)),"-12345"));
		assert(eq(tostr("foo"),"\"foo\""));
	END

	TRY
		assert(eq(binstr(0x2345),"01000101"));
		assert(eq(binstr(0x2345,"abcdefghijkl","ABCDEFGHIJKL"),"abCDeFghiJkL"));
		assert(eq(binstr(0x2345,"0000.0000.0000","1111.1111.1111"),"0011.0100.0101"));
	END

	TRY
		assert(eq(hexstr(0x2345,8),"00002345"));
		assert(eq(hexstr(0x2345L,8),"00002345"));
		assert(eq(hexstr(0xC3456789ABCDEF01L,16),"C3456789ABCDEF01"));
		assert(eq(hexstr(0x2345LL,8),"00002345"));
		assert(eq(hexstr(0xC3456789ABCDEF01LL,16),"C3456789ABCDEF01"));
		assert(eq(hexstr(" 0Ab"),"20304162"));
	END

	TRY
		assert(eq(charstr('c'),"c"));
		assert(eq(charstr('A','B'),"AB"));
		assert(eq(charstr('A','B','C'),"ABC"));
		assert(eq(charstr('A','B','C','D'),"ABCD"));
		assert(eq(charstr('A','B','C','D','E'),"ABCDE"));
	END

	TRY
		time_t m = 60;
		time_t h = 60*m;
		time_t d = 24*h;
		time_t y = 365*d;
		time_t y1970 = 0;
		time_t y1980 = y1970 + 10*y + 2*d;
		time_t y1990 = y1980 + 10*y + 3*d;
		time_t y2000 = y1990 + 10*y + 2*d;
		time_t y2010 = y2000 + 10*y + 3*d;
		time_t jan=0,        feb=jan+31*d, mrz=feb+28*d, apr=mrz+31*d, mai=apr+30*d, jun=mai+31*d,
			   jul=jun+30*d, aug=jul+31*d, sep=aug+31*d, okt=sep+30*d, nov=okt+31*d, dez=nov+30*d;

		// These tests fail if your timezone ist not CET/CEST:
		assert(eq(datestr(31*d),"1970-02-01"));
		assert(eq(datestr(31*d-1),"1970-02-01"));
		assert(eq(datestr(31*d -1*h),"1970-02-01"));		// CET is 1h ahead
		assert(eq(datestr(31*d -2*h -1),"1970-01-31"));
		assert(eq(timestr(31*d +45*m +21),"01:45:21"));
		assert(eq(datetimestr(y1970 +feb +45*m +21),"1970-02-01 01:45:21"));
		assert(eq(datetimestr(y1990 +feb +45*m +21),"1990-02-01 01:45:21"));
		assert(eq(datetimestr(y1990 +feb +45*m +21),"1990-02-01 01:45:21"));
		assert(eq(datetimestr(y2000 +feb +45*m +21),"2000-02-01 01:45:21"));
		assert(eq(datetimestr(y2000 +feb +28*d +45*m +21),"2000-02-29 01:45:21"));
		assert(eq(datetimestr(y2000 +jun +1*d +28*d -1*h +5*m +21),"2000-06-29 01:05:21"));
		assert(eq(datetimestr(y2010 +dez +23*d -1*h +18*h +30*m +0),"2010-12-24 18:30:00"));
		assert(eq(datetimestr(y2010 +jan -1*h -1),"2009-12-31 23:59:59"));
	END

	TRY
		// This test fails if your timezone ist not CET:
		assert(dateval("1970-02-01 01:45:21") == 31*24*60*60 +45*60 +21);

		for(int i=0;i<99;i++)
		{
			time_t n = random();
			assert(dateval(datetimestr(n))==n);
		}
	END

	TRY
		time_t m = 60;
		time_t h = 60*m;
		time_t d = 24*h;
		assert(eq(durationstr(123),"123 sec."));
		assert(eq(durationstr(1230),"20m:30s"));
		assert(eq(durationstr(3*h+5*m+33),"3h:05m:33s"));
		assert(eq(durationstr(2*d+11*h+15*m+33),"2d:11h:15m"));
		assert(eq(durationstr(1230.1),"20m:30s"));
		assert(eq(durationstr(123.145),"123.145 sec."));
		assert(eq(durationstr(66.1),"66.100 sec."));
	END

	TRY
		char s1[] = "Die Kuh lief um den Teich herum zum Wasser.\n";
		Array<str> array;
		split(array,s1,' ');
		assert(array.count()==9 && eq(array[0],"Die") && eq(array[8],"Wasser.\n"));

		char s2[] = "Die Kuh\nlief um\rden Teich\n\rherum zum\r\nWasser.\n";
		split(array,s2);
		assert(array.count()==5 && eq(array[0],"Die Kuh") && eq(array[4],"Wasser."));

		char s3[] = "";
		split(array,s3,int(0));
		assert(array.count()==0);
	END

	TRY
		str s = dupstr("123");
		strcpy(s,"abc",0);
		assert(eq(s,"123"));
		strcpy(s,"abc",1);
		assert(eq(s,""));
		strcpy(s,"abc",3);
		assert(eq(s,"ab"));
		strcpy(s,"ABC",4);
		assert(eq(s,"ABC"));
		strcpy(s,"",99);
		assert(eq(s,""));
		strcpy(s,nullptr,99);
		assert(eq(s,""));
	END

	TRY
		str s = tempmem(99);
		strcpy(s,"123",99);
		assert(eq(s,"123"));
		strcat(s,"456",99);
		assert(eq(s,"123456"));
		s[8]='x'; s[9]=0;
		strcat(s,"789",8);
		assert(eq(s,"1234567"));
	END

	TRY
		assert(ucs1::is_uppercase('A'));
		assert(ucs1::is_uppercase(char(0xc1)));
		assert(!ucs1::is_uppercase(char(0)));
		char c=0; do
		{
			assert(ucs1::is_uppercase(c) == ((c>='A' && c<='Z') || (c>=char(0xc0) && c<=char(0xde) && c!=char(0xd7))));
		}
		while(++c);
	END

	TRY
		assert(ucs1::is_lowercase('a'));
		assert(ucs1::is_lowercase(char(0xe1)));
		char c=0; do
		{
			assert(ucs1::is_lowercase(c) == ((c>='a' && c<='z') ||
				(c>=char(0xdf) && c<=char(0xff) && c!=char(0xf7))));
		}
		while(++c);
	END

	TRY
		errno=0;
		for(int i=0; i<=255; i++)
		{
			assert(utf8::is_fup(char(i)) == (i>=0x80 && i<0xC0));
			assert(utf8::no_fup(char(i)) != (i>=0x80 && i<0xC0));
		}
		assert(utf8::charcount("\t1hGfr&%<'")==10);
		assert(utf8::charcount("ß1≥€•@∆º¶§")==10);
		assert(utf8::max_csz("\t1hGfr&%<'")==1);
		assert(utf8::max_csz("ß2≥€•@∆º¶§")==2);

		ucs4char c1 = 0x09999;
		ucs4char c2 = 0x10000;
		assert(utf8::max_csz(catstr("\t1hGf∆º¶§",utf8::to_utf8(&c1,1),"x")) == 2);
		assert(utf8::max_csz(catstr("\t1hGf∆º¶§",utf8::to_utf8(&c2,1),"x")) == 4);

		assert(utf8::fits_in_ucs1("\t1hGfr&%<'"));
		assert(!utf8::fits_in_ucs1("ß≤4€•@∆º¶§"));
		assert(utf8::fits_in_ucs2("ß3≥€•@∆º¶§"));
		assert(!utf8::fits_in_ucs2(catstr("\t1hGfr&%<'",utf8::to_utf8(&c2,1))));
		assert(errno==0);
	END

	TRY
		static const char s1[] = "\t1hGfr&%<'";
		static const char s2[] = "ß≤5€•@∆º¶§";

		errno=0;
		uint8 bu1[20], *e1 = utf8::utf8_to_ucs1(s1,bu1);
		assert(utf8::utf8len(bu1,uint(e1-bu1)) == NELEM(s1)-1);
		assert(errno==0);
		uint16 bu2[20], *e2 = utf8::utf8_to_ucs2(s2,bu2);
		assert(utf8::utf8len(bu2,uint(e2-bu2)) == NELEM(s2)-1);
		assert(errno==0);
		uint32 bu3[20], *e3 = utf8::utf8_to_ucs4(s2,bu3);
		assert(utf8::utf8len(bu3,uint(e3-bu3)) == NELEM(s2)-1);
		assert(errno==0);
	END

	TRY
		errno=0;
		uint32 bu[10],bu2[10];
		for(uint i=0;i<NELEM(bu);i++) { bu[i] = uint32(random()) + uint32(random()) * 0x10000u; }
		uint n = utf8::utf8len(bu,10);
		str s = tempstr(n);
		ptr e = utf8::ucs4_to_utf8(bu,10,s);
		assert(*e==0);
		assert(e-s==n);
		assert(utf8::charcount(s)==10);
		ssize_t len = utf8::utf8_to_ucs4(s,bu2) - bu2;
		assert(len==10);
		assert(memcmp(bu,bu2,sizeof(bu))==0);
		assert(errno==0);
	END

	TRY	// ucs4_to_utf8(), utf8_to_ucs4()
		char bu[20];
		errno = 0;
		for (int nbits=0; nbits<=32; nbits++)				// max num of bits
		{
			for( int i=0; i<200; i++ )
			{
				uint32 n[2], m[2];
				n[0] = uint32(random()) + uint32(random()) * 0x10000; if(nbits<32) n[0] &= RMASK(nbits);
				n[1] = uint32(random()) + uint32(random()) * 0x10000; if(nbits<32) n[1] &= RMASK(nbits);

				ptr e = utf8::ucs4_to_utf8(n,2,bu);
				assert(*e==0);
				assert(utf8::charcount(bu) == 2);
				assert(utf8::utf8_to_ucs4(bu,m) == m+2);
				assert(memcmp(n,m,sizeof(n)) == 0);
			}
		}
		assert(errno==0);
	END

	TRY	// ucs2_to_utf8(), utf8_to_ucs2()
		char bu[20];
		errno = 0;
		for (int nbits=0; nbits<=16; nbits++)				// max num of bits
		{
			for( int i=0; i<100; i++ )
			{
				uint16 n[2], m[2];
				n[0] = uint16(uint(random()) & RMASK(nbits));
				n[1] = uint16(uint(random()) & RMASK(nbits));

				ptr e = utf8::ucs2_to_utf8(n,2,bu);
				assert(*e==0);
				assert(utf8::charcount(bu) == 2);
				assert(utf8::utf8_to_ucs2(bu,m) == m+2);
				assert(memcmp(n,m,sizeof(n)) == 0);
			}
		}
		assert(errno==0);
	END

	TRY	// ucs1_to_utf8(), utf8_to_ucs1()
		char bu[20];
		errno = 0;
		for (int nbits=0; nbits<=8; nbits++)				// max num of bits
		{
			for( int i=0; i<100; i++ )
			{
				uint8 n[2], m[2];
				n[0] = uint8(uint(random()) & RMASK(nbits));
				n[1] = uint8(uint(random()) & RMASK(nbits));

				ptr e = utf8::ucs1_to_utf8(n,2,bu);
				assert(*e==0);
				assert(utf8::charcount(bu) == 2);
				assert(utf8::utf8_to_ucs1(bu,m) == m+2);
				assert(memcmp(n,m,sizeof(n)) == 0);
			}
		}
		assert(errno==0);
	END

	TRY
		assert(eq(utf8::fromhtmlstr("a&lt;int&amp;&gt;<br>"),"a<int&>\n"));
		assert(eq(utf8::fromhtmlstr("foo<br>a&lt;int&amp;&gt;<br>"),"foo\na<int&>\n"));
	END

	TRY
		assert(eq(utf8::detabstr(" \thällö\t\n",4),"    hällö   \n"));
	END
}






















