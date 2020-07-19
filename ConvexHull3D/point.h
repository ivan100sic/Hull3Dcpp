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

/*
 * Returns a positive value if the four points given in this order are in normal order, a negative
 * value if they are in antinormal order, and zero if they are coplanar.
 * The template parameter F should be a numeric data type.
 */
template<class F>
F orientation(const point<F>& p, const point<F>& q, const point<F>& r, const point<F>& s) {
	F sum {};
	point<F> a = q - p;
	point<F> b = r - p;
	point<F> c = s - p;
	sum += a.x * (b.y * c.z - b.z * c.y);
	sum += a.y * (b.z * c.x - b.x * c.z);
	sum += a.z * (b.x * c.y - b.y * c.x);
	return sum;
}

/*
 * Returns true if the three given points lie on a single line, and false otherwise.
 * The template parameter F should be a numeric data type.
 */
template<class F>
bool collinear(const point<F>& p, const point<F>& q, const point<F>& r) {
	point<F> a = q - p;
	point<F> b = r - p;
	return a.x * b.y == b.x * a.y
		&& a.y * b.z == b.y * a.z
		&& a.z * b.x == b.z * a.x;
}
