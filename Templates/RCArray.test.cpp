// Copyright (c) 2014 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Templates/RCArray.h"
#include "doctest/doctest/doctest.h"
#include "unix/FD.h"
#include <vector>


static uint num_objects;

class MyRCObject
{
public:
	enum { id1a = 47, id1b, id2a, id2b };

	RCDATA

	uint value;
	uint _padding = 0;

	MyRCObject() : value(0) { num_objects++; }
	MyRCObject(uint n) : value(n) { num_objects++; }
	explicit MyRCObject(const MyRCObject& q) : value(q.value) { num_objects++; }
	MyRCObject(MyRCObject&& q) : value(q.value) { num_objects++; }
	virtual ~MyRCObject() throws
	{
		CHECK(refcnt() == 0);
		num_objects--;
	}

	virtual MyRCObject& operator=(const MyRCObject& q)
	{
		value = q.value;
		return *this;
	}
	virtual MyRCObject& operator=(MyRCObject&& q)
	{
		value = q.value;
		return *this;
	}

	bool operator==(const MyRCObject& q) const { return value == q.value; }
	bool operator!=(const MyRCObject& q) const { return value != q.value; }
	bool operator>(const MyRCObject& q) const { return value > q.value; }
	bool operator<(const MyRCObject& q) const { return value < q.value; }

	virtual void	   serialize(FD&) const;
	virtual void	   deserialize(FD&);
	static MyRCObject* newFromFile(FD&); // factory method

	static void test1(uint& num_tests, uint& num_errors);
	static void test2(uint& num_tests, uint& num_errors);
	//static void clear() { retained = released = 0; }
	//static void log() { logline("num=%u, ret=%u, rel=%u\n", num_objects, retained, released); }
	//static void log() { logline("num=%u, ret+rel=%u\n", num_objects, refcnt()); }
};

static bool foo_gt(const RCPtr<MyRCObject>& a, const RCPtr<MyRCObject>& b) { return (a->value ^ 3) > (b->value ^ 3); }
static cstr tostr(const MyRCObject& object) { return usingstr("MyRCObject{%u}", object.value); }

void MyRCObject::serialize(FD& fd) const
{
	static const char ids[] = {id1a, id1b};
	fd.write_bytes(ids, 2);
	fd.write_data(&value, 1);
}

void MyRCObject::deserialize(FD& fd)
{
	uint8 id = fd.read_uint8();
	if (id == id1a) id = fd.read_uint8();
	CHECK(id == id1b);
	fd.read_data(&value, 1);
}

Array<RCPtr<MyRCObject>> array7()
{
	Array<RCPtr<MyRCObject>> z;
	z << new MyRCObject(7);
	return z;
}

