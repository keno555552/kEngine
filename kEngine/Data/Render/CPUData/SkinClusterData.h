#pragma once
#include <span>
#include "Renderer/Resource/BasicResource.h"
#include "Data/Render/CPUData/VertexInfluence.h"
#include "Data/Render/GPUData/WellForGPU.h"
#include "Data/Render/CPUData/ModelData.h"
#include "Data/Animation/Skeleton.h"
#include "DescriptorManager/SrvManager/SrvManager.h"
#include <string>

class SkinClusterData {
public:
	const std::shared_ptr<ModelData>GetModelData() const { return modelData_.lock(); }
	void SetModelData(std::shared_ptr<ModelData> modelData, int modelIndex);

	/// 
	void CreateResourceClass(ID3D12Device* device);
	void SetVertexInfluences();

	void UpdatePalette(const Skeleton& skeleton);

	/// SkinningDataのセッターとゲッター
	Microsoft::WRL::ComPtr<ID3D12Resource> GetSkinClusterResource() { return skinClusterResource_; }
	D3D12_VERTEX_BUFFER_VIEW GetInfluenceBufferView() { return influenceBufferView; }
	std::span<VertexInfluence> GetVertexInfluences() { return mappedInfluences; }

	std::string GetFilePath() { return modelData_.lock()->filePath; }
	std::string GetTexturePatch();

	void SetTextureHandle(int textureHandle) { textureHandle_ = textureHandle; }
	UINT GetTextureHandle() { return textureHandle_; }

	UINT GetVertexNum() { return vertexNum_; }
	void SetVertexNum(int num);


protected:

	/// ModelDataを保存する
	std::weak_ptr<ModelData> modelData_ = {};
	int modelIndex_ = -1;

	/// Joint
	int jointCount_ = 0;
	std::vector<Matrix4x4> inverseBindPoseMatrices;

	/// SkinClusterDataを保存するどころ
	Microsoft::WRL::ComPtr<ID3D12Resource> skinClusterResource_;
	D3D12_VERTEX_BUFFER_VIEW influenceBufferView{};
	std::span<VertexInfluence> mappedInfluences;

	/// MatrixPaletteを保存するどころ
	Microsoft::WRL::ComPtr<ID3D12Resource> paletteInfluences;
	std::span<WellForGPU> mappedPalette{};
	std::pair<D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE> paletteSrvHandles_;

	/// SrvAllocate
	uint32_t paletteSrvIndex_ = -1;

	///　描画用変数
	int textureHandle_ = -1;
	int vertexNum_ = 0;
};

