#pragma once
#include "DirectXCore.h"
#include "BasicResource.h"
#include "externals/DirectXTex/DirectXTex.h"
//#include "WinAPI.h"
#include <wrl.h>
#include <string>
#include <vector>
#include "config.h"

class TextureManager
{
public:
	/// シングルトン取得
	static TextureManager* GetInstance();

	/// 初期化
	void Initialize(DirectXCore* core);
	/// シングルトン解放
	void Finalize();

	/// <summary>
	/// 一般のテクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	/// <returns>テキスチャハンドル</returns>
	int LoadCommonTexture(const std::string& filePath);

	/// <summary>
	/// モデルのテクスチャファイルの読み込み
	/// </summary>
	/// <param name="filePath">テクスチャファイルのパス</param>
	/// <returns>テキスチャハンドル</returns>
	int LoadModelTexture(const std::string& filePath);


	/// <summary>
	/// 
	/// </summary>
	/// <param name="textureHandle"></param>
	/// <returns></returns>
	DirectX::TexMetadata GetTextureMetadata(int textureHandle);

	int GetCommonTextureHandle(int textureHandle) {return commonTextureSRVMap_[textureHandle];}

	int GetModelTextureHandle(int textureHandle) { return modelTextureSRVMap_[textureHandle]; }


	int CheckSameCommonTextureLoaded(const std::string& filePath);

	int CheckSameModelTextureLoaded(const std::string& filePath);

	static int GetDefaultTextureHandle();

	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUDescriptorHandle(int textureHandle) { return textureDatas[textureHandle].srvHandleCPU;}
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUDescriptorHandle(int textureHandle) { return textureDatas[textureHandle].srvHandleGPU;}


	int GetTextureCounter() { return descriptorIndex_; }
	void TextureCounterPlus(int index) { descriptorIndex_ += index; }
	void TextuerCounterAdjust(int index) { descriptorIndex_ = index;};

	void EndUploadingTexture();

private:
	/// テクスチャ1枚分のデータ
	struct TextureData {
		std::string filePath{};
		DirectX::TexMetadata metadata{};
		Microsoft::WRL::ComPtr<ID3D12Resource> resource;
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandleCPU{};
		D3D12_GPU_DESCRIPTOR_HANDLE srvHandleGPU{};
	};
	std::vector<TextureData>textureDatas;

	BasicResource* intermediateResource_ = new BasicResource;/*EndDrawでTextrueを作ったら解放する*/

private:
	/// 借りのDevice
	static DirectXCore* core_;
	static ID3D12Device* device_;

	/// シングルトンインスタンス
	static TextureManager* instance_;

	TextureManager() = default;
	~TextureManager() = default;
	TextureManager(TextureManager&) = delete;
	TextureManager& operator= (TextureManager&) = delete;

	static uint32_t descriptorIndex_;
	std::vector<int> commonTextureSRVMap_;
	std::vector<int> modelTextureSRVMap_;
	static const int defaultTextureHandle_ = 0;

private:
	ID3D12Resource* CreateTextureResource(const DirectX::TexMetadata& metadata);

	int MakeCommonTextureShaderResourceView(TextureData* textureData);

	int MakeModelTextureShaderResourceView(TextureData* textureData);

	ID3D12Resource* UploadTextureData(const DirectX::ScratchImage& mipImages, TextureData* textureData);

};

