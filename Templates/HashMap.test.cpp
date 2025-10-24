// Copyright (c) 2014 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


#include "Templates/HashMap.h"
#include "Templates/Array.h"
#include "doctest/doctest/doctest.h"
#include "unix/FD.h"


static const int FREE = -1;


TEST_CASE("HashMap")
{
	SUBCASE("") { logline("●●● %s:", __FILE__); }

	SUBCASE("")
	{
		HashMap<int, int> map(8);
		CHECK(map.count() == 0);
		CHECK(map.getMapSize() == 16); // expected, not required
		CHECK(map.getMap()[0] == FREE);
		CHECK(map.getKeys().count() == 0);
		CHECK(map.getItems().count() == 0);
	}

	SUBCASE("")
	{
		const HashMap<int, int> map(8);
		CHECK(map.count() == 0);
		CHECK(map.getMapSize() == 16); // expected, not required
		CHECK(map.getMap()[0] == FREE);
		CHECK(map.getKeys().count() == 0);
		CHECK(map.getItems().count() == 0);
	}

	SUBCASE("")
	{
		HashMap<int, int> map1;
		for (int i = 0; i < 999; i++) map1.add(i * 7, i * 77);
		for (int i = 0; i < 999 * 7; i++) CHECK_UNARY(i % 7 ? map1.get(i, -1) == -1 : map1[i] == i * 11);

		const HashMap<int, int> map2(map1);
		CHECK(map1.getItems() == map2.getItems());
		CHECK(map1.getKeys() == map2.getKeys());
		CHECK(map1.getMapSize() == map2.getMapSize());
		CHECK(memcmp(map1.getMap(), map2.getMap(), map1.getMapSize() * sizeof(*map1.getMap())) == 0);
		for (int i = 0; i < 999 * 7; i++) CHECK_UNARY(i % 7 ? map2.get(i, -1) == -1 : map2[i] == i * 11);
	}

	SUBCASE("")
	{
		HashMap<int, int> map1(8);
		map1.add(1, 10).add(5, 19).add(0, 27);
		HashMap<int, int> map2(std::move(map1));
		CHECK(map1.count() == 0);
		CHECK(!map1.contains(1));
		CHECK(!map1.contains(5));
		CHECK(!map1.contains(0));
		CHECK(map2.count() == 3);
		map2.add(4, 88);
		CHECK(map2[1] == 10);
		CHECK(map2[5] == 19);
		CHECK(map2[0] == 27);
		CHECK(map2[4] == 88);
	}

	SUBCASE("")
	{
		HashMap<int, int> map1(8);
		HashMap<int, int> map2;
		map1.add(1, 10).add(5, 19).add(0, 27);

		map2 = map1;
		CHECK(map1 == map2);

		map1.remove(1);
		map2.add(4, 88);
		CHECK(map1 == (HashMap<int, int>().add(5, 19).add(0, 27)));
		CHECK(map2 == (HashMap<int, int>().add(5, 19).add(0, 27).add(4, 88).add(1, 10)));
	}

	SUBCASE("")
	{
		HashMap<int, int> map(8);

		map.add(1, 10);
		map.add(2, 19);
		map.add(0, 0);
		CHECK_UNARY(map.count() == 3 && map[0] == 0 && map[1] == 10 && map[2] == 19);
		for (int i = 0; i < int(map.count()); i++) CHECK(map[i] == map.get(i));
		// }
		// SUBCASE("")
		// {
		map.add(2, 20);
		CHECK_UNARY(map.count() == 3 && map[0] == 0 && map[1] == 10 && map[2] == 20);
		CHECK_UNARY(map.contains(1) && map.contains(2) && map.contains(0));
		CHECK_UNARY_FALSE(map.contains(3) || map.contains(4) || map.contains(5));
		// }
		// SUBCASE("")
		// {
		map.add(4, 40);
		map.add(5, 50);
		map.add(7, 70);
		map.add(3, 30);
		map.add(8, 80);
		map.add(6, 60);
		CHECK(map.count() == 9);
		for (int i = 0; i < 9; i++) CHECK(map[i] == i * 10);
		// }
		// SUBCASE("")
		// {
		for (int i = 9; i < 20; i++) map.add(i, i * 10); // note: this grows the hash map
		CHECK(map.count() == 20);
		for (int i = 0; i < 20; i++) CHECK(map[i] == i * 10);
		for (int i = 0; i < 20; i++) CHECK(map.get(i, -1) == i * 10);
		// }
		// SUBCASE("")
		// {
		map.remove(13);
		CHECK(map.count() == 19);
		for (int i = 0; i < 20; i++) CHECK(map.contains(i) == (i != 13));
		// }
		// SUBCASE("")
		// {
		map.purge();
		CHECK_UNARY(map.count() == 0 && !map.contains(0));
		// }
		// SUBCASE("")
		// {
		map.add(0, 0);
		map.add(32, 320); // note: => hash collision
		CHECK(map.count() == 2);
		CHECK(map[0] == 0);
		CHECK(map[32] == 320);
		// }
		// SUBCASE("")
		// {
		map.add(33, 330); // hash collision
		CHECK(map.count() == 3);
		CHECK(map[0] == 0);
		CHECK(map[32] == 320);
		CHECK(map[33] == 330);
		// }
		// SUBCASE("")
		// {
		for (int i = 0; i <= 80; i += 16) map.add(i, i * 10);
		CHECK(map.count() == 7);
		for (int i = 0; i <= 80; i += 16) CHECK(map.get(i, -1) == i * 10);
		CHECK(map[33] == 330);
		// }
		// SUBCASE("")
		// {
		map.remove(33); // in the middle of a thread: 0-32-33-64
		CHECK(map.count() == 6);
		for (int i = 0; i <= 80; i += 16) CHECK(map.get(i, -1) == i * 10);
		CHECK(!map.contains(33));
		// }
		// SUBCASE("")
		// {
		map.remove(80); // at end of a thread: 16-48-80
		CHECK(map.count() == 5);
		for (int i = 0; i < 80; i += 16) CHECK(map.get(i, -1) == i * 10);
		CHECK(!map.contains(80));
	}

	SUBCASE("")
	{
		HashMap<cstr, uint> a;
		HashMap<cstr, uint> b;

		a.add("A", 2).add("Ccc", 22).add("Bb", 44);
		FD fd;
		fd.open_tempfile();
		a.serialize(fd);
		fd.write_char('X');

		fd.rewind_file();
		b.deserialize(fd);
		CHECK(a == b);
		CHECK(fd.read_char() == 'X');
	}

	SUBCASE("")
	{
		HashMap<cstr, uint> a;
		a.add("Aaa", 33).add("Ccc", 22).add("Bbb", 44);

		if (loglevel >= 1) a.print(FD::_stdout, "•");

		FD fd;
		fd.open_tempfile();
		a.print(fd, "•");
		fd.write_char('X');

		//     idx   hash  key  item
		// "%s[%2u] [#%8x] %s = %s"

		fd.rewind_file();
		CHECK(eq(fd.read_str(), "•HashMap[3]"));
		CHECK(eq(fd.read_str(), usingstr("  •[ 0] [#%8x] \"Aaa\" = 33", kio::sdbm_hash("Aaa"))));
		CHECK(eq(fd.read_str(), usingstr("  •[ 1] [#%8x] \"Ccc\" = 22", kio::sdbm_hash("Ccc"))));
		CHECK(eq(fd.read_str(), usingstr("  •[ 2] [#%8x] \"Bbb\" = 44", kio::sdbm_hash("Bbb"))));
		CHECK(fd.read_char() == 'X');
	}
}

