#include "InstanceManager.h"

InstanceManager::~InstanceManager() {
	ClearInstance(materialConfigList_);
	ClearInstance(spriteList_);
	ClearInstance(tile2DList_);
	ClearInstance(modelList_);
	ClearInstance(tile3DList_);
}

void InstanceManager::Update() {

	UpdateInstance(spriteList_);
	UpdateInstance(tile2DList_);
	UpdateInstance(modelList_);
	UpdateInstance(tile3DList_);


	if (tileLayerCount > 1) {
		tileLayerCount = (int)tile2DList_[tile2DList_.size() - 1]->position.z;
	}
	if (spriteLayerCount > 1) {
		spriteLayerCount = (int)(spriteList_[spriteList_.size() - 1]->position.z + 0.001f);
	}
}

void InstanceManager::AddSpriteInstance(Vector2 pos, MaterialConfig material) {
	SpriteInstance instance;
	instance.position = { pos.x,pos.y,(float)tileLayerCount * -0.001f };
	instance.scale = { 1.0f,1.0f };		/// まだ使ってない
	instance.rotate = { 0.0f,0.0f,0.0f };  /// まだ使ってない
	instance.layer = 0;					/// まだ使ってない
	instance.drawState = STANDBY;

	auto checker = std::find_if(materialConfigList_.begin(),
		materialConfigList_.end(),
		[&](MaterialConfig* ptr) {return *ptr == material; });

	if (checker == materialConfigList_.end()) {
		MaterialConfig* newMaterial = new MaterialConfig(material);
		materialConfigList_.push_back(newMaterial);
		instance.materialConfigIndex = int(materialConfigList_.size() - 1);
	} else {
		instance.materialConfigIndex = (int)std::distance(materialConfigList_.begin(), checker);
	}

	auto checker2 = std::find_if(spriteList_.begin(),
		spriteList_.end(),
		[&](SpriteInstance* ptr) {return ptr->CheckSame(instance); });

	if (checker2 != spriteList_.end()) {
		(*checker2)->drawState = STANDBY;
		(*checker2)->materialConfigIndex = instance.materialConfigIndex;
	} else {
		SpriteInstance* newInstance = new SpriteInstance(instance);
		spriteList_.push_back(newInstance);
	}
	spriteLayerCount++;
	return;
}

void InstanceManager::AddModelInstance(TransformationMatrix* wvpData, MaterialConfig material) {


}

void InstanceManager::Add2DTileInstance(Vector2 pos, MaterialConfig material) {
	SpriteInstance instance;
	instance.position = { pos.x,pos.y,(float)tileLayerCount * -0.001f };
	instance.scale = { 1.0f,1.0f };		/// まだ使ってない
	instance.rotate = { 0.0f,0.0f,0.0f };  /// まだ使ってない
	instance.layer = 0;					/// まだ使ってない
	instance.drawState = STANDBY;

	auto checker = std::find_if(materialConfigList_.begin(),
		materialConfigList_.end(),
		[&](MaterialConfig* ptr) {return *ptr == material; });

	if (checker == materialConfigList_.end()) {
		MaterialConfig* newMaterial = new MaterialConfig(material);
		materialConfigList_.push_back(newMaterial);
		instance.materialConfigIndex = int(materialConfigList_.size() - 1);
	} else {
		instance.materialConfigIndex = (int)std::distance(materialConfigList_.begin(), checker);
	}

	auto checker2 = std::find_if(tile2DList_.begin(),
		tile2DList_.end(),
		[&](SpriteInstance* ptr) {return ptr->CheckSame(instance); });

	if (checker2 != tile2DList_.end()) {
		(*checker2)->drawState = STANDBY;
		(*checker2)->materialConfigIndex = instance.materialConfigIndex;
	} else {
		SpriteInstance* newInstance = new SpriteInstance(instance);
		tile2DList_.push_back(newInstance);
	}
	tileLayerCount++;
	return;
}

void InstanceManager::Add3DTileInstance(TransformationMatrix* wvpData, MaterialConfig material) {
	ModelInstance instance;
	instance.WVP = wvpData->WVP;
	instance.world = wvpData->world;
	instance.drawState = STANDBY;

	auto checker = std::find_if(materialConfigList_.begin(),
		materialConfigList_.end(),
		[&](MaterialConfig* ptr) {return *ptr == material; });

	if (checker == materialConfigList_.end()) {
		MaterialConfig* newMaterial = new MaterialConfig(material);
		materialConfigList_.push_back(newMaterial);
		instance.materialConfigIndex = int(materialConfigList_.size() - 1);
	} else {
		instance.materialConfigIndex = (int)std::distance(materialConfigList_.begin(), checker);
	}

	auto checker2 = std::find_if(tile3DList_.begin(),
		tile3DList_.end(),
		[&](ModelInstance* ptr) {return ptr->CheckSame(instance); });

	if (checker2 != tile3DList_.end()) {
		(*checker2)->drawState = STANDBY;
		(*checker2)->materialConfigIndex = instance.materialConfigIndex;
	} else {
		ModelInstance* newInstance = new ModelInstance(instance);
		tile3DList_.push_back(newInstance);
	}
	return;
}

void InstanceManager::AddParticleInstance(TransformationMatrix* wvpData, MaterialConfig material) {
}


