#include "MaterialForGPU.h"

/// NOTE: 今はまだ1レイヤーしか使ってないから、layers[0]のデータを使う

bool MaterialForGPU::operator==(const MaterialForGPU target) {

	/// MaterialForGPUを設定する
	if(layerCount != target.layerCount) return false;

	/// layerことcopyする
	for(int i = 0; i < layerCount; i++){
		if (layers[i].color != target.layers[i].color)return false;
		if (layers[i].color2 != target.layers[i].color2)return false;
		if (layers[i].color3 != target.layers[i].color3)return false;

		if (layers[i].uvTransform != target.layers[i].uvTransform)return false;

		if (layers[i].shininess != target.layers[i].shininess)return false;
		if (layers[i].intensity != target.layers[i].intensity)return false;
		if (layers[i].heightScale != target.layers[i].heightScale)return false;
		if (layers[i].reflectiveStrength != target.layers[i].reflectiveStrength)return false;

		if (layers[i].enableLighting != target.layers[i].enableLighting)return false;
	}


	return true;
}
bool MaterialForGPU::operator!=(const MaterialForGPU target) {
	return !(*this == target);
}

void MaterialForGPU::inputMaterialConfig(MaterialConfig target) {

	/// MaterialForGPUを設定する
	layerCount = 1;

	if(target.enableLighting) layers[0].enableLighting = 1;
	else layers[0].enableLighting = 0;

	/// layer[0]にデイタをcopyする
	layers[0].color = target.textureColor;
	layers[0].color2 = target.textureColor2;
	layers[0].color3 = target.textureColor3;
	layers[0].enableLighting = target.enableLighting;
	layers[0].uvTransform = target.uvTransformMatrix;
	layers[0].shininess = target.shininess;
	layers[0].intensity = target.intensity;
	layers[0].heightScale = target.heightScale;
	layers[0].reflectiveStrength = target.reflectiveStrength;
}

//bool MaterialForGPU::operator==(const MaterialForGPU target) {
//
//	if (color != target.color)return false;
//	if (color2 != target.color2)return false;
//	if (color3 != target.color3)return false;
//
//	if (uvTransform != target.uvTransform)return false;
//
//	if (shininess != target.shininess)return false;
//	if (intensity != target.intensity)return false;
//	if (heightScale != target.heightScale)return false;
//	if (reflectiveStrength != target.reflectiveStrength)return false;
//
//	if (enableLighting != target.enableLighting)return false;
//
//
//	return true;
//}
//
//bool MaterialForGPU::operator!=(const MaterialForGPU target) {
//	return !(*this == target);
//}
//
//void MaterialForGPU::inputMaterialConfig(MaterialConfig target) {
//
//	/// layer[0]にデイタをcopyする
//	color = target.textureColor;
//	color2 = target.textureColor2;
//	color3 = target.textureColor3;
//	enableLighting = target.enableLighting;
//	uvTransform = target.uvTransformMatrix;
//	shininess = target.shininess;
//	intensity = target.intensity;
//	heightScale = target.heightScale;
//	reflectiveStrength = target.reflectiveStrength;
//}