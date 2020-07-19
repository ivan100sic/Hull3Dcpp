#pragma once

#include "hullgraph.h"
#include "point.h"

#include <random>
#include <chrono>
#include <unordered_map>

template<class F>
F facePointOrientation(std::shared_ptr<hullgraph::face<point<F>>> theFace, const point<F>& thePoint) {
	point<F> points[3];
	std::shared_ptr<edge<point<F>>> walkingEdge = theFace->outerComponent();
	points[0] = walkingEdge->origin()->data();
	walkingEdge = walkingEdge->next();
	points[1] = walkingEdge->origin()->data();
	points[2] = walkingEdge->destination()->data();
	return orientation(points[0], points[1], points[2], thePoint);
}

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

		// Integrity check
		// Gather all faces
		auto allEdges = exploreGraph(peakVertex);
		std::unordered_set<faceptr> allFaces;
		for (auto e : allEdges) {
			allFaces.insert(e->incidentFace());
		}

		size_t pairsFound1 = 0;
		size_t pairsFound2 = 0;
		size_t pairsFound3 = 0;
		for (auto f : allFaces) {
			for (size_t id = 0; id < remainingPoints.size(); id++) {
				if (facePointOrientation(f, remainingPoints[id]) > F(0)) {
					pairsFound1++;
					if (pointToFaces[id].count(f) == 0) {
						throw 111;
					}

					if (std::find(faceToPoints[f].begin(), faceToPoints[f].end(), id) == faceToPoints[f].end()) {
						throw 222;
					}
				}
			}
		}

		for (auto f : faceToPoints) {
			pairsFound2 += f.second.size();
		}

		for (auto f : pointToFaces) {
			pairsFound3 += f.size();
		}

		if (pairsFound1 != pairsFound2) {
			throw 333;
		}

		if (pairsFound2 != pairsFound3) {
			throw 444;
		}

		if (pointToFaces[i].size()) {
			std::vector<faceptr> faceSetToVector(pointToFaces[i].begin(), pointToFaces[i].end());
			join_faces_result<point<F>> joinResult = joinFaces(faceSetToVector);

			std::vector<vertexptr> borderVertices(joinResult.borderEdges.size());
			for (size_t i = 0; i < joinResult.borderEdges.size(); i++) {
				borderVertices[i] = joinResult.borderEdges[i]->origin();
			}

			vertexptr newVertex = inscribeVertex(joinResult.newFace, remainingPoints[i]);
			peakVertex = newVertex;

			std::vector<edgeptr> newVertexEdges = adjacentEdges(newVertex);
			std::vector<bool> shouldMerge(newVertexEdges.size(), false);

			// Check whether the i-th new face should be merged
			for (size_t j = 0; j < newVertexEdges.size(); j++) {
				faceptr newTriangle = newVertexEdges[j]->incidentFace();
				vertexptr adjacentVertex = newVertexEdges[j]->next()->twin()->next()->destination();
				if (facePointOrientation(newTriangle, adjacentVertex->data()) == F(0)) {
					shouldMerge[j] = true;
				}
			}

			// Process the new faces
			for (size_t j = 0; j < newVertexEdges.size(); j++) {
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
				} else {
					// Check the union of the two faces around this edge
					faceptr newTriangle = newVertexEdges[j]->incidentFace();
					faceptr adjacentFace = newVertexEdges[j]->next()->twin()->incidentFace();
					std::unordered_set<size_t> newConflicts;

					for (const faceptr& interestingFace : {joinResult.borderFaces[j], adjacentFace}) {
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
				removeRedundantVertex(borderVertex);
			}
		}
	}

	return peakVertex;
}
