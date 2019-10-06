/*	Copyright  (c)	Günter Woigk 2001 - 2019
					mailto:kio@little-bat.de

	This file is free software.

	Permission to use, copy, modify, distribute, and sell this software
	and its documentation for any purpose is hereby granted without fee,
	provided that the above copyright notice appears in all copies and
	that both that copyright notice, this permission notice and the
	following disclaimer appear in supporting documentation.

	THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT ANY WARRANTY, NOT EVEN THE
	IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
	AND IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES
	ARISING FROM THE USE OF THIS SOFTWARE,
	TO THE EXTENT PERMITTED BY APPLICABLE LAW.
*/

#undef NDEBUG
#define SAFETY 2
#define LOGLEVEL 1
#include <time.h>
#include "kio/kio.h"
#undef  assert
#define assert(X) do{ if(X){}else{throw internal_error(__FILE__, __LINE__, "FAILED: " #X);} }while(0)

#include "String.h"

#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}


void logOK() { log(" ...ok"); logNl(); }
#define TRAP(X) assert(!(X))
#define	RMASK(n) (~(0xFFFFFFFF<<(n)))					// mask to select n bits from the right (BUMMER: max. 31!!)
static uint random(uint n) { return (uint32(n) * uint16(random())) >> 16; } // 16 bit random number in range [0 ... [n




