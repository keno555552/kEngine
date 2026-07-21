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
    MeshBuffer* mesh;
    TextureHandle textureHandle;
    PSOKey psoKey;

    ///　実際データ
	MaterialID materialID;
    TransformationMatrix transformData;

    /// 将来使うかも
    uint32_t subMeshIndex;

	bool CheckSame(const RenderData target);
};

