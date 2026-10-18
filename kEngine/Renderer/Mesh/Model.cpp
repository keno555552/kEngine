#include "Model.h"
#include "filesystem"

void Model::SetModelData(std::shared_ptr<ModelData> modelData,uint32_t meshIndex) {
	modelData_ = modelData;
	meshIndex_ = meshIndex;
}

ID3D12Resource* Model::CreateVertexResource_(ID3D12Device* device) {

	MeshData& modelData = modelData_.lock()->meshDataList[meshIndex_];

	vertexResource_->CreateResourceClass_(device, sizeof(VertexData) * modelData.vertices.size());
	CreateVertexBufferView_(int(modelData.vertices.size()));
	VertexNum_ = int(modelData.vertices.size());

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size());
	vertexResource_->GetResource()->Unmap(0, nullptr);

	// スキニングデータがある場合、SkinClusterDataを準備する
	if (modelData_.lock()->haveSkinning) {
		int skinIndex = modelData.skinIndex;
		skinClusterData_.SetModelData(modelData_.lock(), meshIndex_);
		skinClusterData_.CreateResourceClass(device);
		skinClusterData_.SetVertexInfluences();
	}

	vertexResource_->SetName("ModelVertexResource:" + modelData.name);

	return vertexResource_->GetResource().Get();
}

ID3D12Resource* Model::CreateSkinningVertexResource(ID3D12Device* device) {

	MeshData& modelData = modelData_.lock()->meshDataList[meshIndex_];
	const size_t vertexCount = modelData.vertices.size();
	const size_t sizeOfSkinningExtraVertexData = sizeof(SkinningExtraVertexData) * vertexCount;

	skinningVertexResource_->CreateResourceClass_(device, sizeOfSkinningExtraVertexData);

	// 填入 weight / index
	SkinningExtraVertexData* skinningData = nullptr;
	skinningVertexResource_->GetResource()->Map(0, nullptr,
		reinterpret_cast<void**>(&skinningData));

	//for (size_t i = 0; i < vertexCount; ++i) {
	//	// 權重（假設你有 modelData.vertexWeights）
	//	skinningData[i].weights = modelData.vertexWeights[i];
	//
	//	// 骨骼索引（假設你有 modelData.vertexJoints[i][0..3]）
	//	for (int j = 0; j < 4; ++j) {
	//		skinningData[i].index[j] = modelData.vertexJoints[i][j];
	//	}
	//}
	skinningVertexResource_->GetResource()->Unmap(0, nullptr);

	// スキニングデータがある場合、SkinClusterDataを準備する
	if (modelData_.lock()->haveSkinning) {
		int skinIndex = modelData.skinIndex;
		skinClusterData_.SetModelData(modelData_.lock(), meshIndex_);
		skinClusterData_.CreateResourceClass(device);
		skinClusterData_.SetVertexInfluences();
	}

	skinningVertexResource_->SetName("SkinningVertexResource:" + modelData.name);

	return skinningVertexResource_->GetResource().Get();

}

ID3D12Resource* Model::CreateIndexResource_(ID3D12Device* device) {

	MeshData& modelData = modelData_.lock()->meshDataList[meshIndex_];

	indexResource_->CreateResourceClass_(device, sizeof(uint32_t) * modelData.indices.size());
	CreateIndexBufferView_(int(modelData.indices.size()));
	indexNum_ = int(modelData.indices.size());

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, modelData.indices.data(), sizeof(uint32_t) * modelData.indices.size());
	indexResource_->GetResource()->Unmap(0, nullptr);

	indexResource_->SetName("ModelIndexResource:" + modelData.name);

	return indexResource_->GetResource().Get();
}

std::string Model::GetTexturePatch() {
	if (modelData_.expired())return std::string();
	int textureHandle = modelData_.lock()->meshDataList[meshIndex_].materialIndex;

	return modelData_.lock()->materialList[textureHandle].textureFilePath;
}

void Model::SetVertexNum(int num) {
	VertexNum_ = num;
}



