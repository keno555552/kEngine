#include "BasicResource.h"

ID3D12Resource* CreateResource(ID3D12Device* device, size_t sizeInBytes) {
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う

	// 頂点リソースの設定
	D3D12_RESOURCE_DESC vertexResourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes; // リソースのサイズ。今回はVector4を3頂点分<<<<外へ移送
	// バッファの場合はこれらは1にする決まり
	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	// 実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	//vertexResource_ = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vertexResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexResource)
	); 
	assert(SUCCEEDED(hr));
	vertexResource->SetName(L"BasicResource");

	return vertexResource;
}

BasicResource::~BasicResource() {
	ClearResource();
}

Microsoft::WRL::ComPtr<ID3D12Resource> BasicResource::CreateResourceClass_(ID3D12Device* device, size_t sizeInBytes) {
	Microsoft::WRL::ComPtr<ID3D12Resource> res; 
	res.Attach(CreateResource(device, sizeInBytes));

	resource_.push_back(res);
	resourceCounter++;

	return resource_[resourceCounter - 1];
}

Microsoft::WRL::ComPtr<ID3D12Resource> BasicResource::SaveResource_(ID3D12Resource* resource) {
	Microsoft::WRL::ComPtr<ID3D12Resource> res;
	res.Attach(resource);
	resource_.push_back(res);
	resourceCounter++;
	return resource_[resourceCounter - 1];
}

Microsoft::WRL::ComPtr<ID3D12Resource> BasicResource::GetResource(int Index) {
	if (Index < 0) {
		return resource_[resourceCounter - 1];
	} else {
		return resource_[Index];
	}
	assert(false);
	return nullptr;
}

void BasicResource::ClearResource() {
	resource_.clear();
	resourceCounter = 0;
}
