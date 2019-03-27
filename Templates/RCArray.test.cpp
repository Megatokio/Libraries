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
#include "Templates/RCPtr.h"
#include "Templates/Array.h"


#define TRY num_tests++; try{
#define END }catch(std::exception& e){num_errors++; logline("%s",e.what());}
#define EXPECT(X) num_errors++; logline("%s line %i: FAILED: did not throw",__FILE__,__LINE__);}catch(X&){}\
  catch(std::exception&){num_errors++;logline("%s line %i: FAILED: wrong type of exception thrown",__FILE__,__LINE__);}



// --------------------------------
// test RCArray

class RCObject
{
public:
	static uint num_objects;
	static uint retained;
	static uint released;
	enum { id1a = 47, id1b, id2a, id2b };

	mutable uint cnt;
	uint value;

	void	retain () const 				{ retained++; cnt++; }
	void	release () const				{ released++; if(--cnt == 0) delete this; }

	RCObject ()								:cnt(0),value(0){num_objects++;}
	RCObject (uint n)						:cnt(0),value(n){num_objects++;}
	explicit RCObject (RCObject const& q)	:cnt(0),value(q.value){num_objects++; }
	RCObject (RCObject&& q)					:cnt(0),value(q.value){num_objects++; }
	virtual ~RCObject ()					{ assert(cnt==0); num_objects--; }

	virtual RCObject& operator= (RCObject const& q)	{ value=q.value; return *this; }
	virtual RCObject& operator= (RCObject&& q)		{ value=q.value; return *this; }

	bool operator== (RCObject const& q) const { return value == q.value; }
	bool operator!= (RCObject const& q) const { return value != q.value; }
	bool operator>  (RCObject const& q) const { return value >  q.value; }
	bool operator<  (RCObject const& q) const { return value <  q.value; }

	virtual void serialize (FD&) const;
	virtual void deserialize (FD&);
	static RCObject* restore (FD&);	// factory method

	static void test1(uint& num_tests, uint& num_errors);
	static void test2(uint& num_tests, uint& num_errors);
	static void clear(){retained=released=0;}
	static void log(){logline("num=%u, ret=%u, rel=%u\n",num_objects,retained,released);}
};
class RCObjectWithPrint : public RCObject
{
public:
	uint v2,dummy;

	// print: slightly different than tostr():
	void print (FD& fd, cstr indent) const { fd.write_fmt("%sRCObject[%u]\n",indent,value); }

	RCObjectWithPrint (uint n=0)					:RCObject(n),v2(n){}
	explicit RCObjectWithPrint (RCObject const& q)	:RCObject(q),v2(value){}
	RCObjectWithPrint (RCObject&& q)				:RCObject(std::move(q)),v2(value){}
	virtual RCObjectWithPrint& operator= (RCObject const& q)	{ value=v2=q.value; return *this; }
	virtual RCObjectWithPrint& operator= (RCObject&& q)			{ value=v2=q.value; return *this; }

	virtual void serialize (FD&) const;
	virtual void deserialize (FD&);
};
uint RCObject::retained=0;
uint RCObject::released=0;
uint RCObject::num_objects=0;
Array<RCPtr<RCObject>> array7() { Array<RCPtr<RCObject>> z; z << new RCObject(7); return z; }
static bool foo_gt(RCPtr<RCObject> const& a, RCPtr<RCObject> const& b) { return (a->value^3)>(b->value^3); }
static cstr tostr(RCObject const& object) { return usingstr("RCObject{%u}",object.value); }

void RCObject::serialize (FD& fd) const
{
	static const char ids[]={id1a,id1b};
	fd.write_bytes(ids,2);
	fd.write_data(&value,1);
}

void RCObject::deserialize (FD& fd)
{
	uint8 id = fd.read_uint8();
	if(id==id1a) id = fd.read_uint8();
	assert(id==id1b);
	fd.read_data(&value,1);
}

void RCObjectWithPrint::serialize (FD& fd) const
{
	static const char ids[]={id2a,id2b};
	fd.write_bytes(ids,2);
	RCObject::serialize(fd);
	fd.write_data(&v2,1);
}

