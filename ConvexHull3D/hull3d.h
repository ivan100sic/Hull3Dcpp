#pragma once

#include "hullgraph.h"
#include "point.h"

template<class F>
std::shared_ptr<hullgraph::vertex<point<F>>> computeConvexHull3D(const std::vector<point<F>>& points) {
	using namespace hullgraph;
	using vertexptr = std::shared_ptr<vertex<point<F>>>;
	using edgeptr = std::shared_ptr<edge<point<F>>>;
	using faceptr = std::shared_ptr<face<point<F>>>;

	std::vector<point<F>> firstFourPoints, remainingPoints;
	for (const point<F>& point : points) {
		switch (firstFourPoints.size()) {
		case 0:
			firstFourPoints.push_back(point);
			break;
		case 1:
			if (point != firstFourPoints[0]) {
				firstFourPoints.push_back(point);
			}
			else {
				remainingPoints.push_back(point);
			}
			break;
		case 2:
			if (!collinear(point, firstFourPoints[0], firstFourPoints[1])) {
				firstFourPoints.push_back(point);
			}
			else {
				remainingPoints.push_back(point);
			}
			break;
		case 3:
		{
			F orientationValue = orientation(firstFourPoints[0], firstFourPoints[1], firstFourPoints[2], point);
			if (orientationValue == F(0)) {
				remainingPoints.push_back(point);
			}
			else if (orientationValue > F(0)) {
				firstFourPoints.push_back(point);
			}
			else {
				std::swap(firstFourPoints[0], firstFourPoints[1]);
				firstFourPoints.push_back(point);
			}
			break;
		}
		default:
			remainingPoints.push_back(point);
		}
	}

	if (firstFourPoints.size() < 4) {
		// Degenerate cases, won't handle them for now.
		return nullptr;
	}

	faceptr baseTriangle = makeTriangle(firstFourPoints[0], firstFourPoints[1], firstFourPoints[2]);
	vertexptr peakVertex = inscribeVertex(baseTriangle, firstFourPoints[3]);

	// shuffle the remaining points and add them...
	return peakVertex;
}
