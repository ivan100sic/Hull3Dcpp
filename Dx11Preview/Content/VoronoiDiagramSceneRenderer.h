#pragma once

#include "SceneRenderer.h"

namespace Dx11Preview
{
	class VoronoiDiagramSceneRenderer : public SceneRenderer
	{
	public:
		VoronoiDiagramSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void UpdateViewport();
		void InitializeScene();

	private:
	};
}
