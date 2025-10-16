#include "ResourceManager.h"

ResourceManager::ResourceManager(ID3D12Device* device) {

	Bdevice_ = device;

	CreateTriangleResource();
	CreateCubeResource();

	Sprite2D* vertexResourceSprite_ = new Sprite2D;
	vertexResourceSprite_->CreateVertexResource_(Bdevice_);
	vertexResourceSprite_->CreateVertexBufferView_(5);
	vertexResourceSprite_->CreateIndexResource_(Bdevice_);
	vertexResourceSprite_->CreateIndexBufferView_(12);
	vertexResourceSprite_->SetKeep(true);
	vertexResourceSpriteGroup_.push_back(vertexResourceSprite_);
}

ResourceManager::~ResourceManager() {
	///ID3D12Resource*の解放する


	textureResource_->ClearResource();
	intermediateResource_->ClearResource();
	//
	lightingResource_->ClearResource();



	for (auto& tex : textureData_) {
		if (tex.texture) {
			tex.texture->Release();
			tex.texture = nullptr;
		}
	}
	textureData_.clear();
	vertexResourceTriangle_->ClearVertexResource();
	vertexResourceCube_->ClearAllResource();
	vertexResourceSphere_->ClearAllResource();

	delete textureResource_;
	delete intermediateResource_;
	delete lightingResource_;
	delete vertexResourceTriangle_;
	delete vertexResourceCube_;
	delete vertexResourceSphere_;
	delete instanceManager_;
}


void ResourceManager::ClearTurnResource() {
	deleteParameter++;
	intermediateResource_->ClearResource();
	instanceManager_->Update();
}

void ResourceManager::CreateTriangleResource() {
	vertexResourceTriangle_->CreateVertexResource_(Bdevice_);
	vertexResourceTriangle_->CreateVertexBufferView_(6);
}

void ResourceManager::CreateCubeResource() {
	vertexResourceCube_->CreateVertexResource_(Bdevice_);
	vertexResourceCube_->CreateVertexBufferView_(24);
	vertexResourceCube_->CreateIndexResource_(Bdevice_);
	vertexResourceCube_->CreateIndexBufferView_(36);
}

void ResourceManager::ColletSprite(Vector2 pos, MaterialConfig material) {
	int instanceNum = instanceManager_->spriteList_.size();
	instanceManager_->AddSpriteInstance(pos, material);
	if (instanceNum < instanceManager_->spriteList_.size()) {
	}
}

void ResourceManager::ColletModel(TransformationMatrix* wvpData, MaterialConfig material) {
	instanceManager_->AddModelInstance(wvpData, material);
}

void ResourceManager::Collet2DTile(Vector2 pos, MaterialConfig material) {
	instanceManager_->Add2DTileInstance(pos, material);
}

void ResourceManager::Collet3DTile(TransformationMatrix* wvpData, MaterialConfig material) {
	instanceManager_->Add3DTileInstance(wvpData, material);
}
