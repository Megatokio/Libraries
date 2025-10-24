// Copyright (c) 2018 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Templates/StrArray.h"
#include "doctest/doctest/doctest.h"
#include "unix/FD.h"


static bool foo_gt(str a, str b) { return a && b && *a && *b ? (*a ^ 3) > (*b ^ 3) : a && *a; }

TEST_CASE("StrArray")
{
	SUBCASE("") { logline("●●● %s:", __FILE__); }

	SUBCASE("")
	{
		StrArray array;
		CHECK(array.count() == 0);
	}

	SUBCASE("")
	{
		cstr	 s[] = {"abc", "def", "gh", "ijklmn"};
		StrArray a(s, 4);
		CHECK_UNARY(eq(a[0], "abc") && eq(a[3], "ijklmn"));
		CHECK(s[0] != a[0]);
	}

	SUBCASE("")
	{
		StrArray a;
		CHECK(a.getData() == nullptr);
		a << 1 << 2;
		CHECK(a.count() == 2);
		CHECK(a.getData() == &a[0]);
		const StrArray b(a);
		CHECK(b.count() == 2);
		CHECK(b.getData() == &b[0]);
		CHECK(a == b);
		CHECK(a.getData() != b.getData());
	}

	SUBCASE("")
	{
		StrArray a;
		a << 42 << 7 << 99;
		CHECK(eq(a.first(), "42"));
		CHECK(eq(a.last(), "99"));
		const StrArray b(a);
		CHECK(eq(b.first(), "42"));
		CHECK(eq(b.last(), "99"));
		CHECK(&b.first() != &a.first());
		StrArray c;
		c << tostr(1.01f);
		CHECK(eq(c.first(), tostr(1.01f)));
		CHECK(&c.first() == &c.last());
	}

	/*SUBCASE(""){
		StrArray a;
		a.first() = nullptr;
	EXPECT(InternalError)*/

	SUBCASE("")
	{
		StrArray a;
		a.resize(2);
		CHECK(a.count() == 2);
		CHECK(a[0] == nullptr);
		a[0] = newcopy("44");
		CHECK(a[1] == nullptr);
		a[1] = newcopy("1");
		a.resize(10);
		CHECK(a.count() == 10);
		CHECK(eq(a[0], "44"));
		CHECK(eq(a[1], "1"));
		CHECK(a[2] == nullptr);
		a[2] = newcopy("99");
		CHECK(a[9] == nullptr);
		a.resize(3);
		CHECK(a.count() == 3);
		CHECK(eq(a[0], "44"));
		CHECK(eq(a[1], "1"));
		CHECK(eq(a[2], "99"));
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8;
		CHECK(a.copyofrange(2, 99) == StrArray() << 3 << 4 << 5 << 6 << 7 << 8);
		CHECK(a.copyofrange(0, 3) == StrArray() << 1 << 2 << 3);
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 2 << 3;
		CHECK(StrArray() != a);
		CHECK(a != StrArray() << 1 << 2 << 4);
		CHECK(a != StrArray() << 1 << 2);
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 2 << 3 << 4;
		static const cstr s[] = {"5", "6", "7", "8", "9"};
		a.insertat(2, s, 5);
		CHECK(a.count() == 9);
		CHECK(a == StrArray() << 1 << 2 << 5 << 6 << 7 << 8 << 9 << 3 << 4);
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 2 << 3 << 4;
		StrArray s;
		s << 5 << 6 << 7 << 8 << 9;
		a.insertat(2, s);
		CHECK(a.count() == 9);
		CHECK(a == StrArray() << 1 << 2 << 5 << 6 << 7 << 8 << 9 << 3 << 4);
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 3 << 5 << 7;
		a.insertsorted("4");
		CHECK(a == StrArray() << 1 << 3 << 4 << 5 << 7);
		a.insertsorted("4");
		CHECK(a == StrArray() << 1 << 3 << 4 << 4 << 5 << 7);
	}

	SUBCASE("")
	{
		cstr		a[] = {"a", "bb", "Ccc", "Dddd", "EeE", nullptr};
		Array<cstr> b(a, 6);
		StrArray	c(b);
		CHECK(c == b);
		CHECK(b[0] == a[0]);
		CHECK(c[0] != a[0]);
	}

	SUBCASE("")
	{
		cstr		a[] = {"a", "bb", "Ccc", "Dddd", "EeE", nullptr};
		Array<cstr> b(a, 6);
		StrArray	c;
		c << "123";
		c = b;
		CHECK(c == b);
		CHECK(b[0] == a[0]);
		CHECK(c[0] != a[0]);
	}

	SUBCASE("")
	{
		str		   a[] = {dupstr("a"), dupstr("bb"), dupstr("Ccc"), dupstr(nullptr)};
		Array<str> b(a, 4);
		StrArray   c;
		c << "123";
		c = b;
		CHECK(c == b);
		CHECK(b[0] == a[0]);
		CHECK(c[0] != a[0]);
	}

	SUBCASE("")
	{
		cstr	 a[] = {"Aa", "BoB", "Ccc"};
		StrArray b;
		b << 1 << 2 << 3;
		b.append(a, 3);
		CHECK(b == StrArray() << 1 << 2 << 3 << "Aa" << "BoB" << "Ccc");
	}

	SUBCASE("")
	{
		str		 a[] = {dupstr("Aa"), dupstr("BoB"), dupstr("Ccc")};
		StrArray b;
		b << 1 << 2 << 3;
		b.append(a, 3);
		CHECK(b == StrArray() << 1 << 2 << 3 << "Aa" << "BoB" << "Ccc");
	}

	SUBCASE("")
	{
		StrArray a;
		a << 1 << 2 << 3 << 4 << 5;
		StrArray b;
		b << 6 << 7 << 8 << 9;
		a.insertat(2, std::move(b));
		CHECK(b.count() == 0);
		CHECK(a == StrArray() << 1 << 2 << 6 << 7 << 8 << 9 << 3 << 4 << 5);
	}

	SUBCASE("default ctor")
	{
		StrArray array;
		CHECK(array.count() == 0);
	}

	SUBCASE("append")
	{
		StrArray array;
		array.append("3");
		array.append("6");
		CHECK(array.count() == 2);
		CHECK(eq(array.first(), "3"));
		CHECK(eq(array.last(), "6"));
	}

	SUBCASE("operator<<")
	{
		StrArray array;
		array << "3" << "6";
		array << "9" << "12";
		CHECK_UNARY(array.count() == 4 && eq(array[2], "9") && eq(array[3], "12"));
	}

	SUBCASE("insertat, removeat")
	{
		StrArray array = std::move(StrArray() << "3" << "6" << "9" << "12");

		array.insertat(2, "5");
		CHECK(array == StrArray() << "3" << "6" << "5" << "9" << "12");
		array.removeat(1);
		CHECK(array == StrArray() << "3" << "5" << "9" << "12");
		array.removeat(0);
		CHECK(array == StrArray() << "5" << "9" << "12");
		array.removeat(2);
		CHECK(array == StrArray() << "5" << "9");
		array.insertat(0, "6");
		CHECK(array == StrArray() << "6" << "5" << "9");
		array.insertat(3, "7");
		CHECK(array == StrArray() << "6" << "5" << "9" << "7");
	}

	/*SUBCASE(""){ array[4]; EXPECT(InternalError)*/

	/*SUBCASE(""){ array[uint(-1)]; EXPECT(InternalError)*/

	SUBCASE("str& operator[]")
	{
		auto array = std::move(StrArray() << "6" << "5" << "9" << "7");
		str	 s	   = newcopy("8");
		std::swap(s, array[3]);
		delete[] s;
		CHECK(array == StrArray() << "6" << "5" << "9" << "8");
	}

	SUBCASE("ctor StrArray(StrArray)")
	{
		auto	 array = std::move(StrArray() << "6" << "5" << "9" << "8");
		StrArray array2(array);
		CHECK_UNARY(array == array2 && array2 == StrArray() << "6" << "5" << "9" << "8");
	}

	SUBCASE("ctor Array(Array&&)")
	{
		auto	 array = std::move(StrArray() << "6" << "5" << "9" << "8");
		StrArray array2(std::move(array));
		CHECK_UNARY(array.count() == 0 && array2 == StrArray() << "6" << "5" << "9" << "8");
		array = array2;
		CHECK_UNARY(array2 == array && array == StrArray() << "6" << "5" << "9" << "8");
	}

	SUBCASE("operator=(Array&&)")
	{
		StrArray array2 = std::move(StrArray() << "6" << "5" << "9" << "8");
		StrArray array;

		array = std::move(array2);
		CHECK_UNARY(array2.count() == 0 && array == StrArray() << "6" << "5" << "9" << "8");
	}

	SUBCASE("purge")
	{
		StrArray array = std::move(StrArray() << "6" << "5" << "9" << "8");
		array.purge();
		CHECK(array.count() == 0);
	}

	SUBCASE("revert")
	{
		StrArray array;
		array.revert();
		CHECK(array.count() == 0);

		array.append("0");
		array.revert();
		CHECK_UNARY(array.count() == 1 && eq(array[0], "0"));

		array << "1" << "2" << "3" << "4" << "5" << "6" << "7";
		array.revert();
		CHECK(array == StrArray() << "7" << "6" << "5" << "4" << "3" << "2" << "1" << "0");
	}

	SUBCASE("rol ror")
	{
		auto array = std::move(StrArray() << "7" << "6" << "5" << "4" << "3" << "2" << "1" << "0");
		array.rol();
		CHECK(array == StrArray() << "6" << "5" << "4" << "3" << "2" << "1" << "0" << "7");

		array.ror();
		CHECK(array == StrArray() << "7" << "6" << "5" << "4" << "3" << "2" << "1" << "0");

		array.rol(1, 1);
		CHECK(array == StrArray() << "7" << "6" << "5" << "4" << "3" << "2" << "1" << "0");

		array.rol(2, 0);
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);

		array.ror(1, 1);
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);

		array.ror(2, 0);
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);

		array.ror(2, 8);
		CHECK(array == StrArray() << 7 << 6 << 0 << 5 << 4 << 3 << 2 << 1);

		array.rol(2, 8);
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);

		array.rol(2, 9);
		CHECK(array == StrArray() << 7 << 6 << 4 << 3 << 2 << 1 << 0 << 5);

		array.ror(2, 9);
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);
	}

	SUBCASE("pop drop")
	{
		auto array = std::move(StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1 << 0);

		str s = array.pop();
		CHECK_UNARY(eq(s, "0") && array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2 << 1);
		delete[] s;

		array.drop();
		CHECK(array == StrArray() << 7 << 6 << 5 << 4 << 3 << 2);
	}

	SUBCASE("removerange, insertrange")
	{
		auto array = std::move(StrArray() << 7 << 6 << 5 << 4 << 3 << 2);

		array.removerange(2, 4);
		CHECK(array == StrArray() << 7 << 6 << 3 << 2);

		array.insertrange(2, 4);
		CHECK(array == StrArray() << 7 << 6 << nullptr << nullptr << 3 << 2);

		array.removerange(4, 99);
		CHECK(array == StrArray() << 7 << 6 << nullptr << nullptr);
	}

	SUBCASE("")
	{
		auto array = std::move(StrArray() << 7 << 6 << nullptr << nullptr);

		delete[] array[2];
		array[2] = newcopy("5");
		delete[] array.last();
		array.last() = newcopy("4");
		CHECK(array == StrArray() << 7 << 6 << 5 << 4);
	}

	SUBCASE("appen(StrArray)")
	{
		auto array	= std::move(StrArray() << 7 << 6 << 5 << 4);
		auto array2 = std::move(StrArray() << 1 << 2 << 3);
		array.append(array2);
		CHECK_UNARY(array2.count() == 3 && array == StrArray() << 7 << 6 << 5 << 4 << 1 << 2 << 3);
	}

	SUBCASE("shrink")
	{
		auto array = std::move(StrArray() << 7 << 6 << 5 << 4 << 1 << 2 << 3);
		array.shrink(2);
		CHECK(array == StrArray() << 7 << 6);
	}

	SUBCASE("append(array)")
	{
		auto array = std::move(StrArray() << 7 << 6);
		array.append(StrArray(array));
		CHECK(array == StrArray() << 7 << 6 << 7 << 6);
	}

	SUBCASE("sort rsort")
	{
		StrArray array;
		array.sort();
		CHECK(array.count() == 0);

		array << 1;
		array.sort();
		CHECK_UNARY(array.count() == 1 && eq(array[0], "1"));

		array << 7 << 5 << 9 << 3 << 8 << 5 << 0;
		array.sort();
		CHECK(array == StrArray() << 0 << 1 << 3 << 5 << 5 << 7 << 8 << 9);

		array.rsort();
		CHECK(array == StrArray() << 9 << 8 << 7 << 5 << 5 << 3 << 1 << 0);

		array.sort(7, 99);
		CHECK(array == StrArray() << 9 << 8 << 7 << 5 << 5 << 3 << 1 << 0);

		array.sort(4, 8);
		CHECK(array == StrArray() << 9 << 8 << 7 << 5 << 0 << 1 << 3 << 5);

		array.sort(foo_gt);
		CHECK(array == StrArray() << 3 << 1 << 0 << 7 << 5 << 5 << 9 << 8);
	}

	SUBCASE("swap")
	{
		StrArray a(uint(0), 5);
		a << "1" << "2" << "3";
		StrArray b(uint(0), 6);
		b << "4" << "5" << "6" << "8";
		StrArray a2(a);
		StrArray b2(b);
		std::swap(a, b);
		CHECK(a.count() == 4);
		CHECK(a == b2);
		CHECK(b == a2);
	}

	SUBCASE("serialize")
	{
		StrArray array = std::move(StrArray() << "11" << "22" << "33");

		FD fd;
		fd.open_tempfile();
		array.serialize(fd);

		array.shrink(1);
		fd.rewind_file();
		array.deserialize(fd);
		CHECK(array == StrArray() << "11" << "22" << "33");
	}

	SUBCASE("")
	{
		auto array = std::move(StrArray() << "11" << "22" << "33");

		array.appendifnew("44");
		CHECK(array.count() == 4);

		array.appendifnew("22");
		CHECK(array.count() == 4);
	}

	SUBCASE("remove, removeat")
	{
		auto array = std::move(StrArray() << "11" << "22" << "33" << "44");
		array.remove("33");
		CHECK(array == StrArray() << "11" << "22" << "44");

		array.removeat(1);
		CHECK(array == StrArray() << "11" << "44");
	}
}


/*

































*/
