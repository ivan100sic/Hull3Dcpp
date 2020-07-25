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
	class ConvexHullSceneRenderer
	{
	public:
		ConvexHullSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void CreateWindowSizeDependentResources();
		void ReleaseDeviceDependentResources();
		void Update(DX::StepTimer const& timer);
		void Render();
		void SimulationStep();

	private:
		void Rotate(float radians);
		void RecreateScene(const ConvexHullScene& scene);

	private:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;

		// Direct3D resources for cube geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferTriangles;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferLines;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		// System resources for cube geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCountTriangles;
		uint32	m_indexCountLines;

		// Variables used with the rendering loop.
		bool	m_loadingComplete;
		bool	m_recreatingScene;
		float	m_degreesPerSecond;

		// The convex hull solver/simulation
		std::unique_ptr<ConvexHullSceneManager> m_sceneManager;
		std::mutex m_dxBuffersMutex;
	};
}

#pragma once
