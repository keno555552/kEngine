#include "MaterialConfig.h"


void intMaterialConfig(MaterialConfig* materialConfig) {
	materialConfig->useOriginalTexture = true;
	materialConfig->textureHandle = 0,
	materialConfig->uvTranslate = { 0,0,0 },
	materialConfig->uvScale = { 1,1,1 },
	materialConfig->uvRotate = { 0,0,0 },
	materialConfig->uvTransformMatrix = {},
	materialConfig->textureColor = { 1.0f,1.0f,1.0f,1.0f },
	materialConfig->lightModelType = LightModelType::HalfLambert,
	materialConfig->enableLighting = true;
};

bool MaterialConfig::operator==(const MaterialConfig target) {
	if (useOriginalTexture != target.useOriginalTexture)return false;
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
