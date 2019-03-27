/*	Copyright  (c)	Günter Woigk 2018 - 2019
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


#undef NDEBUG
#define SAFETY 2
#define LOGLEVEL 1
#include "Libraries/kio/kio.h"
#undef  assert
#define assert(X) do{ if(X){}else{throw internal_error(__FILE__, __LINE__, "FAILED: " #X);} }while(0)
#include "Libraries/unix/FD.h"
#include "Templates/StrArray.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}


static bool foo_gt(str a, str b) { return a&&b&&*a&&*b ? (*a^3)>(*b^3) : a&&*a; }

static void test1(uint& num_tests, uint& num_errors)
{
	TRY
		StrArray array;
		assert(array.count()==0);
	END

	TRY
		cstr s[] = {"abc","def","gh","ijklmn"};
		StrArray a(s,4);
		assert(eq(a[0],"abc") && eq(a[3],"ijklmn"));
		assert(s[0] != a[0]);
	END

	TRY
		StrArray a;
		assert(a.getData()==nullptr);
		a<<1<<2;
		assert(a.count()==2);
		assert(a.getData()==&a[0]);
		const StrArray b(a);
		assert(b.count()==2);
		assert(b.getData()==&b[0]);
		assert(a==b);
		assert(a.getData()!=b.getData());
	END

	TRY
		StrArray a; a<<42<<7<<99;
		assert(eq(a.first(),"42"));
		assert(eq(a.last(),"99"));
		const StrArray b(a);
		assert(eq(b.first(),"42"));
		assert(eq(b.last(),"99"));
		assert(&b.first()!=&a.first());
		StrArray c; c<<tostr(1.01f);
		assert(eq(c.first(),tostr(1.01f)));
		assert(&c.first()==&c.last());
	END

	/*TRY
		StrArray a;
		a.first() = nullptr;
	EXPECT(internal_error)*/

	TRY
		StrArray a;
		a.resize(2);
		assert(a.count()==2);
		assert(a[0]==nullptr); a[0]=newcopy("44");
		assert(a[1]==nullptr); a[1]=newcopy("1");
		a.resize(10);
		assert(a.count()==10);
		assert(eq(a[0],"44"));
		assert(eq(a[1],"1"));
		assert(a[2]==nullptr); a[2]=newcopy("99");
		assert(a[9]==nullptr);
		a.resize(3);
		assert(a.count()==3);
		assert(eq(a[0],"44"));
		assert(eq(a[1],"1"));
		assert(eq(a[2],"99"));
	END

	TRY
		StrArray a; a<<1<<2<<3<<4<<5<<6<<7<<8;
		assert(a.copyofrange(2,99) == StrArray()<<3<<4<<5<<6<<7<<8);
		assert(a.copyofrange(0,3) == StrArray()<<1<<2<<3);
	END

	TRY
		StrArray a; a<<1<<2<<3;
		assert(StrArray() != a);
		assert(a != StrArray()<<1<<2<<4);
		assert(a != StrArray()<<1<<2);
	END

	TRY
		StrArray a; a<<1<<2<<3<<4;
		static const cstr s[] = {"5","6","7","8","9"};
		a.insertat(2,s,5);
		assert(a.count()==9);
		assert(a == StrArray()<<1<<2<<5<<6<<7<<8<<9<<3<<4);
	END

	TRY
		StrArray a; a<<1<<2<<3<<4;
		StrArray s; s<<5<<6<<7<<8<<9;
		a.insertat(2,s);
		assert(a.count()==9);
		assert(a == StrArray()<<1<<2<<5<<6<<7<<8<<9<<3<<4);
	END

	TRY
		StrArray a; a<<1<<3<<5<<7;
		a.insertsorted("4");
		assert(a == StrArray()<<1<<3<<4<<5<<7);
		a.insertsorted("4");
		assert(a == StrArray()<<1<<3<<4<<4<<5<<7);
	END

	TRY
		cstr a[] = {"a","bb","Ccc","Dddd","EeE",nullptr};
		Array<cstr> b(a,6);
		StrArray c(b);
		assert(c == b);
		assert(b[0]==a[0]);
		assert(c[0]!=a[0]);
	END

	TRY
		cstr a[] = {"a","bb","Ccc","Dddd","EeE",nullptr};
		Array<cstr> b(a,6);
		StrArray c;
		c<<"123";
		c = b;
		assert(c == b);
		assert(b[0]==a[0]);
		assert(c[0]!=a[0]);
	END

	TRY
		str a[] = {dupstr("a"),dupstr("bb"),dupstr("Ccc"),dupstr(nullptr)};
		Array<str> b(a,4);
		StrArray c;
		c<<"123";
		c = b;
		assert(c == b);
		assert(b[0]==a[0]);
		assert(c[0]!=a[0]);
	END

	TRY
		cstr a[] = {"Aa","BoB","Ccc"};
		StrArray b; b<<1<<2<<3;
		b.append(a,3);
		assert(b==StrArray()<<1<<2<<3<<"Aa"<<"BoB"<<"Ccc");
	END

	TRY
		str a[] = {dupstr("Aa"),dupstr("BoB"),dupstr("Ccc")};
		StrArray b; b<<1<<2<<3;
		b.append(a,3);
		assert(b==StrArray()<<1<<2<<3<<"Aa"<<"BoB"<<"Ccc");
	END

	TRY
		StrArray a; a<<1<<2<<3<<4<<5;
		StrArray b; b<<6<<7<<8<<9;
		a.insertat(2,std::move(b));
		assert(b.count()==0);
		assert(a==StrArray()<<1<<2<<6<<7<<8<<9<<3<<4<<5);
	END

	StrArray array;

	TRY
	assert(array.count()==0);
	END

	TRY
	array.append("3");
	array.append("6");
	assert(array.count()==2 && eq(array.first(),"3") && eq(array.last(),"6"));
	END

	TRY
	array << "9" << "12";
	assert(array.count()==4 && eq(array[2],"9") && eq(array[3],"12"));
	END

	TRY
	array.insertat(2, "5");
	assert(array == StrArray() << "3"<<"6"<<"5"<<"9"<<"12");
	END

	TRY
	array.remove(1);
	assert(array == StrArray() << "3" << "5" << "9" << "12");
	END

	TRY
	array.remove(0);
	assert(array == StrArray() << "5" << "9" << "12");
	END

	TRY
	array.remove(2);
	assert(array == StrArray() << "5" << "9");
	END

	TRY
	array.insertat(0,"6");
	assert(array == StrArray() << "6" << "5" << "9");
	END

	TRY
	array.insertat(3,"7");
	assert(array == StrArray() << "6" << "5" << "9" << "7");
	END

	/*TRY array[4]; EXPECT(internal_error)*/

	/*TRY array[uint(-1)]; EXPECT(internal_error)*/

	TRY
	str s=newcopy("8");
	std::swap(s,array[3]);
	delete[] s;
	assert(array == StrArray() << "6" << "5" << "9" << "8");
	END

	TRY
	StrArray array2(array);
	assert(array == array2 && array2 == StrArray() << "6" << "5" << "9" << "8");
	END

	StrArray array2(std::move(array));
	TRY
	assert(array.count()==0 && array2 == StrArray() << "6" << "5" << "9" << "8");
	END

	TRY
	array = array2;
	assert(array2 == array && array == StrArray() << "6" << "5" << "9" << "8");
	END

	TRY
	array.purge();
	array = std::move(array2);
	assert(array2.count()==0 && array == StrArray() << "6" << "5" << "9" << "8");
	END

	TRY
	array.purge();
	assert(array.count()==0);
	END

	TRY
	array.revert();
	assert(array.count()==0);
	END

	TRY
	array.append("0");
	array.revert();
	assert(array.count()==1 && eq(array[0],"0"));
	END

	TRY
	array << "1"<<"2"<<"3"<<"4"<<"5"<<"6"<<"7";
	array.revert();
	assert(array == StrArray() <<"7"<<"6"<<"5"<<"4"<<"3"<<"2"<<"1"<<"0");
	END

	TRY
	array.rol();
	assert(array == StrArray() <<"6"<<"5"<<"4"<<"3"<<"2"<<"1"<<"0"<<"7");
	END

	TRY
	array.ror();
	assert(array == StrArray() <<"7"<<"6"<<"5"<<"4"<<"3"<<"2"<<"1"<<"0");
	END

	TRY
	array.rol(1,1);
	assert(array == StrArray() <<"7"<<"6"<<"5"<<"4"<<"3"<<"2"<<"1"<<"0");
	END

	TRY
	array.rol(2,0);
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(1,1);
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(2,0);
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(2,8);
	assert(array == StrArray() <<7<<6<<0<<5<<4<<3<<2<<1);
	END

	TRY
	array.rol(2,8);
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.rol(2,9);
	assert(array == StrArray() <<7<<6<<4<<3<<2<<1<<0<<5);
	END

	TRY
	array.ror(2,9);
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	str s = array.pop();
	assert(eq(s,"0") && array == StrArray() <<7<<6<<5<<4<<3<<2<<1);
	delete[] s;
	END

	TRY
	array.drop();
	assert(array == StrArray() <<7<<6<<5<<4<<3<<2);
	END

	TRY
	array.removerange(2, 4);
	assert(array == StrArray() <<7<<6<<3<<2);
	END

	TRY
	array.insertrange(2, 4);
	assert(array == StrArray() <<7<<6<<nullptr<<nullptr<<3<<2);
	END

	TRY
	array.removerange(4,99);
	assert(array == StrArray() <<7<<6<<nullptr<<nullptr);
	END

	TRY
	delete[] array[2]; array[2] = newcopy("5");
	delete[] array.last(); array.last() = newcopy("4");
	assert(array == StrArray() <<7<<6<<5<<4);
	END

	TRY
	array2 = StrArray() << 1<<2<<3;
	array.append(array2);
	assert(array2.count()==3 && array == StrArray() <<7<<6<<5<<4<<1<<2<<3);
	END

	TRY
	array.shrink(2);
	assert(array == StrArray() <<7<<6);
	END

	TRY
	array.append(StrArray(array));
	assert(array == StrArray() <<7<<6<<7<<6);
	END

	TRY
	array.purge();
	array.sort();
	assert(array.count()==0);
	END

	TRY
	array << 1;
	array.sort();
	assert(array.count()==1 && eq(array[0],"1"));
	END

	TRY
	array << 7<<5<<9<<3<<8<<5<<0;
	array.sort();
	assert(array == StrArray() << 0<<1<<3<<5<<5<<7<<8<<9);
	END

	TRY
	array.rsort();
	assert(array == StrArray() << 9<<8<<7<<5<<5<<3<<1<<0);
	END

	TRY
	array.sort(7,99);
	assert(array == StrArray() << 9<<8<<7<<5<<5<<3<<1<<0);
	END

	TRY
	array.sort(4,8);
	assert(array == StrArray() << 9<<8<<7<<5<<0<<1<<3<<5);
	END

	TRY
	array.sort(foo_gt);
	assert(array == StrArray() << 3<<1<<0<<7<<5<<5<<9<<8);
	END
}

