#pragma once
#include <cassert>

#include <string>
#include "Data/Render/CPUData/VertexData.h"
#include "VertexResource.h"

#include "Data/Render/CPUData/ModelData.h"
#include "Data/Render/CPUData/SkinClusterData.h"
#include "Data/Render/GPUData/SkinningExtraVertexData.h"

class Model :public MeshBuffer
{
public:
	const std::shared_ptr<ModelData>GetModelData() const { return modelData_.lock(); }
	MeshData& GetMeshData() { return modelData_.lock()->meshDataList[meshIndex_]; }
	void SetModelData(std::shared_ptr<ModelData> modelData, uint32_t meshIndex);
	ID3D12Resource* CreateVertexResource_(ID3D12Device* device)override;
	ID3D12Resource* CreateSkinningVertexResource(ID3D12Device* device);
	ID3D12Resource* CreateIndexResource_(ID3D12Device* device)override;
	//ID3D12Resource* CreateVertexResourceG_(ID3D12Device* device);
	std::string GetTexturePatch();
	void SetTextureHandle(int textureHandle) { textureHandle_ = textureHandle; }
	UINT GetTextureHandle() { return textureHandle_; }
	UINT GetVertexNum() { return VertexNum_; }
	void SetVertexNum(int num);


private:
	/// ModelDataを保存する
	std::weak_ptr<ModelData> modelData_ = {};
	uint32_t meshIndex_ = -1;

	/// ResourceManagerからもらうハンドルを保存する
	int textureHandle_ = -1;
	int modelResourceHandle_ = -1;
	int VertexNum_ = 0;
};


