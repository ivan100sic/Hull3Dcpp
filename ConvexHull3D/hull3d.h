#pragma once

#include "hullgraph.h"
#include "point.h"

#include <random>
#include <chrono>
#include <unordered_map>
#include <algorithm>
#include <numeric>

template<class Point>
decltype(Point::x) facePointOrientation(std::shared_ptr<hullgraph::face<Point>> theFace, const Point& thePoint) {
	Point points[3];
	auto walkingEdge = theFace->outerComponent();
	points[0] = walkingEdge->origin()->data();
	walkingEdge = walkingEdge->next();
	points[1] = walkingEdge->origin()->data();
	points[2] = walkingEdge->destination()->data();
	return orientation(points[0], points[1], points[2], thePoint);
}

enum class convex_hull_update : char {
	initialTetrahedron,
	afterJoinFaces,
	afterInscribeVertex,
	afterMergeFaces,
	afterRemoveRedundantVertices,
};

template<class Point, class Callback>
std::shared_ptr<hullgraph::vertex<Point>> computeConvexHull3D(const std::vector<Point>& points, Callback callback) {
	using namespace hullgraph;
	using F = decltype(Point::x);
	using vertexptr = std::shared_ptr<vertex<Point>>;
	using edgeptr = std::shared_ptr<edge<Point>>;
	using faceptr = std::shared_ptr<face<Point>>;

	std::vector<Point> firstFourPoints, remainingPoints;
	for (const Point& point : points) {
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

	if (firstFourPoints.size() < 3) {
		// All points are on the same line
		return nullptr;
	}

	if (firstFourPoints.size() == 3) {
		// All points are on the same plane
		// Do the classical convex hull algorithm
		point<F> normalVector = vectorProduct(firstFourPoints[1] - firstFourPoints[0], firstFourPoints[2] - firstFourPoints[0]);

		auto leftTurn = [&](size_t a, size_t b, size_t c) {
			return scalarProduct(normalVector, vectorProduct(points[b] - points[a], points[c] - points[a])) > F(0);
		};

		std::vector<size_t> pointStack[2], pointOrdering(points.size());
		std::iota(pointOrdering.begin(), pointOrdering.end(), size_t(0));
		std::sort(pointOrdering.begin(), pointOrdering.end(), [&](size_t i, size_t j) {
			return points[i] < points[j];
			});

		for (int stackNum : {0, 1}) {
			std::vector<size_t>& stack = pointStack[stackNum];
			for (size_t i : pointOrdering) {
				while (stack.size() >= 2 && leftTurn(stack[stack.size() - 1], stack[stack.size() - 2], i)) {
					stack.pop_back();
				}
				stack.push_back(i);
			}
			stack.pop_back();
			std::reverse(pointOrdering.begin(), pointOrdering.end());
		}

		pointStack[0].insert(pointStack[0].end(), pointStack[1].begin(), pointStack[1].end());

		std::vector<Point> hullPoints(pointStack[0].size());
		for (size_t i = 0; i < pointStack[0].size(); i++) {
			hullPoints[i] = points[pointStack[0][i]];
		}

		return makePolygon(hullPoints)->outerComponent()->origin();
	}

	faceptr baseTriangle = makeTriangle(firstFourPoints[0], firstFourPoints[1], firstFourPoints[2]);
	vertexptr peakVertex = inscribeVertex(baseTriangle, firstFourPoints[3]);

	callback(convex_hull_update::initialTetrahedron, peakVertex);

	// shuffle the remaining points
	{
		static std::mt19937_64 rngEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
		std::shuffle(remainingPoints.begin(), remainingPoints.end(), rngEngine);
	}

	// Initialize the conflict graph
	std::vector<std::unordered_set<faceptr>> pointToFaces(remainingPoints.size());
	std::unordered_map<faceptr, std::vector<size_t>> faceToPoints;

	{
		faceptr faces[4];
		edgeptr walkingEdge = peakVertex->incidentEdge();
		faces[0] = walkingEdge->incidentFace();
		walkingEdge = walkingEdge->twin();
		faces[1] = walkingEdge->incidentFace();
		walkingEdge = walkingEdge->next()->twin();
		faces[2] = walkingEdge->incidentFace();
		walkingEdge = walkingEdge->prev()->twin();
		faces[3] = walkingEdge->incidentFace();

		for (size_t i = 0; i < 4; i++) {
			for (size_t j = 0; j < remainingPoints.size(); j++) {
				if (facePointOrientation(faces[i], remainingPoints[j]) > F(0)) {
					pointToFaces[j].insert(faces[i]);
					faceToPoints[faces[i]].push_back(j);
				}
			}
		}
	}

	// Add the points
	for (size_t i = 0; i < remainingPoints.size(); i++) {
		if (pointToFaces[i].size()) {
			std::vector<faceptr> faceSetToVector(pointToFaces[i].begin(), pointToFaces[i].end());
			join_faces_result<Point> joinResult = joinFaces(faceSetToVector);
			callback(convex_hull_update::afterJoinFaces, peakVertex);

			std::vector<vertexptr> borderVertices(joinResult.borderEdges.size());
			for (size_t i = 0; i < joinResult.borderEdges.size(); i++) {
				borderVertices[i] = joinResult.borderEdges[i]->origin();
			}

			vertexptr newVertex = inscribeVertex(joinResult.newFace, remainingPoints[i]);
			peakVertex = newVertex;
			callback(convex_hull_update::afterInscribeVertex, peakVertex);

			std::vector<edgeptr> newVertexEdges = adjacentEdges(newVertex);
			std::vector<bool> shouldMerge(newVertexEdges.size(), false);
			std::vector<bool> shouldSkip(newVertexEdges.size(), false);

			// Check whether the i-th new face should be merged
			for (size_t j = 0; j < newVertexEdges.size(); j++) {
				faceptr newTriangle = newVertexEdges[j]->incidentFace();
				vertexptr adjacentVertex = newVertexEdges[j]->next()->twin()->next()->destination();
				if (facePointOrientation(newTriangle, adjacentVertex->data()) == F(0)) {
					shouldMerge[j] = true;
				}
			}

			// First, merge adjacent coplanar new triangles
			for (size_t j = 0; j < newVertexEdges.size(); j++) {
				size_t jNext = j == newVertexEdges.size() - 1 ? 0 : j + 1;
				if (shouldMerge[j]) {
					faceptr adjacentFace1 = newVertexEdges[j]->next()->twin()->incidentFace();
					if (shouldMerge[jNext]) {
						faceptr adjacentFace2 = newVertexEdges[jNext]->next()->twin()->incidentFace();
						if (adjacentFace1 == adjacentFace2) {
							shouldSkip[jNext] = true;
						}
					}
				}
			}

			for (size_t j = 0; j < newVertexEdges.size(); j++) {
				if (shouldSkip[j]) {
					removeEdge(newVertexEdges[j]);
				}
			}

			// Process the new faces
			for (size_t j = 0; j < newVertexEdges.size(); j++) {
				if (shouldSkip[j]) {
					continue;
				}

				if (shouldMerge[j]) {
					// Merge the two faces and adjust the conflict graph
					faceptr adjacentFace = newVertexEdges[j]->next()->twin()->incidentFace();
					faceptr mergedFace = removeEdge(newVertexEdges[j]->next());
					std::swap(faceToPoints[mergedFace], faceToPoints[adjacentFace]);
					faceToPoints.erase(adjacentFace);
					for (size_t pointIdx : faceToPoints[mergedFace]) {
						pointToFaces[pointIdx].erase(adjacentFace);
						pointToFaces[pointIdx].insert(mergedFace);
					}
				}
				else {
					// Check the union of the two faces around this edge
					faceptr newTriangle = newVertexEdges[j]->incidentFace();
					faceptr adjacentFace = newVertexEdges[j]->next()->twin()->incidentFace();
					std::unordered_set<size_t> newConflicts;

					for (const faceptr& interestingFace : { joinResult.borderFaces[j], adjacentFace }) {
						auto mapIt = faceToPoints.find(interestingFace);
						if (mapIt != faceToPoints.end()) {
							for (size_t pointIdx : mapIt->second) {
								if (facePointOrientation(newTriangle, remainingPoints[pointIdx]) > F(0)) {
									newConflicts.insert(pointIdx);
								}
							}
						}
					}

					faceToPoints[newTriangle] = std::vector<size_t>(newConflicts.begin(), newConflicts.end());
					for (size_t pointIdx : newConflicts) {
						pointToFaces[pointIdx].insert(newTriangle);
					}
				}
			}

			callback(convex_hull_update::afterMergeFaces, peakVertex);

			// Delete removed faces from the conflict graph
			for (const faceptr& facePtr : faceSetToVector) {
				auto mapIt = faceToPoints.find(facePtr);
				if (mapIt != faceToPoints.end()) {
					for (size_t pointIdx : mapIt->second) {
						pointToFaces[pointIdx].erase(facePtr);
					}
					faceToPoints.erase(mapIt);
				}
			}

			// Delete vertices worth deleting. The conflict graph isn't changed.
			for (const vertexptr& borderVertex : borderVertices) {
				// Some border vertices may have been deleted during face merger, skip them
				if (borderVertex->incidentEdge()) {
					removeRedundantVertex(borderVertex);
				}
			}

			callback(convex_hull_update::afterRemoveRedundantVertices, peakVertex);
		}
	}

	return peakVertex;
}

template<class Point>
std::shared_ptr<hullgraph::vertex<Point>> computeConvexHull3D(const std::vector<Point>& points) {
	return computeConvexHull3D(points, [](convex_hull_update, const std::shared_ptr<hullgraph::vertex<Point>>&) {});
}
