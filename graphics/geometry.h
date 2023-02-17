// Copyright (c) 1997 - 2023 kio@little-bat.de
// BSD-2-Clause license
// https://opensource.org/licenses/BSD-2-Clause

#pragma once
#include "kio/kio.h"


namespace geometry
{


template<typename coord>
struct Dist
{
	coord dx, dy;

	Dist() noexcept = default;
	Dist(coord w, coord h) noexcept : dx(w), dy(h) {}

	Dist& operator+=(const Dist& d) noexcept
	{
		dx += d.dx;
		dy += d.dy;
		return *this;
	}
	Dist& operator-=(const Dist& d) noexcept
	{
		dx -= d.dx;
		dy -= d.dy;
		return *this;
	}
	Dist& operator/=(coord n) noexcept
	{
		dx /= n;
		dy /= n;
		return *this;
	}
	Dist& operator*=(coord n) noexcept
	{
		dx *= n;
		dy *= n;
		return *this;
	}

	Dist operator+(const Dist& b) noexcept { return Dist(*this) += b; }
	Dist operator-(const Dist& b) noexcept { return Dist(*this) -= b; }
	Dist operator/(coord n) noexcept { return Dist(*this) /= n; }
	Dist operator*(coord n) noexcept { return Dist(*this) *= n; }
	Dist operator-() noexcept { return Dist(-dx, -dy); }

	bool operator==(const Dist& q) const noexcept { return dx == q.dx && dy == q.dy; }
	bool operator!=(const Dist& q) const noexcept { return dx != q.dx || dy != q.dy; }

	double slope() const { return double(dy) / dx; }
};


template<typename coord>
struct Point
{
	coord x, y;

	Point() noexcept = default;
	Point(coord x, coord y) noexcept : x(x), y(y) {}

	explicit Point(const Dist<coord>& d) noexcept : x(d.dx), y(d.dy) {}
	explicit operator Dist<coord>() const noexcept { return Dist<coord>(x, y); }

	Point& operator+=(const Dist<coord>& d) noexcept
	{
		x += d.dx;
		y += d.dy;
		return *this;
	}
	Point& operator-=(const Dist<coord>& d) noexcept
	{
		x -= d.dx;
		y -= d.dy;
		return *this;
	}

	Point operator+(const Dist<coord>& b) noexcept { return Point(*this) += b; }
	Point operator-(const Dist<coord>& b) noexcept { return Point(*this) -= b; }

	friend Dist<coord> operator-(const Point& a, const Point& b) { return Dist<coord>(a.x - b.x, a.y - b.y); }

	bool operator==(const Point& q) const noexcept { return x == q.x && y == q.y; }
	bool operator!=(const Point& q) const noexcept { return x != q.x || y != q.y; }
};


template<typename coord>
struct Size
{
	coord width, height;

	Size() noexcept = default;
	Size(coord w, coord h) noexcept : width(w), height(h) {}

	// Size and Dist are mostly interchangeable:
	Size(const Dist<coord>& d) noexcept : width(d.dx), height(d.dy) {}
	operator Dist<coord>() const noexcept { return Dist<coord>(width, height); }

	bool operator==(const Size& b) const noexcept { return width == b.width && height == b.height; }
	bool operator!=(const Size& b) const noexcept { return width != b.width || height != b.height; }
};


template<typename T>
struct Rect
{
	Point<T> p1; // anchor point: normally the top left corner
	Size<T>	 sz; // size: Rect may be assumed to be empty if width <= 0 or height <= 0

	Rect() noexcept = default;
	Rect(T w, T h) noexcept : p1(0, 0), sz(w, h) { normalize(); }
	Rect(T x, T y, T w, T h) noexcept : p1(x, y), sz(w, h) { normalize(); }
	Rect(const Size<T>& d) noexcept : p1(0, 0), sz(d) { normalize(); }
	Rect(const Point<T>& p, const Size<T>& d) noexcept : p1(p), sz(d) { normalize(); }
	Rect(const Point<T>& a, const Point<T>& b) noexcept : p1(a), sz(b - a) { normalize(); }

	bool isNormalized() const noexcept { return sz.width >= 0 && sz.height >= 0; }
	void normalize() noexcept
	{
		if (sz.width < 0)
		{
			p1.x += sz.width;
			sz.width = -sz.width;
		}
		if (sz.height < 0)
		{
			p1.y += sz.height;
			sz.height = -sz.height;
		}
	}
	bool isEmpty() const noexcept { return sz.width <= 0 || sz.height <= 0; }