void TestStringClass(uint& num_tests, uint& num_errors)
{
	logIn("test VString/String");

	//{
	//	log("= %i",(int)sizeof(String));
	//	TRAP(sizeof(String)!=20);
	//}


	TRY // ~String()
		String a,b(33),c(40,csz4),d(50,' ');
		String e(c,13,33),f(e);
		String viele[500];
		String g(viele[123]);
	END

	TRY // String()
		String s;
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
	END

	TRY // String(UCS4Char)
		String a('a');
		TRAP( a.Csz()!=1 );
		TRAP( a.Len()!=1 );
		TRAP( a[0]!='a' );
		//TRAP( a.NotWritable() );
		for (uint i=0;i<256;i++)
		{
			String b(i);
			TRAP( b.Csz()!=1 );
			TRAP( b.Len()!=1 );
			TRAP( b[0]!=i );
			//TRAP( b.NotWritable() );
		}

		a = String(300);
		TRAP( a.Csz()!=2 );
		TRAP( a.Len()!=1 );
		TRAP( a[0]!=300 );
		//TRAP( a.NotWritable() );
		for (uint i=256;i<0x10000;i+=17)
		{
			String b(i);
			TRAP( b.Csz()!=2 );
			TRAP( b.Len()!=1 );
			TRAP( b[0]!=i );
			//TRAP( b.NotWritable() );
		}

		a = String(0x10030);
		TRAP( a.Csz()!=4 );
		TRAP( a.Len()!=1 );
		TRAP( a[0]!=0x10030 );
		//TRAP( a.NotWritable() );
		for (ucs4char i=0x10000;i<0xF0000000;i+=i/2001)
		{
			String b(i);
			TRAP( b.Csz()!=4 );
			TRAP( b.Len()!=1 );
			TRAP( b[0]!=i );
			//TRAP( b.NotWritable() );
		}
	END

	TRY // String(cUCS1CharPtr,long)
		ucs1char c[]={ 'a','n','t','o','n','x','x' };
		String s(c,5);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<5;i++) TRAP(s.UCS1Text()[i]!=c[i]);

		s=String(c,0);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
	END

	TRY // String(cUCS2CharPtr,long)
		ucs2char c[]={ 'a','n','t','o','n','x','x' };
		String s(c,5);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		for(int i=0;i<5;i++) TRAP(s.UCS2Text()[i]!=c[i]);

		s=String(c,0);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
	END

	TRY // String(cUCS4CharPtr,long)
		ucs4char c[]={ 'a','n','t','o','n','x','x' };
		String s(c,5);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=4);
		TRAP(s.NotWritable());
		for(int i=0;i<5;i++) TRAP(s.UCS4Text()[i]!=c[i]);

		s=String(c,0);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
	END

	TRY // String(cUTF8CharPtr,long)
		cstr c={ "antonxx" };
		String s(c,5);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) TRAP( s[i] != ucs1char("anton"[i]) );

		ucs2char c3[]={ 'a','n','t',0xf6,'n','z','x' };
		s=String("antönxx",6);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) TRAP( s[i] != c3[i] );

		ucs2char c5[]={ 'a','n','t',0x160,'n','z','x' };
		s=String("antŠnxx",6);
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) TRAP( s[i] != c5[i] );

		ucs4char c2[]={'<','a','h','a',8226,8364,'>'};
		s=String("<aha•€>",11);
		TRAP(s.Len()!=7);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) { TRAP(s[i]!=c2[i]); }

		s=String(c,0);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
	END

	TRY // String(long,CharSize)
		String s(66,csz1);
		TRAP(s.Csz()!=1);
		TRAP(s.Len()!=66);
		TRAP(s.NotWritable());

		s=String(0,csz1);
		TRAP(s.Csz()!=1);
		TRAP(s.Len()!=0);
		TRAP(s.NotWritable());

		s=String(77,csz2);
		TRAP(s.Csz()!=2);
		TRAP(s.Len()!=77);
		TRAP(s.NotWritable());

		s=String(0,csz2);
		TRAP(s.Csz()!=1);
		TRAP(s.Len()!=0);
		TRAP(s.NotWritable());

		s=String(33,csz4);
		TRAP(s.Csz()!=4);
		TRAP(s.Len()!=33);
		TRAP(s.NotWritable());

		s=String(0,csz4);
		TRAP(s.Csz()!=1);
		TRAP(s.Len()!=0);
		TRAP(s.NotWritable());
	END

	TRY // String(long,UCS4Char)
		TRAP(String(7,'a')!="aaaaaaa");
		TRAP(String(int32(0),'s')!=emptyString);
		TRAP(String(4,0x160)!="ŠŠŠŠ");
		TRAP(String(7,' ')!="       ");

		String s = String(int32(0),99999);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());

		s = String(int32(0),999);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());

		s = String(int32(0),99);
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());

		s = String(9,'a');
		TRAP(s.Len()!=9);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		TRAP(s!="aaaaaaaaa");

		s = String(9,0x160);
		TRAP(s.Len()!=9);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		TRAP(s!="ŠŠŠŠŠŠŠŠŠ");
	END

	TRY // String(cUTF8Str)
		String s=String("Trottellumme_0123FF");
		TRAP(s.Len()!=19);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) TRAP(s[i] != uint("Trottellumme_0123FF"[i]));

		uchar c[]={'G','r',0xfc,0xdf,' ','G','o','t','t','!'};
		s=String("Grüß Gott!");
		TRAP(s.Len()!=10);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) TRAP(s[i]!=c[i]);

		s=String("");
		TRAP(s.Len()!=0);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());

		ucs4char c2[]={'<','a','h','a',8226,8364,'>'};
		s=String("<aha•€>");
		TRAP(s.Len()!=7);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) { TRAP(s[i]!=c2[i]); }

		uchar bogus[] = { 0x80,'a',0x93,'b',0xbf,0xbf,0xc0,0xc1,0xc2,0x80,0 };
		uchar anton[] = {      'a',     'b',          '?', '?', 0x80,       };
		s=String(ptr(bogus));
		TRAP(s.Len()!=5);
		TRAP(s.Csz()!=1);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) { TRAP(s[i]!=anton[i]); }

		uchar  bummer[] = { 0x80,'a',0x93,'b',0xbf,0xbf,0xc0,0xc1,0xc2,0x80,0xdf,0xbf,0 };
		ushort anton2[] = {      'a',     'b',          '?', '?', 0x80,     (1<<11)-1      };
		s=String(ptr(bummer));
		TRAP(s.Len()!=6);
		TRAP(s.Csz()!=2);
		TRAP(s.NotWritable());
		for(int i=0;i<s.Len();i++) { TRAP(s[i]!=anton2[i]); }
	END

	TRY // String(cString)
		String s="test";
		String a(s);
		TRAP(s!=a);
		TRAP(s.IsWritable());
		TRAP(a.IsWritable());
		TRAP(a.Csz()!=1);
		TRAP(a.Len()!=4);
		TRAP(a!="test");

		s=String("<aha•€>");
		TRAP(a.NotWritable());
		TRAP(s.NotWritable());
		String b(s);
		TRAP(s!=b);
		TRAP(s.IsWritable());
		TRAP(b.IsWritable());
		TRAP(b.Csz()!=2);
		TRAP(b.Len()!=7);
		TRAP(b!="<aha•€>");
	END

	TRY // String(cString,long,long)
		TRAP(String("Esmeralda",1,6) != "smera");
		TRAP(String("<aha•€>",1,6) != "aha•€");
		TRAP(String("Esmeralda",-1,6) != "Esmera");
		TRAP(String("<aha•€>",-111,6) != "<aha•€");
		TRAP(String("Esmeralda",1,66) != "smeralda");
		TRAP(String("<aha•€>",1,66) != "aha•€>");
		TRAP(String("Esmeralda",111,66) != "");
		TRAP(String("Esmeralda",66,111) != "");
		TRAP(String("Esmeralda",5,2) != "");
		TRAP(String("Esmeralda",5,5) != "");
		TRAP(String("Esmeralda",0,0) != "");
		TRAP(String("Esmeralda",0,-1) != "");
		TRAP(String("Esmeralda",9,9) != "");
		TRAP(String("Esmeralda",10,9) != "");
		TRAP(String("Esmeralda",9,10) != "");
	END

	TRY // compare(cString)
		String a[]={""," ","2•","2€","A","Antimon","Anton","Antonov","a","a ","aa","aaa","aab","anton","b","b ","berta","cÄsar","cäsar","Äffchen","•","•2"};
		for( int i=0;i<int(NELEM(a));i+=2 ) a[i] = (SpaceString(i)+a[i]).MidString(i);
		for( int i=0;i<int(NELEM(a));i++ )
		for( int j=0;j<int(NELEM(a));j++ )
		{
			TRAP ( sign(a[i].compare(a[j])) != sign(i-j) );
		}
	END

	TRY // Truncate(long)
		String s="antonov";
		s.Truncate(5);
		TRAP(s!="anton");
		s.Truncate(99);
		TRAP(s!="anton");
		s.Truncate(0);
		TRAP(s!="");
		s="antonov";
		s.Truncate(-1);
		TRAP(s!="");
	END

	TRY // Resize(long,UCS4Char)
		String s="antonov";
		s.Resize(5);
		TRAP(s!="anton");
		s.Resize(9);
		TRAP(s!="anton    ");
		s.Resize(0);
		TRAP(s!="");
		s="antonov";
		s.Resize(-1);
		TRAP(s!="");
		s.Resize(10,'x');
		TRAP(s!="xxxxxxxxxx");
		s="antonov";
		s.Resize(10,'x');
		TRAP(s!="antonovxxx");
		s="anton";
		s.Resize(6,8364);
		TRAP(s!="anton€");
		s.Resize(10,'x');
		TRAP(s!="anton€xxxx");
	END

	TRY // Len()
		TRAP(SpaceString(0).Len()!=0);
		TRAP(SpaceString(1).Len()!=1);
		TRAP(SpaceString(55).Len()!=55);
		TRAP(SpaceString(999).Len()!=999);
		TRAP(String("ö").Len()!=1);
		TRAP(String("€").Len()!=1);
		TRAP(String("22,--€").Len()!=6);
		TRAP(String(0x1010).Len()!=1 );
		TRAP(String(22,0x1010).Len()!=22 );
		TRAP(String(0x10010).Len()!=1 );
		TRAP(String(22,0x10010).Len()!=22 );
		String s="xxanton";s=s.MidString(2);
		TRAP(s.Len()!=5);
	END

	TRY // Csz()
		TRAP( String("").Csz()!=1 );
		TRAP( String("anton").Csz()!=1 );
		TRAP( String("Äffe").Csz()!=1 );
		TRAP( String("€").Csz()!=2 );
		TRAP( String("33,€").Csz()!=2 );
		TRAP( String(0x1010).Csz()!=2 );
		TRAP( String(77,0x1010).Csz()!=2 );
		TRAP( String(0x10010).Csz()!=4 );
		TRAP( String(77,0x10010).Csz()!=4 );
	END

	TRY // operator[](long)
		String s = "antonov";
		TRAP(s[0]!='a');
		TRAP(s[1]!='n');
		TRAP(s[s.Len()-1]!='v');
		s = "antonov€ic";
		TRAP(s[0]!='a');
		TRAP(s[1]!='n');
		TRAP(s[7]!=8364);
		TRAP(s[s.Len()-1]!='c');
		s+=String(0x101010);
		TRAP(s[s.Len()-1]!=0x101010);
		TRAP(s[0]!='a');
		TRAP(s[1]!='n');
		TRAP(s[7]!=8364);
		s=s.MidString(2);
		TRAP(s[0]!='t');
		TRAP(s[1]!='o');
		TRAP(s[5]!=8364);
		s = "anton_0815";
		s = s.MidString(2);
		TRAP(s[0]!='t');
		TRAP(s[7]!='5');
		s = "ant€n_0815";
		s = s.MidString(2);
		TRAP(s[0]!='t');
		TRAP(s[7]!='5');
	END

	TRY // UCS1Text()
		String s="xxanton";
		TRAP(String(s.UCS1Text()+2,5)!="anton");
		s=s.MidString(2);
		TRAP(String(s.UCS1Text()+0,5)!="anton");
	END

	TRY // UCS2Text()
		String s="x€anton";
		TRAP(String(s.UCS2Text()+2,5)!="anton");
		s=s.MidString(2);
		TRAP(String(s.UCS2Text()+0,5)!="anton");
	END

	TRY // UCS4Text()
		String s=String(0x101010)+"€anton";
		TRAP(String(s.UCS4Text()+2,5)!="anton");
		s=s.MidString(2);
		TRAP(String(s.UCS4Text()+0,5)!="anton");
	END

	TRY // Text()
		String s="xxanton";
		TRAP(s.Text()!=ptr(s.UCS1Text()));
		s="x€anton";
		TRAP(s.Text()!=ptr(s.UCS2Text()));
		s=String(0x101010)+"€anton";
		TRAP(s.Text()!=ptr(s.UCS4Text()));
	END

	TRY // IsWritable()
		String s = String(55,csz1);
		TRAP(!s.IsWritable());
		s = String(55,csz2);
		TRAP(!s.IsWritable());
		s = String(55,csz4);
		TRAP(!s.IsWritable());
		String z(s);
		TRAP(s.IsWritable());
		TRAP(z.IsWritable());
		z="";
		TRAP(!s.IsWritable());
		TRAP(!z.IsWritable());
		z=s.SubString(3,5);
		TRAP(s.IsWritable());
		TRAP(z.IsWritable());
	END


	TRY // NotWritable()
		String s = String(55,csz1);
		TRAP(s.NotWritable());
		s = String(55,csz2);
		TRAP(s.NotWritable());
		s = String(55,csz4);
		TRAP(s.NotWritable());
		String z(s);
		TRAP(!s.NotWritable());
		TRAP(!z.NotWritable());
		z="";
		TRAP(s.NotWritable());
		TRAP(z.NotWritable());
		z=s.SubString(3,5);
		TRAP(!s.NotWritable());
		TRAP(!z.NotWritable());
	END

	TRY // MakeWritable()
		String s="anton";
		s.MakeWritable();
		TRAP(s.NotWritable());
		TRAP(s!="anton");
		String z(s);
		z.MakeWritable();
		TRAP(z.NotWritable());
		TRAP(z!="anton");
		TRAP(s.NotWritable());
		TRAP(s!="anton");
		z.UCS1Text()[0] = 'A';
		TRAP(s!="anton");
		TRAP(z!="Anton");
	END

	TRY // SubString(long,long)
		String s="anton";
		TRAP(s.SubString(0,s.Len())!=s);
		TRAP(s.SubString(0,3)!="ant");
		TRAP(s.SubString(1,3)!="nt");
		TRAP(s.SubString(1,8)!="nton");
		TRAP(s.SubString(1,5)!="nton");
		TRAP(s.SubString(-1,3)!="ant");
		TRAP(s.SubString(3,1)!="");
		TRAP(s.SubString(1,1)!="");
		TRAP(s.SubString(8,8)!="");
	END

	TRY // MidString(long,long)
		String s="anton";
		TRAP(s.MidString(0,s.Len())!=s);
		TRAP(s.MidString(0,3)!="ant");
		TRAP(s.MidString(1,2)!="nt");
		TRAP(s.MidString(1,7)!="nton");
		TRAP(s.MidString(1,4)!="nton");
		TRAP(s.MidString(-1,4)!="ant");
		TRAP(s.MidString(3,-2)!="");
		TRAP(s.MidString(1,0)!="");
		TRAP(s.MidString(8,0)!="");
		SpaceString(33,33);
	END

	TRY // MidString(long)
		String s="anton";
		TRAP(s.MidString(0)!=s);
		TRAP(s.MidString(-1)!=s);
		TRAP(s.MidString(1)!="nton");
		TRAP(s.MidString(-1)!=s);
		TRAP(s.MidString(4)!="n");
		TRAP(s.MidString(5)!="");
	END

	TRY // LeftString(long)
		String s="anton";
		TRAP(s.LeftString(s.Len())!=s);
		TRAP(s.LeftString(3)!="ant");
		TRAP(s.LeftString(0)!="");
		TRAP(s.LeftString(8)!="anton");
		TRAP(s.LeftString(-1)!="");
	END

	TRY // RightString(long)
		String s="anton";
		TRAP(s.RightString(s.Len())!=s);
		TRAP(s.RightString(3)!="ton");
		TRAP(s.RightString(0)!="");
		TRAP(s.RightString(8)!="anton");
		TRAP(s.RightString(-1)!="");
	END

	TRY // operator=(cString)
		String s = "anton";
		TRAP(s!="anton");
		TRAP(s.NotWritable());
		String z = s;
		TRAP(s.IsWritable());
		TRAP(z.IsWritable());
		TRAP(s!="anton");
		TRAP(z!="anton");
		s="";
		TRAP(s!="");
		s="€";
		TRAP(s!="€");
	END

	TRY // operator+(cString)
		String s="anto";
		TRAP(s+""!=s);
		s="antonov";
		TRAP(s+""!=s);
		s="anton";
		TRAP(s+""!=s);
		TRAP(String("")+s!=s);
		TRAP(String("")+s!=s);
		TRAP(s+s!="antonanton");
		TRAP(s+String("xyz")!="antonxyz");
		TRAP(s+"xyz"!="antonxyz");
		TRAP(String("")+""!="");
		s = s + "xyz";
		s = s.LeftString(5) + s.RightString(3);
		TRAP(s!="antonxyz");
		s = s.RightString(3) + s.LeftString(5);
		TRAP(s!="xyzanton");
		s = s.RightString(5) + "x€z";
		s = s.LeftString(5) + s.RightString(3);
		TRAP(s!="antonx€z");
		s = s.RightString(3) + s.LeftString(5);
		TRAP(s!="x€zanton");
	END

	TRY // operator+=(cString)
		String s="";
		TRAP((s+="")!="");
		TRAP(s!="");
		TRAP((s+="anton")!="anton");
		TRAP(s!="anton");
		TRAP((s+="bert")!="antonbert");
		TRAP(s!="antonbert");
		TRAP((s+="")!="antonbert");
		TRAP(s!="antonbert");
		s = s.MidString(5);
		s += "xyz";
		TRAP(s!="bertxyz");
		s += s.RightString(4);
		TRAP(s!="bertxyztxyz");
		s = s.RightString(3);
		s += "ant€n";
		TRAP(s!="xyzant€n");
		s += s.RightString(5);
		TRAP(s!="xyzant€nant€n");
	END

	TRY // operator*(long)
		String s="";
		TRAP(s*0!="");
		TRAP(s*1!="");
		TRAP(s*5!="");
		TRAP(s*-5!="");
		s="X";
		TRAP(s*0!="");
		TRAP(s*1!="X");
		TRAP(s*5!="XXXXX");
		TRAP(s*-5!="");
		s="anton";
		TRAP(s*0!="");
		TRAP(s*1!="anton");
		TRAP(s*5!="antonantonantonantonanton");
		TRAP(s*-5!="");
		s=s.RightString(3);
		TRAP(s*5!="tontontontonton");
	END

	TRY // operator*=(long)
		String s="anton";
		TRAP((s*=3)!="antonantonanton");
		TRAP(s!="antonantonanton");
		s="";
		TRAP((s*=3)!="");
		TRAP(s!="");
		s="anton";
		TRAP((s*=1)!="anton");
		TRAP(s!="anton");
		s="anton";
		TRAP((s*=0)!="");
		TRAP(s!="");
		s="beton";
		s=s.RightString(3);
		s*=5;
		TRAP(s!="tontontontonton");
	END

	TRY // operator==(cString)			uses compare(cString) --> only basic tests here
		TRAP  ( String("a")=="b" );
		TRAP  ( String("b")=="a" );
		assert( String("a")=="a" );
	END

	TRY // operator!=(cString)			uses compare(cString) --> only basic tests here
		assert( String("a")!="b" );
		assert( String("b")!="a" );
		TRAP  ( String("a")!="a" );
	END

	TRY // operator<(cString)			uses compare(cString) --> only basic tests here
		assert( String("a")<"b" );
		TRAP  ( String("b")<"a" );
		TRAP  ( String("a")<"a" );
	END

	TRY // operator>(cString)			uses compare(cString) --> only basic tests here
		TRAP  ( String("a")>"b" );
		assert( String("b")>"a" );
		TRAP  ( String("a")>"a" );
	END

	TRY // operator>=(cString)			uses compare(cString) --> only basic tests here
		TRAP  ( String("a")>="b" );
		assert( String("b")>="a" );
		assert( String("a")>="a" );
	END

	TRY // operator<=(cString)			uses compare(cString) --> only basic tests here
		assert( String("a")<="b" );
		TRAP  ( String("b")<="a" );
		assert( String("a")<="a" );
	END

	TRY // CString()
		TRAP(strcmp(String("anton").CString(),"anton")!=0);
		TRAP(strcmp(String("").CString(),"")!=0);
		TRAP(strcmp(String("antons €").CString(),"antons €")!=0);
		TRAP(strcmp(String("xyzanton").MidString(3).CString(),"anton")!=0);
	END

	TRY // NumVal()
		double f;
		TRAP(NumVal("123")!=123);
		TRAP(NumVal("34567890")!=34567890);
		TRAP(NumVal("-1000000")!=-1000000);
		TRAP(NumVal("+123.00e-00")!=123);
		TRAP(NumVal("-123.00e+00")!=-123);
		TRAP(NumVal("47.5")!=47.5);
		TRAP(NumVal("47.12")!=47.12);
		TRAP(NumVal("47.12Effe")!=47.12);
		TRAP(NumVal("-432e+123")!=-432e123);
		TRAP(NumVal("-56789056.7890")!=-56789056.7890);
		TRAP(NumVal("+56789056.7890")!= 56789056.7890);
		TRAP(NumVal("+432.432e-123")!=432.432e-123);
		f = NumVal("XXX");	TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);

		TRAP(NumVal("'A'")!='A');
		TRAP(NumVal("'A'xxx")!='A');
		TRAP(NumVal("-'A'")!=-'A');
		TRAP(NumVal("'\\''")!='\'');
		TRAP(NumVal("'\\x'")!='x');
		TRAP(NumVal("'\\017'")!=15);
		TRAP(NumVal("'\\017\\100'")!=15*256+8*8);
		TRAP(NumVal("'ABCD'")!=0x41424344u);
		f = NumVal("''");	TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);
		f = NumVal("'12");	TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);
		f = NumVal("'12\\");TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);

		TRAP(NumVal("$1234")!=0x1234);
		TRAP(NumVal("$1234ggg")!=0x1234);
		TRAP(NumVal("-$12345678")!=-0x12345678);
		TRAP(NumVal("-$1234.5678")!=-(0x1234+1.0*0x5678/0x10000));
		TRAP(NumVal("$1")!=0x1);
		TRAP(NumVal("+$1")!=0x1);
		f = NumVal("+$");	TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);

		TRAP(NumVal("%1010101001011010")!=0xAA5A);
		TRAP(NumVal("%10101010222")!=0xAA);
		TRAP(NumVal("-%00010010001101000101011001111000")!=-0x12345678);
		TRAP(NumVal("-%0001001000110100.0101011001111000")!=-(0x1234+1.0*0x5678/0x10000));
		TRAP(NumVal("%1") != 1.0);
		TRAP(NumVal("+%1") != 1.0);
		f = NumVal("+%");	TRAP(errno!=notanumber);
		errno=0;			TRAP(f==f);

		String s="xyz123";
		TRAP(NumVal(s.MidString(3)) != 123.0);
		s="x€z123";
		TRAP(NumVal(s.MidString(3)) != 123.0);
		TRAP(NumVal("  +123") != 123.0);
		TRAP(NumVal("  +123€") != 123.0);
	END

	TRY // Find(UCS4Char,long)
		TRAP(String("12345").Find('6')!=-1);
		TRAP(String("").Find('6')!=-1);
		TRAP(String("12345").Find('1')!=0);
		TRAP(String("12345").Find('5')!=4);
		TRAP(String("12325").Find('2')!=1);
		TRAP(String("12345").Find('6',3)!=-1);
		TRAP(String("").Find('6',3)!=-1);
		TRAP(String("12345").Find('1',0)!=0);
		TRAP(String("12345").Find('1',1)!=-1);
		TRAP(String("12345").Find('1',-1)!=0);
		TRAP(String("12345").Find('5',3)!=4);
		TRAP(String("12345").Find('5',4)!=4);
		TRAP(String("12345").Find('5',5)!=-1);
		TRAP(String("12325").Find('2',1)!=1);
		TRAP(String("12325").Find('2',2)!=3);
		TRAP(String("12325").Find('2',3)!=3);
		TRAP(String("12325").Find('2',4)!=-1);
		String s="00012325";
		TRAP(s.MidString(3).Find('2',3)!=3);
	END

	TRY // RFind(UCS4Char,long)
		TRAP(String("12345").RFind('6')!=-1);
		TRAP(String("").RFind('6')!=-1);
		TRAP(String("12345").RFind('1')!=0);
		TRAP(String("12345").RFind('5')!=4);
		TRAP(String("12325").RFind('2')!=3);
		TRAP(String("12345").RFind('6',3)!=-1);
		TRAP(String("").RFind('6',3)!=-1);
		TRAP(String("12345").RFind('1',0)!=0);
		TRAP(String("12345").RFind('1',1)!=0);
		TRAP(String("12345").RFind('1',-1)!=-1);
		TRAP(String("12345").RFind('5',3)!=-1);
		TRAP(String("12345").RFind('5',4)!=4);
		TRAP(String("12345").RFind('5',5)!=4);
		TRAP(String("12325").RFind('2',1)!=1);
		TRAP(String("12325").RFind('2',2)!=1);
		TRAP(String("12325").RFind('2',3)!=3);
		TRAP(String("12325").RFind('2',4)!=3);
		String s="00012325";
		TRAP(s.MidString(3).RFind('2',4)!=3);
	END

	TRY // Find(cString,long)
		TRAP(String("ABCBCD34CDE").Find(String(""))!=0);
		TRAP(String("").Find(String(""))!=0);
		TRAP(String("").Find(String(""),0)!=0);
		TRAP(String("").Find(String(""),1)!=-1);
		TRAP(String("123").Find(String(""),-1)!=0);
		TRAP(String("123").Find(String(""),2)!=2);
		TRAP(String("ABCBCD34CDE").Find(String("XYZ"))!=-1);
		TRAP(String("").Find(String("6sdf"))!=-1);
		TRAP(String("ABCBCD34CDE").Find(String("ABC"))!=0);
		TRAP(String("ABCBCD34CDE").Find(String("CDE"))!=8);
		TRAP(String("ABCBCD3BCDCDE").Find(String("BCD"))!=3);
		TRAP(String("ABCBCD34CDE").Find(String("678"),3)!=-1);
		TRAP(String("").Find(String("678"),3)!=-1);
		TRAP(String("ABCBCD34CDE").Find(String("ABC"),0)!=0);
		TRAP(String("ABCBCD34CDE").Find(String("ABC"),1)!=-1);
		TRAP(String("ABCBCD34CDE").Find(String("ABC"),-1)!=0);
		TRAP(String("ABCBCD34CDE").Find(String("CDE"),7)!=8);
		TRAP(String("ABCBCD34CDE").Find(String("CDE"),8)!=8);
		TRAP(String("ABCBCD34CDE").Find(String("CDE"),9)!=-1);
		TRAP(String("ABCBCD3BCDCDE").Find(String("BCD"),3)!=3);
		TRAP(String("ABCBCD3BCDCDE").Find(String("BCD"),6)!=7);
		TRAP(String("ABCBCD3BCDCDE").Find(String("BCD"),7)!=7);
		TRAP(String("ABCBCD3BCDCDE").Find(String("BCD"),8)!=-1);
		String s="xxxABCBCD3BCDCDE";
		TRAP(s.MidString(3).Find(String("BCD"),6)!=7);
	END

	TRY // RFind(cString,long)
		TRAP(String("ABCBCD34CDE").RFind(String(""))!=11);
		TRAP(String("").RFind(String(""))!=0);
		TRAP(String("").RFind(String(""),0)!=0);
		TRAP(String("").RFind(String(""),1)!=0);
		TRAP(String("123").RFind(String(""),-2)!=-1);
		TRAP(String("123").RFind(String(""),2)!=2);
		TRAP(String("ABCBCD34CDE").RFind(String("XYZ"))!=-1);
		TRAP(String("").RFind(String("6sdf"))!=-1);
		TRAP(String("ABCBCD34CDE").RFind(String("ABC"))!=0);
		TRAP(String("ABCBCD34CDE").RFind(String("CDE"))!=8);
		TRAP(String("ABCBCD3BCDCDE").RFind(String("BCD"))!=7);
		TRAP(String("ABCBCD34CDE").RFind(String("678"),3)!=-1);
		TRAP(String("").RFind(String("678"),3)!=-1);
		TRAP(String("ABCBCD34CDE").RFind(String("ABC"),0)!=0);
		TRAP(String("ABCBCD34CDE").RFind(String("ABC"),1)!=0);
		TRAP(String("ABCBCD34CDE").RFind(String("ABC"),-1)!=-1);
		TRAP(String("ABCBCD34CDE").RFind(String("CDE"),7)!=-1);
		TRAP(String("ABCBCD34CDE").RFind(String("CDE"),8)!=8);
		TRAP(String("ABCBCD34CDE").RFind(String("CDE"),9)!=8);
		TRAP(String("ABCBCD3BCDCDE").RFind(String("BCD"),3)!=3);
		TRAP(String("ABCBCD3BCDCDE").RFind(String("BCD"),6)!=3);
		TRAP(String("ABCBCD3BCDCDE").RFind(String("BCD"),7)!=7);
		TRAP(String("ABCBCD3BCDCDE").RFind(String("BCD"),8)!=7);
		String s = "xxxABCBCD3BCDCDE";
		TRAP(s.MidString(3).RFind(String("BCD"),8)!=7);
	END

	TRY // Find(cUTF8Str,long)
		TRAP(String("ABCBCD34CDE").Find("")!=0);
		TRAP(String("").Find("")!=0);
		TRAP(String("").Find("",0)!=0);
		TRAP(String("").Find("",1)!=-1);
		TRAP(String("123").Find("",-1)!=0);
		TRAP(String("123").Find("",2)!=2);
		TRAP(String("ABCBCD34CDE").Find("XYZ")!=-1);
		TRAP(String("").Find("6sdf")!=-1);
		TRAP(String("ABCBCD34CDE").Find("ABC")!=0);
		TRAP(String("ABCBCD34CDE").Find("CDE")!=8);
		TRAP(String("ABCBCD3BCDCDE").Find("BCD")!=3);
		TRAP(String("ABCBCD34CDE").Find("678",3)!=-1);
		TRAP(String("").Find("678",3)!=-1);
		TRAP(String("ABCBCD34CDE").Find("ABC",0)!=0);
		TRAP(String("ABCBCD34CDE").Find("ABC",1)!=-1);
		TRAP(String("ABCBCD34CDE").Find("ABC",-1)!=0);
		TRAP(String("ABCBCD34CDE").Find("CDE",7)!=8);
		TRAP(String("ABCBCD34CDE").Find("CDE",8)!=8);
		TRAP(String("ABCBCD34CDE").Find("CDE",9)!=-1);
		TRAP(String("ABCBCD3BCDCDE").Find("BCD",3)!=3);
		TRAP(String("ABCBCD3BCDCDE").Find("BCD",6)!=7);
		TRAP(String("ABCBCD3BCDCDE").Find("BCD",7)!=7);
		TRAP(String("ABCBCD3BCDCDE").Find("BCD",8)!=-1);
		String s="xxxABCBCD3BCDCDE";
		TRAP(s.MidString(3).Find("BCD",6)!=7);
	END

	TRY // RFind(cUTF8Str,long)
		TRAP(String("ABCBCD34CDE").RFind("")!=11);
		TRAP(String("").RFind("")!=0);
		TRAP(String("").RFind("",0)!=0);
		TRAP(String("").RFind("",1)!=0);
		TRAP(String("123").RFind("",-2)!=-1);
		TRAP(String("123").RFind("",2)!=2);
		TRAP(String("ABCBCD34CDE").RFind("XYZ")!=-1);
		TRAP(String("").RFind("6sdf")!=-1);
		TRAP(String("ABCBCD34CDE").RFind("ABC")!=0);
		TRAP(String("ABCBCD34CDE").RFind("CDE")!=8);
		TRAP(String("ABCBCD3BCDCDE").RFind("BCD")!=7);
		TRAP(String("ABCBCD34CDE").RFind("678",3)!=-1);
		TRAP(String("").RFind("678",3)!=-1);
		TRAP(String("ABCBCD34CDE").RFind("ABC",0)!=0);
		TRAP(String("ABCBCD34CDE").RFind("ABC",1)!=0);
		TRAP(String("ABCBCD34CDE").RFind("ABC",-1)!=-1);
		TRAP(String("ABCBCD34CDE").RFind("CDE",7)!=-1);
		TRAP(String("ABCBCD34CDE").RFind("CDE",8)!=8);
		TRAP(String("ABCBCD34CDE").RFind("CDE",9)!=8);
		TRAP(String("ABCBCD3BCDCDE").RFind("BCD",3)!=3);
		TRAP(String("ABCBCD3BCDCDE").RFind("BCD",6)!=3);
		TRAP(String("ABCBCD3BCDCDE").RFind("BCD",7)!=7);
		TRAP(String("ABCBCD3BCDCDE").RFind("BCD",8)!=7);
		String s = "xxxABCBCD3BCDCDE";
		TRAP(s.MidString(3).RFind("BCD",8)!=7);
	END

	TRY // Replace(UCS4Char,UCS4Char)
		String s;
		s = " Stetes Klopfen hört das Schwein. ";
		s.Replace(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdasXSchwein.X");

		s = " Stetes Klopfen hört das Schwein. ";
		s.Replace(246,'X');
		TRAP(s!=" Stetes Klopfen hXrt das Schwein. ");

		s = "•Stetes•Klopfen•hört•das•Schwein.•";
		s.Replace(8226,' ');
		TRAP(s!=" Stetes Klopfen hört das Schwein. ");

		s = "xxx xxx Stetes Klopfen hört das Schwein. ";
		s = s.MidString(7);
		s.Replace(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdasXSchwein.X");

		s = "xxx€xxx Stetes Klopfen hört das Schwein. ";
		s = s.MidString(7);
		s.Replace(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdasXSchwein.X");
	END

	TRY // Swap(UCS4Char,UCS4Char)
		String s;
		s = " Stetes Klopfen hört das Schwein. ";

		s.Swap(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdasXSchwein.X");
		s.Swap(' ','X');
		TRAP(s!=" Stetes Klopfen hört das Schwein. ");

		s.Swap(246,'X');
		TRAP(s!=" Stetes Klopfen hXrt das Schwein. ");
		s.Swap(246,'X');
		TRAP(s!=" Stetes Klopfen hört das Schwein. ");

		s.Swap(8226,' ');
		TRAP(s!="•Stetes•Klopfen•hört•das•Schwein.•");
		s.Swap(8226,' ');
		TRAP(s!=" Stetes Klopfen hört das Schwein. ");

		s = "xxxXxxx Stetes Klopfen hört dasXSchwein.X";
		s = s.MidString(7);
		s.Swap(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdas Schwein. ");

		s = "xxx€xxx Stetes Klopfen hört dasXSchwein.X";
		s = s.MidString(7);
		s.Swap(' ','X');
		TRAP(s!="XStetesXKlopfenXhörtXdas Schwein. ");
	END

	TRY // ToUpper()
		String s;
		s = "AnTönchenöäüßÖÄÜß";
		TRAP(s.ToUpper()!="ANTÖNCHENÖÄÜßÖÄÜß");
		s = "AnTönchenöäüßÖÄÜßÿ";
		TRAP(s.ToUpper()!="ANTÖNCHENÖÄÜßÖÄÜßŸ");
		s = "AnTönchenöäüßÖÄÜß€";
		TRAP(s.ToUpper()!="ANTÖNCHENÖÄÜßÖÄÜß€");
		s = "xxxxxxAnTönchenöäüßÖÄÜß";
		TRAP(s.MidString(6).ToUpper()!="ANTÖNCHENÖÄÜßÖÄÜß");
		s = "xxx€xxAnTönchenöäüßÖÄÜß";
		TRAP(s.MidString(6).ToUpper()!="ANTÖNCHENÖÄÜßÖÄÜß");
	END

	TRY // ToLower()
		String s;
		s = "AnTönchenöäüßÖÄÜß";
		TRAP(s.ToLower()!="antönchenöäüßöäüß");
		s = "AnTönchenöäüßÖÄÜßŸ";
		TRAP(s.ToLower()!="antönchenöäüßöäüßÿ");
		s = "AnTönchenöäüßÖÄÜß€";
		TRAP(s.ToLower()!="antönchenöäüßöäüß€");
		s = "xxxxxxAnTönchenöäüßÖÄÜß";
		TRAP(s.MidString(6).ToLower()!="antönchenöäüßöäüß");
		s = "xxx€xxAnTönchenöäüßÖÄÜß";
		TRAP(s.MidString(6).ToLower()!="antönchenöäüßöäüß");
	END

	TRY // ToHtml()
		TRAP(String("").ToHtml()!="");
		TRAP(String("Antons Äffchen").ToHtml()!="Antons Äffchen");
		TRAP(String("< > & \"").ToHtml()!="&lt; &gt; &amp; &quot;");
		TRAP(String("<€>").ToHtml()!="&lt;€&gt;");
		String s = "xxx<€>";
		TRAP(s.MidString(3).ToHtml()!="&lt;€&gt;");
	END

	TRY // FromHtml()
		TRAP(String("").FromHtml()!="");
		TRAP(String("Antons Äffchen").FromHtml()!="Antons Äffchen");
		TRAP(String("&lt; &gt; &amp; &quot;").FromHtml()!="< > & \"");
		TRAP(String("Antons &Auml;ffchen").FromHtml()!="Antons Äffchen");
		TRAP(String("Pat & Paterson").FromHtml()!="Pat & Paterson");
		TRAP(String("&lt;€&gt;").FromHtml()!="<€>");
		TRAP(String("&lt;&euro;&gt;").FromHtml()!="<€>");
		TRAP(String("&lt;&#8364;&gt;").FromHtml()!="<€>");
		TRAP(String("&lt;&#x20AC;&gt;").FromHtml()!="<€>");
		TRAP(String("&lt;&Auml;&gt;").FromHtml()!="<Ä>");
		TRAP(String("&lt;&#214;&gt;").FromHtml()!="<Ö>");
		TRAP(String("&lt;&#xDC;&gt;").FromHtml()!="<Ü>");
		TRAP(String("€&lt;&Auml;&gt;").FromHtml()!="€<Ä>");
		TRAP(String("€&lt;&#214;&gt;").FromHtml()!="€<Ö>");
		TRAP(String("€&lt;&#xDC;&gt;").FromHtml()!="€<Ü>");
		String s = "xxx&lt; &gt; &amp; &quot;";
		TRAP(s.MidString(3).FromHtml()!="< > & \"");
		s = "x€x&lt; &gt; &amp; &quot;";
		TRAP(s.MidString(3).FromHtml()!="< > & \"");
	END

	TRY // ToEscaped(bool)					***TODO*** ToEscaped(UCS4Char)
		TRAP(ToEscaped("anton")!="anton");
		TRAP(ToEscaped("anton€")!="anton€");
		TRAP(ToEscaped("anton\n",1)!="\"anton\\n\"");
		TRAP(ToEscaped("anton€\n",1)!="\"anton€\\n\"");
		TRAP(ToEscaped("\tanton\n")!="\\tanton\\n");
		TRAP(ToEscaped("\tanton€\n")!="\\tanton€\\n");
		TRAP(ToEscaped("\016anton\n")!="\\016anton\\n");
		TRAP(ToEscaped("\016€anton\n")!="\\016€anton\\n");
		TRAP(ToEscaped("\177anton\n",1)!="\"\\177anton\\n\"");
		TRAP(ToEscaped("\177€anton\n",1)!="\"\\177€anton\\n\"");
		String s = "xxx\177Eanton\n";
		TRAP(s.MidString(3).ToEscaped(1)!="\"\\177Eanton\\n\"");
		s = "xxx\177€anton\n";
		TRAP(s.MidString(3).ToEscaped(1)!="\"\\177€anton\\n\"");
	END

	TRY // FromEscaped(bool)				***TODO*** FromEscaped(UCS4Char)
		TRAP(FromEscaped("anton")!="anton");
		TRAP(FromEscaped("anton€")!="anton€");
		TRAP(FromEscaped("\"anton\\n\"",1)!="anton\n");
		TRAP(FromEscaped("\"anton€\\n\"",1)!="anton€\n");
		TRAP(FromEscaped("\\tanton\\n")!="\tanton\n");
		TRAP(FromEscaped("\\tanton€\\n")!="\tanton€\n");
		TRAP(FromEscaped("\\016anton\\n")!="\016anton\n");
		TRAP(FromEscaped("\\016€anton\\n")!="\016€anton\n");
		TRAP(FromEscaped("\"\\177anton\\n\"",1)!="\177anton\n");
		TRAP(FromEscaped("\"\\177€anton\\n\"",1)!="\177€anton\n");
		TRAP(FromEscaped("\\x0eanton\\n")!="\016anton\n");
		TRAP(FromEscaped("\\xe€anton\\n")!="\016€anton\n");
		TRAP(FromEscaped("\"\\x7fanton\\n\"",1)!="\177anton\n");
		TRAP(FromEscaped("\"\\x7f€anton\\n\"",1)!="\177€anton\n");
		String s = "xxx\"\\x7fEanton\\n\"";
		TRAP(s.MidString(3).FromEscaped(1)!="\177Eanton\n");
		s = "xxx\"\\x7f€anton\\n\"";
		TRAP(s.MidString(3).FromEscaped(1)!="\177€anton\n");
	END

	TRY // String::ToUTF8() / FromUTF8()
		for( int bu=0; bu<=32; bu++ )				// max num of bits
		{
			for( int nu=0; nu<300; nu+=1+nu/2 )		// str.len
			{
				ucs4char u4[300];
				for( int j=0; j<nu; j++ )
				{
					u4[j] = (uint32(random())+uint32(random())*0x10000u);
					if (bu<32) u4[j] &= RMASK(bu);
				}
				String su = String(ptr(u4),nu,csz4);
				su = su.ToUTF8();
				assert(su.Csz() == csz1);
				su = su.FromUTF8();
				assert(su.Len() == nu);
				su.ResizeCsz(csz4);
				assert(memcmp(u4,su.Text(),uint(nu)*4) == 0);
			}
		}
	END

	//	TODO		ToUCS1			( ) const;
	//	TODO		FromUCS1		( ) const;
	//	TODO		ToUCS2			( ) const;
	//	TODO		FromUCS2		( ) const;
	//	TODO		ToUCS4			( ) const;
	//	TODO		FromUCS4		( ) const;

	TRY // CharString(UCS4Char)
		TRAP(CharString('a')!="a");
		TRAP(CharString(8364)!="€");
	END

	TRY // SpaceString(long,UCS4Char)
		TRAP(SpaceString(10     )!="          ");
		TRAP(SpaceString(10,'X' )!="XXXXXXXXXX");
		TRAP(SpaceString(10,8364)!="€€€€€€€€€€");
		TRAP(SpaceString(0      )!="");
		TRAP(SpaceString(0,'X'  )!="");
		TRAP(SpaceString(-5     )!="");
		TRAP(SpaceString(-5,'X' )!="");
		TRAP(SpaceString(1      )!=" ");
		TRAP(SpaceString(1,'X'  )!="X");
	END

	TRY // NumString(double)
		TRAP(NumString(0.0)!="0");
		TRAP(NumString(26473246.0)!="26473246");
		TRAP(NumString(2147483647.0)!="2147483647");
		TRAP(NumString(-26473246.0)!="-26473246");
		TRAP(NumString(-2147483648.0)!="-2147483648");
		TRAP(NumString(1.2)!="1.2");
		TRAP(NumString(1.22e-4)!="0.000122");
		TRAP(NumString(-5.5)!="-5.5");
		TRAP(NumString(0.1/3)!="0.033333333333333");
		TRAP(NumString(0.00001/-3)!="-3.3333333333333e-06");
		TRAP(NumString(1e200)!="1e+200");
		TRAP(NumString(1e200/3)!="3.3333333333333e+199");
		TRAP(NumString(1e200/-3)!="-3.3333333333333e+199");
		TRAP(NumString(1e-200/-3)!="-3.3333333333333e-201");
	END

	TRY // NumString(ulong)
		TRAP(NumString(0u)!="0");
		TRAP(NumString(88u)!="88");
		TRAP(NumString(26473246u)!="26473246");
		TRAP(NumString(2147483647u)!="2147483647");
		TRAP(NumString(0x80000000u)!="2147483648");
		TRAP(NumString(0xFFFFFFFFu)!="4294967295");
	END

	TRY // NumString(long)
		TRAP(NumString(0)!="0");
		TRAP(NumString(-0)!="0");
		TRAP(NumString(88)!="88");
		TRAP(NumString(-88)!="-88");
		TRAP(NumString(-1)!="-1");
		TRAP(NumString(-26473246)!="-26473246");
		TRAP(NumString(26473246)!="26473246");
		TRAP(NumString(0x7FFFFFFF)!= "2147483647");
		TRAP(NumString(int32(0x80000000))!="-2147483648");
	END

	TRY // HexString(ulong,int)
		TRAP(HexString(0x444333u,0)!="00444333");
		TRAP(HexString(0x444333u,-3)!="");
		TRAP(HexString(0x2342d2u,2)!="D2");
		TRAP(HexString(0x2342d2f1u,8)!="2342D2F1");
		TRAP(HexString(0xE342d2f1u,8)!="E342D2F1");
		TRAP(HexString(0x2342d2f1u,9)!="02342D2F1");
	END

	TRY // allocation test
		const uint SZ=5000;
		String* v = new String[SZ];
		for (uint i=0;i<SZ;i++) { v[i]=v[0]; }
		for (uint i=0;i<SZ;i++) { v[i]=SpaceString(33,33); }
		for (uint i=0;i<SZ;i++) { v[i]=v[0]; }
		delete[] v;
	END

	TRY // allocation test
		srandom(uint(time(nullptr)));
		const uint SZ=5000;
		String* v[SZ];
		for (uint i=0;i<SZ;i++) v[i]=nullptr;

		int i,j;

		for( uint ii=0; ii<50000; ii++ )
		{
			i = int(random(NELEM(v)));
			j = int(random(NELEM(v)));

			if(v[i])
			{
				if(v[j])
				{
					*v[i] += *v[j];
					*v[i] = v[i]->MidString(int(random(uint(v[i]->Len()))),int(random(100000)));
					delete v[j]; v[j]=nullptr;
				}
				else
				{
					*v[i] += *v[i];
					*v[i] = v[i]->MidString(int(random(uint(v[i]->Len()))),int(random(100000)));
				}
			}
			else
			{
				if(v[j])
				{
					v[i] = new String(*v[j]);
				}
				else
				{
					CharSize csz = random(5) ? csz1 : random(5) ? csz2 : csz4;
					int32 count = int32(random(10000));
					v[i] = new String(count,csz);
				}
			}
		}

		for (uint ii=0;ii<NELEM(v);ii++) delete v[ii];
	END
}







