#pragma once

#include "hull3d.h"

/*
 * Computes whether the given face looks up, i.e. whether its normal vector's z-value
 * is nonnegative.
 */
template<class Point>
bool isFaceDirectedUpOrVertical(const std::shared_ptr<hullgraph::face<Point>>& theFace) {
	using F = decltype(Point::x);
	point<F> points[3];
	auto walkingEdge = theFace->outerComponent();
	points[0] = walkingEdge->origin()->data();
	walkingEdge = walkingEdge->next();
	points[1] = walkingEdge->origin()->data();
	points[2] = walkingEdge->destination()->data();

	point<F> zPlus{ F(0), F(0), F(1) };

	return determinant(points[1] - points[0], points[2] - points[0], zPlus) >= F(0);
}

/*
 * Computes the Delaunay triangulation of a set of points in the plane.
 * The template parameter Point2D should have two members x and y of the same type,
 * which should be a numeric type. Returns the external face of the triangulation graph.
 */
template<class Point2D>
std::shared_ptr<hullgraph::face<labeled_point<decltype(Point2D::x), size_t>>> delaunayTriangulation(const std::vector<Point2D>& points) {
	using namespace hullgraph;
	using F = decltype(Point2D::x);
	using local_point = labeled_point<F, size_t>;

	std::vector<local_point> paraboloidPoints(points.size());
	for (size_t i = 0; i < points.size(); i++) {
		paraboloidPoints[i].x = points[i].x;
		paraboloidPoints[i].y = points[i].y;
		paraboloidPoints[i].z = points[i].x * points[i].x + points[i].y * points[i].y;
		paraboloidPoints[i].label = i;
	}

	auto theVertex = computeConvexHull3D(paraboloidPoints);
	auto allEdges = exploreGraph(theVertex);

	std::unordered_set<std::shared_ptr<face<local_point>>> facesToJoinSet;

	// Join together all faces which look up
	for (const auto& theEdge : allEdges) {
		auto theFace = theEdge->incidentFace();
		if (isFaceDirectedUpOrVertical(theFace)) {
			facesToJoinSet.insert(theFace);
		}
	}

	std::vector<std::shared_ptr<face<local_point>>> facesToJoin(facesToJoinSet.begin(), facesToJoinSet.end());
	return joinFaces(facesToJoin).newFace;
}
