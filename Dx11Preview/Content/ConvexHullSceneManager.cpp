#include "pch.h"

#include "ConvexHullSceneManager.h"

std::vector<labeled_point<float, size_t>> Dx11Preview::GenerateRandomPoints(size_t numPoints)
{
	std::vector<labeled_point<float, size_t>> result(numPoints);
	static std::mt19937_64 randomEngine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<float> unitSegment(-0.5f, 0.5f);

	for (size_t i = 0; i < numPoints; i++)
	{
		result[i].label = i;
		result[i].x = unitSegment(randomEngine);
		result[i].y = unitSegment(randomEngine);
		result[i].z = unitSegment(randomEngine);
	}

	return result;
}

Dx11Preview::ConvexHullScene Dx11Preview::GenerateScene(
	const std::shared_ptr<hullgraph::vertex<input_point>>& hullVertex,
	const std::vector<input_point>& inputPoints
) {
	using DirectX::XMFLOAT3;
	const float cubeSize = 0.01f;
	XMFLOAT3 cubeColor{ 1.0f, 0.0f, 0.0f };
	XMFLOAT3 edgeColor{ 0.5f, 0.5f, 0.5f };
	
	const unsigned short cubeIdxOffsets[] =
	{
		0, 2, 1,
		1, 2, 3,
		4, 5, 6,
		5, 7, 6,
		0, 1, 5,
		0, 5, 4,
		2, 6, 7,
		2, 7, 3,
		0, 4, 6,
		0, 6, 2,
		1, 3, 7,
		1, 7, 5,
	};

	ConvexHullScene scene;
	
	auto allEdges = exploreGraph(hullVertex);

	// Generate line segments corresponding to convex hull edges
	for (auto& edge : allEdges)
	{
		size_t u = edge->origin()->data().label;
		size_t v = edge->destination()->data().label;

		float ux = inputPoints[u].x;
		float uy = inputPoints[u].y;
		float uz = inputPoints[u].z;

		float vx = inputPoints[v].x;
		float vy = inputPoints[v].y;
		float vz = inputPoints[v].z;

		unsigned short baseIdx = static_cast<unsigned short>(scene.sceneVertices.size());
		scene.sceneVertices.push_back({ XMFLOAT3{ ux, uy, uz }, edgeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ vx, vy, vz }, edgeColor });

		scene.sceneLineIndices.push_back(baseIdx);
		scene.sceneLineIndices.push_back(baseIdx + 1);
	}

	// Generate vertices corresponding to input point cubes
	for (size_t i = 0; i < inputPoints.size(); i++)
	{
		float x = inputPoints[i].x;
		float y = inputPoints[i].y;
		float z = inputPoints[i].z;
		unsigned short baseIdx = static_cast<unsigned short>(scene.sceneVertices.size());
		scene.sceneVertices.push_back({ XMFLOAT3{ x - cubeSize, y - cubeSize, z - cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x - cubeSize, y - cubeSize, z + cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x - cubeSize, y + cubeSize, z - cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x - cubeSize, y + cubeSize, z + cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x + cubeSize, y - cubeSize, z - cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x + cubeSize, y - cubeSize, z + cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x + cubeSize, y + cubeSize, z - cubeSize }, cubeColor });
		scene.sceneVertices.push_back({ XMFLOAT3{ x + cubeSize, y + cubeSize, z + cubeSize }, cubeColor });

		for (size_t j = 0; j < std::size(cubeIdxOffsets); j++)
		{
			scene.sceneTriangleIndices.push_back(cubeIdxOffsets[j] + baseIdx);
		}
	}

	return scene;
}