TEST_CASE("test1" * doctest::skip(false))
{
	SUBCASE("") { logline("●●● %s:", __FILE__); }

	typedef ::RCPtr<MyRCObject> RCPtr;
	typedef Array<RCPtr>		RCArray;

	SUBCASE("")
	{
		{
			RCPtr a(new MyRCObject());
			CHECK_UNARY(num_objects == 1 && a->refcnt() == 1);
			RCArray array;
			CHECK_UNARY(num_objects == 1 && a->refcnt() == 1);
			array.append(RCPtr(nullptr));
			CHECK_UNARY(num_objects == 1 && a->refcnt() == 1);
			array.append(a);
			CHECK_UNARY(num_objects == 1 && a->refcnt() == 2);
			a = nullptr;
			CHECK_UNARY(num_objects == 1 && array[1]->refcnt() == 1);
		}
		CHECK_UNARY(num_objects == 0);
	}

	SUBCASE("")
	{
		{
			RCArray a1(0u, 2);
			a1.append(RCPtr(new MyRCObject(3)));
			a1.append(new MyRCObject(4));
			RCArray a2(0u, 2);
			a2 << new MyRCObject(5) << RCPtr(new MyRCObject(6));
			CHECK_UNARY(a1.count() == 2 && a2.count() == 2);
			CHECK_UNARY(num_objects == 4);
			a1.append(a2); // grows data[]
			CHECK_UNARY(a1.count() == 4 && a2.count() == 2);
			CHECK_UNARY(num_objects == 4);
			CHECK_UNARY(
				a1 == RCArray() << new MyRCObject(3) << new MyRCObject(4) << new MyRCObject(5) << new MyRCObject(6));
			CHECK_UNARY(num_objects == 4);
		}
		CHECK_UNARY(num_objects == 0);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(5) << new MyRCObject(6);
		RCArray a2(a1);
		CHECK_UNARY(a1 == a2);
		CHECK_UNARY(a1[0] == a2[0]);
		CHECK_UNARY(num_objects == 2);

		{
			RCArray a3(std::move(a1));
			CHECK_UNARY(a3 == a2);
			CHECK_UNARY(a1.count() == 0);
			CHECK_UNARY(a3[0] == a2[0]);
			CHECK_UNARY(num_objects == 2);
		}
		CHECK_UNARY(num_objects == 2);

		RCArray a4(array7());
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(*a4[0] == 7);
		a4.purge();
		CHECK_UNARY(num_objects == 2);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(5) << new MyRCObject(6);
		RCArray a2;
		a2 = a1;
		CHECK_UNARY(a1 == a2);
		CHECK_UNARY(a1[0] == a2[0]);
		CHECK_UNARY(num_objects == 2);

		RCArray a3;
		a3 << new MyRCObject(7) << new MyRCObject(8);
		CHECK_UNARY(num_objects == 4);

		a1 = std::move(a3);
		CHECK_UNARY(a3 == a2); // expected, not required: move() = swap()
		a3.purge();
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(7) << new MyRCObject(8));
		CHECK_UNARY(a2 == RCArray() << new MyRCObject(5) << new MyRCObject(6));
		CHECK_UNARY(num_objects == 4);
	}

	SUBCASE("")
	{
		RCPtr	q[] = {new MyRCObject(7), nullptr, new MyRCObject(8)};
		RCArray a1(q, NELEM(q));
		CHECK_UNARY(num_objects == 2);
		CHECK_UNARY(a1.count() == 3);
		CHECK_UNARY(a1[0] == q[0]);
		CHECK_UNARY(a1[1] == nullptr);
	}

	SUBCASE("")
	{
		RCArray a;
		a << new MyRCObject(5) << new MyRCObject(6);
		RCPtr* p = a.getData();
		CHECK_UNARY(*p[0] == 5 && *p[1] == 6);
	}

	SUBCASE("")
	{
		RCPtr	q[] = {new MyRCObject(7), new MyRCObject(8), new MyRCObject(9)};
		RCArray a(q, NELEM(q));
		CHECK_UNARY(num_objects == 3);
		q[1] = nullptr;
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(*a[1] == 8);
		CHECK_UNARY(a.first() == q[0]);
		CHECK_UNARY(a.last() == q[2]);
		CHECK_UNARY(num_objects == 3);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 = RCArray() << new MyRCObject(5) << new MyRCObject(6);
		CHECK_UNARY(num_objects == 2);

		RCArray a2;
		a2 = RCArray() << new MyRCObject(5) << new MyRCObject(6);
		CHECK_UNARY(num_objects == 4);

		CHECK_UNARY(a1 == a2);
		CHECK_UNARY(!(a1 != a2));
		a1 << new MyRCObject(7);
		CHECK_UNARY(num_objects == 5);
		CHECK_UNARY(a1 != a2);
		CHECK_UNARY(!(a1 == a2));

		a2 << a1.last();
		CHECK_UNARY(num_objects == 5);
		CHECK_UNARY(a1 == a2);
		CHECK_UNARY(!(a1 != a2));

		*a1.last() = 99; // a1.last is a2.last
		CHECK_UNARY(num_objects == 5);
		CHECK_UNARY(a1 == a2);

		*a1.first() = 0;
		CHECK_UNARY(num_objects == 5);
		CHECK_UNARY(a1 != a2);
		CHECK_UNARY(!(a1 == a2));

		a1.first() = nullptr;
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(a1 != a2);
		CHECK_UNARY(!(a1 == a2));
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(5) << new MyRCObject(6) << new MyRCObject(7) << new MyRCObject(8);
		CHECK_UNARY(num_objects == 4);
		RCArray a2 = a1.copyofrange(1, 3);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(a2 == RCArray() << new MyRCObject(6) << new MyRCObject(7));
	}

	SUBCASE("")
	{
		RCPtr	o6 = new MyRCObject(6);
		RCPtr	o7 = new MyRCObject(7);
		RCArray a1;
		a1 << nullptr << o6 << o7 << nullptr;
		CHECK_UNARY(a1.contains(o6));
		CHECK_UNARY(a1.contains(nullptr));
		CHECK_UNARY(!a1.contains(new MyRCObject(3)));
		CHECK_UNARY(!a1.contains(new MyRCObject(7)));
		CHECK_UNARY(a1.indexof(o7) == 2);
		CHECK_UNARY(a1.indexof(new MyRCObject(9)) == ~0u);
		CHECK_UNARY(a1.indexof(new MyRCObject(7)) == ~0u);
		CHECK_UNARY(a1.indexof(nullptr) == 0);
	}

	SUBCASE("")
	{
		RCArray a1(0u, 2);
		a1 << new MyRCObject(5) << new MyRCObject(6);
		CHECK_UNARY(num_objects == 2);
		a1.grow() = new MyRCObject(7);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1.count() == 3);
		CHECK_UNARY(a1[2]->value == 7);
	}

	SUBCASE("")
	{
		RCArray a1;
		CHECK_UNARY(a1.count() == 0);
		a1.grow(2, 2);
		CHECK_UNARY(num_objects == 0);
		CHECK_UNARY(a1 == RCArray() << nullptr << nullptr);
		a1[1] = new MyRCObject(4);
		a1.grow(2, 3);
		CHECK_UNARY(num_objects == 1);
		a1.grow(3);
		a1[2]	  = new MyRCObject(5);
		a1.grow() = new MyRCObject(6);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(4) << new MyRCObject(5) << new MyRCObject(6));
		CHECK_UNARY(num_objects == 3);
		a1.shrink(2);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(4));
		CHECK_UNARY(num_objects == 1);

		a1.resize(8);
		a1.resize(2);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(4));
		CHECK_UNARY(num_objects == 1);
		a1.resize(5);
		a1[4] = new MyRCObject(9);
		CHECK_UNARY(num_objects == 2);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(4) << nullptr << nullptr << new MyRCObject(9));
		CHECK_UNARY(num_objects == 2);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(4) << nullptr << new MyRCObject(6) << nullptr << new MyRCObject(7);
		CHECK_UNARY(num_objects == 3);
		a1.drop();
		CHECK_UNARY(num_objects == 2);
		a1.drop();
		CHECK_UNARY(num_objects == 2);
		CHECK_UNARY(eq(a1.pop(), RCPtr(new MyRCObject(6))));
		CHECK_UNARY(num_objects == 1);
		CHECK_UNARY(a1.pop() == nullptr);
		CHECK_UNARY(num_objects == 1);
	}

	SUBCASE("")
	{
		RCPtr o4 = new MyRCObject(4);
		CHECK_UNARY(num_objects == 1);

		RCArray a1;
		a1.append(o4);
		CHECK_UNARY(num_objects == 1);
		CHECK_UNARY(a1[0]->value == 4);

		a1.appendifnew(o4);
		CHECK_UNARY(num_objects == 1);

		a1.appendifnew(a1[0]);
		CHECK_UNARY(num_objects == 1);

		a1.appendifnew(new MyRCObject(5));
		CHECK_UNARY(num_objects == 2);

		o4 = nullptr;
		CHECK_UNARY(num_objects == 2);

		CHECK_UNARY(a1 == RCArray() << new MyRCObject(4) << new MyRCObject(5));
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(4);
		RCPtr bu[] = {new MyRCObject(5), new MyRCObject(6), nullptr};
		CHECK_UNARY(num_objects == 3);
		a1.append(bu, 3);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(4) << new MyRCObject(5) << new MyRCObject(6) << nullptr);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(4) << nullptr << new MyRCObject(6) << new MyRCObject(7) << new MyRCObject(8)
		   << new MyRCObject(9);
		CHECK_UNARY(num_objects == 5);

		a1.removeat(2);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(4) << nullptr << new MyRCObject(7) << new MyRCObject(8)
							<< new MyRCObject(9));
		CHECK_UNARY(num_objects == 4);

		a1.removeat(2, yes);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(4) << nullptr << new MyRCObject(9) << new MyRCObject(8));
		CHECK_UNARY(num_objects == 3);

		a1 << new MyRCObject(1) << nullptr << new MyRCObject(2) << new MyRCObject(3);
		CHECK_UNARY(num_objects == 6);

		a1.removerange(3, 6);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(4) << nullptr << new MyRCObject(9) << new MyRCObject(2)
							<< new MyRCObject(3));
		CHECK_UNARY(num_objects == 4);

		a1.removeitem(a1[4]);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(4) << nullptr << new MyRCObject(9) << new MyRCObject(2));
		CHECK_UNARY(num_objects == 3);

		a1.removeitem(nullptr, yes);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(4) << new MyRCObject(2) << new MyRCObject(9));
		CHECK_UNARY(num_objects == 3);
	}

	SUBCASE("")
	{
		RCArray a1;
		RCPtr	rc1 = new MyRCObject(1);
		RCPtr	rc2 = new MyRCObject(2);
		RCPtr	rc3 = new MyRCObject(3);
		RCPtr	rc4 = new MyRCObject(4);
		CHECK_UNARY(num_objects == 4);

		a1.insertat(0, rc1);
		a1.insertat(1, rc2);
		CHECK_UNARY(num_objects == 4);

		RCPtr p1[] = {rc3, rc4, nullptr};
		CHECK_UNARY(num_objects == 4);

		a1.insertat(1, p1, 3);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(a1 == RCArray() << rc1 << rc3 << rc4 << nullptr << rc2);
		CHECK_UNARY(num_objects == 4);

		RCArray a2 = a1.copyofrange(1, 3);
		CHECK_UNARY(num_objects == 4);
		a1.insertat(3, a2);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(a1 == RCArray() << rc1 << rc3 << rc4 << rc3 << rc4 << nullptr << rc2);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(1) << new MyRCObject(2);
		a1.insertrange(1, 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(1) << nullptr << nullptr << new MyRCObject(2));
		CHECK_UNARY(num_objects == 2);
	}

	SUBCASE("")
	{
		RCArray a1;
		a1 << new MyRCObject(1) << new MyRCObject(3);
		CHECK_UNARY(num_objects == 2);

		a1.insertsorted(new MyRCObject(2));
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(3));
		CHECK_UNARY(num_objects == 3);

		a1.insertsorted(nullptr);
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(3));
		CHECK_UNARY(num_objects == 3);

		a1.removeat(0);
		a1.insertsorted(new MyRCObject(4));
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(3) << new MyRCObject(4));
	}

	SUBCASE("revert rol ror")
	{
		RCArray a1;
		a1 << new MyRCObject(1) << new MyRCObject(3) << new MyRCObject(2) << nullptr;
		CHECK_UNARY(num_objects == 3);
		a1.revert();
		CHECK_UNARY(num_objects == 3);
		CHECK_UNARY(a1 == RCArray() << nullptr << new MyRCObject(2) << new MyRCObject(3) << new MyRCObject(1));
		CHECK_UNARY(num_objects == 3);

		a1 << new MyRCObject(5);
		CHECK_UNARY(num_objects == 4);
		a1.revert(1, 4);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << nullptr << new MyRCObject(1) << new MyRCObject(3) << new MyRCObject(2)
							<< new MyRCObject(5));
		CHECK_UNARY(num_objects == 4);

		a1.rol();
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(1) << new MyRCObject(3) << new MyRCObject(2) << new MyRCObject(5)
							<< nullptr);
		CHECK_UNARY(num_objects == 4);

		a1.rol(1, 4);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(5) << new MyRCObject(3)
							<< nullptr);
		CHECK_UNARY(num_objects == 4);

		a1.ror(3, 9);
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(5) << nullptr
							<< new MyRCObject(3));
		CHECK_UNARY(num_objects == 4);

		a1.ror();
		CHECK_UNARY(num_objects == 4);
		CHECK_UNARY(
			a1 == RCArray() << new MyRCObject(3) << new MyRCObject(1) << new MyRCObject(2) << new MyRCObject(5)
							<< nullptr);
	}

	SUBCASE("shuffle sort")
	{
		RCPtr o1 = new MyRCObject(1), o6 = new MyRCObject(6), o5 = new MyRCObject(5), o7 = new MyRCObject(7),
			  o4 = new MyRCObject(4), o0 = new MyRCObject(0), o2 = new MyRCObject(2), o3 = new MyRCObject(3);
		RCArray a;
		a << o0 << o1 << o2 << o3 << o4 << o5 << o6 << o7;
		uint b[8][8];
		memset(b, 0, sizeof(b));
		uint n, l;

		for (n = 0, l = 0; l < 10000 && n < 64; l++)
		{
			uint no = num_objects;
			a.shuffle();
			CHECK_EQ(num_objects, no);
			for (uint i = 0; i < 8; i++)
			{
				uint z = a[i]->value;
				if (++b[z][i] == 1) n++;
			}
		}

		CHECK_UNARY(l < 10000);
		logline("shuffle: all assignments seen after %u runs", l);
		uint no = num_objects;
		a.sort();
		CHECK_EQ(num_objects, no);
		CHECK_UNARY(a == RCArray() << o0 << o1 << o2 << o3 << o4 << o5 << o6 << o7);
	}

	SUBCASE("shuffle, sort, rsort, sort(fu)")
	{
		RCPtr o1 = new MyRCObject(1), o6 = new MyRCObject(6), o5 = new MyRCObject(5), o7 = new MyRCObject(7),
			  oo = nullptr, o4 = new MyRCObject(4), o0 = new MyRCObject(0), o2 = new MyRCObject(2),
			  o3 = new MyRCObject(3);
		RCArray a;
		a << oo << o0 << o1 << o2 << o3 << o4 << o5 << o6 << o7;
		CHECK_UNARY(num_objects == 8);

		a.shuffle();
		a.sort();
		CHECK_UNARY(num_objects == 8);
		CHECK_UNARY(a == RCArray() << oo << o0 << o1 << o2 << o3 << o4 << o5 << o6 << o7);
		CHECK_UNARY(num_objects == 8);

		a.shuffle();
		a.rsort();
		CHECK_UNARY(num_objects == 8);
		CHECK_UNARY(a == RCArray() << o7 << o6 << o5 << o4 << o3 << o2 << o1 << o0 << oo);
		CHECK_UNARY(num_objects == 8);

		a.shuffle();
		a.removeitem(nullptr);
		a.sort(foo_gt);
		CHECK_UNARY(num_objects == 8);
		CHECK_UNARY(a == RCArray() << o3 << o2 << o1 << o0 << o7 << o6 << o5 << o4);
	}
}

