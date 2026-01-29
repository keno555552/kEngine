#include "Logger.h"
#include "DrawDataCollector.h"
#include "CameraManager/CameraManager.h"
#include "LightManager/LightManager.h"
#include "ResourceManager.h"

DrawDataCollector::DrawDataCollector(ResourceManager* rm, InstanceManager* im, CameraManager* cm, LightManager* lm) :
	resourceManager_(rm),
	instanceManager_(im),
	cameraManager_(cm),
	lightManager_(lm)
{}

void DrawDataCollector::PreCollect() {

	/// bucketクリア
	opaqueBuckets2D_.clear();
	transparentObjectParts2D_.clear();
	opaqueBuckets3D_.clear();
	transparentObjectParts3D_.clear();

	/// レイヤードスプライト用リストクリア
	simpleSpriteCounter_ = 0;
	unlayeredSpriteCounter_ = 0;


}

void DrawDataCollector::Collect2D(SpriteData* sprite) {
	/// nullチェック
	if (!sprite)return;

	for (auto& object : sprite->objectParts_) {

		/// ========================================  RenderData作成  ========================================///

		/// metaData取得
		DirectX::TexMetadata metaData = resourceManager_->GetTextureMetaData(object.materialConfig->textureHandle);

		/// スプライトメッシュのリサイズ
		resourceManager_->ResizeSimpleSpriteMesh(
			metaData,
			simpleSpriteCounter_,
			object.conerData,
			object.anchorPoint,
			object.cropLT,
			object.cropSize
		);

		/// RenderData作成
		RenderData renderData;

		/// メッシュ設定
		renderData.mesh = resourceManager_->simpleSpriteMeshList_[simpleSpriteCounter_];

		/// マテリアル設定
		object.materialConfig->MakeUVMatrix();
		renderData.materialID = resourceManager_->InputMaterialConfig(object.materialConfig);

		/// 変換行列設定
		renderData.transformData = SpriteWVPAdjustment(*sprite, object);

		/// PSO設定
		renderData.psoID = PSODecision(*object.materialConfig);

		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddSpriteToBucket(renderData, simpleSpriteCounter_);

		/// スプライトカウンターインクリメント
		simpleSpriteCounter_++;
	}
}



void DrawDataCollector::Collect3D(ObjectData* object) {
	/// nullチェック
	if (!object)return;

	for (auto& objectPart : object->objectParts_) {

		/// ========================================  RenderData作成  ========================================///

		/// RenderData作成
		RenderData renderData;

		/// メッシュ設定
		renderData.mesh = resourceManager_->meshBufferList_[object->modelHandle_];

		/// マテリアル設定
		objectPart.materialConfig->MakeUVMatrix();
		renderData.materialID = resourceManager_->InputMaterialConfig(objectPart.materialConfig);

		/// 変換行列設定
		renderData.transformData = ObjectWVPAdjustment(*object, objectPart);

		/// PSO設定
		renderData.psoID = PSODecision(*objectPart.materialConfig);

		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddObjectToBucket(renderData, object->modelHandle_);
	}
}

Vector3 DrawDataCollector::GetCameraPosition() const {
	return cameraManager_->GetActiveCamera()->GetTransform().translate;
}

float DrawDataCollector::SpriteLayerManagement(float zBuffer) {

	if (zBuffer == 0) {
		if (unlayeredSpriteCounter_ > 4000) {
			Logger::Log("[kWarning]DDC:unlayered Sprite meet max number!");
			return (float)unlayeredSpriteCounter_ * -layerDepth_Sprite;
		}
		float z = (float)unlayeredSpriteCounter_ * -layerDepth_Sprite;
		unlayeredSpriteCounter_++;
		return z;
	} else {
		float z = -layeredSpriteDepth_ + zBuffer * -layerDepth_Sprite;
		if (z < -1.0f) {
			Logger::Log("[kWarning]DDC:layered Sprite meet max number!");
			return -1.0f;
		}
		return z;
	}
	Logger::Log("[kWarning]DDC:Unknow Error!");
	return 0.0f;
}

