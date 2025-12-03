#include "TextureManager.h"
#include <externals/DirectXTex/d3dx12.h>

DirectXCore* TextureManager::core_ = nullptr;
ID3D12Device* TextureManager::device_ = nullptr;
TextureManager* TextureManager::instance_ = nullptr;
uint32_t TextureManager::descriptorIndex_ = 0;

int TextureManager::GetDefaultTextureHandle() { return defaultTextureHandle_; }

void TextureManager::EndUploadingTexture() {
	if (intermediateResource_->GetResourceCounter() != 0)intermediateResource_->ClearResource();
}

TextureManager* TextureManager::GetInstance() {
	if (instance_ == nullptr) {
		instance_ = new TextureManager();
	}
	return instance_;
}

void TextureManager::Initialize(DirectXCore* core) {
	core_ = core;
	device_ = core->GetDriver();
	textureDatas.reserve(config::GetMaxSRVNum());
	descriptorIndex_ = 0;
}

void TextureManager::Finalize() {
	for (auto& ptr : textureDatas) {
		ptr.resource.Reset();
	}
	textureDatas.clear();

	//commonTextureSRVMap_.clear();
	//modelTextureSRVMap_.clear();

	/// 解放の保険
	intermediateResource_->ClearResource();
	delete intermediateResource_;

	delete instance_;
	instance_ = nullptr;
}

DirectX::TexMetadata TextureManager::GetTextureMetadata(int textureHandle) {
	return textureDatas[textureHandle].metadata;
}

#pragma region CommonTexture

int TextureManager::LoadCommonTexture(const std::string& filePath) {
	/// すでに同じテクスチャが読み込まれているか確認
	int textureHandle = CheckSameCommonTextureLoaded(filePath);
	if (textureHandle != -1)return textureHandle;

	/// WICファイルからテクスチャデータを読み込む
	DirectX::ScratchImage image{};
	HRESULT hr = DirectX::LoadFromWICFile(std::wstring(filePath.begin(), filePath.end()).c_str(),
		DirectX::WIC_FLAGS_NONE, nullptr, image);

	/// ミップマップ生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_DEFAULT, 0, mipImages);

	/// 追加したテキスチャテータの参照を取得
	TextureData& textureData = textureDatas.emplace_back();
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource.Attach(CreateTextureResource(textureData.metadata));

	/// シェーダーリソースビュー作成とテクスチャデータのアップロード
	textureHandle = MakeCommonTextureShaderResourceView(&textureData);

	/// テクスチャデータのアップロード
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	intermediateResource.Attach(UploadTextureData(mipImages, &textureData));
	intermediateResource_->SaveResource_(intermediateResource);;

	return textureHandle;
}

int TextureManager::CheckSameCommonTextureLoaded(const std::string& filePath) {
	auto checker1 = std::find_if(textureDatas.begin(),
		textureDatas.end(),
		[&](const TextureData& data) { return data.filePath == filePath; });

	if (checker1 == textureDatas.end()) {
		return -1;
	} else {
		int textureDatasIndex = static_cast<int>(std::distance(textureDatas.begin(), checker1));
		auto checker2 = std::find_if(commonTextureSRVMap_.begin(),
			commonTextureSRVMap_.end(),
			[&](const int& data) { return data == textureDatasIndex; });
		int commonTextureIndex = static_cast<int>(std::distance(commonTextureSRVMap_.begin(), checker2));

		return commonTextureIndex;
	}
}

int TextureManager::MakeCommonTextureShaderResourceView(TextureData* textureData) {
	//
	DirectX::TexMetadata metadata = textureData->metadata;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // シェーダーでのコンポーネントマッピング
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels); // 最初のMipLevelを使用

	//
	textureData->srvHandleCPU = core_->GetCPUDescriptorHandle(core_->GetSrvDescriptorHeap(), core_->GetDesriptorSizeSRV(), descriptorIndex_);
	textureData->srvHandleGPU = core_->GetGPUDescriptorHandle(core_->GetSrvDescriptorHeap(), core_->GetDesriptorSizeSRV(), descriptorIndex_);

	//
	device_->CreateShaderResourceView(
		textureData->resource.Get(),					// Resource
		&srvDesc,									// SRVの設定
		textureData->srvHandleCPU					// CPU用のハンドル
	);

	int counter = (int)textureDatas.size() - 1;
	commonTextureSRVMap_.push_back(counter);
	descriptorIndex_++;

	return (int)commonTextureSRVMap_.size() - 1;
}

#pragma endregion



