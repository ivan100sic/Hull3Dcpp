﻿#include "pch.h"
#include "Dx11PreviewMain.h"
#include "Common\DirectXHelper.h"

using namespace Dx11Preview;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

// Loads and initializes application assets when the application is loaded.
Dx11PreviewMain::Dx11PreviewMain(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	m_deviceResources(deviceResources)
{
	// Register to be notified if the Device is lost or recreated
	m_deviceResources->RegisterDeviceNotify(this);

	m_convexHullsceneRenderer = std::make_unique<ConvexHullSceneRenderer>(m_deviceResources);
	m_voronoiDiagramSceneRenderer = std::make_unique<VoronoiDiagramSceneRenderer>(m_deviceResources);
	m_renderingConvexHullScene = true;

	m_fpsTextRenderer = std::unique_ptr<SampleFpsTextRenderer>(new SampleFpsTextRenderer(m_deviceResources));

	// TODO: Change the timer settings if you want something other than the default variable timestep mode.
	// e.g. for 60 FPS fixed timestep update logic, call:
	/*
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.0 / 60);
	*/
}

Dx11PreviewMain::~Dx11PreviewMain()
{
	// Deregister device notification
	m_deviceResources->RegisterDeviceNotify(nullptr);
}

// Updates application state when the window size changes (e.g. device orientation change)
void Dx11PreviewMain::CreateWindowSizeDependentResources() 
{
	// TODO: Replace this with the size-dependent initialization of your app's content.
	m_convexHullsceneRenderer->UpdateViewport();
	m_voronoiDiagramSceneRenderer->UpdateViewport();
}

// Updates the application state once per frame.
void Dx11PreviewMain::Update() 
{
	// Update scene objects.
	m_timer.Tick([&]()
	{
		// TODO: Replace this with your app's content update functions.
		m_convexHullsceneRenderer->Update(m_timer);
		m_fpsTextRenderer->Update(m_timer);
	});
}

// Renders the current frame according to the current application state.
// Returns true if the frame was rendered and is ready to be displayed.
bool Dx11PreviewMain::Render() 
{
	// Don't try to render anything before the first Update.
	if (m_timer.GetFrameCount() == 0)
	{
		return false;
	}

	auto context = m_deviceResources->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = m_deviceResources->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { m_deviceResources->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, m_deviceResources->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(m_deviceResources->GetBackBufferRenderTargetView(), DirectX::Colors::Black);
	context->ClearDepthStencilView(m_deviceResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	if (m_renderingConvexHullScene)
	{
		m_convexHullsceneRenderer->Render();
	}
	else
	{
		m_voronoiDiagramSceneRenderer->Render();
	}

	m_fpsTextRenderer->Render();

	return true;
}

void Dx11Preview::Dx11PreviewMain::SimulationStep()
{
	if (m_renderingConvexHullScene)
	{
		if (m_convexHullsceneRenderer)
		{
			m_convexHullsceneRenderer->SimulationStep();
		}
	}
	else
	{
		if (m_voronoiDiagramSceneRenderer)
		{
			m_voronoiDiagramSceneRenderer->InitializeScene();
		}
	}
}

void Dx11PreviewMain::ToggleRenderScene()
{
	m_renderingConvexHullScene ^= true;
}

// Notifies renderers that device resources need to be released.
void Dx11PreviewMain::OnDeviceLost()
{
	m_convexHullsceneRenderer->ReleaseDeviceDependentResources();
	m_voronoiDiagramSceneRenderer->ReleaseDeviceDependentResources();
	m_fpsTextRenderer->ReleaseDeviceDependentResources();
}

// Notifies renderers that device resources may now be recreated.
void Dx11PreviewMain::OnDeviceRestored()
{
	m_convexHullsceneRenderer->CreateDeviceDependentResources();
	m_voronoiDiagramSceneRenderer->CreateDeviceDependentResources();
	m_fpsTextRenderer->CreateDeviceDependentResources();
	CreateWindowSizeDependentResources();
}
