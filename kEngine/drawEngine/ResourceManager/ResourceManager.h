#pragma once
#include "BasicResource.h"
#include "WVPResource.h"
#include "VertexResource.h"
#include "VertexData.h"
#include "Material.h"
#include "MaterialConfig.h"
#include "VertexIndex.h"
#include "InstanceManager.h"
#include "config.h"
#include "ObjectData.h"

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
	void ColletModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = 0, bool useDefaultModel = false);
	void Collet2DTile(Vector2 pos, MaterialConfig material);
	void Collet3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle = 0,bool useDefaultModel = false);

	void Collet3D(ObjectData* object);


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

	WVPResource* wvpResource_ = nullptr;

	//////////////////////////////InstanceBuffer関係

	/// Instance管理
	InstanceManager* instanceManager_ = new InstanceManager;

	//////////////////////////////Vertex\Index関係

	/// 図形関係
	std::vector<MeshBuffer*> meshBufferList_;		/// すべでのモデルを収納するどころ		これを使って解放する
	std::vector<MeshBuffer*> spriteMeshHandles_;	/// スブライドのハンドルを収納する		解放に使えない
	std::vector<ModelGroup*> modelGroupList_;		/// モデルグループを	収納する			解放に使えない

	/// ModelHandle
	int modelHandleCounter_ = 0;


private:
	/// リソース作り
	int CreateSprite2DResource();
	int CreateSprite2DResource(Vector2 LTpos, Vector2 LBpos,
		Vector2 RTpos, Vector2 RBpos,
		float TsizeX, float TsizeY,
		Vector2 TCLTPos, Vector2 TCRBPos);

	int CreateTriangleResource();
	int CreateCubeResource();
	int CreateSphereResource(int sudivision);

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

