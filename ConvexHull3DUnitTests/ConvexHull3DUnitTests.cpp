#include "pch.h"
#include "CppUnitTest.h"
#include "../ConvexHull3D/voronoi.h"

#include <set>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace hullgraph;

namespace ConvexHull3DUnitTests {
	TEST_CLASS(PointUnitTests) {
	public:
		
		TEST_METHOD(PointOrientation1) {
			point<int> pts[4] = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
			Assert::IsTrue(orientation(pts[0], pts[1], pts[2], pts[3]) > 0);
		}

		TEST_METHOD(PointOrientation2) {
			point<int> pts[4] = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, -1} };
			Assert::IsTrue(orientation(pts[0], pts[1], pts[2], pts[3]) < 0);
		}

		TEST_METHOD(PointOrientation3) {
			point<int> pts[4] = { {1, 2, 3}, {5, 4, 4}, {4, 2, 9}, {2, 8, -16} };
			Assert::IsTrue(orientation(pts[0], pts[1], pts[2], pts[3]) == 0);
		}

		TEST_METHOD(PointsCollinear1) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			Assert::IsTrue(collinear(pts[0], pts[1], pts[2]));
		}

		TEST_METHOD(PointsCollinear2) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {7, 11, 15} };
			Assert::IsFalse(collinear(pts[0], pts[1], pts[2]));
		}

		TEST_METHOD(LabeledPointCompiles) {
			labeled_point<int, std::string> pts[3] = { {0, 1, 2, "one"}, {4, 7, 10, "two"}, {7, 11, 15, "three"} };
			Assert::IsFalse(collinear(pts[0], pts[1], pts[2]));
		}
	};

	TEST_CLASS(HullGraphUnitTests) {
	public:

		TEST_METHOD(MakeTriangleCompiles) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			auto f = makeTriangle(pts[0], pts[1], pts[2]);
			Assert::IsTrue(!!f);
		}

		TEST_METHOD(InscribePointCompiles) {
			point<int> pts[4] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14}, {0, 0, 0} };
			auto f = makeTriangle(pts[0], pts[1], pts[2]);
			auto newPoint = inscribeVertex(f, pts[3]);
			Assert::IsTrue(!!f);
			Assert::IsTrue(!!newPoint);
		}

		TEST_METHOD(FaceToEdgeListTriangle) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			auto f = makeTriangle(pts[0], pts[1], pts[2]);
			Assert::IsTrue(faceToEdgeList(f).size() == 3);
		}

		TEST_METHOD(InscribePointAppearsToWork) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			auto newPointEdges = adjacentEdges(newPoint);
			Assert::AreEqual(3, (int)newPointEdges.size());
		}

		TEST_METHOD(RemoveEdgeCompilesAndAppearsToWork) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			auto newFace = removeEdge(newPoint->incidentEdge());
			Assert::IsTrue(!!newFace);
			Assert::AreEqual(4, (int)faceToEdgeList(newFace).size());
		}

		TEST_METHOD(JoinFacesCompilesAndAppearsToWork) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			std::vector<std::shared_ptr<face<int>>> faces = {
				newPoint->incidentEdge()->incidentFace(),
				newPoint->incidentEdge()->twin()->incidentFace()
			};

			auto result = joinFaces(faces);

			Assert::IsTrue(!!result.newFace);
			Assert::AreEqual(0, (int)result.removedVertices.size());
			Assert::AreEqual(2, (int)result.removedEdges.size());
			Assert::AreEqual(4, (int)faceToEdgeList(result.newFace).size());
			Assert::AreEqual(4, (int)result.borderEdges.size());
			Assert::AreEqual(4, (int)result.borderFaces.size());
		}

		TEST_METHOD(RemoveRedundantVertexCompilesAndReturnsNull) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			auto result = removeRedundantVertex(newPoint);

			Assert::IsTrue(!result);
		}

		TEST_METHOD(RemoveRedundantVertexCompilesAndAppearsToWork) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			auto someEdge = newPoint->incidentEdge();
			auto otherPoint = someEdge->destination();
			auto newFace = removeEdge(someEdge);
			auto bridge = removeRedundantVertex(otherPoint);

			Assert::AreEqual(3, (int)faceToEdgeList(bridge->incidentFace()).size());
			Assert::AreEqual(2, (int)faceToEdgeList(bridge->twin()->incidentFace()).size());
		}

		TEST_METHOD(ExploreGraphCompilesAndAppearsToWork) {
			auto f = makeTriangle(0, 1, 2);
			auto newPoint = inscribeVertex(f, 3);
			auto allEdges = exploreGraph(newPoint);

			Assert::AreEqual(12, (int)allEdges.size());

			std::set<std::pair<int, int>> pendingEdges;
			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (i != j) {
						pendingEdges.insert({ i, j });
					}
				}
			}

			for (const auto& theEdge : allEdges) {
				pendingEdges.erase({ theEdge->origin()->data(), theEdge->destination()->data() });
			}

			Assert::AreEqual(0, (int)pendingEdges.size());
		}
	};

	TEST_CLASS(Hull3DUnitTests) {
	public:

		TEST_METHOD(ComputeConvexHull3DCompiles) {
			std::vector<point<int>> pts = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
			auto hullVertex = computeConvexHull3D(pts);
			Assert::IsTrue(!!hullVertex);
		}

		TEST_METHOD(VerifyConvexHullOrientation) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10} };
			auto hullVertex = computeConvexHull3D(pts);
			point<int> pt = { 1, 1, 1 };
			
			auto faceEdges = faceToEdgeList(hullVertex->incidentEdge()->incidentFace());
			std::vector<point<int>> facePoints(faceEdges.size());

			Assert::AreEqual(3, (int)faceEdges.size());

			for (size_t i = 0; i < 3; i++) {
				facePoints[i] = faceEdges[i]->origin()->data();
			}
			
			Assert::IsTrue(orientation(facePoints[0], facePoints[1], facePoints[2], pt) < 0);
		}

		TEST_METHOD(Hull3DCubeLattice) {
			std::vector<point<int>> pts;

			for (int i = 0; i < 6; i++) {
				for (int j = 0; j < 6; j++) {
					for (int k = 0; k < 6; k++) {
						pts.push_back({ i, j, k });
					}
				}
			}

			for (size_t repetition = 0; repetition < 100; repetition++) {
				auto hullVertex = computeConvexHull3D(pts);
				auto allEdges = exploreGraph(hullVertex);
				Assert::AreEqual(24, (int)allEdges.size());
			}
		}

		TEST_METHOD(Hull3DParaboloid) {
			std::vector<point<int>> pts;
			for (int i = 0; i < 30; i++) {
				for (int j = 0; j < 30; j++) {
					pts.push_back({ i, j, i*i+j*j });
				}
			}

			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);
		}

		TEST_METHOD(Hull3DSphere) {
			const int numPoints = 1000;
			const double pi = acos(double(-1));
			
			std::vector<point<double>> pts(numPoints);
			std::mt19937_64 randomEngine;
			std::uniform_real_distribution<double> angleGen(0, pi);

			for (int i = 0; i < numPoints; i++) {
				double phi = 2 * angleGen(randomEngine);
				double theta = angleGen(randomEngine) - pi / 2;
				double x = cos(phi) * cos(theta);
				double y = sin(phi) * cos(theta);
				double z = sin(theta);

				pts[i] = { x, y, z };
			}

			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);

			Assert::AreEqual(6 * numPoints - 12, (int)allEdges.size());
		}

		TEST_METHOD(Hull3DTetrahedron1) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10}, {1, 1, 1} };
			auto hullVertex = computeConvexHull3D(pts);

			Assert::AreEqual(12, (int)exploreGraph(hullVertex).size());
		}

		TEST_METHOD(Hull3DTetrahedron2) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10}, {-100, -100, -100} };
			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);

			Assert::AreEqual(12, (int)allEdges.size());

			for (const auto& theEdge : allEdges) {
				if (theEdge->origin()->data() == point<int>{0, 0, 0}) {
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(Hull3DTetrahedron3) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10}, {6, 6, 6} };
			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);

			Assert::AreEqual(18, (int)allEdges.size());
		}

		TEST_METHOD(Hull3DTriangularDipyramid1) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10}, {6, 6, 6}, {7, 7, 7} };
			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);

			Assert::AreEqual(18, (int)allEdges.size());
		}

		TEST_METHOD(Hull3DTriangularDipyramid2) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10} };
			for (int t = 6; t <= 100; t++) {
				pts.push_back({ t, t, t });
			}
			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);

			Assert::AreEqual(18, (int)allEdges.size());

			for (const auto& theEdge : allEdges) {
				auto thePoint = theEdge->origin()->data();
				if (thePoint.x % 10 + thePoint.y % 10 + thePoint.z % 10 != 0) {
					Assert::Fail();
				}
			}
		}

		TEST_METHOD(Hull3DPlanar1) {
			std::vector<point<int>> pts = { {0, 0, 0}, {0, 1, 10}, {2, 5, 4} };
			auto hullVertex = computeConvexHull3D(pts);
			auto allEdges = exploreGraph(hullVertex);
			Assert::AreEqual(6, (int)allEdges.size());
		}

		TEST_METHOD(Hull3DPlanar2) {
			std::vector<point<int>> pts = { {0, 0, 0}, {0, 0, 10}, {0, 10, 0}, {0, 10, 10}, {0, 5, 5} };
			auto hullVertex = computeConvexHull3D(pts);
			auto hullFace = hullVertex->incidentEdge()->incidentFace();
			Assert::AreEqual(4, (int)faceToEdgeList(hullFace).size());
		}

		TEST_METHOD(Hull3DPlanar3) {
			std::vector<point<int>> pts = { {0, 0, 0}, {0, 0, 10}, {0, 10, 0}, {0, 10, 10}, {0, 5, 5}, {0, 13, 5} };
			auto hullVertex = computeConvexHull3D(pts);
			auto hullFace = hullVertex->incidentEdge()->incidentFace();
			Assert::AreEqual(5, (int)faceToEdgeList(hullFace).size());
		}
	};

	TEST_CLASS(DelaunayTriangulationTests) {
	public:

		TEST_METHOD(IsFaceDirectedUpTest1) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {10, 0, 10} };
			auto triangle = makePolygon(pts);
			Assert::IsTrue(isFaceDirectedUpOrVertical(triangle));
		}

		TEST_METHOD(IsFaceDirectedUpTest2) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {10, 1, 10} };
			auto triangle = makePolygon(pts);
			Assert::IsTrue(isFaceDirectedUpOrVertical(triangle));
		}

		TEST_METHOD(IsFaceDirectedUpTest3) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {10, -1, 10} };
			auto triangle = makePolygon(pts);
			Assert::IsFalse(isFaceDirectedUpOrVertical(triangle));
		}

		TEST_METHOD(DelaunayTriangulationBasicTest1) {
			std::vector<point<int>> pts = { {0, 0}, {0, 10}, {10, 0}, {6, 6} };
			auto extFace = delaunayTriangulation(pts);
			Assert::AreEqual(4, (int)faceToEdgeList(extFace).size());

			std::pair<size_t, size_t> expectedEdges[] = { {0, 1}, {0, 2}, {0, 3}, {1, 3}, {2, 3} };
			std::set<std::pair<size_t, size_t>> actualEdges;

			for (const auto& theEdge : exploreGraph(extFace->outerComponent()->origin())) {
				size_t u = theEdge->origin()->data().label;
				size_t v = theEdge->destination()->data().label;
			
				if (u < v) {
					actualEdges.insert({ u, v });
				}
			}

			Assert::IsTrue(actualEdges == std::set<std::pair<size_t, size_t>>(std::begin(expectedEdges), std::end(expectedEdges)));
		}

		TEST_METHOD(DelaunayTriangulationBasicTest2) {
			std::vector<point<int>> pts = { {0, 0}, {0, 10}, {10, 0}, {13, 13} };
			auto extFace = delaunayTriangulation(pts);
			Assert::AreEqual(4, (int)faceToEdgeList(extFace).size());

			std::pair<size_t, size_t> expectedEdges[] = { {0, 1}, {0, 2}, {1, 2}, {1, 3}, {2, 3} };
			std::set<std::pair<size_t, size_t>> actualEdges;

			for (const auto& theEdge : exploreGraph(extFace->outerComponent()->origin())) {
				size_t u = theEdge->origin()->data().label;
				size_t v = theEdge->destination()->data().label;

				if (u < v) {
					actualEdges.insert({ u, v });
				}
			}

			Assert::IsTrue(actualEdges == std::set<std::pair<size_t, size_t>>(std::begin(expectedEdges), std::end(expectedEdges)));
		}

		TEST_METHOD(DelaunayTriangulationBasicTest3) {
			std::vector<point<int>> pts = { {0, 0}, {0, 10}, {10, 0}, {10, 10}, {17, 5} };
			auto extFace = delaunayTriangulation(pts);
			Assert::AreEqual(5, (int)faceToEdgeList(extFace).size());

			std::pair<size_t, size_t> expectedEdges[] = { {0, 1}, {0, 2}, {1, 3}, {2, 3}, {2, 4}, {3, 4} };
			std::set<std::pair<size_t, size_t>> actualEdges;

			for (const auto& theEdge : exploreGraph(extFace->outerComponent()->origin())) {
				size_t u = theEdge->origin()->data().label;
				size_t v = theEdge->destination()->data().label;

				if (u < v) {
					actualEdges.insert({ u, v });
				}
			}

			Assert::IsTrue(actualEdges == std::set<std::pair<size_t, size_t>>(std::begin(expectedEdges), std::end(expectedEdges)));
		}

		TEST_METHOD(DelaunayTriangulationBasicTest4) {
			std::vector<point<int>> pts = { {0, 0}, {0, 10}, {10, 0}, {10, 10} };
			auto extFace = delaunayTriangulation(pts);
			Assert::AreEqual(4, (int)faceToEdgeList(extFace).size());

			std::pair<size_t, size_t> expectedEdges[] = { {0, 1}, {0, 2}, {1, 3}, {2, 3} };
			std::set<std::pair<size_t, size_t>> actualEdges;

			for (const auto& theEdge : exploreGraph(extFace->outerComponent()->origin())) {
				size_t u = theEdge->origin()->data().label;
				size_t v = theEdge->destination()->data().label;

				if (u < v) {
					actualEdges.insert({ u, v });
				}
			}

			Assert::IsTrue(actualEdges == std::set<std::pair<size_t, size_t>>(std::begin(expectedEdges), std::end(expectedEdges)));
		}

		TEST_METHOD(DelaunayTriangulationGridTest) {
			std::vector<point<int>> pts;
			const int gridSize = 10;
			for (int x = 0; x < gridSize; x++) {
				for (int y = 0; y < gridSize; y++) {
					pts.push_back({ x, y });
				}
			}

			auto extFace = delaunayTriangulation(pts);
			auto allEdges = exploreGraph(extFace->outerComponent()->origin());
			Assert::AreEqual(4 * gridSize * (gridSize - 1), (int)allEdges.size());

			for (const auto& theEdge : allEdges) {
				auto theFace = theEdge->incidentFace();
				if (theFace == extFace) {
					Assert::AreEqual(gridSize * 4 - 4, (int)faceToEdgeList(theFace).size());
				}
				else {
					Assert::AreEqual(4, (int)faceToEdgeList(theFace).size());
				}
			}
		}
	};

	TEST_CLASS(VoronoiDiagramTests) {
	public:

		TEST_METHOD(VoronoiDiagramCompilesAndSeemsToWork) {
			std::vector<point<float>> pts = { {0, 0}, {0, 10}, {10, 0}, {10, 10}, {17, 5} };
			auto voronoiDiagram = computeVoronoiDiagram(pts);
			Assert::IsTrue(voronoiDiagram.edgeList.size() == 6);
			Assert::IsTrue(voronoiDiagram.pointList.size() == 7);
		}
	};
}