#if 0
// class RCObjectWithPrint : public RCObject
// {
// public:
// 	uint v2, dummy;

// 	// print: slightly different than tostr():
// 	void print(FD& fd, cstr indent) const { fd.write_fmt("%sRCObject[%u]\n", indent, value); }

// 	RCObjectWithPrint(uint n = 0) : RCObject(n), v2(n) {}
// 	explicit RCObjectWithPrint(const RCObject& q) : RCObject(q), v2(value) {}
// 	RCObjectWithPrint(RCObject&& q) : RCObject(std::move(q)), v2(value) {}
// 	virtual RCObjectWithPrint& operator=(const RCObject& q) override
// 	{
// 		value = v2 = q.value;
// 		return *this;
// 	}
// 	virtual RCObjectWithPrint& operator=(RCObject&& q) override
// 	{
// 		value = v2 = q.value;
// 		return *this;
// 	}

// 	virtual void serialize(FD&) const override;
// 	virtual void deserialize(FD&) override;
// };
// uint				   RCObject::retained	 = 0;
// uint				   RCObject::released	 = 0;
// uint				   RCObject::num_objects = 0;
// Array<RCPtr<RCObject>> array7()
// {
// 	Array<RCPtr<RCObject>> z;
// 	z << new RCObject(7);
// 	return z;
// }

