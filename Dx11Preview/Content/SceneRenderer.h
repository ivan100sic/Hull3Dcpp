#pragma once

#include "../Common/DeviceResources.h"
#include "ShaderStructures.h"

#include <vector>
#include <mutex>

namespace Dx11Preview
{
	struct RenderingScene
	{
		std::vector<VertexPositionColor> sceneVertices;
		std::vector<unsigned short> sceneTriangleIndices;
		std::vector<unsigned short> sceneLineIndices;
	};

	// The base class for classes which can render objects.
	class SceneRenderer
	{
	public:
		SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void CreateDeviceDependentResources();
		void ReleaseDeviceDependentResources();
		void Render();

	protected:
		void RecreateScene(const RenderingScene& scene);

	protected:
		// Cached pointer to device resources.
		std::shared_ptr<DX::DeviceResources> m_deviceResources;
		bool m_ready;

		// Direct3D resources for model geometry.
		Microsoft::WRL::ComPtr<ID3D11InputLayout>	m_inputLayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferTriangles;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_indexBufferLines;
		Microsoft::WRL::ComPtr<ID3D11VertexShader>	m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader>	m_pixelShader;
		Microsoft::WRL::ComPtr<ID3D11Buffer>		m_constantBuffer;

		std::mutex m_dxBuffersMutex;

		// System resources for model geometry.
		ModelViewProjectionConstantBuffer	m_constantBufferData;
		uint32	m_indexCountTriangles;
		uint32	m_indexCountLines;
	};
}