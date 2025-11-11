#pragma once
#include <vector>
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MaterialConfig.h"
#include "MathsIncluder.h"
#include "drawEngine.h"
#include "Camera.h"

class Object {
public:
	Transform transform = CreateDefaultTransform();

	Matrix4x4 worldMatrix = {};

	Matrix4x4 worldViewProjectionMatrix{};

	TransformationMatrix transformationMatrix{};

	std::vector<MaterialConfig> materialConfig;

	int modelHandle = 0;

	bool isDelete = false;

public:
	
	void CreateDefaultData();
	
	void Update(Camera* camera);
};
