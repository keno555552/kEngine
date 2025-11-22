#pragma once
#include "BasicResource.h"
#include "WVPResource.h"
#include "VertexResource.h"
#include "VertexData.h"
#include "Material.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"
#include "InstanceManager.h"
#include "TextureManager.h"
#include "config.h"
#include "DrawData/ObjectData.h"
#include "DrawData/SpriteData.h"

class ResourceManager
{
public:
	struct TextureInfo {
		ID3D12Resource* texture;
		int width;
		int height;
		Vector4 uvOffset;
	};

#pragma region Instance管理


#pragma endregion

public:
	/// 一回だけ作成するResource
	ResourceManager(DirectXCore* device);
	/// 最後で解放するResource
	~ResourceManager();

	/// ランクこと作成するResource
	void CreateTurnResource();

	/// ランクこと解放するResource
	void ClearTurnResource();

public:
	//////////////////////////////命令

	/// Resource Collect
	void ColletSprite(Vector2 pos, MaterialConfig material);
	void ColletModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = 0, bool useDefaultModel = false);
	void Collet2DTile(Vector2 pos, MaterialConfig material);
	void Collet3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = 0,bool useDefaultModel = false);

	void Collet2D(SpriteData* sprite);
	void Collet3D(ObjectData* object);


	/// モデル読み込み
	int LoadModel(std::string Path);

public:
	//////////////////////////////命令

	/// Texture指令

	int LoadCommonTexture(const std::string& filePath);
	int LoadModelTexture(const std::string& filePath);

	int GetTextureHandleFromCommonList(int index);
	int GetTextureHandleFromModelGroup(int modelHandle, int part);

	DirectX::TexMetadata GetTextureMetadata(int textureHandle);

	int ReadModelTextureHandle(int index);
	int ReadCommenTextureHandle(int index);

	bool SetModelTexture(Model* model);

	void ResizeSimpleSpriteMesh(DirectX::TexMetadata Metadata);

	/// 暫くのCounter管理
	int GetTextureCounter();
	void TextureCounterPlus(int index = 1);
	void TextuerCounterAdjust(int index);

	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUDescriptorHandle(int handle);
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUDescriptorHandle(int handle);

public:

	/// 借りのDevice
	DirectXCore* core_ = nullptr;
	ID3D12Device* Bdevice_ = nullptr;

	//////////////////////////////Texture関係
	/// テクスチャ関係
	BasicResource* textureResource_ = new BasicResource; ///テキスチャーの収納のどころ

	/// Lighting関係
	BasicResource* lightingResource_ = new BasicResource;

	/// Material関係
	std::vector<BasicResource*> materialResourceList_;

	//WVPResource* wvpResource_ = nullptr;

	//////////////////////////////InstanceBuffer関係

	/// Instance管理
	InstanceManager* instanceManager_ = new InstanceManager;

	//////////////////////////////Vertex\Index関係

	/// 図形関係
	std::vector<MeshBuffer*> meshBufferList_;		/// すべでのモデルを収納するどころ		これを使って解放する
	std::vector<Sprite2D*> spriteMeshHandles_;	/// スブライドのハンドルを収納する		解放に使えない
	std::vector<ModelGroup*> modelGroupList_;		/// モデルグループを	収納する			解放に使えない
	SimpleSpriteMesh* simpleSpriteMesh_ = nullptr;	/// デフォルトのスプライトメッシュ

	/// ModelHandle
	int modelHandleCounter_ = 0;


private:
	/// リソース作り
	
	int CreateSimpleSpriteMeshResource();
	//int CreateSprite2DResource(Vector2 LTpos, Vector2 LBpos,
	//	Vector2 RTpos, Vector2 RBpos,
	//	float TsizeX, float TsizeY,
	//	Vector2 TCLTPos, Vector2 TCRBPos);
	//int CreateSprite2DResource(const DirectX::TexMetadata mipData);
	int CreateTriangleResource();
	int CreateCubeResource();
	int CreateSphereResource(int sudivision);

	int CreateModelRosource(std::string Path);

private:
	////////////////////////////// 関数テンプレート

	/// ポインタ解放テンプレート
	template<typename T>
	void ClearPointer(std::vector< T* >& list) {
		for (auto& ptr : list) {
			delete ptr;
			ptr = nullptr;
		}
		list.clear();
	}

	template<typename T>
	bool CheckInstance(std::vector< T* >& list, T target, bool useCustomCheck) {

		auto checker = std::find_if(list.begin(),
				list.end(),
				[&](T* ptr) {return *ptr == target; });

		return checker == list.end();
	}


};