// void RCObjectWithPrint::serialize(FD& fd) const
// {
// 	static const char ids[] = {id2a, id2b};
// 	fd.write_bytes(ids, 2);
// 	RCObject::serialize(fd);
// 	fd.write_data(&v2, 1);
// }

// void RCObjectWithPrint::deserialize(FD& fd)
// {
// 	uint8 id = fd.read_uint8();
// 	if (id == id2a) id = fd.read_uint8();
// 	CHECK(id == id2b);
// 	RCObject::deserialize(fd);
// 	fd.read_data(&v2, 1);
// 	CHECK(v2 == value);
// }

// RCObject* RCObject::newFromFile(FD& fd)
// {
// 	uint8 id = fd.read_uint8();
// 	CHECK(id == id1a || id == id2a);
// 	RCObject* object = id == id1a ? new RCObject() : new RCObjectWithPrint();
// 	object->deserialize(fd);
// 	return object;
// }

TEST_CASE("RCObjectWithPrint")
{
	// SUBCASE(""){ // print
	// clear();
	// RCArray a;
	// a << new RCObject(12) << new RCObjectWithPrint(34) << nullptr;
	// a.print(FD::_stdout, "•");

	// // note: RCPtr<RCObject>::print() uses tostr() for both items because it tests for RCObject::print()
	// FD fd;
	// fd.open_tempfile();
	// a.print(fd, "•");
	// fd.write_char('X');

	// fd.rewind_file();
	// cstr s = fd.read_str();
	// CHECK(eq("•Array[3]", s));
	// for (uint i = 0; i < a.count(); i++)
	// {
	// s = fd.read_str();
	// if (a[i]) CHECK(eq(s, usingstr("  •[%2u] RCObject{%u}", i, a[i]->value)));
	// else CHECK(eq(s, usingstr("  •[%2u] nullptr", i)));
	// }
	// CHECK(fd.read_char() == 'X');
	// }

	// SUBCASE(""){ // print
	// ::RCArray<RCObjectWithPrint>
	// a;
	// a << new RCObjectWithPrint(12) << new RCObjectWithPrint(34) << nullptr;
	// a.print(FD::_stdout, "•");

	// // note: RCPtr<RCObjectWithPrint>::print() tests for and uses RCObjectWithPrint::print()
	// FD fd;
	// fd.open_tempfile();
	// a.print(fd, "•");
	// fd.write_char('X');

	// fd.rewind_file();
	// cstr s = fd.read_str();
	// CHECK(eq("•Array[3]", s));
	// for (uint i = 0; i < a.count(); i++)
	// {
	// s = fd.read_str();
	// if (a[i]) CHECK(eq(s, usingstr("  •[%2u] RCObject[%u]", i, a[i]->value)));
	// else CHECK(eq(s, usingstr("  •[%2u] nullptr", i)));
	// }
	// CHECK(fd.read_char() == 'X');
	// }

	// SUBCASE(""){ // serialize/deserialize/restore
	// RCArray a;
	// a << new RCObject(12) << new RCObjectWithPrint(34) << nullptr;
	// RCObject		  b(123);
	// RCObjectWithPrint c(234);

	// FD fd;
	// fd.open_tempfile();
	// a.serialize(fd);
	// b.serialize(fd);
	// c.serialize(fd);
	// fd.write_uint8('X');

	// fd.rewind_file();
	// RCArray d;
	// d << nullptr;
	// d.deserialize(fd);
	// b.deserialize(fd);
	// c.deserialize(fd);
	// CHECK(fd.read_uint8() == 'X');

	// CHECK(a == d);
	// CHECK(b.value == 123);
	// CHECK(c.value == 234);
	// }
}
#endif


