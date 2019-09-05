/*	Copyright  (c)	Günter Woigk 2018 - 2019
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
#include "kio/kio.h"
#undef  assert
#define assert(X) do{ if(X){}else{throw internal_error(__FILE__, __LINE__, "FAILED: " #X);} }while(0)
#include "unix/FD.h"
#include "tempmem.h"
#include "hash/sdbm_hash.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}


static uint random(uint n) { return (uint32(n) * uint16(random())) >> 16; }

static void alloc_some_bytes(uint n=99);
static void alloc_some_bytes(uint n)
{
	for(uint i=0;i<n;i++) (void) tempstr(random(999));
}

void test_tempmem(uint& num_tests, uint& num_errors)
{
	logIn("test TempMemPool");

	TRY
		TempMemPool z;
		(void) tempstr(0);
		(void) tempstr(8);
		assert( size_t(tempstr(79)) % _MAX_ALIGNMENT != 0 ); // not required but expected
		assert( size_t(tempstr(79)) % _MAX_ALIGNMENT != 0 ); // not required but expected
		assert( size_t(tempmem(80)) % _MAX_ALIGNMENT == 0 ); // required
		(void) tempstr(12345);
		z.purge();
		(void) tempstr(8);
	END

	TempMemPool* outerpool = TempMemPool::getPool();
	TempMemPool tempmempool;
	TRY
		assert(TempMemPool::getPool() == &tempmempool);
		assert(TempMemPool::getXPool() == outerpool);
	END

	static const int N = 666;

	TRY
		ptr list1[N];
		ptr list2[N];
		uint size[N];
		for(uint i=0;i<N;i++)
		{
			uint n = min(random(0x1fff),random(0x1fff));
			list1[i] = tempmempool.alloc(n);
			list2[i] = new char[n];
			size[i] = n;
			while(n--) list1[i][n] = char(random(256));
			memcpy(list2[i],list1[i],size[i]);
		}
		for(uint i=0;i<N;i++)
		{
			assert(memcmp(list1[i],list2[i],size[i]) == 0);
			delete[] list2[i];
		}
	END

	TRY
		for(uint i=0;i<N;i++)
		{
			uint n = min(random(0x1fff),random(0x1fff));
			str a = tempmempool.allocStr(n);
			str b = tempmempool.allocMem(n);
			assert(a[n] == 0);
			assert(size_t(b) % _MAX_ALIGNMENT == 0);
		}
	END

	TRY
		tempmempool.purge();
	END

	TRY
		for(uint i=0;i<N;i++)
		{
			uint n = min(random(0x1fff),random(0x1fff));
			str a = tempstr(n);
			str b = tempmem(n);
			str c = xtempstr(n);
			str d = xtempmem(n);
			assert(a[n] == 0);
			assert(size_t(b) % _MAX_ALIGNMENT == 0);
			assert(c[n] == 0);
			assert(size_t(d) % _MAX_ALIGNMENT == 0);
		}
	END

	TRY
		ptr list1[N];
		ptr list2[N];
		uint size[N];
		uint hash[N];
		{
			TempMemPool z;
			for(uint i=0;i<N;i++)
			{
				uint n = min(random(0x1fff),random(0x1fff));
				list1[i] = tempmem(n);
				list2[i] = xtempmem(n);
				size[i] = n;
				while(n--) list1[i][n] = char(random(256));
				hash[i] = sdbm_hash(list1[i],size[i]);
				memcpy(list2[i],list1[i],size[i]);
			}
			alloc_some_bytes();
			for(uint i=0;i<N;i++)
			{
				assert(memcmp(list1[i],list2[i],size[i]) == 0);
			}
			alloc_some_bytes();
			purgeTempMem();
			alloc_some_bytes();
		}
		alloc_some_bytes();
		for(uint i=0;i<N;i++)
		{
			assert(sdbm_hash(list2[i],size[i]) == hash[i]);
		}
	END
}





