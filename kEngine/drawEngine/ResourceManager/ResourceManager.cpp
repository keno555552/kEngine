#include "ResourceManager.h"
#include <Object/Object.h>

ResourceManager::ResourceManager(DirectXCore* device) {

	core_ = device;
	Bdevice_ = core_->GetDriver();

	//wvpResource_ = new WVPResource(Bdevice_);
	config::default_Sprite2D_MeshBufferHandle_ = CreateSimpleSpriteMeshResource();
	config::default_Triangle_MeshBufferHandle_ = CreateTriangleResource();
	config::default_Cube_MeshBufferHandle_ = CreateCubeResource();
	config::default_Sphere_MeshBufferHandle_ = CreateSphereResource(1);

	TextureManager::GetInstance()->Initialize(device);
}

ResourceManager::~ResourceManager() {

	ClearPointer(materialResourceList_);
	ClearPointer(meshBufferList_);
	spriteMeshHandles_.clear();
	modelGroupList_.clear();
	ClearPointer(simpleSpriteMeshList_);

	/// comptr自動解放
	delete textureResource_;
	delete lightingResource_;

	/// TextureManager解放
	TextureManager::GetInstance()->Finalize();

	/// WVPResource解放
	//delete wvpResource_;

	/// InstanceManager解放
	delete instanceManager_;
}

void ResourceManager::CreateTurnResource() {
}


void ResourceManager::ClearTurnResource() {
	TextureManager::GetInstance()->EndUploadingTexture();
	instanceManager_->Update();
	//wvpResource_->ClearWVPResource();
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
		instanceManager_->materialConfigList_.back()->materialResourceHandle = materialNum;
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
		instanceManager_->materialConfigList_.back()->materialResourceHandle = materialNum;
	}
}

void ResourceManager::ColletModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle, bool useDefaultModel) {

	int modelNum = 1;
	if (!useDefaultModel) modelNum = modelGroupList_[modelHandle]->GetModelNum();

	for (int i = 0; i < modelNum; i++) {

		int before = (int)instanceManager_->materialConfigList_.size();

		/// 処理してるマテリアルをまとめる
		MaterialConfig usingMaterial;
		if (i < (int)material.size())usingMaterial = material[i];
		else usingMaterial = material.back();


		/// Instance追加
		if (!useDefaultModel) {

			if (usingMaterial.useModelTexture == true) {
				usingMaterial.textureHandle = (modelGroupList_[modelHandle])->GetModel(i)->GetTextureHandle();
			}

			instanceManager_->AddModelInstance(wvpData, usingMaterial,
				modelGroupList_[modelHandle]->GetModel(i)->GetVertexNum(),
				modelGroupList_[modelHandle]->GetModelHandle(i),
				useDefaultModel);

		} else {

			if (usingMaterial.useModelTexture == true) {
				usingMaterial.textureHandle = 0;
			}

			if (modelHandle == config::default_Cube_MeshBufferHandle_) {
				instanceManager_->AddModelInstance(wvpData, usingMaterial,
					36,
					config::default_Cube_MeshBufferHandle_,
					useDefaultModel);
			}
		}

		int after = (int)instanceManager_->materialConfigList_.size();

		/// マテリアルが足すがによってリソース追加
		if (after > before) {
			/// 新しいResourceを追加
			BasicResource* newResource = new BasicResource;
			newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
			materialResourceList_.push_back(newResource);

			/// MaterialとMapする
			Material* newData = nullptr;
			newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
			newData->inputMaterialConfig(usingMaterial);
			newResource->GetResource()->Unmap(0, nullptr);

			/// instanceにResourceのHandleを設定
			instanceManager_->materialConfigList_.back()->materialResourceHandle = (int)materialResourceList_.size() - 1;
		}
	}
}