static void test2(uint& num_tests, uint& num_errors)
{
	TRY
		StrArray a(uint(0),5); a << "1" << "2" << "3";
		StrArray b(uint(0),6); b << "4" << "5" << "6" << "8";
		StrArray a2(a);
		StrArray b2(b);
		std::swap(a,b);
		assert(a.count()==4);
		assert(a==b2);
		assert(b==a2);
	END

	StrArray array = std::move(StrArray() << "11"<<"22"<<"33");
	FD fd;

	TRY
	fd.open_tempfile();
	array.serialize(fd);
	END

	TRY
	array.shrink(1);
	fd.rewind_file();
	array.deserialize(fd);
	assert(array == StrArray() << "11"<<"22"<<"33");
	END

	TRY
	array.appendifnew("44");
	assert(array.count()==4);
	END

	TRY
	array.appendifnew("22");
	assert(array.count()==4);
	END

	TRY
	array.removeitem("33");
	assert(array.count()==3);
	END

	TRY
	array.remove(1);
	assert(array == StrArray() << "11"<<"44");
	END
}

/*static void test3(uint& num_tests, uint& num_errors)
{
	StrArray array;
}*/

/*static void test4(uint& num_tests, uint& num_errors)
{
	StrArray array;
}*/




void test_StrArray(uint& num_tests, uint& num_errors)
{
	logIn("test StrArray");
	test1(num_tests,num_errors);
	test2(num_tests,num_errors);
	//test3(num_tests,num_errors);
	//test4(num_tests,num_errors);
}















