#pragma once

#include "pch.h"

#include "..\..\ConvexHull3D\hull3d.h"
#include "ShaderStructures.h"
#include <thread>
#include <condition_variable>

namespace Dx11Preview
{
	using input_point = labeled_point<float, size_t>;

	struct ConvexHullScene
	{
		std::vector<VertexPositionColor> sceneVertices;
		std::vector<unsigned short> sceneTriangleIndices;
		std::vector<unsigned short> sceneLineIndices;
	};

	class ConvexHullSceneManager
	{
		std::vector<input_point> m_inputPoints;
		std::shared_ptr<hullgraph::vertex<input_point>> m_hullVertex;
		std::thread m_computeThread;
		std::unordered_set<std::shared_ptr<hullgraph::edge<input_point>>> m_previousStepEdges;
		
		bool m_canResumeFlag;
		std::mutex m_dataMutex;
		std::condition_variable m_canResumeCv;
		ConvexHullScene GenerateScene();
	public:
		ConvexHullSceneManager(const std::vector<input_point>& inputPoints);
		ConvexHullScene SimulationStep();
	};

	std::vector<input_point> GenerateRandomPoints(size_t numPoints);
}