void ResourceManager::Collet3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle, bool useDefaultModel) {

	int modelNum = 1;
	if (!useDefaultModel) modelNum = modelGroupList_[modelHandle]->GetModelNum();

	for (int i = 0; i < modelNum; i++) {

		int before = (int)instanceManager_->materialConfigList_.size();

		/// 処理してるマテリアルをまとめる
		MaterialConfig usingMaterial;
		if (i < (int)material.size())usingMaterial = material[i];
		else usingMaterial = material.back();

		if (usingMaterial.useModelTexture == true) {
			usingMaterial.textureHandle = (modelGroupList_[modelHandle])->GetModel(i)->GetTextureHandle();
		}

		/// Instance追加
		if (!useDefaultModel) {

			if (usingMaterial.useModelTexture == true) {
				usingMaterial.textureHandle = (modelGroupList_[modelHandle])->GetModel(i)->GetTextureHandle();
			}

			instanceManager_->Add3DTileInstance(wvpData, usingMaterial,
				modelGroupList_[modelHandle]->GetModel(i)->GetVertexNum(),
				modelGroupList_[modelHandle]->GetModelHandle(i),
				useDefaultModel);

		} else {

			if (usingMaterial.useModelTexture == true) {
				usingMaterial.textureHandle = 0;
			}

			if (modelHandle == config::default_Cube_MeshBufferHandle_) {
				instanceManager_->Add3DTileInstance(wvpData, usingMaterial,
					36,
					config::default_Cube_MeshBufferHandle_,
					useDefaultModel);
			}
		}


		int after = (int)instanceManager_->materialConfigList_.size();

		/// マテリアルが足すがによってリソース追加
		if (after > before) {
			/// 新しいResourceを追加
			BasicResource* newResource = new BasicResource;
			newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
			materialResourceList_.push_back(newResource);

			/// MaterialとMapする
			Material* newData = nullptr;
			newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
			newData->inputMaterialConfig(usingMaterial);
			newResource->GetResource()->Unmap(0, nullptr);

			/// instanceにResourceのHandleを設定
			instanceManager_->materialConfigList_.back()->materialResourceHandle = (int)materialResourceList_.size() - 1;
		}
	}
}






void ResourceManager::Collet2D(SpriteData* sprite) {
	int materialNum = (int)instanceManager_->materialConfigList_.size();
	if (materialNum < (int)instanceManager_->materialConfigList_.size()) {
		/// 新しいResourceを追加
		BasicResource* newResource = new BasicResource;
		newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
		materialResourceList_.push_back(newResource);

		/// MaterialとMapする
		Material* newData = nullptr;
		newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
		//newData->inputMaterialConfig(material);
		newResource->GetResource()->Unmap(0, nullptr);

		/// instanceにResourceのHandleを設定
		instanceManager_->materialConfigList_.back()->materialResourceHandle = materialNum;
	}

	int modelNum = (int)sprite->objectParts_.size();

	for (int i = 0; i < modelNum; i++) {

		int before = (int)instanceManager_->materialConfigList_.size();

		/// 処理してるマテリアルをまとめる
		MaterialConfig usingMaterial{};
		usingMaterial = *sprite->objectParts_[i].materialConfig;

		/// Instance追加
		instanceManager_->Add2DInstance(sprite->objectParts_[i].worldTransform, usingMaterial);


		int after = (int)instanceManager_->materialConfigList_.size();

		/// マテリアルが足すがによってリソース追加
		if (after > before) {
			/// 新しいResourceを追加
			BasicResource* newResource = new BasicResource;
			newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
			materialResourceList_.push_back(newResource);

			/// MaterialとMapする
			Material* newData = nullptr;
			newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
			newData->inputMaterialConfig(usingMaterial);
			newResource->GetResource()->Unmap(0, nullptr);

			/// instanceにResourceのHandleを設定
			instanceManager_->materialConfigList_.back()->materialResourceHandle = (int)materialResourceList_.size() - 1;
		}
	}
}

void ResourceManager::Collet3D(ObjectData* object) {
	int modelNum = modelGroupList_[object->modelHandle_]->GetModelNum();

	for (int i = 0; i < modelNum; i++) {

		int before = (int)instanceManager_->materialConfigList_.size();

		/// 処理してるマテリアルをまとめる
		TransformationMatrix wvpData{};
		MaterialConfig usingMaterial{};
		if (i < (int)object->objectParts_.size()) {
			wvpData = object->objectParts_[i].transformationMatrix;
			usingMaterial = *object->objectParts_[i].materialConfig;
		} else {
			wvpData = object->objectParts_.back().transformationMatrix;
			usingMaterial = *object->objectParts_.back().materialConfig;
		}

		/// Instance追加

		instanceManager_->Add3DInstance(wvpData, usingMaterial,
			modelGroupList_[object->modelHandle_]->GetModel(i)->GetVertexNum(),
			modelGroupList_[object->modelHandle_]->GetModelHandle(i),
			object->modelHandle_);


		int after = (int)instanceManager_->materialConfigList_.size();

		/// マテリアルが足すがによってリソース追加
		if (after > before) {
			/// 新しいResourceを追加
			BasicResource* newResource = new BasicResource;
			newResource->CreateResourceClass_(Bdevice_, sizeof(Material));
			materialResourceList_.push_back(newResource);

			/// MaterialとMapする
			Material* newData = nullptr;
			newResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&newData));
			newData->inputMaterialConfig(usingMaterial);
			newResource->GetResource()->Unmap(0, nullptr);

			/// instanceにResourceのHandleを設定
			instanceManager_->materialConfigList_.back()->materialResourceHandle = (int)materialResourceList_.size() - 1;
		}
	}

}



