#pragma once

template<class F>
struct point {
	F x, y, z;

	point operator- (const point& b) const {
		return { x - b.x, y - b.y, z - b.z };
	}

	point& operator-= (const point& b) {
		return *this = *this - b;
	}

	bool operator!= (const point& b) const {
		return x != b.x || y != b.y || z != b.z;
	}

	bool operator== (const point& b) const {
		return x == b.x && y == b.y && z == b.z;
	}
};

template<class F, class Label>
struct labeled_point : point<F> {
	Label label;
	
	labeled_point() {}
	labeled_point(const F& x, const F& y, const F& z, const Label& label) : point<F>{ x, y, z }, label(label) {}
};

/*
 * Returns a positive value if the four points given in this order are in normal order, a negative
 * value if they are in antinormal order, and zero if they are coplanar.
 * The argument should be of the same type (template parameter Point), and should be point-like.
 * The struct labeled_point works.
 */
template<class Point>
decltype(Point::x) orientation(const Point& p, const Point& q, const Point& r, const Point& s) {
	decltype(Point::x) sum(0);
	auto a = q - p;
	auto b = r - p;
	auto c = s - p;
	sum += a.x * (b.y * c.z - b.z * c.y);
	sum += a.y * (b.z * c.x - b.x * c.z);
	sum += a.z * (b.x * c.y - b.y * c.x);
	return sum;
}

/*
 * Returns true if the three given points lie on a single line, and false otherwise.
 * The argument should be of the same type (template parameter Point), and should be point-like.
 * The struct labeled_point works.
 */
template<class Point>
bool collinear(const Point& p, const Point& q, const Point& r) {
	auto a = q - p;
	auto b = r - p;
	return a.x * b.y == b.x * a.y
		&& a.y * b.z == b.y * a.z
		&& a.z * b.x == b.z * a.x;
}