#undef RCObject
#include "Templates/RCObject.h"

static int rc1_objects = 0;
class RCObject1 : public RCBase // non virtual
{
public:
	RCObject1() noexcept { rc1_objects++; }
	~RCObject1() { rc1_objects--; }
};

static int rc2_objects = 0;
class RCObject2 : public RCObject // virtual
{
public:
	RCObject2() noexcept { rc2_objects++; }
	~RCObject2() override { rc2_objects--; }
};

static int rc3_objects = 0;
class RCObject3
{
	RCDATA_NOWEAK
	int _padding = 0;

public:
	RCObject3() noexcept { rc3_objects++; }
	~RCObject3() { rc3_objects--; }
};

static int rc4_objects = 0;
class RCObject4
{
	RCDATA_WITHWEAK
public:
	RCObject4() noexcept { rc4_objects++; }
	virtual ~RCObject4() { rc4_objects--; }
};

static int rc6_objects = 0;
class RCObject6 // std::shared_ptr
{
public:
	RCObject6() noexcept { rc6_objects++; }
	virtual ~RCObject6() { rc6_objects--; }
};


#ifdef _POSIX_THREAD_CPUTIME
  #define CLOCK CLOCK_THREAD_CPUTIME_ID
#else
  #define CLOCK
#endif