#pragma region ModelTexture

int TextureManager::LoadModelTexture(const std::string& filePath) {
	/// すでに同じテクスチャが読み込まれているか確認
	int textureHandle = CheckSameModelTextureLoaded(filePath);
	if (textureHandle != -1)return textureHandle;

	/// WICファイルからテクスチャデータを読み込む
	DirectX::ScratchImage image{};
	HRESULT hr = DirectX::LoadFromWICFile(std::wstring(filePath.begin(), filePath.end()).c_str(),
		DirectX::WIC_FLAGS_NONE, nullptr, image);

	/// ミップマップ生成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_DEFAULT, 0, mipImages);

	/// 追加したテキスチャテータの参照を取得
	TextureData& textureData = textureDatas.emplace_back();
	textureData.filePath = filePath;
	textureData.metadata = mipImages.GetMetadata();
	textureData.resource.Attach(CreateTextureResource(textureData.metadata));

	/// シェーダーリソースビュー作成とテクスチャデータのアップロード
	textureHandle = MakeModelTextureShaderResourceView(&textureData);

	///// テクスチャデータのアップロード
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource;
	intermediateResource.Attach(UploadTextureData(mipImages, &textureData));
	intermediateResource_->SaveResource_(intermediateResource);

	return textureHandle;
}




int TextureManager::CheckSameModelTextureLoaded(const std::string& filePath) {
	auto it = std::find_if(textureDatas.begin(),
		textureDatas.end(),
		[&](const TextureData& data) { return data.filePath == filePath; });
	
	if (it == textureDatas.end()) {
		return -1;
	} else {
		int index = static_cast<int>(std::distance(textureDatas.begin(), it));
		return modelTextureSRVMap_[index];
	}
}


int TextureManager::MakeModelTextureShaderResourceView(TextureData* textureData) {
	//
	DirectX::TexMetadata metadata = textureData->metadata;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // シェーダーでのコンポーネントマッピング
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels); // 最初のMipLevelを使用

	//
	textureData->srvHandleCPU = core_->GetCPUDescriptorHandle(core_->GetSrvDescriptorHeap(), core_->GetDesriptorSizeSRV(), descriptorIndex_);
	textureData->srvHandleGPU = core_->GetGPUDescriptorHandle(core_->GetSrvDescriptorHeap(), core_->GetDesriptorSizeSRV(), descriptorIndex_);

	//
	device_->CreateShaderResourceView(
		textureData->resource.Get(),					// Resource
		&srvDesc,									// SRVの設定
		textureData->srvHandleCPU					// CPU用のハンドル
	);

	int counter = (int)textureDatas.size()-1;
	modelTextureSRVMap_.push_back(counter);
	descriptorIndex_++;

	return (int)modelTextureSRVMap_.size() -1;
}
#pragma endregion


ID3D12Resource* TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {

	///  textureの元、時関があればこれをセーブして以降使う<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	/// 1. metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // MipLevelの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行きor配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使ってるのは2次元


	/// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //プロセッサの近くに配置

	/// 3.Resourceを生成する
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties,					// Heapの設定
		D3D12_HEAP_FLAG_NONE,				// Heapの特殊な設定。特になし。
		&resourceDesc,						// Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,		// 初回のResourceState。Textureは基本読むだけ
		nullptr,							// Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));			// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	//char buffer[128];
	//sprintf_s(buffer, "Create resource at %p\n", resource);
	//OutputDebugStringA(buffer);

	return resource;
}

[[nodiscard]]//<---c++属性,戻り値が無視されることを防ぐ
ID3D12Resource* TextureManager::UploadTextureData(const DirectX::ScratchImage& mipImages, TextureData* textureData) {


	DirectX::TexMetadata metadata = textureData->metadata;
	// PrepareUploadを使って、DirectX12用のSubresource配列を作成する
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	// Subresource配列を使って、コピー元のIntermediateResourceに必要のサイズを計算する
	uint64_t intermediateSize = GetRequiredIntermediateSize(textureData->resource.Get(), 0, UINT(subresources.size()));
	// 計算したサイズでIntermediateResourceを作成する
	ID3D12Resource* intermediateResource = CreateResource(device_, intermediateSize);
	// UpdataSubresourcesを使って、IntermediateResourceにSubresource配列を書き込み,textureに転送し頭む積む
	UpdateSubresources(core_->GetCommandList(), textureData->resource.Get(), intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());
	// Tetureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = textureData->resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	core_->GetCommandList()->ResourceBarrier(1, &barrier);
	return intermediateResource;
}
