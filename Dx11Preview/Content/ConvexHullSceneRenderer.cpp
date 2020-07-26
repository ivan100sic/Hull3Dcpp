#include "pch.h"
#include "ConvexHullSceneRenderer.h"
#include "ConvexHullSceneManager.h"

#include "..\Common\DirectXHelper.h"

using namespace Dx11Preview;

using namespace DirectX;
using namespace Windows::Foundation;

// Loads vertex and pixel shaders from files and instantiates the cube geometry.
ConvexHullSceneRenderer::ConvexHullSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	SceneRenderer(deviceResources),
	m_degreesPerSecond(30)
{
	UpdateViewport();
}

void ConvexHullSceneRenderer::UpdateViewport()
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

	static const XMVECTORF32 eye = { 0.0f, 0.55f, 1.2f, 0.0f };
	static const XMVECTORF32 at = { 0.0f, -0.15f, 0.0f, 0.0f };
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

void ConvexHullSceneRenderer::SimulationStep()
{
	if (!m_sceneManager) {
		m_sceneManager = std::make_unique<ConvexHullSceneManager>(GenerateCubicLattice(5));
	}
		
	RecreateScene(m_sceneManager->SimulationStep());
}
