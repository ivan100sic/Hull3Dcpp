#include "pch.h"

#include "ConvexHullSceneManager.h"

namespace Dx11Preview {

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

	ConvexHullSceneManager::ConvexHullSceneManager(const std::vector<input_point>& inputPoints) :
		m_inputPoints(inputPoints)
	{
	}

	void ConvexHullSceneManager::SimulationStep()
	{
		m_hullVertex = computeConvexHull3D(m_inputPoints);
	}

	ConvexHullScene ConvexHullSceneManager::GenerateScene()
	{
		if (!m_hullVertex)
		{
			return {};
		}

		using DirectX::XMFLOAT3;
		const float cubeSize = 0.007f;
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

		auto allEdges = exploreGraph(m_hullVertex);

		// Generate line segments corresponding to convex hull edges
		for (auto& edge : allEdges)
		{
			size_t u = edge->origin()->data().label;
			size_t v = edge->destination()->data().label;

			float ux = m_inputPoints[u].x;
			float uy = m_inputPoints[u].y;
			float uz = m_inputPoints[u].z;

			float vx = m_inputPoints[v].x;
			float vy = m_inputPoints[v].y;
			float vz = m_inputPoints[v].z;

			unsigned short baseIdx = static_cast<unsigned short>(scene.sceneVertices.size());
			scene.sceneVertices.push_back({ XMFLOAT3{ ux, uy, uz }, edgeColor });
			scene.sceneVertices.push_back({ XMFLOAT3{ vx, vy, vz }, edgeColor });

			scene.sceneLineIndices.push_back(baseIdx);
			scene.sceneLineIndices.push_back(baseIdx + 1);
		}

		// Generate vertices corresponding to input point cubes
		for (size_t i = 0; i < m_inputPoints.size(); i++)
		{
			float x = m_inputPoints[i].x;
			float y = m_inputPoints[i].y;
			float z = m_inputPoints[i].z;
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
}
