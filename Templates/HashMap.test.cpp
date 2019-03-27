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
#include "Templates/HashMap.h"
#include "Libraries/unix/tempmem.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}


static const int FREE = -1;


static void test1(uint& num_tests, uint& num_errors)
{
	TRY
	HashMap<int,int> map(8);
	assert(map.count()==0);
	assert(map.getMapSize()==16);	// expected, not required
	assert(map.getMap()[0]==FREE);
	assert(map.getKeys().count()==0);
	assert(map.getItems().count()==0);
	END

	TRY
	const HashMap<int,int> map(8);
	assert(map.count()==0);
	assert(map.getMapSize()==16);	// expected, not required
	assert(map.getMap()[0]==FREE);
	assert(map.getKeys().count()==0);
	assert(map.getItems().count()==0);
	END

	TRY
	HashMap<int,int> map1;
	for (int i=0; i<999;   i++) map1.add(i*7,i*77);
	for (int i=0; i<999*7; i++) assert(i%7 ? map1.get(i,-1)==-1 : map1[i]==i*11);

	const HashMap<int,int> map2(map1);
	assert(map1.getItems() == map2.getItems());
	assert(map1.getKeys() == map2.getKeys());
	assert(map1.getMapSize()==map2.getMapSize());
	assert(memcmp(map1.getMap(),map2.getMap(),map1.getMapSize()*sizeof(*map1.getMap()))==0);
	for (int i=0; i<999*7; i++) assert(i%7 ? map2.get(i,-1)==-1 : map2[i]==i*11);
	END

	TRY
	HashMap<int,int> map1(8);
	map1.add(1,10).add(5,19).add(0,27);
	HashMap<int,int> map2(std::move(map1));
	assert(map1.count()==0);
	assert(!map1.contains(1));
	assert(!map1.contains(5));
	assert(!map1.contains(0));
	assert(map2.count()==3);
	map2.add(4,88);
	assert(map2[1]==10);
	assert(map2[5]==19);
	assert(map2[0]==27);
	assert(map2[4]==88);
	END

	TRY
	HashMap<int,int> map1(8);
	HashMap<int,int> map2;
	map1.add(1,10).add(5,19).add(0,27);

	map2 = map1;
	assert(map1 == map2);

	map1.remove(1);
	map2.add(4,88);
	assert(map1 == (HashMap<int,int>().add(5,19).add(0,27)));
	assert(map2 == (HashMap<int,int>().add(5,19).add(0,27).add(4,88).add(1,10)));
	END

	HashMap<int,int> map(8);

	TRY
	map.add(1,10);
	map.add(2,19);
	map.add(0,0);
	assert(map.count()==3 && map[0]==0 && map[1]==10 && map[2]==19);
	for(int i=0;i<int(map.count());i++) assert(map[i]==map.get(i));
	END

	TRY
	map.add(2,20);
	assert(map.count()==3 && map[0]==0 && map[1]==10 && map[2]==20);
	assert(map.contains(1) && map.contains(2) && map.contains(0));
	assert(!(map.contains(3)||map.contains(4)||map.contains(5)));
	END

	TRY
	map.add(4,40);
	map.add(5,50);
	map.add(7,70);
	map.add(3,30);
	map.add(8,80);
	map.add(6,60);
	assert(map.count()==9);
	for(int i=0;i<9;i++) assert(map[i]==i*10);
	END

	TRY
	for(int i=9;i<20;i++) map.add(i,i*10);		// note: this grows the hash map
	assert(map.count()==20);
	for(int i=0;i<20;i++) assert(map[i]==i*10);
	for(int i=0;i<20;i++) assert(map.get(i,-1)==i*10);
	END

	TRY
	map.remove(13);
	assert(map.count()==19);
	for(int i=0;i<20;i++) assert(map.contains(i) == (i!=13));
	END

	TRY
	map.purge();
	assert(map.count()==0 && !map.contains(0));
	END

	TRY
	map.add(0,0);
	map.add(32,320);			// note: => hash collision
	assert(map.count()==2);
	assert(map[0]==0);
	assert(map[32]==320);
	END

	TRY
	map.add(33,330);			// hash collision
	assert(map.count()==3);
	assert(map[0]==0);
	assert(map[32]==320);
	assert(map[33]==330);
	END

	TRY
	for(int i=0;i<=80;i+=16) map.add(i,i*10);
	assert(map.count()==7);
	for(int i=0;i<=80;i+=16) assert(map.get(i,-1) == i*10);
	assert(map[33]==330);
	END

	TRY
	map.remove(33);				// in the middle of a thread: 0-32-33-64
	assert(map.count()==6);
	for(int i=0;i<=80;i+=16) assert(map.get(i,-1) == i*10);
	assert(!map.contains(33));
	END

	TRY
	map.remove(80);				// at end of a thread: 16-48-80
	assert(map.count()==5);
	for(int i=0;i<80;i+=16) assert(map.get(i,-1) == i*10);
	assert(!map.contains(80));
	END

	TRY
	HashMap<cstr,uint> a;
	HashMap<cstr,uint> b;

	a.add("A",2).add("Ccc",22).add("Bb",44);
	FD fd; fd.open_tempfile();
	a.serialize(fd);
	fd.write_char('X');

	fd.rewind_file();
	b.deserialize(fd);
	assert(a==b);
	assert(fd.read_char()=='X');
	END

	TRY
	HashMap<cstr,uint> a;
	a.add("Aaa",33).add("Ccc",22).add("Bbb",44);

	a.print(FD::stdout,"•");

	FD fd; fd.open_tempfile();
	a.print(fd,"•");
	fd.write_char('X');

	//     idx   hash  key  item
	// "%s[%2u] [#%8x] %s = %s"

	fd.rewind_file();
	assert(eq(fd.read_str(),"•HashMap[3]"));
	assert(eq(fd.read_str(), usingstr("  •[ 0] [#%8x] \"Aaa\" = 33",sdbm_hash("Aaa"))));
	assert(eq(fd.read_str(), usingstr("  •[ 1] [#%8x] \"Ccc\" = 22",sdbm_hash("Ccc"))));
	assert(eq(fd.read_str(), usingstr("  •[ 2] [#%8x] \"Bbb\" = 44",sdbm_hash("Bbb"))));
	assert(fd.read_char()=='X');
	END
}