TEST_CASE("HashMap stress test")
{
	static const uint N = 10000;

	Array<uint> a(N);
	for (uint i = 0; i < N; i++) { a[i] = i; }
	a.shuffle();

	HashMap<uint, uint> map(8);
	for (uint i = 0; i < N; i++) { map.add(a[i], a[i] ^ 1); }
	CHECK(map.count() == N);
	for (uint i = 0; i < N; i += 9) { map.add(a[i], a[i] ^ 1); }
	CHECK(map.count() == N);
	for (uint i = 0; i < N; i++) CHECK(map[i] == (i ^ 1));

	a.shuffle();
	Array<uint> r(&a[0], N / 2);
	a.removerange(0, N / 2);
	for (uint i = 0; i < r.count(); i++) map.remove(r[i]);
	CHECK(map.count() == a.count());
	for (uint i = 0; i < r.count(); i++) map.remove(r[i]);
	CHECK(map.count() == a.count());

	for (uint i = 0; i < a.count(); i++) CHECK(map.contains(a[i]));
	for (uint i = 0; i < r.count(); i++) CHECK(!map.contains(r[i]));

	for (uint i = 0; i < 10000; i++)
	{
		if (random() & 1)
		{
			if (a.count() == 0) continue;
			uint ai = uint(random() % a.count());
			map.remove(a[ai]);
			r.append(a[ai]);
			a.removeat(ai);
		}
		else
		{
			if (r.count() == 0) continue;
			uint ri = uint(random() % r.count());
			map.add(r[ri], r[ri] ^ 1);
			a.append(r[ri]);
			r.removeat(ri);
		}
		CHECK(map.count() == a.count());

		uint n = uint(random());
		if (map.contains(n)) continue;
		if (r.contains(n)) continue;
		map.add(n, n ^ 1);
		a.append(n);
	}

	for (uint i = 0; i < a.count(); i++) CHECK(map.contains(a[i]));
	for (uint i = 0; i < r.count(); i++) CHECK(!map.contains(r[i]));
	for (uint i = 0; i < a.count(); i++) CHECK(map[a[i]] == (a[i] ^ 1));
}

