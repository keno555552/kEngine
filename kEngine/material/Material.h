#pragma once
#include <stdint.h>
#include "Vector4.h"
#include "Matrix4x4.h"
#include "MaterialConfig.h"

struct Material
{
	Vector4 color;
	int32_t enableLighting;
	float padding[3];

	Matrix4x4 uvTransform;
	float shininess;
	float padding1[3];

	bool operator==(const Material target);
	bool operator!=(const Material target);
	void inputMaterialConfig(MaterialConfig target);
};

