#include "Object.h"
#include "Vector3.h"
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

	Matrix4x4 parentMatrix = Identity();
	if (followObject_!= nullptr) {
		parentMatrix = MakeAffineMatrix(
			followObject_->transform.scale,
			followObject_->transform.rotate,
			followObject_->transform.translate
		);
	}

	Matrix4x4 objectMainMatrix = MakeAffineMatrix(
		mainPosition.transform.scale,
		mainPosition.transform.rotate,
		mainPosition.transform.translate
	);
	Matrix4x4 objectWorldMatrix = objectMainMatrix * parentMatrix;
	mainPosition.transformationMatrix = camera->transformationMatrixTransform(objectWorldMatrix);

	for (auto& part : objectParts_) {
		Matrix4x4 objectParentMatrix = Identity();
		if (part.parentPart != nullptr) {
			objectParentMatrix = MakeAffineMatrix(
				part.parentPart->transform.scale,
				part.parentPart->transform.rotate,
				part.parentPart->transform.translate
			);
		}

		Matrix4x4 localMatrix = MakeAffineMatrix(
			part.transform.scale,
			part.transform.rotate,
			part.transform.translate
		);

		Matrix4x4 worldMatrix = localMatrix * objectParentMatrix  * objectWorldMatrix;
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
		newObjectPart.materialConfig->useModelTexture = true;
		newObjectPart.materialConfig->textureHandle = system_->GetModelTextureHandle(modelHandle,i);
		objectParts_.push_back(newObjectPart);
	}

}
// �Փ˂����I�u�W�F�N�g�̃|�C���^�[��n��
Object* Object::Collision(const std::vector<Object*>& obj)
{
	return nullptr;
}
// �Փ˂����Ƃ��߂荞�܂Ȃ��ꍇ
void Object::Conflict(const std::vector<Object*>& obj)
{
}
Object* Object::Collision(const Object* obj)
{
	return nullptr;
}
AABB Object::GetAABB(const Object* obj) const
{
  
	//// ���[���h���W�̒��S�ʒu��v�Z
	Vector3 pos = obj->mainPosition.transform.translate ;
 //   
 //   // ���C��: �X�P�[���̍����i�e�̃X�P�[�� �~ �q�̃X�P�[���j
    Vector3 scale = obj->mainPosition.transform.scale;
 
    
   //Vector3 pos = obj->objectParts_[0].transform.translate;
   // 
   // // �I�u�W�F�N�g�̃X�P�[���i���E�����E���s���̔����Ɖ���j
   // // �����f����1x1x1�̗����̂ł���O��ł��B
   // // ���f�����傫���ꍇ�́AScale * 2.0f �Ȃǒ������Ă��������B
   // Vector3 scale = obj->objectParts_[0].transform.scale ; 

    // 半径（Half extents）にするため 0.5倍
    scale *= 0.5f;

    AABB box;
    box.min = { pos.x - scale.x, pos.y - scale.y, pos.z - scale.z };
    box.max = { pos.x + scale.x, pos.y + scale.y, pos.z + scale.z };
    return box;
}
void Object::CopyObject(Object* target) {
	if (target == nullptr)return;
	mainPosition = target->mainPosition;
	followObject_ = target->followObject_;
	for (auto& ptr : target->objectParts_) {
		CreateDefaultData();
		objectParts_.back().transform = ptr.transform;
		objectParts_.back().materialConfig = ptr.materialConfig;
		objectParts_.back().parentPart = ptr.parentPart;
	}
}

