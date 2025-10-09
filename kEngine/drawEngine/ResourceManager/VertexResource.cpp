#include "VertexResource.h"
#include "Vector4.h"
#include "VertexData.h"
#include "TransformationMatrix.h"


Microsoft::WRL::ComPtr< ID3D12Resource> CreateWVPResource(ID3D12Device* device, size_t sizeInBytes) {
	return CreateResource(device, sizeInBytes);
}

D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(ID3D12Resource* vertexResource, uint32_t NumOfVertex) {
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * NumOfVertex;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	return vertexBufferView;
}

D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(ID3D12Resource* vertexResource, uint32_t NumOfVertex) {
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	/// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	/// 使用するリソースのサイズはインデックス頂点分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * NumOfVertex;
	///　インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	return indexBufferView;
}

VertexResource::~VertexResource() {
	ClearVertexResource();
	ClearWVPResource();
}

Microsoft::WRL::ComPtr<ID3D12Resource> VertexResource::CreateWVPResource_(ID3D12Device* device) {
	return wvpResource_->CreateResourceClass_(device, sizeof(TransformationMatrix));
}

D3D12_VERTEX_BUFFER_VIEW VertexResource::CreateVertexBufferView_(uint32_t NumOfVertex) {
	return vertexBufferView = CreateVertexBufferView(vertexResource_->GetResource().Get(), NumOfVertex);
}

D3D12_INDEX_BUFFER_VIEW VertexResource::CreateIndexBufferView_(uint32_t NumOfIndex) {
	return indexBufferView = CreateIndexBufferView(indexResource_->GetResource().Get(), NumOfIndex);
}

void VertexResource::ClearAllResource() {
	ClearVertexResource();
	ClearIndexResource();
	ClearWVPResource();
}

void VertexResource::ClearVertexResource() {
	if (vertexResource_)vertexResource_->ClearResource();
}

void VertexResource::ClearIndexResource() {
	if (indexResource_)indexResource_->ClearResource();
}


void VertexResource::ClearWVPResource() {
	if (wvpResource_)wvpResource_->ClearResource();
}


