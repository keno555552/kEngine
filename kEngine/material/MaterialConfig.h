#pragma once
#include <stdint.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "LightModelType.h"

struct MaterialConfig
{
	bool useOriginalTexture = true;
	int textureHandle = 0;

	Vector3 uvTranslate = { 0,0,0 };
	Vector3 uvScale = { 1,1,1 };
	Vector3 uvRotate = { 0,0,0 };
	Matrix4x4 uvTransformMatrix{};
	Vector4 textureColor{ 1.0f,1.0f,1.0f,1.0f };

	LightModelType lightModelType = LightModelType::HalfLambert;
	bool enableLighting = true;

	int materialResourceHandle = -1;
	int drawState = 0;

	bool operator==(const MaterialConfig target);
	bool operator!=(const MaterialConfig target);

	void MakeUVMatrix();

};

/// MaterialConfig初期化
void InitMaterialConfig(MaterialConfig* materialConfig);
MaterialConfig InitMaterialConfig();