void RCObjectWithPrint::deserialize (FD& fd)
{
	uint8 id = fd.read_uint8();
	if(id==id2a) id = fd.read_uint8();
	assert(id==id2b);
	RCObject::deserialize(fd);
	fd.read_data(&v2,1);
	assert(v2==value);
}

RCObject* RCObject::restore (FD& fd)
{
	uint8 id = fd.read_uint8();
	assert(id==id1a || id==id2a);
	RCObject* object = id==id1a ? new RCObject() : new RCObjectWithPrint();
	object->deserialize(fd);
	return object;
}




void RCObject::test1(uint& num_tests, uint& num_errors)
{
	typedef ::RCPtr<RCObject> RCPtr;
	typedef Array<RCPtr> RCArray;
	(void)num_tests;
	(void)num_errors;

	TRY
	  {
		clear();
		RCPtr a(new RCObject());
		assert(num_objects==1 && retained==1 && released==0 && a->cnt==1);
		RCArray array;
		assert(num_objects==1 && retained==1 && released==0 && a->cnt==1);
		array.append(RCPtr(nullptr));
		assert(num_objects==1 && retained==1 && released==0 && a->cnt==1);
		array.append(a);
		assert(num_objects==1 && retained==2 && released==0 && a->cnt==2);
		a = nullptr;
		assert(num_objects==1 && retained==2 && released==1 && array[1]->cnt==1);
	  }
	  assert(num_objects==0 && retained==2 && released==2);
	END

	TRY
	  {
		clear();
		RCArray a1(0u,2); a1.append(RCPtr(new RCObject(3))); a1.append(new RCObject(4));
		RCArray a2(0u,2); a2 << new RCObject(5) << RCPtr(new RCObject(6));
		assert(a1.count()==2 && a2.count()==2);
		assert(num_objects==4 && retained==4 && released==0);
		a1.append(a2); // grows data[]
		assert(a1.count()==4&&a2.count()==2);
		assert(num_objects==4 && retained==6 && released==0);
		assert(a1 == RCArray() << new RCObject(3) << new RCObject(4) << new RCObject(5) << new RCObject(6));
		assert(num_objects==4 && retained==6+4 && released==4);
	  }
	  assert(num_objects==0 && retained==6+4 && released==4+6);
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(5) << new RCObject(6);
		RCArray a2(a1);
		assert(a1==a2);
		assert(a1[0].is(a2[0]));
		assert(num_objects==2 && retained==4 && released==0);

		{
		  RCArray a3(std::move(a1));
		  assert(a3==a2);
		  assert(a1.count()==0);
		  assert(a3[0].is(a2[0]));
		  assert(num_objects==2 && retained==4 && released==0);
		}
		assert(num_objects==2 && retained==4 && released==2);

		RCArray a4(array7());
		assert(num_objects==3 && retained==5 && released==2);
		assert(*a4[0]==7);
		a4.purge();
		assert(num_objects==2 && retained==5 && released==3);
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(5) << new RCObject(6);
		RCArray a2; a2 = a1;
		assert(a1==a2);
		assert(a1[0].is(a2[0]));
		assert(num_objects==2 && retained==4 && released==0);

		RCArray a3; a3 << new RCObject(7) << new RCObject(8);
		assert(num_objects==4 && retained==6 && released==0);

		a1 = std::move(a3);
		assert(a3==a2);		// expected, not required: move() = swap()
		a3.purge();
		assert(num_objects==4 && retained==6 && released==2);
		assert(a1 == RCArray() << new RCObject(7) << new RCObject(8));
		assert(a2 == RCArray() << new RCObject(5) << new RCObject(6));
		assert(num_objects==4 && retained==6+4 && released==2+4);
	END

	TRY
		clear();
		RCPtr q[] = { new RCObject(7), nullptr, new RCObject(8) };
		RCArray a1(q,NELEM(q));
		assert(num_objects==2 && retained==4 && released==0);
		assert(a1.count()==3);
		assert(a1[0].is(q[0]));
		assert(a1[1].is(nullptr));
	END

	TRY
		clear();
		RCArray a; a << new RCObject(5) << new RCObject(6);
		RCPtr* p = a.getData();
		assert(*p[0] == 5 && *p[1] == 6);
	END

	TRY
		clear();
		RCPtr q[] = { new RCObject(7), new RCObject(8), new RCObject(9) };
		RCArray a(q,NELEM(q));
		assert(num_objects==3 && retained==6 && released==0);
		q[1] = nullptr;
		assert(num_objects==3 && retained==6 && released==1);
		assert(*a[1] == 8);
		assert(a.first().is(q[0]));
		assert(a.last().is(q[2]));
		assert(num_objects==3 && retained==6 && released==1);
	END

	TRY
		clear();
		RCArray a1; a1 = RCArray() << new RCObject(5) << new RCObject(6);
		// can't use operator=(T&&) !!! Bug?
		assert(num_objects==2 && retained==4 && released==2);

		RCArray a2; a2 = RCArray() << new RCObject(5) << new RCObject(6);
		assert(num_objects==4 && retained==8 && released==4);

		assert(a1 == a2);
		assert (!(a1!=a2));
		a1 << new RCObject(7);
		assert(num_objects==5 && retained==9 && released==4);
		assert(a1 != a2);
		assert (!(a1==a2));

		a2 << a1.last();
		assert(num_objects==5 && retained==10 && released==4);
		assert(a1 == a2);
		assert (!(a1!=a2));

		*a1.last() = 99;	// a1.last is a2.last
		assert(num_objects==5 && retained==10 && released==4);
		assert(a1 == a2);

		*a1.first() = 0;
		assert(num_objects==5 && retained==10 && released==4);
		assert(a1 != a2);
		assert (!(a1==a2));

		a1.first() = nullptr;
		assert(num_objects==4 && retained==10 && released==5);
		assert(a1 != a2);
		assert (!(a1==a2));
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(5) << new RCObject(6) << new RCObject(7) << new RCObject(8);
		assert(num_objects==4 && retained==4 && released==0);
		RCArray a2 = a1.copyofrange(1,3);
		assert(num_objects==4 && retained==6 && released==0);
		assert(a2 == RCArray() << new RCObject(6) << new RCObject(7));
	END

	TRY
		clear();
		RCArray a1; a1 << nullptr << new RCObject(6) << new RCObject(7) << nullptr;
		assert(a1.contains(new RCObject(6)));
		assert(a1.contains(nullptr));
		assert(!a1.contains(new RCObject(3)));
		assert(a1.indexof(new RCObject(7))==2);
		assert(a1.indexof(new RCObject(9))==~0u);
		assert(a1.indexof(nullptr)==0);
	END

	TRY
		clear();
		RCArray a1(0u,2); a1 << new RCObject(5) << new RCObject(6);
		assert(num_objects==2 && retained==2 && released==0);
		a1.grow() = new RCObject(7);
		assert(num_objects==3 && retained==3 && released==0);
		assert(a1.count()==3);
		assert(a1[2]->value == 7);
	END

	TRY
		clear();
		RCArray a1;
		assert(a1.count()==0);
		a1.grow(2,2);
		assert(num_objects==0 && retained==0 && released==0);
		assert(a1 == RCArray()<<nullptr<<nullptr);
		a1[1] = new RCObject(4);
		a1.grow(2,3);
		assert(num_objects==1 && retained==1 && released==0);
		a1.grow(3);
		a1[2] = new RCObject(5);
		a1.grow() = new RCObject(6);
		assert(num_objects==3 && retained==3 && released==0);
		assert(a1 == RCArray() << nullptr << new RCObject(4) << new RCObject(5) << new RCObject(6));
		assert(num_objects==3 && retained==6 && released==3);
		a1.shrink(2);
		assert(a1 == RCArray() << nullptr << new RCObject(4));
		assert(num_objects==1 && retained==7 && released==6);

		a1.resize(8);
		a1.resize(2);
		assert(a1 == RCArray() << nullptr << new RCObject(4));
		assert(num_objects==1 && retained==8 && released==7);
		a1.resize(5);
		a1[4] = new RCObject(9);
		assert(num_objects==2 && retained==9 && released==7);
		assert(a1 == RCArray() << nullptr << new RCObject(4) << nullptr << nullptr << new RCObject(9));
		assert(num_objects==2 && retained==11 && released==9);
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(4) << nullptr << new RCObject(6) << nullptr << new RCObject(7);
		assert(num_objects==3 && retained==3 && released==0);
		a1.drop();
		assert(num_objects==2 && retained==3 && released==1);
		a1.drop();
		assert(num_objects==2 && retained==3 && released==1);
		assert(eq(a1.pop(), RCPtr(new RCObject(6))));
		assert(num_objects==1 && retained==4 && released==3);
		assert(a1.pop().is(nullptr));
		assert(num_objects==1 && retained==4 && released==3);
	END

	TRY
		clear();
		RCArray a1; a1.append(new RCObject(4));
		assert(num_objects==1 && retained==1 && released==0);
		assert(a1[0]->value == 4);
		a1.appendifnew(new RCObject(4));
		assert(num_objects==1 && retained==2 && released==1);
		a1.appendifnew(a1[0]);
		assert(num_objects==1 && retained==3 && released==2);
		a1.appendifnew(new RCObject(5));
		assert(num_objects==2 && retained==4 && released==2);
		assert(a1 == RCArray() << new RCObject(4) << new RCObject(5));
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(4);
		RCPtr bu[] = { new RCObject(5), new RCObject(6), nullptr };
		assert(num_objects==3 && retained==3 && released==0);
		a1.append(bu,3);
		assert(num_objects==3 && retained==5 && released==0);
		assert(a1 == RCArray() << new RCObject(4) << new RCObject(5) << new RCObject(6) << nullptr);
	END

	TRY
		clear();
		RCArray a1;
		a1 << new RCObject(4) << nullptr << new RCObject(6) << new RCObject(7) << new RCObject(8) << new RCObject(9);
		assert(num_objects==5 && retained==5 && released==0);

		a1.remove(2);
		assert(num_objects==4 && retained==5 && released==1);
		assert(a1 == RCArray() << new RCObject(4) << nullptr << new RCObject(7) << new RCObject(8) << new RCObject(9));
		assert(num_objects==4 && retained==9 && released==5);

		a1.remove(2,yes);
		assert(num_objects==3 && retained==9 && released==6);
		assert(a1 == RCArray() << new RCObject(4) << nullptr << new RCObject(9) << new RCObject(8));
		assert(num_objects==3 && retained==12 && released==9);

		a1 << new RCObject(1) << nullptr << new RCObject(2) << new RCObject(3);
		assert(num_objects==6 && retained==15 && released==9);

		a1.removerange(3,6);
		assert(num_objects==4 && retained==15 && released==11);
		assert(a1 == RCArray() << new RCObject(4) << nullptr << new RCObject(9) << new RCObject(2) << new RCObject(3));
		assert(num_objects==4 && retained==19 && released==15);

		a1[4]->value = 9;
		a1.removeitem(a1[4]);
		assert(num_objects==3 && retained==19 && released==16);
		assert(a1 == RCArray() << new RCObject(4) << nullptr << new RCObject(2) << new RCObject(9));
		assert(num_objects==3 && retained==22 && released==19);

		a1.removeitem(nullptr,yes);
		assert(num_objects==3 && retained==22 && released==19);
		assert(a1 == RCArray() << new RCObject(4) << new RCObject(9) << new RCObject(2));
		assert(num_objects==3 && retained==25 && released==22);
	END

	TRY
		clear();
		RCArray a1;
		RCPtr rc1 = new RCObject(1);
		RCPtr rc2 = new RCObject(2);
		RCPtr rc3 = new RCObject(3);
		RCPtr rc4 = new RCObject(4);
		assert(num_objects==4 && retained==4 && released==0);

		a1.insertat(0,rc1);
		a1.insertat(1,rc2);
		assert(num_objects==4 && retained==6 && released==0);

		RCPtr p1[] = { rc3,rc4,nullptr };
		assert(num_objects==4 && retained==8 && released==0);

		a1.insertat(1,p1,3);
		assert(num_objects==4 && retained==10 && released==0);
		assert(a1 == RCArray() << rc1 << rc3 << rc4 << nullptr << rc2);
		assert(num_objects==4 && retained==14 && released==4);

		RCArray a2 = a1.copyofrange(1,3);
		assert(num_objects==4 && retained==16 && released==4);
		a1.insertat(3,a2);
		assert(num_objects==4 && retained==18 && released==4);
		assert(a1 == RCArray() << rc1 << rc3 << rc4 << rc3 << rc4 << nullptr << rc2);
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(1) << new RCObject(2);
		a1.insertrange(1,3);
		assert(a1 == RCArray() << new RCObject(1) << nullptr << nullptr << new RCObject(2));
		assert(num_objects==2 && retained==4 && released==2);
	END

	TRY
		clear();
		RCArray a1; a1 << new RCObject(1) << new RCObject(3);
		assert(num_objects==2 && retained==2 && released==0);

		a1.insertsorted(new RCObject(2));
		assert(num_objects==3 && retained==3 && released==0);
		assert(a1 == RCArray() << new RCObject(1) << new RCObject(2) << new RCObject(3));
		assert(num_objects==3 && retained==6 && released==3);

		a1.insertsorted(nullptr);
		assert(num_objects==3 && retained==6 && released==3);
		assert(a1 == RCArray() << nullptr << new RCObject(1) << new RCObject(2) << new RCObject(3));
		assert(num_objects==3 && retained==9 && released==6);

		a1.remove(0);
		a1.insertsorted(new RCObject(4));
		assert(num_objects==4 && retained==10 && released==6);
		assert(a1 == RCArray() << new RCObject(1) << new RCObject(2) << new RCObject(3) << new RCObject(4));
	END

	TRY // revert rol ror
		clear();
		RCArray a1; a1 << new RCObject(1) << new RCObject(3) << new RCObject(2) << nullptr;
		assert(num_objects==3 && retained==3 && released==0);
		a1.revert();
		assert(num_objects==3 && retained==3 && released==0);
		assert(a1 == RCArray() << nullptr << new RCObject(2) << new RCObject(3) << new RCObject(1));
		assert(num_objects==3 && retained==6 && released==3);

		a1 << new RCObject(5);
		assert(num_objects==4 && retained==7 && released==3);
		a1.revert(1,4);
		assert(num_objects==4 && retained==7 && released==3);
		assert(a1 == RCArray() << nullptr << new RCObject(1) << new RCObject(3) << new RCObject(2) << new RCObject(5));
		assert(num_objects==4 && retained==11 && released==7);

		a1.rol();
		assert(num_objects==4 && retained==11 && released==7);
		assert(a1 == RCArray() << new RCObject(1) << new RCObject(3) << new RCObject(2) << new RCObject(5) << nullptr);
		assert(num_objects==4 && retained==15 && released==11);

		a1.rol(1,4);
		assert(num_objects==4 && retained==15 && released==11);
		assert(a1 == RCArray() << new RCObject(1) << new RCObject(2) << new RCObject(5) << new RCObject(3) << nullptr);
		assert(num_objects==4 && retained==19 && released==15);

		a1.ror(3,9);
		assert(num_objects==4 && retained==19 && released==15);
		assert(a1 == RCArray() << new RCObject(1) << new RCObject(2) << new RCObject(5) << nullptr << new RCObject(3));
		assert(num_objects==4 && retained==23 && released==19);

		a1.ror();
		assert(num_objects==4 && retained==23 && released==19);
		assert(a1 == RCArray() << new RCObject(3) << new RCObject(1) << new RCObject(2) << new RCObject(5) << nullptr);
	END

	TRY // shuffle sort
		RCPtr o1=new RCObject(1), o6=new RCObject(6), o5=new RCObject(5), o7=new RCObject(7),
			  o4=new RCObject(4), o0=new RCObject(0), o2=new RCObject(2), o3=new RCObject(3);
		RCArray a; a << o0<<o1<<o2<<o3<<o4<<o5<<o6<<o7;
		uint b[8][8]; memset(b,0,sizeof(b));
		uint n,l;

		for (n=0,l=0; l<10000 && n<64; l++)
		{
			clear();
			a.shuffle();
			assert(retained==0 && released==0);
			for(uint i=0;i<8;i++)
			{
				uint z = a[i]->value;
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
		clear();
		a.sort();
		assert(retained==0 && released==0);
		assert(a == RCArray() << o0<<o1<<o2<<o3<<o4<<o5<<o6<<o7);
	END

	TRY // shuffle, sort, rsort, sort(fu)
		clear();
		RCPtr o1=new RCObject(1), o6=new RCObject(6), o5=new RCObject(5), o7=new RCObject(7), oo=nullptr,
			  o4=new RCObject(4), o0=new RCObject(0), o2=new RCObject(2), o3=new RCObject(3);
		RCArray a; a << oo<<o0<<o1<<o2<<o3<<o4<<o5<<o6<<o7;
		assert(num_objects==8 && retained==16 && released==0);

		a.shuffle();
		a.sort();
		assert(num_objects==8 && retained==16 && released==0);
		assert(a == RCArray() << oo<<o0<<o1<<o2<<o3<<o4<<o5<<o6<<o7);
		assert(num_objects==8 && retained==24 && released==8);

		a.shuffle();
		a.rsort();
		assert(num_objects==8 && retained==24 && released==8);
		assert(a == RCArray() << o7<<o6<<o5<<o4<<o3<<o2<<o1<<o0<<oo);
		assert(num_objects==8 && retained==32 && released==16);

		a.shuffle();
		a.removeitem(nullptr);
		a.sort(foo_gt);
		assert(num_objects==8 && retained==32 && released==16);
		assert(a == RCArray() << o3<<o2<<o1<<o0<<o7<<o6<<o5<<o4);
	END

	TRY // print
		clear();
		RCArray a; a << new RCObject(12) << new RCObjectWithPrint(34) << nullptr;
		a.print(FD::stdout,"•");

		// note: RCPtr<RCObject>::print() uses tostr() for both items because it tests for RCObject::print()
		FD fd; fd.open_tempfile();
		a.print(fd,"•");
		fd.write_char('X');

		fd.rewind_file();
		cstr s = fd.read_str();
		assert(eq("•Array[3]",s));
		for(uint i=0;i<a.count();i++)
		{
			s = fd.read_str();
			if(a[i]) assert(eq(s, usingstr("  •[%2u] RCObject{%u}",i,a[i]->value)));
			else     assert(eq(s, usingstr("  •[%2u] nullptr",i)));
		}
		assert(fd.read_char()=='X');
	END

	TRY // print
		::RCArray<RCObjectWithPrint> a;
		a << new RCObjectWithPrint(12) << new RCObjectWithPrint(34) << nullptr;
		a.print(FD::stdout,"•");

		// note: RCPtr<RCObjectWithPrint>::print() tests for and uses RCObjectWithPrint::print()
		FD fd; fd.open_tempfile();
		a.print(fd,"•");
		fd.write_char('X');

		fd.rewind_file();
		cstr s = fd.read_str();
		assert(eq("•Array[3]",s));
		for(uint i=0;i<a.count();i++)
		{
			s = fd.read_str();
			if(a[i]) assert(eq(s, usingstr("  •[%2u] RCObject[%u]",i,a[i]->value)));
			else     assert(eq(s, usingstr("  •[%2u] nullptr",i)));
		}
		assert(fd.read_char()=='X');
	END

	TRY // serialize/deserialize/restore
		RCArray a; a << new RCObject(12) << new RCObjectWithPrint(34) << nullptr;
		RCObject b(123);
		RCObjectWithPrint c(234);

		FD fd; fd.open_tempfile();
		a.serialize(fd);
		b.serialize(fd);
		c.serialize(fd);
		fd.write_uint8('X');

		fd.rewind_file();
		RCArray d; d << nullptr;
		d.deserialize(fd);
		b.deserialize(fd);
		c.deserialize(fd);
		assert(fd.read_uint8()=='X');

		assert(a==d);
		assert(b.value==123);
		assert(c.value==234);
	END
}

void RCObject::test2(uint& num_tests, uint& num_errors)
{
	(void)num_tests;
	(void)num_errors;
}

void test_RCArray(uint& num_tests, uint& num_errors)
{
	logIn("test template Array");
	RCObject::test1(num_tests,num_errors);
	RCObject::test2(num_tests,num_errors);
}















