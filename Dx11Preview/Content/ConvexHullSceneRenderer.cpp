#include "pch.h"
#include "ConvexHullSceneRenderer.h"
#include "ConvexHullSceneManager.h"

#include "..\Common\DirectXHelper.h"

using namespace Dx11Preview;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
ConvexHullSceneRenderer::ConvexHullSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_loadingComplete(false),
	m_recreatingScene(false),
	m_degreesPerSecond(30),
	m_indexCountTriangles(0),
	m_deviceResources(deviceResources)
{
	CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}

// Initializes view parameters when the window size changes.
void ConvexHullSceneRenderer::CreateWindowSizeDependentResources()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;
	float fovAngleY = 70.0f * XM_PI / 180.0f;

	// This is a simple example of change that can be made when the app is in
	// portrait or snapped view.
	if (aspectRatio < 1.0f)
	{
		fovAngleY *= 2.0f;
	}

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.

	// This sample makes use of a right-handed coordinate system using row-major matrices.
	XMMATRIX perspectiveMatrix = XMMatrixPerspectiveFovRH(
		fovAngleY,
		aspectRatio,
		0.01f,
		100.0f
	);

	XMFLOAT4X4 orientation = m_deviceResources->GetOrientationTransform3D();

	XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

	XMStoreFloat4x4(
		&m_constantBufferData.projection,
		XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
	);

	static const XMVECTORF32 eye = { 0.0f, 0.5f, 1.2f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.08f, 0.0f, 0.0f };
	static const XMVECTORF32 up = { 0.0f, 1.0f, 0.0f, 0.0f };

	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixTranspose(XMMatrixLookAtRH(eye, at, up)));
}

// Called once per frame, rotates the cube and calculates the model and view matrices.
void ConvexHullSceneRenderer::Update(DX::StepTimer const& timer)
{
	// Convert degrees to radians, then convert seconds to rotation angle
	float radiansPerSecond = XMConvertToRadians(m_degreesPerSecond);
	double totalRotation = timer.GetTotalSeconds() * radiansPerSecond;
	float radians = static_cast<float>(fmod(totalRotation, XM_2PI));

	Rotate(radians);
}

// Rotate the 3D cube model a set amount of radians.
void ConvexHullSceneRenderer::Rotate(float radians)
{
	// Prepare to pass the updated model matrix to the shader
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixTranspose(XMMatrixRotationY(radians)));
}

void Dx11Preview::ConvexHullSceneRenderer::RecreateScene(const ConvexHullScene& scene)
{
	if (!m_loadingComplete)
	{
		return;
	}

	std::unique_lock<std::mutex> lock(m_dxBuffersMutex);

	if (!scene.sceneVertices.size() || !scene.sceneTriangleIndices.size() || !scene.sceneLineIndices.size())
	{
		return;
	}

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

	// Set up m_indexBufferTriangles 
	m_indexCountTriangles = static_cast<unsigned int>(scene.sceneTriangleIndices.size());

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

	// Set up m_indexBufferLines
	m_indexCountLines = static_cast<unsigned int>(scene.sceneLineIndices.size());

	indexBufferSz = static_cast<UINT>(scene.sceneLineIndices.size() * sizeof(unsigned short));

	indexBufferData.pSysMem = scene.sceneLineIndices.data();
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	indexBufferDesc = CD3D11_BUFFER_DESC(indexBufferSz, D3D11_BIND_INDEX_BUFFER);
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(
			&indexBufferDesc,
			&indexBufferData,
			&m_indexBufferLines
		)
	);
}

// Renders one frame using the vertex and pixel shaders.
void ConvexHullSceneRenderer::Render()
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

void Dx11Preview::ConvexHullSceneRenderer::SimulationStep()
{
	if (!m_sceneManager) {
		m_sceneManager = std::make_unique<ConvexHullSceneManager>(GenerateRandomPoints(50));
	}
		
	RecreateScene(m_sceneManager->SimulationStep());
}

void ConvexHullSceneRenderer::CreateDeviceDependentResources()
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

void ConvexHullSceneRenderer::ReleaseDeviceDependentResources()
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
