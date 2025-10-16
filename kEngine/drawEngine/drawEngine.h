#pragma once
#include "DircetXBase.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include "shader_compile.h"
#include "PSO.h"
#include "ResourceManager.h"
#include "Vector4.h"
#include "VertexData.h"
#include "Material.h"
#include "DirectionalLight.h"

#include "Matrix4x4.h"
#include "TransformationMatrix.h"
#include "ConvertString.h"
#include "LightModelType.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"

#include <format>

class DrawEngine
{
public:
	~DrawEngine();

	void Initialize(const char* kClientTitle, int kClientWidth, int kClientHeight, DirectXBase* directXDirver);

	void PreDraw();
	void CommitDraw();
	void EndDraw();

	void SetDirectionalLight(DirectionalLight* light);

	/// 三角形関連
	void DrawTriangle(TransformationMatrix* wvpData, MaterialConfig material);


	/// 2D図形関連
	void CollectSprite(Vector2 pos, MaterialConfig material);
	void DrawSprite();
	void DrawSpriteDirect(Vector2 pos, MaterialConfig material);
	void DrawSpriteDirect(Vector2 pos, MaterialConfig material, Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos);

	/// Tile関連
	void Collect2DTile(Vector2 pos, MaterialConfig material);
	void Draw2DTile();

	/// 立方体関連
	void DrawCube(TransformationMatrix* wvpData, MaterialConfig material);
	/// 球体関連
	void DrawSphere(TransformationMatrix* wvpData, MaterialConfig material, int sudivision);
	/// モデル関連
	void DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle);
	void DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material);

	/// Tile関連
	void Collect3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material);
	void Draw3DTile();
	bool SetModelTexture(Model* model);
	bool SetModelGroupTexture(Model* model);
	int SetModel(std::string Path);
	int readCommonTextureHandle(int Handle);
	int readModelTextureHandle(int Handle);
	int GetMuitModelNum(int modelHandle);

	int LoadTexture(const std::string& filePath);
	int LoadModelTexture(const std::string& filePath);

private:
	Shader_compile* shader_compile_ = new Shader_compile;
	PSO* pso_ = new PSO;
	ResourceManager* resourceManager_ = nullptr;
	DirectXBase* directXDriver_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;

	int kClientWidth_ = 0;
	int kClientHeight_ = 0;

	int kMaxSudivision_ = 18;
	int kSudivision_ = 0;

private:
	enum class psoType {
		NONE = -1,
		defaultPSO = 0,
		Normal_Lambert = 0,
		Normal_HalfLambert,
		Tile ,
		Particle_Lambert,
		Particle_HalfLambert,
	};

private:
	/// PSO関連

	LightModelType defaultLightModel_ = LightModelType::Lambert;
	psoType currentPSO_ = psoType::NONE;

private:
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
	std::vector<ID3D12PipelineState*> psoList_;
	ID3D12RootSignature* rootSignature_ = nullptr;
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};

	/// Textrue関連
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE Tile2DSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE Tile3DSrvHandleGPU_{};
	uint32_t textrueCounter = 1;
	std::vector<int> commonTextureSRVMap_;
	std::vector<int> modelTextureSRVMap_;
	int defaultTextureHandle_ = 0;						// white5x5
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;


	///Lighting関連
	DirectionalLight* directionalLightData = {};		// 外部から受ける

	/// 交換用容器
	Microsoft::WRL::ComPtr<ID3D12Resource> tile2DWVPResource_ = nullptr;
	TransformationMatrix* tile2DInstancingData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> tile3DWVPResource_ = nullptr;
	TransformationMatrix* tile3DInstancingData_ = nullptr;
	int instance2DCounter = 0;
	int instance3DCounter = 0;

	BasicResource* instanceOffsetResource_ = new BasicResource;
	std::vector<UINT*> instanceOffsetData_;

	//Material関連
	Material* materialData = nullptr;
	DirectionalLight* lightingData = nullptr;

private:
	/// 内部関数
	D3D12_VIEWPORT createViewport(int kClientWidth, int kClientHeight);
	D3D12_RECT createScissorRect(int kClientWidth, int kClientHeight);
	void SetMaterial(int MaterialHandle);
	void InitializeLighting();
	void SetLighting(DirectionalLight* directionalLight);

	void PSODecition(MaterialConfig& material,bool isParticle = false);
	DirectX::ScratchImage LoadTextrueLow(const std::string& filePath);
	ID3D12Resource* CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata, ResourceManager::TextureInfo* saveData = nullptr);
	ID3D12Resource* UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList);
	int MakeTextureShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);
	int MakeModelShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource);
	D3D12_GPU_DESCRIPTOR_HANDLE CreateTileWVPBuffer(ID3D12Resource* insstancingResource);
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);
	void MakeDepthStencilView();

	//Vector2 calTextruePos(Vector2 pos);

private:
	bool isFinish = false;
};

