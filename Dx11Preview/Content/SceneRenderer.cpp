#include "pch.h"
#include "SceneRenderer.h"
#include "..\Common\DirectXHelper.h"

using namespace Dx11Preview;

using namespace DirectX;
using namespace Windows::Foundation;

Dx11Preview::SceneRenderer::SceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_indexCountTriangles(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
}

void Dx11Preview::SceneRenderer::CreateDeviceDependentResources()
{
	// Load shaders asynchronously.
	auto loadVSTask = DX::ReadDataAsync(L"SampleVertexShader.cso");
	auto loadPSTask = DX::ReadDataAsync(L"SamplePixelShader.cso");

	// After the vertex shader file is loaded, create the shader and input layout.
	auto createVSTask = loadVSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateVertexShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_vertexShader
			)
		);

		static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateInputLayout(
				vertexDesc,
				ARRAYSIZE(vertexDesc),
				&fileData[0],
				fileData.size(),
				&m_inputLayout
			)
		);
		});

	// After the pixel shader file is loaded, create the shader and constant buffer.
	auto createPSTask = loadPSTask.then([this](const std::vector<byte>& fileData) {
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreatePixelShader(
				&fileData[0],
				fileData.size(),
				nullptr,
				&m_pixelShader
			)
		);

		CD3D11_BUFFER_DESC constantBufferDesc(sizeof(ModelViewProjectionConstantBuffer), D3D11_BIND_CONSTANT_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&constantBufferDesc,
				nullptr,
				&m_constantBuffer
			)
		);
		});

	// Once both shaders are loaded, set m_loadingComplete so other methods know
	(createPSTask && createVSTask).then([this]() {
		m_loadingComplete = true;
		});
}

void Dx11Preview::SceneRenderer::ReleaseDeviceDependentResources()
{
	m_loadingComplete = false;
	m_vertexShader.Reset();
	m_inputLayout.Reset();
	m_pixelShader.Reset();
	m_constantBuffer.Reset();
	m_vertexBuffer.Reset();
	m_indexBufferTriangles.Reset();
	m_indexBufferLines.Reset();
}

void Dx11Preview::SceneRenderer::Render()
{
	// Loading is asynchronous. Only draw geometry after it's loaded.
	if (!m_loadingComplete)
	{
		return;
	}

	std::unique_lock<std::mutex> lock(m_dxBuffersMutex);

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Attach our vertex shader.
	context->VSSetShader(m_vertexShader.Get(), nullptr, 0);

	// Attach our pixel shader.
	context->PSSetShader(m_pixelShader.Get(), nullptr, 0);

	// Prepare the constant buffer to send it to the graphics device.
	context->UpdateSubresource1(m_constantBuffer.Get(), 0, NULL, &m_constantBufferData, 0, 0, 0);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers1(0, 1, m_constantBuffer.GetAddressOf(), nullptr, nullptr);

	// Each vertex is one instance of the VertexPositionColor struct.
	// Each index is one 16-bit unsigned integer (short).
	UINT stride = sizeof(VertexPositionColor);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetInputLayout(m_inputLayout.Get());

	// First, draw the line segments
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	context->IASetIndexBuffer(m_indexBufferLines.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(m_indexCountLines, 0, 0);

	// Now drawing triangles
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetIndexBuffer(m_indexBufferTriangles.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->DrawIndexed(m_indexCountTriangles, 0, 0);
}

void Dx11Preview::SceneRenderer::RecreateScene(const RenderingScene& scene)
{
	if (!m_loadingComplete)
	{
		return;
	}

	std::unique_lock<std::mutex> lock(m_dxBuffersMutex);

	// Set up m_vertexBuffer
	if (scene.sceneVertices.size())
	{
		UINT vertexBufferSz = static_cast<UINT>(scene.sceneVertices.size() * sizeof(VertexPositionColor));

		D3D11_SUBRESOURCE_DATA vertexBufferData = { 0 };
		vertexBufferData.pSysMem = scene.sceneVertices.data();
		vertexBufferData.SysMemPitch = 0;
		vertexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC vertexBufferDesc(vertexBufferSz, D3D11_BIND_VERTEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&vertexBufferDesc,
				&vertexBufferData,
				&m_vertexBuffer
			)
		);
	}

	// Set up m_indexBufferTriangles
	m_indexCountTriangles = static_cast<unsigned int>(scene.sceneTriangleIndices.size());

	if (m_indexCountTriangles)
	{
		UINT indexBufferSz = static_cast<UINT>(scene.sceneTriangleIndices.size() * sizeof(unsigned short));

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = scene.sceneTriangleIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(indexBufferSz, D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferTriangles
			)
		);
	}
	
	// Set up m_indexBufferLines
	m_indexCountLines = static_cast<unsigned int>(scene.sceneLineIndices.size());

	if (m_indexCountLines)
	{
		UINT indexBufferSz = static_cast<UINT>(scene.sceneLineIndices.size() * sizeof(unsigned short));

		D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
		indexBufferData.pSysMem = scene.sceneLineIndices.data();
		indexBufferData.SysMemPitch = 0;
		indexBufferData.SysMemSlicePitch = 0;
		CD3D11_BUFFER_DESC indexBufferDesc(indexBufferSz, D3D11_BIND_INDEX_BUFFER);
		DX::ThrowIfFailed(
			m_deviceResources->GetD3DDevice()->CreateBuffer(
				&indexBufferDesc,
				&indexBufferData,
				&m_indexBufferLines
			)
		);
	}
}

