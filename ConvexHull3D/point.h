#pragma once

template<class T>
struct point {
	T x, y, z;

	point operator- (const point& b) const {
		return { x - b.x, y - b.y, z - b.z };
	}

	point& operator-= (const point& b) {
		return *this = *this - b;
	}
};

/*
 * Returns a positive value if the four points given in this order are in normal order, a negative
 * value if they are in antinormal order, and zero if they are coplanar.
 * The template parameter T should be a numeric data type.
 */
template<class T>
T orientation(const point<T>& p, const point<T>& q, const point<T>& r, const point<T>& s) {
	T sum {};
	point<T> a = q - p;
	point<T> b = r - p;
	point<T> c = s - p;
	sum += a.x * (b.y * c.z - b.z * c.y);
	sum += a.y * (b.z * c.x - b.x * c.z);
	sum += a.z * (b.x * c.y - b.y * c.x);
	return sum;
}

/*
 * Returns true if the three given points lie on a single line, and false otherwise.
 * The template parameter T should be a numeric data type.
 */
template<class T>
bool collinear(const point<T>& p, const point<T>& q, const point<T>& r) {
	point<T> a = q - p;
	point<T> b = r - p;
	return a.x * b.y == b.x * a.y
		&& a.y * b.z == b.y * a.z
		&& a.z * b.x == b.z * a.x;
}
