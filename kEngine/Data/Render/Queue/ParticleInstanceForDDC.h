#pragma once
#include "TransformationMatrix.h"
#include "Mesh/VertexResource.h"
#include "Renderer/PSOManager/PSOKeys.h"
using MaterialID = int;
using TextureHandle = int;

struct ParticleInstanceForDDC
{
	MeshBuffer* mesh{};
	PSOKey psoKey{};
	MaterialID materialID{};
	TextureHandle textureHandle{};
	TransformationMatrix transMatrix{};
};