static void test2(uint& num_tests, uint& num_errors)
{
	logline("stress test:");

	TRY
	static const uint N = 10000;

	Array<uint> a(N);
	for(uint i=0;i<N;i++) { a[i]=i; }
	a.shuffle();

	HashMap<uint,uint> map(8);
	for(uint i=0;i<N;i++)  { map.add(a[i],a[i]^1); }
	assert(map.count()==N);
	for(uint i=0;i<N;i+=9) { map.add(a[i],a[i]^1); }
	assert(map.count()==N);
	for(uint i=0;i<N;i++) assert(map[i] == (i^1));

	a.shuffle();
	Array<uint> r(&a[0],N/2);
	a.removerange(0,N/2);
	for(uint i=0;i<r.count();i++) map.remove(r[i]);
	assert(map.count()==a.count());
	for(uint i=0;i<r.count();i++) map.remove(r[i]);
	assert(map.count()==a.count());

	for(uint i=0;i<a.count();i++) assert(map.contains(a[i]));
	for(uint i=0;i<r.count();i++) assert(!map.contains(r[i]));

	for(uint i=0;i<10000;i++)
	{
		if(random()&1)
		{
			if(a.count()==0) continue;
			uint ai = random() % a.count();
			map.remove(a[ai]);
			r.append(a[ai]);
			a.remove(ai);
		}
		else
		{
			if(r.count()==0) continue;
			uint ri = random() % r.count();
			map.add(r[ri],r[ri]^1);
			a.append(r[ri]);
			r.remove(ri);
		}
		assert(map.count()==a.count());

		uint n = uint(random());
		if(map.contains(n)) continue;
		if(r.contains(n)) continue;
		map.add(n,n^1); a.append(n);
	}

	for(uint i=0;i<a.count();i++) assert(map.contains(a[i]));
	for(uint i=0;i<r.count();i++) assert(!map.contains(r[i]));
	for(uint i=0;i<a.count();i++) assert(map[a[i]] == (a[i]^1));
	END
}

static void test3(uint& num_tests, uint& num_errors)
{
	logline("cstr key test:");

	TRY
	TempMemPool zz;
	static char s[] = "Lorem ipsum. The microcode is organized in 2 planes of 16k * 24-bit codes. "
	"There is a 1-instruction pipeline which imposes a 1-instruction latency on code branching: "
	"In every microcode instruction a flag is selected. This flag is tested at the end of the "
	"previous instruction and determines from which code plane the next instruction is read. "
	"The assembler takes care for the most of this brain hazzard.";
	Array<cstr> z;
	split(z,s);

	HashMap<cstr,cstr> map(8);
	Array<cstr> a;
	for(uint i=0;i<z.count();i++)
	for(uint j=0;j<z.count();j++)
	{
		a.appendifnew(catstr(z[i],z[j]));
		map.add(a.last(),tostr(a.count()-1));
	}
	assert(a.count()==map.count());

	map = HashMap<cstr,cstr>(8);
	for(uint i=a.count();i--;)
	{
		map.add(a[i],tostr(i));
		uint n = random() % a.count();
		map.add(a[n],tostr(n));
	}
	assert(a.count()==map.count());

	for(uint i=a.count();i--;)
	{
		assert(eq(map[a[i]],tostr(i)));
	}

	a.shuffle();
	Array<cstr> r;
	while(map.count()>100)
	{
		r.append(a.pop());
		map.remove(r.last());
	}
	assert(a.count()==map.count());
	for(uint i=0;i<a.count();i++) assert(map.contains(a[i]));
	for(uint i=0;i<r.count();i++) assert(!map.contains(r[i]));

	for(uint i=0;i<10000;i++)
	{
		if(random()&1)
		{
			if(a.count()==0) continue;
			uint ai = random() % a.count();
			map.remove(a[ai]);
			r.append(a[ai]);
			a.remove(ai);
		}
		else
		{
			if(r.count()==0) continue;
			uint ri = random() % r.count();
			map.add(r[ri],nullptr);
			a.append(r[ri]);
			r.remove(ri);
		}
		assert(map.count()==a.count());

		cstr n = tostr(random());
		if(map.contains(n)) continue;
		map.add(n,""); a.append(n);
	}
	for(uint i=0;i<a.count();i++) assert(map.contains(a[i]));
	for(uint i=0;i<r.count();i++) assert(!map.contains(r[i]));
	END
}

void test_hashmap(uint& num_tests, uint& num_errors)
{
	logIn("test template HashMap");
	test1(num_tests,num_errors);
	test3(num_tests,num_errors);
	test2(num_tests,num_errors);
}










