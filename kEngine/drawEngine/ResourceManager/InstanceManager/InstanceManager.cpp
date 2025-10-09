#include "InstanceManager.h"

InstanceManager::~InstanceManager() {
	for (auto& ptr : materialConfigList_) {
		delete ptr;
		ptr = nullptr;
	}
	materialConfigList_.clear();

	for (auto& ptr : spriteList_) {
		delete ptr;
		ptr = nullptr;
	}
	spriteList_.clear();

	for (auto& ptr : tileList_) {
		delete ptr;
		ptr = nullptr;
	}
	tileList_.clear();

	for (auto& ptr : modelList_) {
		delete ptr;
		ptr = nullptr;
	}
	modelList_.clear();
}

void InstanceManager::Update() {

	for (auto& ptr : spriteList_) {
		if (ptr->drawState == DRAWLAST) {
			delete ptr;
			ptr = nullptr;
			continue;
		}
		if (ptr->drawState == ISDRAW) {
			ptr->drawState = DRAWLAST;
			continue;
		}
	}

	for (auto& ptr : tileList_) {
		if (ptr->drawState == DRAWLAST) {
			delete ptr;
			ptr = nullptr;
			continue;
		}
		if (ptr->drawState == ISDRAW) {
			ptr->drawState = DRAWLAST;
			continue;
		}
	}

}

void InstanceManager::AddSpriteInstance(Vector2 pos, MaterialConfig material) {
	SpriteInstance instance;
	instance.position = pos;
	instance.scale = { 1.0f,1.0f };		/// まだ使ってない
	instance.rotate = { 0.0f,0.0f,0.0f };  /// まだ使ってない
	instance.layer = 0;					/// まだ使ってない
	instance.drawState = STANDBY;

	auto checker = std::find_if(materialConfigList_.begin(),
		materialConfigList_.end(),
		[&](MaterialConfig* ptr) {return *ptr == material; });

	if (checker != materialConfigList_.end()) {
		instance.materialConfigIndex = (int)std::distance(materialConfigList_.begin(), checker);
	} else {
		MaterialConfig* newMaterial = new MaterialConfig(material);
		materialConfigList_.push_back(newMaterial);
		instance.materialConfigIndex = int(materialConfigList_.size() - 1);
	}

	auto checker2 = std::find_if(spriteList_.begin(),
		spriteList_.end(),
		[&](SpriteInstance* ptr) {return ptr->CheckSame(instance); });

	if (checker2 != spriteList_.end()) {
		(*checker2)->drawState = STANDBY;
	} else {
		SpriteInstance* newInstance = new SpriteInstance(instance);
		spriteList_.push_back(newInstance);
	}
	return;
}

void InstanceManager::AddTileInstance(Vector2 pos, MaterialConfig material) {
	SpriteInstance instance;
	instance.position = pos;
	instance.scale = { 1.0f,1.0f };		/// まだ使ってない
	instance.rotate = { 0.0f,0.0f,0.0f };  /// まだ使ってない
	instance.layer = 0;					/// まだ使ってない
	instance.drawState = STANDBY;

	auto checker = std::find_if(materialConfigList_.begin(),
		materialConfigList_.end(),
		[&](MaterialConfig* ptr) {return *ptr == material; });

	if (checker != materialConfigList_.end()) {
		instance.materialConfigIndex = (int)std::distance(materialConfigList_.begin(), checker);
	} else {
		MaterialConfig* newMaterial = new MaterialConfig(material);
		materialConfigList_.push_back(newMaterial);
		instance.materialConfigIndex = int(materialConfigList_.size() - 1);
	}

	auto checker2 = std::find_if(tileList_.begin(),
		tileList_.end(),
		[&](SpriteInstance* ptr) {return ptr->CheckSame(instance); });

	if (checker2 != tileList_.end()) {
		(*checker2)->drawState = STANDBY;
	} else {
		SpriteInstance* newInstance = new SpriteInstance(instance);
		tileList_.push_back(newInstance);
	}
	return;
}

void InstanceManager::AddModelInstance(TransformationMatrix* wvpData, MaterialConfig material) {
}

void InstanceManager::AddParticleInstance(TransformationMatrix* wvpData, MaterialConfig material) {
}
