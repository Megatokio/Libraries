/*	Copyright  (c)	Günter Woigk 2014 - 2019
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
#include "Templates/Array.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}


static bool foo_gt(int a, int b) { return (a^3)>(b^3); }


static void test_prerequisites(uint& num_tests, uint& num_errors)
{
	// eq() ne() gt() lt()

	TRY
	assert(eq(22,22));
	assert(ne(21,22));
	assert(gt(23,22));
	assert(lt(21,22));
	END

	TRY
	char a=-33,b=22,c=22;
	assert(eq(b,c));
	assert(ne(a,b));
	assert(gt(b,a));
	assert(lt(a,b));
	END

	TRY
	uint64 a=11l<<55,b=22l<<55,c=22l<<55;
	assert(eq(b,c));
	assert(ne(a,b));
	assert(gt(b,a));
	assert(lt(a,b));
	END

	TRY
	float32 a=-3.3e-3f,b=2.2e-3f,c=2.2e-3f;
	assert(eq(b,c));
	assert(ne(a,b));
	assert(gt(b,a));
	assert(lt(a,b));
	END

	TRY
	float64 a=-3.3e33,b=2.2e44,c=2.2e44;
	assert(eq(b,c));
	assert(ne(a,b));
	assert(gt(b,a));
	assert(lt(a,b));
	END

	TRY
	float128 a=-3.3e33l,b=2.2e44l,c=2.2e44l;
	assert(eq(b,c));
	assert(ne(a,b));
	assert(gt(b,a));
	assert(lt(a,b));
	END

	// eq() ne() gt() lt() for str and cstr

	TRY
	cstr a="1.1e33l",b="Anton",c="anton",e=nullptr;
	assert(eq(b,"Anton"));
	assert(ne(b,c));
	assert(gt(b,a));
	assert(lt(a,b));
	assert(gt(b,""));
	assert(lt("",b));
	assert(eq("",e));
	assert(ne(e,b));
	assert(gt(b,e));
	assert(lt(e,b));
	assert(!gt("",e));
	assert(!lt("",e));
	assert(!gt(e,""));
	assert(!lt(e,""));
	assert(!gt(e,e));
	assert(!lt(e,e));
	END

	TRY
	cstr a="Anton", b="anton";
	str  c=dupstr(b);
	assert(eq(b,c));
	assert(eq(c,b));
	assert(ne(a,c));
	assert(ne(c,a));
	assert(gt(c,a));
	assert(!gt(a,c));
	assert(lt(a,c));
	assert(!lt(c,a));
	assert(gt(c,""));
	assert(lt("",c));
	END

	// sort

	TRY
	int array1[]  = {6,4,9,2,1,0,4,8,7,6,2,1,9,9,1}; uint size=sizeof(array1), count = NELEM(array1);
	int array2[]  = {6,4,9,2,1,0,4,8,7,6,2,1,9,9,1};
	int array3[]  = {6,4,9,2,1,0,4,8,7,6,2,1,9,9,1};
	int array4[]  = {6,4,9,2,1,0,4,8,7,6,2,1,9,9,1};
	int sorted[]  = { 0, 1, 1, 1, 2, 2, 4, 4, 6, 6, 7, 8, 9, 9, 9 };
	int rsorted[] = { 9, 9, 9, 8, 7, 6, 6, 4, 4, 2, 2, 1, 1, 1, 0 };
	sort(&array1[0], &array1[count]);		assert(memcmp(array1, sorted, size)==0);
	rsort(&array2[0],&array2[count]);		assert(memcmp(array2,rsorted, size)==0);
	sort(&array3[0], &array3[count], gt);	assert(memcmp(array3, sorted, size)==0);
	sort(&array4[0], &array4[count], lt);	assert(memcmp(array4,rsorted, size)==0);
	END

	TRY
	const char q[] = {'H','e','l','l','o',' ','W','o','r','l','d','!',10,'D','i','e',' ','K','u','h','.',10,0};
	const char sorted[] = {10,10,32,32,33,46,68,72,75,87,100,101,101,104,105,108,108,108,111,111,114,117,0};
	const char rsorted[]= {117,114,111,111,108,108,108,105,104,101,101,100,87,75,72,68,46,33,32,32,10,10,0};
	str s;
	s = dupstr(q); sort(s,strchr(s,0));    assert(eq(s, sorted));
	s = dupstr(q);rsort(s,strchr(s,0));    assert(eq(s,rsorted));
	s = dupstr(q); sort(s,strchr(s,0),gt); assert(eq(s, sorted));
	s = dupstr(q); sort(s,strchr(s,0),lt); assert(eq(s,rsorted));
	END

	// sort str and cstr

	TRY
	cstr q[] = {"Anton","\tfoo",nullptr,"\t","anton","Antonov"}; uint size=sizeof(q), count=NELEM(q);
	cstr sorted[] = { nullptr, "\t", "\tfoo", "Anton", "Antonov", "anton" };
	cstr rsorted[] = { "anton", "Antonov", "Anton", "\tfoo", "\t", nullptr };
	cstr* a;
	a = new cstr[count]; memcpy(a,q,size);  sort(a,a+count);    assert(memcmp(a, sorted,size)==0);
	a = new cstr[count]; memcpy(a,q,size); rsort(a,a+count);    assert(memcmp(a,rsorted,size)==0);
	a = new cstr[count]; memcpy(a,q,size);  sort(a,a+count,gt); assert(memcmp(a, sorted,size)==0);
	a = new cstr[count]; memcpy(a,q,size);  sort(a,a+count,lt); assert(memcmp(a,rsorted,size)==0);
	END

	TRY
	cstr q[] = {"Anton","\tfoo",nullptr,"\t","anton","Antonov"}; uint size=sizeof(q), count=NELEM(q);
	cstr sorted[] = { nullptr, "\t", "\tfoo", "Anton", "Antonov", "anton" };
	cstr rsorted[] = { "anton", "Antonov", "Anton", "\tfoo", "\t", nullptr };
	str* a;
	a = new str[count]; memcpy(a,q,size);  sort(a,a+count);    assert(memcmp(a, sorted,size)==0);
	a = new str[count]; memcpy(a,q,size); rsort(a,a+count);    assert(memcmp(a,rsorted,size)==0);
	a = new str[count]; memcpy(a,q,size);  sort(a,a+count,gt); assert(memcmp(a, sorted,size)==0);
	a = new str[count]; memcpy(a,q,size);  sort(a,a+count,lt); assert(memcmp(a,rsorted,size)==0);
	END
}

static void test1(uint& num_tests, uint& num_errors)
{
	TRY
	Array<int> array;
	assert(array.count()==0);
	END

	TRY
	char const* s = "abc";
	Array<char> a(s,4);
	assert(a[0]=='a' && a[3]==0);
	END

	TRY
	Array<int> a;
	assert(a.getData()==nullptr);
	a<<1<<2;
	assert(a.count()==2);
	assert(a.getData()==&a[0]);
	const Array<int> b(a);
	assert(b.count()==2);
	assert(b.getData()==&b[0]);
	assert(a==b);
	assert(a.getData()!=b.getData());
	END

	TRY
	Array<uint16> a; a<<42<<7<<99;
	assert(a.first()==42);
	assert(a.last()==99);
	const Array<uint16> b(a);
	assert(b.first()==42);
	assert(b.last()==99);
	assert(&b.first()!=&a.first());
	Array<float> c; c<<100.25f;
	assert(c.first()==100.25f);
	assert(&c.first()==&c.last());
	END

	/*TRY
	Array<int> a;
	a.first()++;
	EXPECT(internal_error)*/

	TRY
	Array<int> a;
	a.resize(2);
	assert(a.count()==2);
	assert(a[0]==0); a[0]=44;
	assert(a[1]==0); a[1]++;
	a.resize(10);
	assert(a.count()==10);
	assert(a[0]==44);
	assert(a[1]==1);
	assert(a[2]==0); a[2]=99;
	assert(a[9]==0);
	a.resize(3);
	assert(a.count()==3);
	assert(a[0]==44);
	assert(a[1]==1);
	assert(a[2]==99);
	END

	TRY
	Array<int> a; a<<1<<2<<3<<4<<5<<6<<7<<8;
	assert(a.copyofrange(2,99) == Array<int>()<<3<<4<<5<<6<<7<<8);
	assert(a.copyofrange(0,3) == Array<int>()<<1<<2<<3);
	END

	TRY
	Array<int> a; a<<1<<2<<3;
	assert(Array<int>() != a);
	assert(a != Array<int>()<<1<<2<<4);
	assert(a != Array<int>()<<1<<2);
	END

	TRY
	Array<int> a; a<<1<<2<<3<<4;
	static const int s[] = {5,6,7,8,9};
	a.insertat(2,s,5);
	assert(a.count()==9);
	assert(a == Array<int>()<<1<<2<<5<<6<<7<<8<<9<<3<<4);
	END

	TRY
	Array<int> a; a<<1<<2<<3<<4;
	Array<int> s; s<<5<<6<<7<<8<<9;
	a.insertat(2,s);
	assert(a.count()==9);
	assert(a == Array<int>()<<1<<2<<5<<6<<7<<8<<9<<3<<4);
	END

	TRY
	Array<int> a; a<<1<<3<<5<<7;
	a.insertsorted(4);
	assert(a == Array<int>()<<1<<3<<4<<5<<7);
	a.insertsorted(4);
	assert(a == Array<int>()<<1<<3<<4<<4<<5<<7);
	END

	TRY
	Array<int> a;
	a.grow() = 22;
	assert(a==Array<int>()<<22);

	a.grow(8);
	assert(a==Array<int>()<<22<<0<<0<<0<<0<<0<<0<<0);

	a[3]=33;
	a.shrink(4);
	assert(a==Array<int>()<<22<<0<<0<<33);

	a.purge();
	assert(a.count()==0);

	a.append(11);
	assert(a.count()==1);
	assert(a[0]==11);

	int* aa = &a[0];
	a.grow(2,44);
	assert(a==Array<int>()<<11<<0);
	int* bb = &a[0];
	assert(aa!=bb);

	a.grow(8);
	assert(a.count()==8);
	assert(a[0]==11);
	assert(a[1]==0);
	assert(&a[0]==bb);

	a.append(77);
	assert(a.count()==9);
	assert(a[8]==77);
	assert(&a[0]==bb);

	a.grow(5,20);
	assert(a.count()==9);
	assert(&a[0]==bb);

	a.grow(20,44);
	assert(a.count()==20);
	assert(&a[0]==bb);

	a.grow(0,55);
	assert(a.count()==20);
	assert(a[0]==11);
	assert(&a[0]!=bb);		// expected not required
	END

	Array<int> array;

	TRY
	array.append(3);
	array.append(6);
	assert(array.count()==2 && array.first()==3 && array.last()==6);
	END

	TRY
	array << 9 << 12;
	assert(array.count()==4 && array[2]==9 && array[3]==12);
	END

	TRY
	array.insertat(2, 5);
	assert(array == Array<int>() << 3<<6<<5<<9<<12);
	END

	TRY
	array.remove(1);
	assert(array == Array<int>() << 3 << 5 << 9 << 12);
	END

	TRY
	array.remove(0);
	assert(array == Array<int>() << 5 << 9 << 12);
	END

	TRY
	array.remove(2);
	assert(array == Array<int>() << 5 << 9);
	END

	TRY
	array.insertat(0,6);
	assert(array == Array<int>() << 6 << 5 << 9);
	END

	TRY
	array.insertat(3,7);
	assert(array == Array<int>() << 6 << 5 << 9 << 7);
	END

	/*TRY array[4]; EXPECT(internal_error)*/

	/*TRY array[uint(-1)]; EXPECT(internal_error)*/

	TRY
	array[3]++;
	assert(array == Array<int>() << 6 << 5 << 9 << 8);
	END

	TRY
	Array<int> array2(array);
	assert(array == array2 && array2 == Array<int>() << 6 << 5 << 9 << 8);
	END

	Array<int> array2(std::move(array));
	TRY
	assert(array.count()==0 && array2 == Array<int>() << 6 << 5 << 9 << 8);
	END

	TRY
	array = array2;
	assert(array2 == array && array == Array<int>() << 6 << 5 << 9 << 8);
	END

	TRY
	array.purge();
	array = std::move(array2);
	assert(array2.count()==0 && array == Array<int>() << 6 << 5 << 9 << 8);
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
	array.append(0);
	array.revert();
	assert(array.count()==1 && array[0]==0);
	END

	TRY
	array << 1<<2<<3<<4<<5<<6<<7;
	array.revert();
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.rol();
	assert(array == Array<int>() <<6<<5<<4<<3<<2<<1<<0<<7);
	END

	TRY
	array.ror();
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.rol(1,1);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.rol(2,0);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(1,1);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(2,0);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.ror(2,8);
	assert(array == Array<int>() <<7<<6<<0<<5<<4<<3<<2<<1);
	END

	TRY
	array.rol(2,8);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	array.rol(2,9);
	assert(array == Array<int>() <<7<<6<<4<<3<<2<<1<<0<<5);
	END

	TRY
	array.ror(2,9);
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2<<1<<0);
	END

	TRY
	int n = array.pop();
	assert(n==0 && array == Array<int>() <<7<<6<<5<<4<<3<<2<<1);
	END

	TRY
	array.drop();
	assert(array == Array<int>() <<7<<6<<5<<4<<3<<2);
	END

	TRY
	array.removerange(2, 4);
	assert(array == Array<int>() <<7<<6<<3<<2);
	END

	TRY
	array.insertrange(2, 4);
	assert(array == Array<int>() <<7<<6<<0<<0<<3<<2);
	END

	TRY
	array.removerange(4,99);
	assert(array == Array<int>() <<7<<6<<0<<0);
	END

	TRY
	array[2] = 5; array.last() = 4;
	assert(array == Array<int>() <<7<<6<<5<<4);
	END

	TRY
	array2 = Array<int>() << 1<<2<<3;
	array.append(array2);
	assert(array2.count()==3 && array == Array<int>() <<7<<6<<5<<4<<1<<2<<3);
	END

	TRY
	array.shrink(2);
	assert(array == Array<int>() <<7<<6);
	END

	TRY
	array.append(Array<int>(array));
	assert(array == Array<int>() <<7<<6<<7<<6);
	END

	TRY
	array.purge();
	array.sort();
	assert(array.count()==0);
	END

	TRY
	array << 1;
	array.sort();
	assert(array.count()==1 && array[0]==1);
	END

	TRY
	array << 7<<5<<9<<3<<8<<5<<0;
	array.sort();
	assert(array == Array<int>() << 0<<1<<3<<5<<5<<7<<8<<9);
	END

	TRY
	array.rsort();
	assert(array == Array<int>() << 9<<8<<7<<5<<5<<3<<1<<0);
	END

	TRY
	array.sort(7,99);
	assert(array == Array<int>() << 9<<8<<7<<5<<5<<3<<1<<0);
	END

	TRY
	array.sort(4,8);
	assert(array == Array<int>() << 9<<8<<7<<5<<0<<1<<3<<5);
	END

	TRY
	array.sort(foo_gt);
	assert(array == Array<int>() << 3<<1<<0<<7<<5<<5<<9<<8);
	END

	TRY
	array.rsort(2,7);
	assert(array == Array<int>() << 3<<1<<9<<7<<5<<5<<0<<8);
	END

	TRY
	array.sort(2,99,gt);
	assert(array == Array<int>() << 3<<1<<0<<5<<5<<7<<8<<9);
	END

	TRY
	Array<uint> a;
	a<<0<<1<<2<<3<<4<<5<<6<<7;
	uint b[8][8]; memset(b,0,sizeof(b));
	uint n,l;

	for (n=0,l=0; l<10000 && n<64; l++)
	{
		a.shuffle();
		for(uint i=0;i<8;i++)
		{
			uint z = a[i];
			if (++b[z][i] == 1) n++;
		}
	}

	if(l>=10000)
	{
		logline("  shuffle: %i permutations not seen after %u runs:",64-n,l);
		for(int z=0;z<8;z++) for(int i=0;i<8;i++) if(b[z][i]==0) logline("    number %i at index %i",z,i);
	}

	assert(l<10000);
	logline("  shuffle: all permutations seen after %u runs",l);
	a.sort();
	assert(a == Array<uint>() << 0<<1<<2<<3<<4<<5<<6<<7);
	END

	TRY
	Array<int> a; a << 123 << -99 << 0 << 2127741132 << -2127745532;

	FD fd; fd.open_tempfile();
	a.print(fd,"•");
	fd.write_char('X');

	fd.rewind_file();
	cstr s = fd.read_str();
	assert(eq("•Array[5]",s));
	for(uint i=0;i<a.count();i++)
	{
		s = fd.read_str();
		assert(eq(s, usingstr("  •[%2u] %s",i,tostr(a[i]))));
	}
	assert(fd.read_char()=='X');
	END

	TRY
	int m = Array<int>::MAGIC * 0x10001;
	Array<int> a; a << m << 123 << -99 << 0 << 2127741132 << -2127745532 << m;
	Array<int> b; b << 1;

	FD fd; fd.open_tempfile();
	a.serialize(fd);
	fd.write_char('X');

	fd.rewind_file();
	b.deserialize(fd);
	assert(a==b);
	assert(fd.read_char()=='X');
	END
}

