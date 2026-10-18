#pragma once
#include "TransformationMatrix.h"
#include "Mesh/VertexResource.h"
#include "Data/Render/GPUData/MaterialForGPU.h"
#include "Renderer/PSOManager/PSOKeys.h"
using MaterialID = int;
using TextureHandle = int;

struct RenderData
{
    /// バケツ分け用
    MeshBuffer* mesh = nullptr;
    TextureHandle textureHandle = -1;
    PSOKey psoKey{};

    ///　実際データ
	int skinningPaletteIndex = -1;
	MaterialID materialID = -1;
    TransformationMatrix transformData {};

    /// 将来使うかも
    uint32_t subMeshIndex = 0;

	bool CheckSame(const RenderData target);
};

