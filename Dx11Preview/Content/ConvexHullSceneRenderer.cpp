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

void Dx11Preview::ConvexHullSceneRenderer::SimulationStep()
{
	if (!m_sceneManager) {
		m_sceneManager = std::make_unique<ConvexHullSceneManager>(GenerateCubicLattice(5));
	}
		
	RecreateScene(m_sceneManager->SimulationStep());
}
