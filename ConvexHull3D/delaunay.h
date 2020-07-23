#pragma once

#include "hull3d.h"

/*
 * Computes whether the given face looks up, i.e. whether its normal vector's z-value
 * is positive.
 */
template<class Point>
bool isFaceDirectedUp(const std::shared_ptr<hullgraph::face<Point>>& theFace) {
	Point points[3];
	using F = decltype(Point::x);
	std::shared_ptr<hullgraph::edge<Point>> walkingEdge = theFace->outerComponent();
	points[0] = walkingEdge->origin()->data();
	walkingEdge = walkingEdge->next();
	points[1] = walkingEdge->origin()->data();
	points[2] = walkingEdge->destination()->data();

	Point zPlus;
	zPlus.x = zPlus.y = 0;
	zPlus.z = F(1);

	return determinant(points[1] - points[0], points[2] - points[0], zPlus) > F(0);
}

/*
 * Computes the Delaunay triangulation of a set of points in the plane.
 * The template parameter Point should have two members x and y of the same type,
 * which should be a numeric type. Returns a single vertex of the triangulation graph.
 */
template<class Point2D>
std::shared_ptr<hullgraph::vertex<size_t>> delaunayTriangulation(const std::vector<Point2D>& points) {
	using namespace hullgraph;
	using F = decltype(Point2D::x);
	using local_point = labeled_point<F, size_t>;

	std::vector<local_point>& paraboloidPoints(points.size());
	for (size_t i = 0; i < points.size(); i++) {
		paraboloidPoints[i].x = 2 * points[i].x;
		paraboloidPoints[i].y = 2 * points[i].y;
		paraboloidPoints[i].z = points[i].x * points[i].x + points[i].y * points[i].y;
		paraboloidPoints[i].label = i;
	}

	std::shared_ptr<vertex<local_point>> theVertex = computeConvexHull3D(paraboloidPoints);
	auto allEdges = exploreGraph(theVertex);

	// Invalidate all half-edges and faces which look up
	for (const auto& theEdge : allEdges) {
		auto theFace = theEdge->incidentFace();
		if (isFaceDirectedUp(theFace)) {
			theFace->invalidate();
			theEdge->invalidate();
		}
	}

	return theVertex;
}
