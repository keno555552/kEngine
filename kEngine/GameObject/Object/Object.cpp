#include "Object.h"

void Object::CreateDefaultData() {
	modelHandle_ = 0;

	ObjectPart newObjectPart;
	newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(newObjectPart.materialConfig.get());
	newObjectPart.materialConfig->lightModelType = LightModelType::HalfLambert;
	newObjectPart.materialConfig->enableLighting = true;

	objectParts_.push_back(newObjectPart);
}

void Object::IntObject(kEngine* system) {
	system_ = system;
}

void Object::Update(Camera* camera) {

	Matrix4x4 objectWorldMatrix = MakeAffineMatrix(
		mainPosition.transform.scale,
		mainPosition.transform.rotate,
		mainPosition.transform.translate
	);
	mainPosition.transformationMatrix = camera->transformationMatrixTransform(objectWorldMatrix);


	for (auto& part : objectParts_) {
		Matrix4x4 localMatrix = MakeAffineMatrix(
			part.transform.scale,
			part.transform.rotate,
			part.transform.translate
		);

		Matrix4x4 worldMatrix =  localMatrix * objectWorldMatrix;
		part.transformationMatrix = camera->transformationMatrixTransform(worldMatrix);
		part.materialConfig->MakeUVMatrix();
	}

}

void Object::Draw() {
	system_->Draw3D(this);
}

void Object::CreateModelData(int modelHandle) {
	modelHandle_ = modelHandle;
	int numOfPart = system_->GetMuitModelNum(modelHandle_);

	mainPosition.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(mainPosition.materialConfig.get());

	objectParts_.clear();

	for (int i = 0; i < numOfPart; i++) {
		ObjectPart newObjectPart;
		newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
		InitMaterialConfig(newObjectPart.materialConfig.get());
		newObjectPart.materialConfig->lightModelType = LightModelType::HalfLambert;
		newObjectPart.materialConfig->enableLighting = true;
		newObjectPart.materialConfig->useOriginalTexture = true;
		newObjectPart.materialConfig->textureHandle = system_->GetModelTextureHandle(modelHandle,i);
		objectParts_.push_back(newObjectPart);
	}
}

