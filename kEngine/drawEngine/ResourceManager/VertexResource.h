#pragma once
#include "BasicResource.h"
#include "DirectionalLight.h"

D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView(ID3D12Resource* vertexResource, uint32_t NumOfVertex);
D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(ID3D12Resource* vertexResource, uint32_t NumOfVertex);

class VertexResource
{
public:
	~VertexResource();

	/// 頂点リソースの作成と解放
	virtual ID3D12Resource* CreateVertexResource_(ID3D12Device* device) { return 0; };
	virtual ID3D12Resource* CreateIndexResource_(ID3D12Device* device) { return 0; }
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateWVPResource_(ID3D12Device* device);
	void ClearAllResource();
	void ClearVertexResource();
	void ClearIndexResource();
	void ClearWVPResource();

	/// VertexBufferViewを作成する
	D3D12_VERTEX_BUFFER_VIEW CreateVertexBufferView_(uint32_t NumOfVertex);
	D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView_(uint32_t NumOfIndex);

	/// Resourceを取得する
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexResource() { return vertexResource_->GetResource(); }
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexResource_() { return indexResource_->GetResource(); };
	Microsoft::WRL::ComPtr<ID3D12Resource> GetWVPResource_() { return wvpResource_->GetResource(); };
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() { return vertexBufferView; }
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() { return indexBufferView; }

protected:

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView{};
	BasicResource* vertexResource_ = new BasicResource;
	BasicResource* indexResource_ = new BasicResource;
	BasicResource* wvpResource_ = new BasicResource;
};


/// 将来のVertexResourceは図形ことで生成されるべき
/// 独立できリソースの頂点を管理するクラスであるべき