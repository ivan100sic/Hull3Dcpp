#include "pch.h"

#include "ConvexHullSceneManager.h"

namespace Dx11Preview {

	std::vector<input_point> GenerateRandomPoints(size_t numPoints)
	{
		std::vector<input_point> result(numPoints);
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

	std::vector<input_point> GenerateCubicLattice(size_t latticeSize)
	{
		std::vector<input_point> result;
		for (size_t i = 0; i < latticeSize; i++) {
			for (size_t j = 0; j < latticeSize; j++) {
				for (size_t k = 0; k < latticeSize; k++) {
					float x = i / (latticeSize - 1.0f) - 0.5f;
					float y = j / (latticeSize - 1.0f) - 0.5f;
					float z = k / (latticeSize - 1.0f) - 0.5f;

					input_point point;
					point.label = result.size();
					point.x = x;
					point.y = y;
					point.z = z;

					result.push_back(point);
				}
			}
		}

		return result;
	}

	ConvexHullSceneManager::ConvexHullSceneManager(const std::vector<input_point>& inputPoints) :
		m_inputPoints(inputPoints)
	{
	}

	RenderingScene ConvexHullSceneManager::SimulationStep()
	{
		std::unique_lock<std::mutex> lock(m_dataMutex);

		if (!m_computeThread.joinable())
		{
			auto computeCallback = [this](auto updateType, const auto& newVertex) {
				if (updateType != convex_hull_update::afterRemoveRedundantVertices &&
					updateType != convex_hull_update::initialTetrahedron)
				{
					return;
				}

				std::unique_lock<std::mutex> lock(m_dataMutex);

				m_hullVertex = newVertex;

				// Only continue when SimulationStep is called again
				m_canResumeCv.wait(lock, [this]() { return m_canResumeFlag; });
				m_canResumeFlag = false;
			};

			m_computeThread = std::thread([this, computeCallback]() {
				computeConvexHull3D(m_inputPoints, computeCallback);
				});

			return {};
		}
		else
		{
			auto scene = GenerateScene();
			m_canResumeFlag = true;
			m_canResumeCv.notify_all();
			return scene;
		}
	}

	RenderingScene ConvexHullSceneManager::GenerateScene()
	{
		if (!m_hullVertex)
		{
			return {};
		}

		using DirectX::XMFLOAT3;
		const float cubeSize = 0.007f;
		XMFLOAT3 cubeColor{ 1.0f, 0.0f, 0.0f };
		XMFLOAT3 edgeColor{ 1.0f, 1.0f, 1.0f };
		XMFLOAT3 newEdgeColor{ 0.0f, 0.6f, 1.0f };

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

		RenderingScene scene;

		auto addEdgeToScene = [&](const auto& edge, const auto& color) {
			size_t u = edge->origin()->data().label;
			size_t v = edge->destination()->data().label;

			float ux = m_inputPoints[u].x;
			float uy = m_inputPoints[u].y;
			float uz = m_inputPoints[u].z;

			float vx = m_inputPoints[v].x;
			float vy = m_inputPoints[v].y;
			float vz = m_inputPoints[v].z;

			auto baseIdx = (unsigned short)scene.sceneVertices.size();
			scene.sceneVertices.push_back({ XMFLOAT3{ ux, uy, uz }, color });
			scene.sceneVertices.push_back({ XMFLOAT3{ vx, vy, vz }, color });
			scene.sceneLineIndices.push_back(baseIdx);
			scene.sceneLineIndices.push_back(baseIdx + 1);
		};

		auto allEdges = exploreGraph(m_hullVertex);

		// Generate line segments corresponding to convex hull edges
		for (const auto& edge : allEdges)
		{
			auto thisEdgeColor = m_previousStepEdges.count(edge) ? edgeColor : newEdgeColor;
			addEdgeToScene(edge, thisEdgeColor);
		}

		// Generate vertices corresponding to input point cubes
		for (size_t i = 0; i < m_inputPoints.size(); i++)
		{
			float x = m_inputPoints[i].x;
			float y = m_inputPoints[i].y;
			float z = m_inputPoints[i].z;
			auto baseIdx = (unsigned short)scene.sceneVertices.size();
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

		m_previousStepEdges = decltype(m_previousStepEdges)(allEdges.begin(), allEdges.end());

		return scene;
	}
}