TEST_CASE("RCArray performance test" * doctest::skip(false))
{
	SUBCASE("info")
	{
		logline("compiler = %s", _compiler_str);
#ifdef NO_THREADS
		logline("NO_THREADS");
#else
		logline("thread-safe");
#endif
	}

	double t1 = 0, t2 = 0, t3 = 0, t4 = 0, t6 = 0;

	SUBCASE("performance test")
	{
		for (uint i = 0; i < 9; i++)
		{
			if (i) t1 -= now(CLOCK);

			rc1_objects = 0;
			RCArray<RCObject1> array1(0, 1000000);
			for (uint i = 0; i < 1000; i++) { array1 << new RCObject1(); }
			for (uint i = 1000; i < 1000000; i++) { array1 << array1[i - 1000]; }
			while (array1.count()) { array1.pop(); }
			CHECK(rc1_objects == 0);

			if (i) t1 += now(CLOCK);
			if (i) t2 -= now(CLOCK);

			rc2_objects = 0;
			RCArray<RCObject2> array2(0, 1000000);
			for (uint i = 0; i < 1000; i++) { array2 << new RCObject2(); }
			for (uint i = 1000; i < 1000000; i++) { array2 << array2[i - 1000]; }
			while (array2.count()) { array2.pop(); }
			CHECK(rc2_objects == 0);

			if (i) t2 += now(CLOCK);
			if (i) t3 -= now(CLOCK);

			rc3_objects = 0;
			RCArray<RCObject3> array3(0, 1000000);
			for (uint i = 0; i < 1000; i++) { array3 << new RCObject3(); }
			for (uint i = 1000; i < 1000000; i++) { array3 << array3[i - 1000]; }
			while (array3.count()) { array3.pop(); }
			CHECK(rc3_objects == 0);

			if (i) t3 += now(CLOCK);
			if (i) t4 -= now(CLOCK);

			rc4_objects = 0;
			RCArray<RCObject4> array4(0, 1000000);
			for (uint i = 0; i < 1000; i++) { array4 << new RCObject4(); }
			for (uint i = 1000; i < 1000000; i++) { array4 << array4[i - 1000]; }
			while (array4.count()) { array4.pop(); }
			CHECK(rc4_objects == 0);

			if (i) t4 += now(CLOCK);
			if (i) t6 -= now(CLOCK);

			rc6_objects = 0;
			std::vector<std::shared_ptr<RCObject6>> array6;
			array6.reserve(1000000);
			for (uint i = 0; i < 1000; i++) { array6.push_back(std::make_shared<RCObject6>()); }
			for (uint i = 1000; i < 1000000; i++) { array6.push_back(array6[i - 1000]); }
			while (array6.size()) { array6.pop_back(); }
			CHECK(rc6_objects == 0);

			if (i) t6 += now(CLOCK);
		}

		logline("RCPtr test: %f sec. (Object based on RCBase non-virtual)", t1);
		logline("RCPtr test: %f sec. (Object based on RCObject)", t2);
		logline("RCPtr test: %f sec. (Object NO WEAK)", t3);
		logline("RCPtr test: %f sec. (Object WITH WEAK)", t4); // with/without lock depends on NO_THREADS
		logline("RCPtr test: %f sec. (Object with std::shared_ptr)", t6);
	}
}


/*











































*/