TEST_CASE("HashMap cstr key test:")
{
	TempMemPool zz;
	static char s[] =
		"Lorem ipsum. The microcode is organized in 2 planes of 16k * 24-bit codes. "
		"There is a 1-instruction pipeline which imposes a 1-instruction latency on code branching: "
		"In every microcode instruction a flag is selected. This flag is tested at the end of the "
		"previous instruction and determines from which code plane the next instruction is read. "
		"The assembler takes care for the most of this brain hazzard.";
	Array<cstr> z;
	split(z, s);

	HashMap<cstr, cstr> map(8);
	Array<cstr>			a;
	for (uint i = 0; i < z.count(); i++)
		for (uint j = 0; j < z.count(); j++)
		{
			a.appendifnew(catstr(z[i], z[j]));
			map.add(a.last(), tostr(a.count() - 1));
		}
	CHECK(a.count() == map.count());

	map = HashMap<cstr, cstr>(8);
	for (uint i = a.count(); i--;)
	{
		map.add(a[i], tostr(i));
		uint n = uint(random() % a.count());
		map.add(a[n], tostr(n));
	}
	CHECK(a.count() == map.count());

	for (uint i = a.count(); i--;) { CHECK(eq(map[a[i]], tostr(i))); }

	a.shuffle();
	Array<cstr> r;
	while (map.count() > 100)
	{
		r.append(a.pop());
		map.remove(r.last());
	}
	CHECK(a.count() == map.count());
	for (uint i = 0; i < a.count(); i++) CHECK(map.contains(a[i]));
	for (uint i = 0; i < r.count(); i++) CHECK(!map.contains(r[i]));

	for (uint i = 0; i < 10000; i++)
	{
		if (random() & 1)
		{
			if (a.count() == 0) continue;
			uint ai = uint(random() % a.count());
			map.remove(a[ai]);
			r.append(a[ai]);
			a.removeat(ai);
		}
		else
		{
			if (r.count() == 0) continue;
			uint ri = uint(random() % r.count());
			map.add(r[ri], nullptr);
			a.append(r[ri]);
			r.removeat(ri);
		}
		CHECK(map.count() == a.count());

		cstr n = tostr(random());
		if (map.contains(n)) continue;
		map.add(n, "");
		a.append(n);
	}
	for (uint i = 0; i < a.count(); i++) CHECK(map.contains(a[i]));
	for (uint i = 0; i < r.count(); i++) CHECK(!map.contains(r[i]));
}
