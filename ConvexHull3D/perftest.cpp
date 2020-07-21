#include "hull3d.h"

using namespace std;
using namespace hullgraph;

int main() {
	const int numPoints = 100000;
	const double pi = acos(double(-1));

	vector<point<double>> pts(numPoints);
	mt19937_64 randomEngine;
	uniform_real_distribution<double> angleGen(0, pi);

	for (int i = 0; i < numPoints; i++) {
		double phi = 2 * angleGen(randomEngine);
		double theta = angleGen(randomEngine) - pi / 2;
		double x = cos(phi) * cos(theta);
		double y = sin(phi) * cos(theta);
		double z = sin(theta);

		pts[i] = { x, y, z };
	}

	shared_ptr<vertex<point<double>>> hullVertex = computeConvexHull3D(pts);
	vector<shared_ptr<edge<point<double>>>> allEdges = exploreGraph(hullVertex);
}