#pragma once
#include <stdint.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "LightModelType.h"

class MaterialConfig
{
public:
	/// Texture関連
	bool useModelTexture = true;
	int textureHandle = 0;

	/// UV関連
	Vector3 uvTranslate = { 0,0,0 };
	Vector3 uvScale = { 1,1,1 };
	Vector3 uvRotate = { 0,0,0 };
	Matrix4x4 uvTransformMatrix{};
	Vector4 textureColor{ 1.0f,1.0f,1.0f,1.0f };

	/// Lighting関連
	LightModelType lightModelType = LightModelType::HalfLambert;
	bool enableLighting = true;

	/// 内部処理用
	int materialResourceHandle = -1;
	int drawState = 0;

	/// Phong反射モデル用
	float shininess = 32.0f;

	bool operator==(const MaterialConfig target);
	bool operator!=(const MaterialConfig target);

	void MakeUVMatrix();
};

/// MaterialConfig初期化
void InitMaterialConfig(MaterialConfig* materialConfig);
MaterialConfig InitMaterialConfig();
