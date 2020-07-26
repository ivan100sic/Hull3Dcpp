#pragma once

#include "SceneRenderer.h"

namespace Dx11Preview
{
	// This sample renderer instantiates a basic rendering pipeline.
	class VoronoiDiagramSceneRenderer : public SceneRenderer
	{
	public:
		VoronoiDiagramSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void UpdateViewport();
		void InitializeScene();

	private:
	};
}

#pragma once