int ResourceManager::CreateSimpleSpriteMeshResource() {

	SimpleSpriteMesh* newSprite2D_ = new SimpleSpriteMesh;
	newSprite2D_->CreateVertexResource_(Bdevice_);
	newSprite2D_->CreateVertexBufferView_(4);
	newSprite2D_->CreateIndexResource_(Bdevice_);
	newSprite2D_->CreateIndexBufferView_(6);
	newSprite2D_->SetKeep(true);
	//meshBufferList_.push_back(newSprite2D_);
	simpleSpriteMeshList_.push_back(newSprite2D_);

	return (int)meshBufferList_.size() - 1;
}

//int ResourceManager::CreateSprite2DResource(Vector2 LTpos, Vector2 LBpos,
//	Vector2 RTpos, Vector2 RBpos,
//	float TsizeX, float TsizeY,
//	Vector2 TCLTPos, Vector2 TCRBPos) {
//
//	Sprite2D* newSprite2D_ = new Sprite2D;
//	newSprite2D_->SetSize(LTpos, LBpos, RTpos, RBpos, TsizeX, TsizeY, TCLTPos, TCRBPos);
//	newSprite2D_->CreateVertexResource_(Bdevice_);
//	newSprite2D_->CreateVertexBufferView_(5);
//	newSprite2D_->CreateIndexResource_(Bdevice_);
//	newSprite2D_->CreateIndexBufferView_(12);
//	newSprite2D_->SetKeep(true);
//	meshBufferList_.push_back(newSprite2D_);
//
//	return int(meshBufferList_.size() - 1);
//}
//
//int ResourceManager::CreateSprite2DResource(const DirectX::TexMetadata mipData) {
//	SimpleSpriteMesh* newSprite2D_ = new SimpleSpriteMesh;
//	newSprite2D_->SetSize(Vector2(mipData.width,mipData.height));
//	newSprite2D_->CreateVertexResource_(Bdevice_);
//	newSprite2D_->CreateVertexBufferView_(4);
//	newSprite2D_->CreateIndexResource_(Bdevice_);
//	newSprite2D_->CreateIndexBufferView_(6);
//	newSprite2D_->SetKeep(true);
//	meshBufferList_.push_back(newSprite2D_);
//
//	return int(meshBufferList_.size() - 1);
//}


int ResourceManager::CreateTriangleResource() {
	Triangle* newTriangle = new Triangle;
	newTriangle->CreateVertexResource_(Bdevice_);
	newTriangle->CreateVertexBufferView_(6);
	meshBufferList_.push_back(newTriangle);

	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->PushModel(newTriangle);
	modelGroup->PushModelHandle((int)meshBufferList_.size() - 1);
	modelGroupList_.push_back(modelGroup);

	return (int)modelGroupList_.size() - 1;
}

int ResourceManager::CreateCubeResource() {

	Cube* newCube_ = new Cube;
	newCube_->CreateVertexResource_(Bdevice_);
	newCube_->CreateVertexBufferView_(24);
	newCube_->CreateIndexResource_(Bdevice_);
	newCube_->CreateIndexBufferView_(36);
	meshBufferList_.push_back(newCube_);

	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->PushModel(newCube_);
	modelGroup->PushModelHandle((int)meshBufferList_.size() - 1);
	modelGroupList_.push_back(modelGroup);

	return (int)modelGroupList_.size() - 1;
}


int ResourceManager::CreateSphereResource(int sudivision) {
	sudivision;

	Sphere* newSphere = new Sphere;
	newSphere->CreateVertexResource_(Bdevice_);
	newSphere->CreateIndexResource_(Bdevice_);
	meshBufferList_.push_back(newSphere);

	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->PushModel(newSphere);
	modelGroup->PushModelHandle((int)meshBufferList_.size() - 1);
	modelGroupList_.push_back(modelGroup);

	return (int)modelGroupList_.size() - 1;

}

