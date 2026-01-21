#pragma once
#include "DirectXCore.h"
#include "externals/DirectXTex/DirectXTex.h"
#include "externals/DirectXTex/d3dx12.h"
#include <vector>
#include "Config.h"
#include "PSO.h"
#include "ResourceManager.h"
#include "VertexData.h"
#include "Material.h"
#include "DirectionalLight.h"

#include "MathsIncluder.h"
#include "TransformationMatrix.h"
#include "ConvertString.h"
#include "LightModelType.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"
#include "Camera.h"
#include "DrawData/CameraForGPU.h"

#include "DrawData/ObjectData.h"
#include "DrawData/SpriteData.h"
#include <format>

#include "SrvManager.h"
#include "DrawDataCollector.h"

class DrawEngine
{
public:
	~DrawEngine();

	void Initialize(DirectXCore* directXDirver,
					SrvManager* srvManager, 
					ResourceManager* resourceManager,
					DrawDataCollector* drawDataCollector);

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
	void CollectCube(TransformationMatrix* wvpData, MaterialConfig material);
	void DrawCube();
	/// 球体関連
	void DrawSphere(TransformationMatrix* wvpData, MaterialConfig material, int sudivision);
	/// モデル関連
	void CollectModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = -1);
	void DrawModel();

	/// Tile関連
	void Collect3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = -1);
	void Draw3DTile();



	/// 描くものテータを収集する関数
	void Collect2D(SpriteData* spriteData);
	void Collect3D(ObjectData* object);

	/// 全部描く関数

	/// 2D描画関数
	void Draw2D();
	void Draw2DTransparent();
	void Draw2DOpaque();

	/// 3D描画関数
	void Draw3D();
	void Draw3DTransparent();
	void Draw3DOpaque();

	void DrawCall();

	/// リソースローディング
	int GetModelTextureHandle(int modelHandle, int part);

	int readModelTextureHandle(int Handle);
	int readCommonTextureHandle(int Handle);

	int LoadModelTexture(const std::string& filePath);


private:
	
	PSO* pso_ = new PSO;
	ResourceManager* resourceManager_{};
	DirectXCore* directXDriver_{};					/*依存*/
	ID3D12GraphicsCommandList* commandList_{};		/*依存*/
	SrvManager* srvManager_{};						/*依存*/
	DrawDataCollector* drawDataCollector_{};			/*依存*/

	int kClientWidth_ = 0;
	int kClientHeight_ = 0;

	int kMaxSubdivision_ = 18;
	int kSubdivision_ = 0;

private:
	enum class psoType {
		NONE = -1,
		defaultPSO = 1,
		Sprite2D = 0,
		Lambert,
		HalfLambert,
		PhongReflection,
		BlinnPhongReflection,
	};

private:
	/// PSO関連

	LightModelType defaultLightModel_ = LightModelType::Lambert;
	psoType currentPSO_ = psoType::NONE;

private:
	std::vector<ID3D12PipelineState*> psoList_;
	ID3D12RootSignature* rootSignature_ = nullptr; 			// Listからもセーブしたから解放しなくていい	
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};

	/// Texture関連
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE Tile2DSrvHandleGPU_{};
	D3D12_GPU_DESCRIPTOR_HANDLE Tile3DSrvHandleGPU_{};
	uint32_t descriptorIndex_ = 1;						// 0はImGui用に予約
	std::vector<int> commonTextureSRVMap_;
	std::vector<int> modelTextureSRVMap_;
	int defaultTextureHandle_ = 0;						// white5x5
	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
	ID3D12Resource* depthStencilResource = nullptr;


	///Lighting関連
	DirectionalLight* directionalLightData{};		// 外部から受ける
	DirectionalLight* lightingData = nullptr;

	/// 交換用容器
	BasicResource* tile2DWVPResource_ = new BasicResource;
	TransformationMatrix* tile2DInstancingData_ = nullptr;
	BasicResource* tile3DWVPResource_ = new BasicResource;
	TransformationMatrix* tile3DInstancingData_ = nullptr;
	int instance2DCounter_ = 0;
	int instance3DCounter_ = 0;

	/// カメラ関連
	CameraForGPU* cameraPtr_ = nullptr;
	BasicResource* cameraBuffer_ = new BasicResource;

	//Material関連
	Material* materialData = nullptr;

private:

	struct OffsetData {
		BasicResource* instanceOffsetResource = new BasicResource;
		UINT* instanceOffset{};
		int state = 0;// 0:未使用 1:使用中
	};

	std::vector<OffsetData*> instanceOffsetData_;
	int offsetDataCounter_{};


private:
	/// 内部関数
	D3D12_VIEWPORT createViewport(int kClientWidth, int kClientHeight);
	D3D12_RECT createScissorRect(int kClientWidth, int kClientHeight);
	void SetMaterial(int materialID);
	void SetTexture(int materialID);
	void SetCameraForGPU();
	void InitializeLighting();
	void SetLighting(DirectionalLight* directionalLight);

	void PSODecision(MaterialConfig& material);
	void PSODecision(int psoID);
	ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height);
	void MakeDepthStencilView();

private:
	bool isFinish = false;
};

