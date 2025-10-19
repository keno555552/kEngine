#include "ResourceManager.h"

ResourceManager::ResourceManager(ID3D12Device* device) {

	Bdevice_ = device;

	default_Triangle_MeshBufferHandle_	= CreateTriangleResource();
	default_Sprite2D_MeshBufferHandle_	= CreateSprite2DResource(); 
	default_Cube_MeshBufferHandle_		= CreateCubeResource();
	default_Sphere_MeshBufferHandle_	= CreateSphereResource(1);

}

ResourceManager::~ResourceManager() {

	ClearPointer(materialResourceList_);
	ClearPointer(meshBufferList_);
	spriteMeshHandles_.clear();
	modelMeshHandles_.clear();
	modelGroupList_.clear();

	for (auto& tex : textureData_) {
		if (tex.texture) {
			tex.texture->Release();
			tex.texture = nullptr;
		}
	}
	textureData_.clear();

	/// comptr自動解放
	delete intermediateResource_;
	delete textureResource_;
	delete lightingResource_;

	/// InstanceManager解放
	delete instanceManager_;
}

void ResourceManager::CreateTurnResource() {
}


void ResourceManager::ClearTurnResource() {
	intermediateResource_->ClearResource();
	instanceManager_->Update();
}



void ResourceManager::ColletSprite(Vector2 pos, MaterialConfig material) {
	int materialNum = (int)instanceManager_->materialConfigList_.size();
	instanceManager_->AddSpriteInstance(pos, material);
	if (materialNum < (int)instanceManager_->materialConfigList_.size()) {
		/// 新しいResourceを追加
		BasicResource* newResource = new BasicResource;
		newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
		materialResourceList_.push_back(newResource);

		/// MaterialとMapする
		Material* newData = nullptr;
		newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
		newData->inputMaterialConfig(material);
		newResource->GetResource()->Unmap(0, nullptr);

		/// instanceにResourceのHandleを設定
		instanceManager_->materialConfigList_.end()[-1]->materialResourceHandle = materialNum;
	}
}

void ResourceManager::ColletModel(TransformationMatrix* wvpData, MaterialConfig material) {
	int materialNum = (int)instanceManager_->materialConfigList_.size();
	instanceManager_->AddModelInstance(wvpData, material);
	if (materialNum < (int)instanceManager_->materialConfigList_.size()) {
		/// 新しいResourceを追加
		BasicResource* newResource = new BasicResource;
		newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
		materialResourceList_.push_back(newResource);
	
		/// MaterialとMapする
		Material* newData = nullptr;
		newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
		newData->inputMaterialConfig(material);
		newResource->GetResource()->Unmap(0, nullptr);
	
		/// instanceにResourceのHandleを設定
		instanceManager_->materialConfigList_.end()[-1]->materialResourceHandle = materialNum;
	}
}

void ResourceManager::Collet2DTile(Vector2 pos, MaterialConfig material) {
	int materialNum = (int)instanceManager_->materialConfigList_.size();
	instanceManager_->Add2DTileInstance(pos, material);
	if (materialNum < (int)instanceManager_->materialConfigList_.size()) {
		/// 新しいResourceを追加
		BasicResource* newResource = new BasicResource;
		newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
		materialResourceList_.push_back(newResource);

		/// MaterialとMapする
		Material* newData = nullptr;
		newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
		newData->inputMaterialConfig(material);
		newResource->GetResource()->Unmap(0, nullptr);

		/// instanceにResourceのHandleを設定
		instanceManager_->materialConfigList_.end()[-1]->materialResourceHandle = materialNum;
	}
}

void ResourceManager::Collet3DTile(TransformationMatrix* wvpData, MaterialConfig material) {
	instanceManager_->Add3DTileInstance(wvpData, material);
}


int ResourceManager::CreateTriangleResource() {
	Triangle* newTriangle = new Triangle;
	newTriangle->CreateVertexResource_(Bdevice_);
	newTriangle->CreateVertexBufferView_(6);
	meshBufferList_.push_back(newTriangle);
	return int(meshBufferList_.size() - 1);
}

int ResourceManager::CreateCubeResource() {
	Cube* newCube_ = new Cube;
	newCube_->CreateVertexResource_(Bdevice_);
	newCube_->CreateVertexBufferView_(24);
	newCube_->CreateIndexResource_(Bdevice_);
	newCube_->CreateIndexBufferView_(36);
	meshBufferList_.push_back(newCube_);
	return int(meshBufferList_.size() - 1);
}


int ResourceManager::CreateSprite2DResource() {
	Sprite2D* newSprite2D_ = new Sprite2D;
	newSprite2D_->CreateVertexResource_(Bdevice_);
	newSprite2D_->CreateVertexBufferView_(5);
	newSprite2D_->CreateIndexResource_(Bdevice_);
	newSprite2D_->CreateIndexBufferView_(12);
	meshBufferList_.push_back(newSprite2D_);
	return int(meshBufferList_.size() - 1);
}

int ResourceManager::CreateSprite2DResource(Vector2 LTpos, Vector2 LBpos,
	Vector2 RTpos, Vector2 RBpos,
	float TsizeX, float TsizeY,
	Vector2 TCLTPos, Vector2 TCRBPos) {

	Sprite2D* newSprite2D_ = new Sprite2D;
	newSprite2D_->SetSize(LTpos, LBpos, RTpos, RBpos, TsizeX, TsizeY, TCLTPos, TCRBPos);
	newSprite2D_->CreateVertexResource_(Bdevice_);
	newSprite2D_->CreateVertexBufferView_(5);
	newSprite2D_->CreateIndexResource_(Bdevice_);
	newSprite2D_->CreateIndexBufferView_(12);
	newSprite2D_->SetKeep(true);
	meshBufferList_.push_back(newSprite2D_);
	return int(meshBufferList_.size() - 1);
}

int ResourceManager::CreateSphereResource(int sudivision) {

	Sphere* newSphere = new Sphere;
	newSphere->CreateVertexResource_(Bdevice_);
	newSphere->CreateIndexResource_(Bdevice_);
	meshBufferList_.push_back(newSphere);
	return int(meshBufferList_.size() - 1);
}

int ResourceManager::CreateModelRosource(std::string Path) {

	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->SetModelObj(Path);

	/// パースによって同じモデルがリストにいるかを確認
	if (!modelGroupList_.empty()) {
		for (int i = 0; i < modelGroupList_.size(); i++) {
			std::string checkPath = modelGroupList_[i]->GetDirectoryPath();
			std::string checkName = modelGroupList_[i]->GetObjName_();
			if (checkPath == modelGroup->GetDirectoryPath()) {
				if (checkName == modelGroup->GetObjName_()) {
					return i;
				}
			}
		}
	}

	/// Model読み込み
	std::vector<ModelData> modelList = LoadMuitObjFile(modelGroup->GetDirectoryPath(), modelGroup->GetObjName_());
	for (auto& ptr : modelList) {
		Model* newModel = new Model;
		newModel->GetModelData(ptr);
		newModel->SetModelObj(Path);
		newModel->CreateVertexResourceG_(Bdevice_);
		modelGroup->PushModel(newModel);
		meshBufferList_.push_back(newModel);
		modelGroup->PushModelHandle((int)(meshBufferList_.size() - 1));
	}
	modelGroupList_.push_back(modelGroup);

	return int(modelGroupList_.size() - 1);
}
