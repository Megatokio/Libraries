// Copyright (c) 2007 - 2022 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#include "Colormap.h"


int Colormap::addColor(const Comp q[3]) throws /*bad alloc*/
{
	int n = used_colors++;
	if (n == cmapSize()) growCmap(n + 1);
	memcpy(array + n * 3, q, 3);
	return n;
}


int Colormap::condAddColor(const Comp q[3]) throws /*bad alloc*/
{
	int i = findColor(q);
	return i == not_found ? addColor(q) : i;
}


int Colormap::condAddColor(Comp r, Comp g, Comp b) throws /*bad alloc*/
{
	int i = findColor(r, g, b);
	return i == not_found ? addColor(r, g, b) : i;
}


void Colormap::shrinkCmap(int n) throws /*bad alloc*/
{
	n = valid_count(n);
	Cmap::Shrink(n * 3);
	if (used_colors > n) used_colors = n;
	if (transp >= used_colors) transp = unset;
}


void Colormap::shrinkColors(int n) throws /*bad alloc*/
{
	int m = valid_count(n);
	Cmap::Shrink(m * 3);
	if (used_colors > n) used_colors = n;
	if (transp >= used_colors) transp = unset;
}


int Colormap::findColor(Comp r, Comp g, Comp b) const
{
	const Comp* p0 = getCmap();
	const Comp* p  = p0 + cmapByteSize();

	while (p > p0)
	{
		p -= 3;
		if (p[0] == r && p[1] == g && p[2] == b)
		{
			int i = int(p - p0) / 3;
			if (i != transp) return i;
		}
	}

	return not_found;
}


cstr Colormap::cmapStr() const
{
	str			s = tempstr(usedColors() * 12 + 30 + 2);
	ptr			p = s;
	const Comp* q = getCmap();

	p += sprintf(p, "max/used=%i(%i)/%i(%i), cmap: ", cmapSize(), cmapBits(), usedColors(), usedBits());

	for (int i = 0; i < usedColors(); i++)
	{
		p += sprintf(p, "%hhu,%hhu,%hhu%s ", q[0], q[1], q[2], i == transp ? "*" : "");
		q += 3;
	}
	*--p = 0;
	return s;
}
