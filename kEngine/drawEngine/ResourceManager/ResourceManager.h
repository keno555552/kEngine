#pragma once
#include "BasicResource.h"
#include "VertexResource.h"
#include "VertexData.h"
#include "Material.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"
#include "InstanceManager.h"
#include "config.h"

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
	ResourceManager(ID3D12Device* device);
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
	void ColletModel(TransformationMatrix* wvpData, MaterialConfig material);
	void Collet2DTile(Vector2 pos, MaterialConfig material);
	void Collet3DTile(TransformationMatrix* wvpData, MaterialConfig material);

	/// モデル読み込み
	int CreateModelRosource(std::string Path);

public:

	/// 借りのDevice
	ID3D12Device* Bdevice_ = nullptr;

	/// テクスチャ関係
	BasicResource* intermediateResource_ = new BasicResource;/*EndDrawでTextrueを作ったら解放する*/
	std::vector<std::string> commonTextureFilePath_;
	std::vector<TextureInfo> textureData_;				///テキスチャーのデイタの収納のどころ
	BasicResource* textureResource_ = new BasicResource; ///テキスチャーの収納のどころ

	/// Lighting関係
	BasicResource* lightingResource_ = new BasicResource;

	/// Material関係
	std::vector<BasicResource*> materialResourceList_;


	//////////////////////////////InstanceBuffer関係

	/// Instance管理
	InstanceManager* instanceManager_ = new InstanceManager;

	//////////////////////////////Vertex\Index関係

	/// 図形関係
	std::vector<MeshBuffer*> meshBufferList_;
	std::vector<int> spriteMeshHandles_;
	std::vector<int> modelMeshHandles_;
	std::vector<ModelGroup*> modelGroupList_;

	/// デフォルトMeshBuffer
	int default_Triangle_MeshBufferHandle_ = 0; // 三角形
	int default_Sprite2D_MeshBufferHandle_ = 0; // 2Dスブライド
	int default_Cube_MeshBufferHandle_ = 0; // キューブ
	int default_Sphere_MeshBufferHandle_ = 0; // 球体


private:
	/// リソース作り
	int CreateTriangleResource();
	int CreateCubeResource();
	int CreateSprite2DResource();
	int CreateSprite2DResource(Vector2 LTpos, Vector2 LBpos, 
								Vector2 RTpos, Vector2 RBpos,
								float TsizeX, float TsizeY, 
								Vector2 TCLTPos, Vector2 TCRBPos);
	int CreateSphereResource(int sudivision);


	template<typename T>
	void ClearPointer(std::vector< T* >& list) {
		for (auto& ptr : list) {
			delete ptr;
			ptr = nullptr;
		}
		list.clear();
	}
};

