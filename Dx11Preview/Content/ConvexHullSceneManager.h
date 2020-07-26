#pragma once

#include "../../ConvexHull3D/convexhull3d.h"
#include "SceneRenderer.h"
#include "ShaderStructures.h"

#include <thread>
#include <condition_variable>

namespace Dx11Preview
{
	using input_point = labeled_point<float, size_t>;

	class ConvexHullSceneManager
	{
		std::vector<input_point> m_inputPoints;
		std::shared_ptr<hullgraph::vertex<input_point>> m_hullVertex;
		std::thread m_computeThread;
		std::unordered_set<std::shared_ptr<hullgraph::edge<input_point>>> m_previousStepEdges;

		bool m_canResumeFlag;
		std::mutex m_dataMutex;
		std::condition_variable m_canResumeCv;
		RenderingScene GenerateScene();
	public:
		ConvexHullSceneManager(const std::vector<input_point>& inputPoints);
		RenderingScene SimulationStep();
	};

	std::vector<input_point> GenerateRandomPoints(size_t numPoints);
	std::vector<input_point> GenerateCubicLattice(size_t latticeSize);
}