static void test2(uint& num_tests, uint& num_errors)
{
	TRY
		Array<int> a(uint(0),5); a << 1 << 2 << 3;
		Array<int> b(uint(0),6); b << 4 << 5 << 6 << 8;
		Array<int> a2(a);
		Array<int> b2(b);
		std::swap(a,b);
		assert(a.count()==4);
		assert(a==b2);
		assert(b==a2);
	END

	Array<cstr> array = std::move(Array<cstr>() << "11"<<"22"<<"33");
	FD fd;

	TRY
	fd.open_tempfile();
	array.serialize(fd);
	END

	TRY
	array.shrink(1);
	fd.rewind_file();
	array.deserialize(fd);
	assert(array == Array<cstr>() << "11"<<"22"<<"33");
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
	assert(array == Array<cstr>() << "11"<<"44");
	END
}

/*static void test3(uint& num_tests, uint& num_errors)
{
	Array<int> array;
}*/

/*static void test4(uint& num_tests, uint& num_errors)
{
	Array<int> array;
}*/




void test_Array(uint& num_tests, uint& num_errors)
{
	logIn("test template Array");
	test_prerequisites(num_tests,num_errors);
	test1(num_tests,num_errors);
	test2(num_tests,num_errors);
	//test3(num_tests,num_errors);
	//test4(num_tests,num_errors);
}