Matrix4x4 DrawDataCollector::MakeFollowObjectMatrix(SpriteData* sprite) {

	float zBuffer = sprite->mainPosition.transform.translate.z;

	Matrix4x4 objectMainMatrix = MakeAffineMatrix(
		sprite->mainPosition.transform.scale,
		sprite->mainPosition.transform.rotate,
		sprite->mainPosition.transform.translate
	);

	Matrix4x4 parentMatrix = Identity();

	SpriteData* parent = sprite->followObject_;
	while (parent != nullptr) {
		Matrix4x4 local = MakeAffineMatrix(
			parent->mainPosition.transform.scale,
			parent->mainPosition.transform.rotate,
			parent->mainPosition.transform.translate
		);

		parentMatrix = local * parentMatrix;
		parent = parent->followObject_;
	}

	Matrix4x4 resultMatrix = objectMainMatrix * parentMatrix;
	resultMatrix.m[3][2] = zBuffer;

	return resultMatrix;
}

TransformationMatrix DrawDataCollector::SpriteWVPAdjustment(SpriteData& sprite, SpritePart& part) {

	// 2D UI 用：View 取單位矩陣，Projection 用螢幕尺寸的正交矩陣（左上原點，Y 向下）
	Matrix4x4 viewMatrix = Identity();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(
		0.0f,                                     // left
		0.0f,                                     // top
		static_cast<float>(config::GetClientWidth()),   // right
		static_cast<float>(config::GetClientHeight()),  // bottom
		-1.0f,                                    // nearZ，留空間給負的 layer depth
		1.0f                                      // farZ
	);

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix(&sprite);

	Matrix4x4 localMatrix = MakeAffineMatrix(
		part.transform.scale,
		part.transform.rotate,
		part.transform.translate
	);

	Matrix4x4 worldMatrix = localMatrix * followWorldMatrix;

	TransformationMatrix result{};
	result.world = worldMatrix;
	result.WVP = worldMatrix * viewMatrix * projectionMatrix;
	result.WorldInverseTranspose = worldMatrix.Inverse().Transpose();
	return result;
}

Matrix4x4 DrawDataCollector::MakeFollowObjectMatrix(ObjectData* object) {

	Matrix4x4 objectMainMatrix = MakeAffineMatrix(
		object->mainPosition.transform.scale,
		object->mainPosition.transform.rotate,
		object->mainPosition.transform.translate
	);

	Matrix4x4 parentMatrix = Identity();

	ObjectPart* parent = object->followObject_;
	while (parent != nullptr) {
		Matrix4x4 local = MakeAffineMatrix(
			parent->transform.scale,
			parent->transform.rotate,
			parent->transform.translate
		);

		parentMatrix = local * parentMatrix;
		parent = parent->parentPart;
	}

	return objectMainMatrix * parentMatrix;
}

