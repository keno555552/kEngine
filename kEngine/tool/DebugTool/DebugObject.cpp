#include "DebugObject.h"

DebugObject::DebugObject(kEngine* kEngine) {
	system_ = kEngine;
	TH_centerPoint = system_->LoadTexture("./resources/TemplateResource/texture/centerPoint.png");
}

DebugObject::~DebugObject() {
	if (centerPoint_)delete centerPoint_, centerPoint_ = nullptr;
}

void DebugObject::Update(Camera* camera) {
	if(camera){
		camera_ = camera;
		updateDebugParts();
	}
}

void DebugObject::Draw() {
	if (centerPoint_) { 
		centerPoint_->Draw(); 
	}
}

void DebugObject::SetFollowObject(ObjectPart* followObject) {
	followObject_ = followObject;
}

void DebugObject::ClearFollowObject() {
	followObject_ = nullptr;
}

void DebugObject::SetShowCenterPoint(bool isShow) {
	isShowCenterPoint_ = isShow;
	if (isShowCenterPoint_) {
		if (centerPoint_ == nullptr) {
			centerPoint_ = new SimpleSprite();
			centerPoint_->IntObject(system_);
			centerPoint_->CreateDefaultData();
			centerPoint_->objectParts_[0].anchorPoint = {0.5f,0.5f};
			centerPoint_->objectParts_[0].materialConfig->textureHandle = TH_centerPoint;
		}
	}
}


/// ============= 全体更新 ============= ///
void DebugObject::updateDebugParts() {
	updateCenterPoint();
}
/// 中心点更新 ///
void DebugObject::updateCenterPoint() {
	if (isShowCenterPoint_) {
		if (centerPoint_ != nullptr) {
			Vector2 pos = camera_->GetObjectScreenPos(mainPosition.transform.translate);
			centerPoint_->mainPosition.transform.translate = Vector3(0,0, 0.0f);
		}
	}
}


