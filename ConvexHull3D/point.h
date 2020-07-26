#pragma once

#include <tuple>

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

	bool operator< (const point& b) const {
		return std::tie(x, y, z) < std::tie(b.x, b.y, b.z);
	}
};

template<class F, class Label>
struct labeled_point : point<F> {
	Label label;

	labeled_point() {}
	labeled_point(const F& x, const F& y, const F& z, const Label& label) : point<F>{ x, y, z }, label(label) {}
};

/*
 * Computes the vector product of two 3d vectors.
 */
template<class Point>
point<decltype(Point::x)> vectorProduct(const Point& a, const Point& b) {
	point<decltype(Point::x)> result;
	result.x = a.y * b.z - a.z * b.y;
	result.y = a.z * b.x - a.x * b.z;
	result.z = a.x * b.y - a.y * b.x;
	return result;
}

/*
 * Computes the scalar product of two 3d vectors.
 */
template<class Point1, class Point2>
decltype(Point1::x) scalarProduct(const Point1& a, const Point2& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

/*
 * Computes the determinant of the 3x3 matrix whose columns are the given points.
 */
template<class Point>
decltype(Point::x) determinant(const Point& a, const Point& b, const Point& c) {
	return scalarProduct(vectorProduct(a, b), c);
}

/*
 * Returns a positive value if the four points given in this order are in normal order, a negative
 * value if they are in antinormal order, and zero if they are coplanar.
 * The arguments should be of the same type (template parameter Point), and should be 3d point-like.
 * The struct labeled_point works.
 */
template<class Point>
decltype(Point::x) orientation(const Point& p, const Point& q, const Point& r, const Point& s) {
	return determinant(q - p, r - p, s - p);
}

/*
 * Returns true if the three given points lie on a single line, and false otherwise.
 * The argument should be of the same type (template parameter Point), and should be 3d point-like.
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
