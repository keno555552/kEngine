#pragma once
#include <vector>
#include <memory>
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MaterialConfig.h"
#include "MathsIncluder.h"
#include "kEngine.h"
#include "Camera.h"
#include "ObjectData.h"


class Object : public ObjectData {
public:
	void IntObject(kEngine* system);

	virtual void Update(Camera* camera);

	int GetPartIndex()const { return (int)objectParts_.size(); }

	void CreateDefaultData();
	void CreateModelData(int modelHandle);

private:
	kEngine* system_ = nullptr;
};
