#include "pch.h"
#include "CppUnitTest.h"
#include "../ConvexHull3D/point.h"
#include "../ConvexHull3D/hullgraph.h"

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

		TEST_METHOD(MakeTriangle) {
			point<int> pts[3] = { {0, 1, 2}, {4, 7, 10}, {6, 10, 14} };
			std::shared_ptr<face<point<int>>> f = makeTriangle(pts[0], pts[1], pts[2]);
			Assert::IsTrue(!!f);
		}
	};
}
