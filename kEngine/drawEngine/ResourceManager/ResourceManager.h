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

	/// 借りのDevice
	ID3D12Device* Bdevice_ = nullptr;

	/// Material関係
	BasicResource* materialResource_ = new BasicResource;
	std::vector<BasicResource*> materialResourceModelGroup_;

	/// テクスチャ関係
	std::vector<std::string> commonTextureFilePath_;
	std::vector<TextureInfo> textureData_;
	BasicResource* textureResource_ = new BasicResource;
	BasicResource* intermediateResource_ = new BasicResource;/*EndDrawでTextrueを作ったら解放する*/

	/// Lighting関係
	BasicResource* lightingResource_ = new BasicResource;

	/// 図形関係
	Triangle* vertexResourceTriangle_ = new Triangle;
	std::vector<Sprite2D*> vertexResourceSpriteGroup_;
	Cube* vertexResourceCube_ = new Cube;
	Sphere* vertexResourceSphere_ = new Sphere;
	std::vector<ModelGroup*> vertexResourceModelGroup_;

	/// Instance管理
	InstanceManager* instanceManager_ = new InstanceManager;

	/// 無用もの消し関連
	int deleteParameter = 0;
};

