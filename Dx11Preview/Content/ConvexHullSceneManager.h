#pragma once

#include "pch.h"

#include "..\..\ConvexHull3D\hull3d.h"
#include "ShaderStructures.h"

namespace Dx11Preview
{
	using input_point = labeled_point<float, size_t>;

	struct ConvexHullScene
	{
		std::vector<VertexPositionColor> sceneVertices;
		std::vector<unsigned short> sceneIndices;
	};

	std::vector<input_point> GenerateRandomPoints(size_t numPoints);
	ConvexHullScene GenerateScene(
		const std::shared_ptr<hullgraph::vertex<input_point>>& hullVertex,
		const std::vector<input_point>& inputPoints
	);

}