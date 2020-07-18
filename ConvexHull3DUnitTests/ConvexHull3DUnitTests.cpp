#include "pch.h"
#include "CppUnitTest.h"
#include "../ConvexHull3D/point.h"
#include "../ConvexHull3D/hullgraph.h"
#include "../ConvexHull3D/hull3d.h"

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
	};

	TEST_CLASS(HullGraphUnitTests) {
	public:

		TEST_METHOD(MakeTriangleCompiles) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			std::shared_ptr<face<point<int>>> f = makeTriangle(pts[0], pts[1], pts[2]);
			Assert::IsTrue(!!f);
		}

		TEST_METHOD(InscribePointCompiles) {
			point<int> pts[4] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14}, {0, 0, 0} };
			std::shared_ptr<face<point<int>>> f = makeTriangle(pts[0], pts[1], pts[2]);
			std::shared_ptr<vertex<point<int>>> newPoint = inscribeVertex(f, pts[3]);
			Assert::IsTrue(!!f);
			Assert::IsTrue(!!newPoint);
		}

		TEST_METHOD(FaceToEdgeListTriangle) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			std::shared_ptr<face<point<int>>> f = makeTriangle(pts[0], pts[1], pts[2]);
			Assert::IsTrue(faceToEdgeList(f).size() == 3);
		}

		TEST_METHOD(InscribePointAppearsToWork) {
			std::shared_ptr<face<int>> f = makeTriangle(0, 1, 2);
			std::shared_ptr<vertex<int>> newPoint = inscribeVertex(f, 3);
			std::vector<std::shared_ptr<edge<int>>> newPointEdges = adjacentEdges(newPoint);
			Assert::AreEqual(3, (int)newPointEdges.size());
		}

		TEST_METHOD(RemoveEdgeCompilesAndAppearsToWork) {
			std::shared_ptr<face<int>> f = makeTriangle(0, 1, 2);
			std::shared_ptr<vertex<int>> newPoint = inscribeVertex(f, 3);
			std::shared_ptr<face<int>> newFace = removeEdge(newPoint->incidentEdge());
			Assert::IsTrue(!!newFace);
			Assert::AreEqual(4, (int)faceToEdgeList(newFace).size());
		}

		TEST_METHOD(JoinFacesCompilesAndAppearsToWork) {
			std::shared_ptr<face<int>> f = makeTriangle(0, 1, 2);
			std::shared_ptr<vertex<int>> newPoint = inscribeVertex(f, 3);
			std::vector<std::shared_ptr<face<int>>> faces = {
				newPoint->incidentEdge()->incidentFace(),
				newPoint->incidentEdge()->twin()->incidentFace()
			};

			join_faces_result<int> result = joinFaces(faces);

			Assert::IsTrue(!!result.newFace);
			Assert::AreEqual(0, (int)result.removedVertices.size());
			Assert::AreEqual(2, (int)result.removedEdges.size());
			Assert::AreEqual(4, (int)faceToEdgeList(result.newFace).size());
			Assert::AreEqual(4, (int)result.borderEdges.size());
			Assert::AreEqual(4, (int)result.borderFaces.size());
		}
	};

	TEST_CLASS(Hull3DUnitTests) {
	public:

		TEST_METHOD(ComputeConvexHull3DCompiles) {
			std::vector<point<int>> pts = { {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1} };
			std::shared_ptr<vertex<point<int>>> hullVertex = computeConvexHull3D(pts);
			Assert::IsTrue(!!hullVertex);
		}

		TEST_METHOD(VerifyConvexHullOrientation) {
			std::vector<point<int>> pts = { {0, 0, 0}, {10, 0, 0}, {0, 10, 0}, {0, 0, 10} };
			std::shared_ptr<vertex<point<int>>> hullVertex = computeConvexHull3D(pts);
			point<int> pt = { 1, 1, 1 };
			
			std::vector<std::shared_ptr<edge<point<int>>>> faceEdges = faceToEdgeList(hullVertex->incidentEdge()->incidentFace());
			std::vector<point<int>> facePoints(faceEdges.size());

			Assert::AreEqual(3, (int)faceEdges.size());

			for (size_t i = 0; i < 3; i++) {
				facePoints[i] = faceEdges[i]->origin()->data();
			}
			
			Assert::IsTrue(orientation(facePoints[0], facePoints[1], facePoints[2], pt) < 0);
		}
	};


}
