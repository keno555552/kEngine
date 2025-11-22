#include "MaterialConfig.h"

bool MaterialConfig::operator==(const MaterialConfig target) {
	if (useModelTexture != target.useModelTexture)return false;
	if (textureHandle != target.textureHandle)return false;
	if (uvTranslate != target.uvTranslate)return false;
	if (uvScale != target.uvScale)return false;
	if (uvRotate != target.uvRotate)return false;
	if (textureColor != target.textureColor)return false;
	if (lightModelType != target.lightModelType)return false;
	if (enableLighting != target.enableLighting)return false;
	return true;
}

bool MaterialConfig::operator!=(const MaterialConfig target) {
	return !(*this == target);
}

void MaterialConfig::MakeUVMatrix() {
	uvTransformMatrix = MakeAffineMatrix(uvScale, uvRotate, uvTranslate);
}

void InitMaterialConfig(MaterialConfig* materialConfig) {
	materialConfig->useModelTexture = true;
	materialConfig->textureHandle = 0;
	materialConfig->uvTranslate = { 0,0,0 };
	materialConfig->uvScale = { 1,1,1 };
	materialConfig->uvRotate = { 0,0,0 };
	materialConfig->uvTransformMatrix = {};
	materialConfig->textureColor = { 1.0f,1.0f,1.0f,1.0f };
	materialConfig->lightModelType = LightModelType::HalfLambert;
	materialConfig->enableLighting = true;
	materialConfig->materialResourceHandle = -1;
	materialConfig->drawState = 0;
}

MaterialConfig InitMaterialConfig() {
	MaterialConfig newMaterialConfig;
	InitMaterialConfig(&newMaterialConfig);
	return newMaterialConfig;
}
