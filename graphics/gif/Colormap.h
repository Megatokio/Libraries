#pragma once
// Copyright (c) 2007 - 2025 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause


/*	Maintain a color map for indexed color images and Gif files
	-----------------------------------------------------------

Overview:

	the Colormap contains an array of colors, each made out of 3 bytes for R, G and B.

	the Colormap can be sized to contain no colors  or  up to 2^n colors with n = 1 to 8  <=>  2 to 256 colors.
	the size of the Colormap is always extended to full 2^n.

	a Colormap size of 2^0 colors <=> 1 color is not allowed and set to 2^1 <=> 2 colors instead.
	this is basically to support gif color maps, which must be at least 2^1 <=> 2 colors in size, if present.

	the Colormap maintains a transparent color index.

	the Colormap maintains the number of actually used colors.

	when adding colors with AddColor() or CondAddColor() or AddTranspColor() to a fully occupied Colormap,
	then the colormap is extended to the next allowed size, that is, to 2^(n+1). (but stepping from 0 to 2 directly.)

	if the Colormap is shrinked, then it is again only shrinked to the next allowed size,
	e.g. shrinking to 5 colors will actually shrink to 2^3 = 8 colors,
	and the used color count and the transparent color index are validated.

	used colors can be accessed with operator[], which returns a pointer (!) to the first color component.
*/

#include "GifArray.h"
#include "kio/kio.h"
#include "kio/util/msbit.h"

using Comp = uint8;
using Cmap = GifArray<Comp>;
#define throws /*bad alloc*/ noexcept(false) // bad_alloc,limit_error


class Colormap : protected Cmap
{
protected:
	//	Comp*	Cmap::array		// allocated memory
	//	int		Cmap::count		// allocated memory size
	int used_colors; // number of actually used colors:   [0 .. [count/3
	int transp;		 // transparent color index: unset or [0 .. [used_colors

	static int bits(uint16 n) { return n == 0 ? 0 : msbit(uint8(n - 1)) + 1; } // f(0)=0, f(1)=1, f(2^n)=n
	static int valid_count(uint16 n)
	{
		assert(n <= 256);
		return n == 0 ? 0 : 1 << bits(n);
	}
	// 0 or 2,4,8,16,32,64,128,256

private:
	// init, copy, kill:
	void init()
	{
		used_colors = 0;
		transp		= unset;
	}
	void init(const Comp* q, int n, int i)
	{
		memcpy(array, q, n * 3);
		used_colors = n;
		transp		= i;
	}


public:
	// constants for transparent color index and find_color(..)
	enum { unset = -1, not_found = -1 };


	// create, copy, destroy:
	Colormap() noexcept { init(); }
	Colormap(int n) throws /*bad alloc*/ : Cmap(valid_count(n) * 3) { init(); }
	Colormap(const Comp* q, int n, int i = unset) throws /*bad alloc*/ : Cmap(valid_count(n) * 3) { init(q, n, i); }
	//			~Colormap		()						{ kill(); }
	//			Colormap		( const Colormap& q )		{ init(q); }
	//  Colormap& operator=  	( const Colormap& q )		{ copy(q); }


	// color map:

	Comp*		getCmap() noexcept { return array; }
	const Comp* getCmap() const noexcept { return array; }
	int			cmapByteSize() const noexcept { return count; }

	void purgeCmap() noexcept
	{
		Cmap::Purge();
		init();
	}
	int	 cmapSize() const noexcept { return count / 3; }
	int	 cmapBits() const noexcept { return bits(count / 3); }
	void growCmap(int n) throws /*bad alloc*/
	{
		n = valid_count(n);
		Cmap::Grow(n * 3);
	}
	void shrinkCmap(int n) throws /*bad alloc*/;


	// transparent color:

	bool hasTranspColor() const noexcept { return transp != unset; }
	int	 transpColor() const noexcept { return transp; }
	void setTranspColor(int i) noexcept
	{
		assert(i + 1 <= used_colors + 1);
		transp = i;
	}
	void clearTranspColor() noexcept { transp = unset; }
	int	 addTranspColor() throws /*bad alloc*/
	{
		if (transp == unset) transp = addColor(0, 0, 0);
		return transp;
	}


	// actually used colors:

	int	 usedColors() const noexcept { return used_colors; }
	int	 usedBits() const noexcept { return bits(used_colors); }
	void growColors(int n) throws /*bad alloc*/
	{
		if (n > used_colors)
		{
			growCmap(n);
			used_colors = n;
		}
	}
	void shrinkColors(int n) throws /*bad alloc*/;

	Comp* operator[](int i) noexcept
	{
		assert(i < used_colors);
		return array + i * 3;
	}
	const Comp* operator[](int i) const noexcept
	{
		assert(i < used_colors);
		return array + i * 3;
	}

	int addColor(Comp r, Comp g, Comp b) throws /*bad alloc*/
	{
		Comp q[] = {r, g, b};
		return addColor(q);
	}
	int addColor(const Comp q[3]) throws /*bad alloc*/;

	int condAddColor(const Comp q[3]) throws /*bad alloc*/;
	int condAddColor(Comp r, Comp g, Comp b) throws /*bad alloc*/;


	// find color by it's rgb components:
	// skip transparent color (if any); return index or not_found

	int findColor(Comp r, Comp g, Comp b) const;
	int findColor(const Comp p[3]) const { return findColor(p[0], p[1], p[2]); }


	// debug:

	cstr cmapStr() const;
};