TransformationMatrix DrawDataCollector::ObjectWVPAdjustment(ObjectData& object, ObjectPart& part) {
	Camera* cam = cameraManager_->GetActiveCamera();
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix(&object);

	// Billboard 子物件：XY 旋轉を0にする
	if (object.isBillboard_) {
		Vector3 camRot = cam->GetTransform().rotate;
		part.transform.rotate.x = camRot.x;
		part.transform.rotate.y = camRot.y;
		part.transform.rotate.z = 0.0f;
	}

	Matrix4x4 localMatrix = MakeAffineMatrix(
		part.transform.scale,
		part.transform.rotate,
		part.transform.translate
	);

	//Matrix4x4 worldMatrix = localMatrix * partParentMatrix * followWorldMatrix;
	Matrix4x4 worldMatrix = localMatrix * followWorldMatrix;
	
	TransformationMatrix result{};
	result.WVP = worldMatrix * viewMatrix * projectionMatrix;
	result.world = worldMatrix;
	result.WorldInverseTranspose = worldMatrix.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddSpriteToBucket(RenderData& renderData,int meshID) {

	auto checker = resourceManager_->idToIndex_.find(renderData.materialID);

	if (checker == resourceManager_->idToIndex_.end()) {
		Logger::Log("[kError]DDC:MaterialID not found in ResourceManager!");
		return;
	} else {
		Material* material = resourceManager_->materialList_[checker->second].cpuMaterial.get();

		if (material->color.w < 1.0f) {

			float z = renderData.transformData.world.m[3][2];

			auto checker2 = std::find_if(
				transparentObjectParts2D_.begin(),
				transparentObjectParts2D_.end(),
				[z](const RenderData& data) {
					float dataZ = data.transformData.world.m[3][2];
					return z > dataZ;
				}
			);

			if (checker2 == transparentObjectParts2D_.end()) {
				transparentObjectParts2D_.emplace_back(renderData);
			} else {
				std::size_t index = std::distance(transparentObjectParts2D_.begin(), checker2);
				transparentObjectParts2D_.insert(transparentObjectParts2D_.begin() + index, renderData);
			}
		} else {
			/// 不透明オブジェクトバケットへ追加
			opaqueBuckets2D_[static_cast<PSOType>(renderData.psoID)][renderData.materialID][meshID].emplace_back(renderData);
		}
	}
}

void DrawDataCollector::AddObjectToBucket(RenderData& renderData,int meshID) {

	auto checker = resourceManager_->idToIndex_.find(renderData.materialID);

	if (checker == resourceManager_->idToIndex_.end()) {
		Logger::Log("[kError]DDC:MaterialID not found in ResourceManager!");
		return;
	} else {
		Material* material = resourceManager_->materialList_[checker->second].cpuMaterial.get();

		if (material->color.w < 1.0f) {

			float z = renderData.transformData.world.m[3][2];

			auto checker2 = std::find_if(
				transparentObjectParts3D_.begin(),
				transparentObjectParts3D_.end(),
				[z](const RenderData& data) {
					float dataZ = data.transformData.world.m[3][2];
					return z < dataZ;
				}
			);

			if (checker2 == transparentObjectParts3D_.end()) {
				transparentObjectParts3D_.emplace_back(renderData);
			} else {
				std::size_t index = std::distance(transparentObjectParts3D_.begin(), checker2);
				transparentObjectParts3D_.insert(transparentObjectParts3D_.begin() + index, renderData);
			}
		} else {
			/// 不透明オブジェクトバケットへ追加
			opaqueBuckets3D_[static_cast<PSOType>(renderData.psoID)][renderData.materialID][meshID].emplace_back(renderData);
		}
	}
}

uint32_t DrawDataCollector::PSODecision(MaterialConfig& material) {

	LightModelType lightModelType = (LightModelType)(int)material.lightModelType;

	switch (lightModelType) {
	case LightModelType::Sprite2D:
		return (uint32_t)PSOType::Sprite2D;
	case LightModelType::Lambert:
		return (uint32_t)PSOType::Lambert;
	case LightModelType::HalfLambert:
		return (uint32_t)PSOType::HalfLambert;
	case LightModelType::PhongReflection:
		return (uint32_t)PSOType::PhongReflection;
	case LightModelType::BlinnPhongReflection:
		return (uint32_t)PSOType::BlinnPhongReflection;
	case LightModelType::FlameNeonGlow:
		return (uint32_t)PSOType::FlameNeonGlow;
	}
	return (uint32_t)PSOType::NONE;
}

void DrawDataCollector::UpdateLightData() {
	if(lightManager_)lightManager_->TurnDataToGPUData();
}

std::vector<LightGPU> DrawDataCollector::GetLightGPUBuffer() {
	return lightManager_->GetGPUBuffer();
}

uint32_t DrawDataCollector::GetLightCount() {
	return (uint32_t)lightManager_->GetLightCount();
}
