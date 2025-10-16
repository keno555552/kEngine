#pragma once
#include "BasicResource.h"
#include "VertexResource.h"
#include "VertexData.h"
#include "Material.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"
#include "InstanceManager.h"

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

	void CreateTriangleResource();
	void CreateCubeResource();

	/// Resource Collect
	void ColletSprite(Vector2 pos, MaterialConfig material);
	void ColletModel(TransformationMatrix* wvpData, MaterialConfig material);
	void Collet2DTile(Vector2 pos, MaterialConfig material);
	void Collet3DTile(TransformationMatrix* wvpData, MaterialConfig material);

public:

	/// 借りのDevice
	ID3D12Device* Bdevice_ = nullptr;

	/// テクスチャ関係
	BasicResource* intermediateResource_ = new BasicResource;/*EndDrawでTextrueを作ったら解放する*/
	std::vector<std::string> commonTextureFilePath_;
	std::vector<TextureInfo> textureData_;///テキスチャーの収納のどころ
	BasicResource* textureResource_ = new BasicResource;

	/// Lighting関係
	BasicResource* lightingResource_ = new BasicResource;

	/// Material関係
	std::vector<BasicResource*> materialResource_;

	//////////////////////////////InstanceBuffer関係

	/// Instance管理
	InstanceManager* instanceManager_ = new InstanceManager;

	//////////////////////////////Vertex\Index関係

	/// 図形関係
	std::vector<MeshBuffer*> meshBufferList_;
	int default_Triangle_MeshBufferHandle_ = 0; // 三角形
	int default_Cube_MeshBufferHandle_ = 0; // キューブ

	/// デフォルトMeshBuffer
	Triangle* vertexResourceTriangle_ = new Triangle;
	Cube* vertexResourceCube_ = new Cube;
	Sphere* vertexResourceSphere_ = new Sphere;

	/// 無用もの消し関連
	int deleteParameter = 0;
};

