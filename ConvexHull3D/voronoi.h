#pragma once

#include "delaunay.h"

template<class F>
struct voronoi_diagram {
	struct point {
		F x, y;
		bool atInfinity;
	};

	struct edge {
		size_t u, v;
	};

	std::vector<point> pointList;
	std::vector<edge> edgeList;
};

/*
 * Given an edge of the triangulation graph, computes the circumcenter of the face of the triangulation
 * corresponding to the inner face of the given halfEdge. In case that face is the outer face of the
 * triangulation, returns the corresponding point at infinity.
 */
template<class Point>
typename voronoi_diagram<decltype(Point::x)>::point circumcenter(
	std::shared_ptr<hullgraph::edge<Point>> halfEdge,
	std::shared_ptr<hullgraph::face<Point>> outerFace)
{
	using F = decltype(Point::x);
	using vd_point = voronoi_diagram<F>::point;

	if (halfEdge->incidentFace() == outerFace) {
		vd_point result;

		Point a = halfEdge->origin()->data();
		Point b = halfEdge->destination()->data();

		result.x = b.y - a.y;
		result.y = a.x - b.x;
		result.atInfinity = true;
		
		return result;
	}

	vd_point result;
	result.atInfinity = false;

	Point a = halfEdge->origin()->data();
	Point b = halfEdge->destination()->data();
	Point c = halfEdge->next()->destination()->data();
	F dInverse = F(1) / (F(2) * (a.x * (b.y - c.y) + b.x * (c.y - a.y) + c.x * (a.y - b.y)));

	F aNorm2 = a.x * a.x + a.y * a.y;
	F bNorm2 = b.x * b.x + b.y * b.y;
	F cNorm2 = c.x * c.x + c.y * c.y;

	result.x = (
		aNorm2 * (b.y - c.y) +
		bNorm2 * (c.y - a.y) +
		cNorm2 * (a.y - b.y)) * dInverse;

	result.y = (
		aNorm2 * (c.x - b.x) +
		bNorm2 * (a.x - c.x) +
		cNorm2 * (b.x - a.x)) * dInverse;

	return result;
}

/*
 * Given the outer face of the Delaunay triangulation, computes the Voronoi diagram.
 */
template<class Point>
voronoi_diagram<decltype(Point::x)> computeVoronoiDiagram(const std::shared_ptr<hullgraph::face<Point>>& outerFace) {
	using F = decltype(Point::x);
	using faceptr = std::shared_ptr<hullgraph::face<Point>>;
	using edgeptr = std::shared_ptr<hullgraph::edge<Point>>;
	using vertexptr = std::shared_ptr<hullgraph::vertex<Point>>;
	using vertexptr = std::shared_ptr<hullgraph::vertex<Point>>;
	using vd_point = voronoi_diagram<F>::point;

	voronoi_diagram<F> result;

	// Each internal face and each edge of the outer face will get mapped to a single point
	// Do not compute circumcircles more than once, and do not produce more points than needed.
	std::unordered_map<faceptr, size_t> internalFaceToPointIdx;
	std::unordered_map<edgeptr, size_t> outerFaceEdgeToPointIdx;

	auto allEdges = exploreGraph(outerFace->outerComponent()->origin());

	for (auto& theEdge : allEdges) {
		auto theFace = theEdge->incidentFace();
		if (theFace != outerFace) {
			auto it = internalFaceToPointIdx.find(theFace);
			if (it == internalFaceToPointIdx.end()) {
				vd_point newPoint = circumcenter(theEdge, outerFace);
				internalFaceToPointIdx[theFace] = result.pointList.size();
				result.pointList.push_back(newPoint);
			}
		}
		else {
			vd_point newPoint = circumcenter(theEdge, outerFace);
			outerFaceEdgeToPointIdx[theEdge] = result.pointList.size();
			result.pointList.push_back(newPoint);
		}
	}

	for (auto& theEdge : allEdges) {
		auto twinEdge = theEdge->twin();

		// Ensure that we process edge/twin edge pair exactly once
		if (theEdge < twinEdge) {
			if (theEdge->incidentFace() == outerFace) {
				result.edgeList.push_back({
					outerFaceEdgeToPointIdx[theEdge],
					internalFaceToPointIdx[twinEdge->incidentFace()]
					});
			}
			else if (twinEdge->incidentFace() == outerFace) {
				result.edgeList.push_back({
					outerFaceEdgeToPointIdx[twinEdge],
					internalFaceToPointIdx[theEdge->incidentFace()]
					});
			}
			else {
				result.edgeList.push_back({
					internalFaceToPointIdx[theEdge->incidentFace()],
					internalFaceToPointIdx[twinEdge->incidentFace()]
					});
			}
		}
	}

	return result;
}

/*
 * Given a list of points, compute the Voronoi diagram.
 */
template<class Point>
voronoi_diagram<decltype(Point::x)> computeVoronoiDiagram(const std::vector<Point>& points) {
	return computeVoronoiDiagram(delaunayTriangulation(points));
}
