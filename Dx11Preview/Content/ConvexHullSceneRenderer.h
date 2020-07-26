#pragma once

#include "..\Common\DeviceResources.h"
#include "ShaderStructures.h"
#include "ConvexHullSceneManager.h"
#include "..\Common\StepTimer.h"

#include "..\..\ConvexHull3D/hull3d.h"
#include <mutex>

namespace Dx11Preview
{
	// This sample renderer instantiates a basic rendering pipeline.
	class ConvexHullSceneRenderer : public SceneRenderer
	{
	public:
		ConvexHullSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update(DX::StepTimer const& timer);
		void SimulationStep();
		void UpdateViewport();

	private:
		void Rotate(float radians);

	private:
		float m_degreesPerSecond;

		// The convex hull solver/simulation
		std::unique_ptr<ConvexHullSceneManager> m_sceneManager;
	};
}

#pragma once
