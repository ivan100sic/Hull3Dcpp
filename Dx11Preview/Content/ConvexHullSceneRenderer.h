#pragma once

#include "../../ConvexHull3D/convexhull3d.h"
#include "../Common/DeviceResources.h"
#include "../Common/StepTimer.h"
#include "ShaderStructures.h"
#include "ConvexHullSceneManager.h"

namespace Dx11Preview
{
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
