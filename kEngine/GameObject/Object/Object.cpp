#include "Object.h"

void Object::CreateDefaultData() {
	modelHandle = 0;
	MaterialConfig newMaterialConfig;
	InitMaterialConfig(&newMaterialConfig);
	newMaterialConfig.lightModelType = LightModelType::HalfLambert;
	newMaterialConfig.enableLighting = false;
	materialConfig.push_back(newMaterialConfig);
}

void Object::Update(Camera* camera) {
	transformationMatrix = camera->transformationMatrixTransform(transform);
	for (auto& ptr2 : materialConfig) {
		ptr2.uvTransformMatrix = MakeAffineMatrix(ptr2.uvScale,
			ptr2.uvRotate,
			ptr2.uvTranslate);
	}
}
