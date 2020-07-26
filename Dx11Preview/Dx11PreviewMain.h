#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\ConvexHullSceneRenderer.h"
#include "Content\SampleFpsTextRenderer.h"
#include "Content\VoronoiDiagramSceneRenderer.h"

// Renders Direct2D and 3D content on the screen.
namespace Dx11Preview
{
	class Dx11PreviewMain : public DX::IDeviceNotify
	{
	public:
		Dx11PreviewMain(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		~Dx11PreviewMain();
		void CreateWindowSizeDependentResources();
		void Update();
		bool Render();
		void SimulationStep();
		void ToggleRenderScene();

		// IDeviceNotify
		virtual void OnDeviceLost();
		virtual void OnDeviceRestored();

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// TODO: Replace with your own content renderers.
		std::unique_ptr<VoronoiDiagramSceneRenderer> m_voronoiDiagramSceneRenderer;
		std::unique_ptr<ConvexHullSceneRenderer> m_convexHullsceneRenderer;
		std::unique_ptr<SampleFpsTextRenderer> m_fpsTextRenderer;
		bool m_renderingConvexHullScene;

		// Rendering loop timer.
		DX::StepTimer m_timer;
	};
}