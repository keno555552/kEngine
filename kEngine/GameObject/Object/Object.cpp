#include "Object.h"

#include "kEngine.h"
#include "Vector3.h"

#ifdef _DEBUG
#include "CheatEngine/ObjectEditor/ObjectRegistrant.h"
#endif

ObjectPart* Object::FindPartByName(const std::string& name) {

	for (auto& part : objectParts_) {
		if (part.name == name) return &part;
	}
	Logger::Log("Can't Find Part '%s' ", name.c_str());
	return nullptr;

}

void Object::CreateDefaultData() {
	modelHandle_ = 0;

	ObjectPart newObjectPart;
	newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(newObjectPart.materialConfig.get());
	newObjectPart.materialConfig->lightModelType = LightModelType::HalfLambert;
	newObjectPart.materialConfig->enableLighting = true;

	objectParts_.push_back(newObjectPart);
}

Object::~Object() {

#ifdef _DEBUG
	ObjectRegistrant::GetInstance().Unregister(this);
#endif


	if (system_) {
	system_->ClearModelRoot(this);
	}
}

void Object::IntObject(kEngine* system) {
	system_ = system;
	ObjectData::ownerObject = this;

#ifdef _DEBUG
	ObjectRegistrant::GetInstance().Register(this);
#endif
}

void Object::Update(Camera* camera) {
}

void Object::Draw() {
	system_->Draw3D(this);
}

void Object::CreateModelData(int modelHandle) {
	modelHandle_ = modelHandle;
	int numOfPart = system_->GetMutiModelNum(modelHandle_);

	mainPosition.materialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(mainPosition.materialConfig.get());

	objectParts_.clear();

	for (int i = 0; i < numOfPart; i++) {
		ObjectPart newObjectPart;

		auto modelData = system_->GetModel(modelHandle, i)->GetModelData().get();

		newObjectPart.name = modelData->meshDataList[i].name;
		newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
		InitMaterialConfig(newObjectPart.materialConfig.get());
		newObjectPart.materialConfig->lightModelType = LightModelType::HalfLambert;
		newObjectPart.materialConfig->enableLighting = true;
		newObjectPart.materialConfig->useModelTexture = true;
		newObjectPart.materialConfig->textureHandle = system_->GetModelTextureHandle(modelHandle, i);
		objectParts_.push_back(newObjectPart);
	}

	/// Modelに必要なものルードを準備する
	// 今はskinning用な行列を生成だけ
	system_->CreateModelRoot(this);
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
	modelHandle_ = target->modelHandle_;
	isBillboard_ = target->isBillboard_;
	ownerObject = target->ownerObject;
	followObject_ = target->followObject_;
	for (auto& ptr : target->objectParts_) {
		CreateDefaultData();
		objectParts_.back().name = ptr.name;
		objectParts_.back().transform = ptr.transform;
		objectParts_.back().materialConfig = ptr.materialConfig;
		objectParts_.back().parentPart = ptr.parentPart;
		objectParts_.back().forward = ptr.forward;
	}
}

