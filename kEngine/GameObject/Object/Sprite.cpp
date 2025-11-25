#include "Sprite.h"


void SimpleSprite::CreateDefaultData() {
	modelHandle_ = 0;


	mainPosition.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(mainPosition.materialConfig.get());

	SimpleSpritePart newObjectPart;
	newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(newObjectPart.materialConfig.get());
	newObjectPart.materialConfig->enableLighting = false;
	newObjectPart.materialConfig->MakeUVMatrix();

	objectParts_.push_back(newObjectPart);
}

void SimpleSprite::IntObject(kEngine* system) {
	system_ = system;
}

void SimpleSprite::Update(Camera* camera) {

	for (auto& ptr : objectParts_) {
		ptr.materialConfig->MakeUVMatrix();
	}

	mainPosition.materialConfig->MakeUVMatrix();


	for (auto& part : objectParts_) {

		if (part.anchorPoint != Vector2{ 0,0 }) {
			float left = 0.0f - part.anchorPoint.x;
			float right = 1.0f - part.anchorPoint.x;
			float top = 0.0f - part.anchorPoint.y;
			float bottom = 1.0f - part.anchorPoint.y;
		}


		part.worldTransform.scale		= part.transform.scale * mainPosition.transform.scale;
		part.worldTransform.rotate		= part.transform.rotate + mainPosition.transform.rotate;
		part.worldTransform.translate = part.transform.translate + mainPosition.transform.translate 
										+ Vector3{part.anchorPoint.x,part.anchorPoint.y,0}
										+ Vector3{ mainPosition.anchorPoint.x,mainPosition.anchorPoint.y,0};

		part.materialConfig->MakeUVMatrix();
	}

}

void SimpleSprite::Draw() {
	system_->Draw2D(this);
}

Vector3 SimpleSprite::TransRotation(float theat) {
	return Vector3(0, 0, theat);
}

Vector3 SimpleSprite::TransTransform(Vector2 Pos) {
	return Vector3(Pos.x, Pos.y, 0);
}

float SimpleSprite::TransLayerDepth(int layer) {
	return layer * layerDepth_Sprite;
}