int ResourceManager::CreateModelRosource(std::string Path) {

	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->SetModelObj(Path);

	/// パースによって同じモデルがリストにいるかを確認
	if (!modelGroupList_.empty()) {
		for (int i = 0; i < (int)modelGroupList_.size(); i++) {
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
		modelGroup->PushModelHandle((int)meshBufferList_.size() - 1);
	}
	modelGroupList_.push_back(modelGroup);

	return (int)modelGroupList_.size() - 1;
}

int ResourceManager::LoadModel(std::string Path) {
	/// Resourceに同じものがあるがどうか捜索
	auto target = std::find_if(
		modelGroupList_.begin(),
		modelGroupList_.end(),
		[&](ModelGroup* ptr) {return ptr->GetFullPath_() == Path; }
	);
	if (target != modelGroupList_.end()) {
		return (int)std::distance(modelGroupList_.begin(), target);
	}

	/// モデルの読み込み
	int ModelGroupHandle = CreateModelRosource(Path);

	/// 各モテルのテキスチャーを設定する
	for (int i = 0; i < modelGroupList_[ModelGroupHandle]->GetModelNum(); i++) {
		SetModelTexture(modelGroupList_[ModelGroupHandle]->GetModel(i));
	}
	return ModelGroupHandle;


}

int ResourceManager::LoadCommonTexture(const std::string& filePath) {
	/// 同じテクスチャーがあるか確認、あったらそのハンドルを返す
	int textureHandle = TextureManager::GetInstance()->CheckSameCommonTextureLoaded(filePath);
	if (textureHandle != -1) return textureHandle;

	/// テクスチャー読み込み
	textureHandle = TextureManager::GetInstance()->LoadCommonTexture(filePath);

	return textureHandle;
}

int ResourceManager::LoadModelTexture(const std::string& filePath) {
	/// 同じテクスチャーがあるか確認、あったらそのハンドルを返す
	int textureHandle = TextureManager::GetInstance()->CheckSameModelTextureLoaded(filePath);
	if (textureHandle != -1) return textureHandle;

	/// テクスチャー読み込み
	textureHandle = TextureManager::GetInstance()->LoadModelTexture(filePath);

	return textureHandle;
}

int ResourceManager::GetTextureHandleFromCommonList(int index) {
	return TextureManager::GetInstance()->GetCommonTextureHandle(index);
}

int ResourceManager::GetTextureHandleFromModelGroup(int modelHandle, int part) {
	return modelGroupList_[modelHandle]->GetModel(part)->GetTextureHandle();
}

DirectX::TexMetadata ResourceManager::GetTextureMetadata(int textureHandle) {
	return TextureManager::GetInstance()->GetTextureMetadata(textureHandle);
}

int ResourceManager::ReadModelTextureHandle(int index) {
	return TextureManager::GetInstance()->GetModelTextureHandle(index);
}

int ResourceManager::ReadCommenTextureHandle(int index) {
	return TextureManager::GetInstance()->GetCommonTextureHandle(index);
}

bool ResourceManager::SetModelTexture(Model* model) {
	if (!model->GetTexturePatch().empty()) {
		model->SetTextureHandle(TextureManager::GetInstance()->LoadModelTexture(model->GetTexturePatch()));
		return true;
	}
	model->SetTextureHandle(TextureManager::GetDefaultTextureHandle() + 1);
	return false;
}

void ResourceManager::ResizeSimpleSpriteMesh(DirectX::TexMetadata Metadata, int counter, CornerData corner, Vector2 anchorPoint) {
	if (anchorPoint != Vector2{ 0, 0 }) {
		simpleSpriteMeshList_[counter]->SetAnchor(anchorPoint);
	} else if (!CheckCornerDataNull(corner)) {
		simpleSpriteMeshList_[counter]->SetSize(corner);
	} else {
		simpleSpriteMeshList_[counter]->SetSize(Vector2((float)Metadata.width, (float)Metadata.height));
	}
}

int ResourceManager::GetTextureCounter() {
	return TextureManager::GetInstance()->GetTextureCounter();
}

void ResourceManager::TextureCounterPlus(int index) {
	TextureManager::GetInstance()->TextureCounterPlus(index);
}

void ResourceManager::TextuerCounterAdjust(int index) {
	TextureManager::GetInstance()->TextuerCounterAdjust(index);
}

D3D12_CPU_DESCRIPTOR_HANDLE ResourceManager::GetTextureCPUDescriptorHandle(int handle) {
	return TextureManager::GetInstance()->GetTextureCPUDescriptorHandle(handle);
}

D3D12_GPU_DESCRIPTOR_HANDLE ResourceManager::GetTextureGPUDescriptorHandle(int handle) {
	return TextureManager::GetInstance()->GetTextureGPUDescriptorHandle(handle);
}

void ResourceManager::CreateSpriteMesh() {
	int counter = (int)instanceManager_->tile2DList_.size() - (int)simpleSpriteMeshList_.size();
	if (counter == 0)return;
	if (counter > 0) {
		for (int i = 0; i < counter; i++) {
			CreateSimpleSpriteMeshResource();
		}
	} else {
		for (int i = 0; i < -counter; i++) {
			auto& it = simpleSpriteMeshList_.back();
			simpleSpriteMeshList_.pop_back();
			delete it;
		}
	}
}
