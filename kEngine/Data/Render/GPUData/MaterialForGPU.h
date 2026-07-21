#pragma once
#include <stdint.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Data/Render/CPUData/MaterialConfig.h"
#include "config.h"

struct MaterialLayerGPU
{
	Vector4 color;
	Vector4 color2;
	Vector4 color3;
	// ------ padding counter line-------
	Matrix4x4 uvTransform;
	// ------ padding counter line-------
	float shininess;
	float intensity;
	float heightScale;
	float reflectiveStrength;
	int32_t enableLighting;
	float padding[3];
};

struct MaterialForGPU
{
	MaterialLayerGPU layers[3];
	// ------ padding counter line-------
	int32_t layerCount;
	int32_t enableLighting;               //全部のレイヤーに影響する
	float padding[2];
	// ------ padding counter line-------

	bool operator==(const MaterialForGPU target);
	bool operator!=(const MaterialForGPU target);
	void inputMaterialConfig(MaterialConfig target);

};


//struct MaterialForGPU
//{
//	Vector4 color;
//	Vector4 color2;
//	Vector4 color3;
//
//	int32_t enableLighting;
//	float padding[3];
//
//	Matrix4x4 uvTransform;
//	float shininess;
//	float intensity;
//	float heightScale;
//
//	float reflectiveStrength;
//
//	//float extra[4];
//
//	bool operator==(const MaterialForGPU target);
//	bool operator!=(const MaterialForGPU target);
//	void inputMaterialConfig(MaterialConfig target);
//
//};
