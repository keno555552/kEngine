#pragma once
#include <vector>
#include "MathsIncluder.h"
#include <memory>
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MaterialConfig.h"
//#include "drawEngine.h"

struct ObjectPart {
	Transform transform = CreateDefaultTransform();

	TransformationMatrix transformationMatrix{};

	std::shared_ptr<MaterialConfig>materialConfig;

	ObjectPart* parentPart = nullptr;
};


class ObjectData {
public:
	std::vector<ObjectPart> objectParts_;

	ObjectPart mainPosition;

	int modelHandle_ = 0;

	bool isDelete_ = false;

	ObjectPart followObject_;
};

