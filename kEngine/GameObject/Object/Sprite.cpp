#include "Sprite.h"


void SimpleSprite::CreateDefaultData() {
	modelHandle_ = 0;

	SimpleSpritePart newObjectPart;
	newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(newObjectPart.materialConfig.get());
	newObjectPart.materialConfig->enableLighting = false;

	objectParts_.push_back(newObjectPart);
}

void SimpleSprite::IntObject(kEngine* system) {
	system_ = system;
}

void SimpleSprite::Draw() {
	system_->Draw2D(this);
}

Vector3 SimpleSprite::TransRotation(float theat) {
	return Vector3(0, 0, theat);
}

Vector3 SimpleSprite::TransTransform(Vector2 Pos) {
	return Vector3( Pos.x, Pos.y, 0);
}

float SimpleSprite::TransLayerDepth(int layer) {
	return layer * layerDepth_Sprite;
}