	T		 left() const noexcept { return p1.x; }
	T		 right() const noexcept { return p1.x + sz.width; }
	T		 top() const noexcept { return p1.y; } // assumes Y coord is top down
	T		 bottom() const noexcept { return p1.y + sz.height; }
	T		 width() const noexcept { return sz.width; }
	T		 height() const noexcept { return sz.height; }
	Size<T>	 size() const noexcept { return sz; }
	Point<T> center() const noexcept { return p1 + size() / 2; }

	const Point<T>& topLeft() const noexcept { return p1; }
	Point<T>		topRight() const noexcept { return Point<T>(left(), top()); }
	Point<T>		bottomLeft() const noexcept { return Point<T>(left(), bottom()); }
	Point<T>		bottomRight() const noexcept { return Point<T>(right(), bottom()); }

	void moveTo(const Point<T>& p) noexcept { p1 = p; }
	void move(const Dist<T>& d) noexcept { p1 += d; }
	void setSize(const Size<T>& s) noexcept { sz = s; }
	void setLeft(T x) noexcept
	{
		sz.width += p1.x - x;
		p1.x = x;
	}
	void setTop(T y) noexcept
	{
		sz.height += p1.y - y;
		p1.y = y;
	}
	void setRight(T x) noexcept { sz.width = x - p1.x; }
	void setBottom(T y) noexcept { sz.height = y - p1.y; }
	void setWidth(T w) noexcept { sz.width = w; }
	void setHeight(T h) noexcept { sz.height = h; }
	void setTopLeft(const Point<T>& p) noexcept
	{
		setTop(p.y);
		setLeft(p.x);
	}
	void setTopRight(const Point<T>& p) noexcept
	{
		setTop(p.y);
		setRight(p.x);
	}
	void setBottomLeft(const Point<T>& p) noexcept
	{
		setBottom(p.y);
		setLeft(p.x);
	}
	void setBottomRight(const Point<T>& p) noexcept
	{
		setBottom(p.y);
		setRight(p.x);
	}

	// grow / shrink box at all 4 corners:
	void grow(T d) noexcept
	{
		p1.x -= d;
		p1.y -= d;
		sz.width += 2 * d;
		sz.height += 2 * d;
	}

	bool operator==(const Rect& q) const noexcept { return p1 == q.p1 && sz == q.sz; }
	bool operator!=(const Rect& q) const noexcept { return p1 != q.p1 || sz != q.sz; }

	bool encloses(const Point<T>& q) const noexcept
	{
		// does this rect enclose the point?
		// note: points on the right or bottom border are outside.

		return q.x >= left() && q.x < right() && q.y >= top() && q.y < bottom();
	}
	bool encloses(const Rect& q) const noexcept
	{
		// does this rect fully enclose the other rect?

		return q.left() >= left() && q.right() <= right() && q.top() >= top() && q.bottom() <= bottom();
	}
	bool intersects(const Rect& q) const noexcept
	{
		// do this rect and the other rect intersect?
		// note: also returns true if either of both is empty and p1 is inside the other rect

		return top() < q.bottom() && bottom() > q.top() && left() < q.right() && right() > q.left();
	}

	Rect& uniteWith(const Rect& b) noexcept
	{
		// set this rect to the enclosing rect of this rect and the other rect

		assert(isNormalized());
		assert(b.isNormalized());

		if (b.left() < left()) setLeft(b.left());
		if (b.right() > right()) setRight(b.right());
		if (b.top() < top()) setTop(b.top());
		if (b.bottom() > bottom()) setBottom(b.bottom());
		return *this;
	}

	Rect& intersectWith(const Rect& b) noexcept
	{
		// set this rect to the intersection of this rect and the other rect

		assert(isNormalized());
		assert(b.isNormalized());

		if (b.left() > left()) setLeft(b.left());
		if (b.right() < right()) setRight(b.right());
		if (b.top() > top()) setTop(b.top());
		if (b.bottom() < bottom()) setBottom(b.bottom());
		if (!isNormalized()) setSize(Size<T>(0, 0)); // => empty
		return *this;
	}

	friend Rect united(Rect a, const Rect& b) noexcept { return a.unite_with(b); }
	friend Rect intersected(Rect a, const Rect& b) noexcept { return a.intersect_with(b); }
};


template<typename T>
inline Rect<T> alertRect(const Rect<T>& box, Size<T>& size)
{
	// calculate box on alert position inside other box

	return Rect<T>(box.p1 + Dist<T>((box.width() - size.width) / 2, 20 + (box.height() - size.height - 20) / 3), size);
}

} // namespace geometry
