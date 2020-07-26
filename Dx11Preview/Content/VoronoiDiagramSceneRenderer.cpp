#include "pch.h"

#include "../../ConvexHull3D/voronoi.h"
#include "VoronoiDiagramSceneRenderer.h"
#include "ConvexHullSceneManager.h"

using namespace Dx11Preview;
using namespace DirectX;
using namespace Windows::Foundation;

VoronoiDiagramSceneRenderer::VoronoiDiagramSceneRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources) :
	SceneRenderer(deviceResources)
{
	UpdateViewport();
}

void VoronoiDiagramSceneRenderer::UpdateViewport()
{
	Size outputSize = m_deviceResources->GetOutputSize();
	float aspectRatio = outputSize.Width / outputSize.Height;

	XMMATRIX projectionMatrix = XMMatrixOrthographicRH(2.0f * aspectRatio, 2.0, -1.0f, 1.0f);
	
	XMStoreFloat4x4(&m_constantBufferData.model, XMMatrixIdentity());
	XMStoreFloat4x4(&m_constantBufferData.projection, projectionMatrix);
	XMStoreFloat4x4(&m_constantBufferData.view, XMMatrixIdentity());
}

void VoronoiDiagramSceneRenderer::InitializeScene()
{
	auto inputPoints = GenerateRandomPoints(20);
	auto outerFace = delaunayTriangulation(inputPoints);
	auto voronoiDiagram = computeVoronoiDiagram(outerFace);
	const float pointRadius = 0.005f;
	const XMFLOAT3 pointColor{ 1.0f, 0.0f, 0.0f };
	const XMFLOAT3 voronoiEdgeColor{ 0.0f, 0.5f, 1.0f };
	const XMFLOAT3 triangulationEdgeColor{ 0.2f, 0.5f, 0.0f };

	const size_t pointOffsets[6] = { 0, 1, 2, 1, 0, 3 };

	RenderingScene scene;

	for (size_t i = 0; i < inputPoints.size(); i++)
	{
		size_t baseIdx = scene.sceneVertices.size();
		scene.sceneVertices.push_back({ XMFLOAT3{inputPoints[i].x - pointRadius, inputPoints[i].y, 0.0f}, pointColor });
		scene.sceneVertices.push_back({ XMFLOAT3{inputPoints[i].x + pointRadius, inputPoints[i].y, 0.0f}, pointColor });
		scene.sceneVertices.push_back({ XMFLOAT3{inputPoints[i].x, inputPoints[i].y - pointRadius, 0.0f}, pointColor });
		scene.sceneVertices.push_back({ XMFLOAT3{inputPoints[i].x, inputPoints[i].y + pointRadius, 0.0f}, pointColor });

		for (size_t j = 0; j < std::size(pointOffsets); j++)
		{
			scene.sceneTriangleIndices.push_back(static_cast<unsigned short>(baseIdx + pointOffsets[j]));
		}

		// This will be needed for triangulation
		scene.sceneVertices.push_back({ XMFLOAT3{inputPoints[i].x, inputPoints[i].y, 0.0f}, triangulationEdgeColor });
	}

	// Resolve points at infinity
	for (const auto& theEdge : voronoiDiagram.edgeList)
	{
		size_t u = theEdge.u;
		size_t v = theEdge.v;

		if (voronoiDiagram.pointList[v].atInfinity)
		{
			std::swap(u, v);
		}

		if (voronoiDiagram.pointList[u].atInfinity)
		{
			float ux = voronoiDiagram.pointList[u].x;
			float uy = voronoiDiagram.pointList[u].y;

			float uNorm = sqrt(ux * ux + uy * uy);
			ux /= uNorm;
			uy /= uNorm;

			voronoiDiagram.pointList[u].x = voronoiDiagram.pointList[v].x + 6.0f * ux;
			voronoiDiagram.pointList[u].y = voronoiDiagram.pointList[v].y + 6.0f * uy;
		}
	}

	// Add triangulation edges to the scene
	for (const auto& theEdge : exploreGraph(outerFace->outerComponent()->origin()))
	{
		size_t u = theEdge->origin()->data().label;
		size_t v = theEdge->destination()->data().label;
		scene.sceneLineIndices.push_back(static_cast<unsigned short>(5 * u + 4));
		scene.sceneLineIndices.push_back(static_cast<unsigned short>(5 * v + 4));
	}

	// Add Voronoi edges to the scene
	{
		size_t edgeBaseIdx = scene.sceneVertices.size();

		for (const auto& pt : voronoiDiagram.pointList)
		{
			scene.sceneVertices.push_back({ XMFLOAT3{pt.x, pt.y, 0.0f}, voronoiEdgeColor });
		}

		for (const auto& theEdge : voronoiDiagram.edgeList)
		{
			scene.sceneLineIndices.push_back(static_cast<unsigned short>(theEdge.u + edgeBaseIdx));
			scene.sceneLineIndices.push_back(static_cast<unsigned short>(theEdge.v + edgeBaseIdx));
		}
	}

	RecreateScene(scene);
}
