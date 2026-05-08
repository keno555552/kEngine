#include "Logger.h"
#include "DrawDataCollector.h"
#include "CameraManager/CameraManager.h"
#include "LightManager/LightManager.h"
#include "Resource/ResourceManager.h"


void DrawDataCollector::Initialize(CameraManager* cm, LightManager* lm, AnimationManager* am) {
	cameraManager_ = cm;
	lightManager_ = lm;
	animationManager_ = am;
}

void DrawDataCollector::Finalize() {
	/// bucketクリア
	opaqueBucket2D_.clear();
	transparentBucket2D_.clear();
	opaqueBucket3D_.clear();
	transparentBucket3D_.clear();
	bucketParticleC_.clear();

	instanceCounter2D_ = 0;
	instanceCounter3D_ = 0;
	instanceCounterParticleC_ = 0;

	instancingList2D_ = nullptr;
	instancingList3D_ = nullptr;
	instancingListParticleC_ = nullptr;
}

void DrawDataCollector::PreCollect() {

	/// bucketクリア
	opaqueBucket2D_.clear();
	transparentBucket2D_.clear();
	opaqueBucket3D_.clear();
	transparentBucket3D_.clear();

	memset(instancingListDL_, 0, sizeof(TransformationMatrix) * maxDebugLineInstance);
	debugLinesVertexBucket_.clear();

	/// レイヤードスプライト用リストクリア
	simpleSpriteCounter_ = 0;
	unlayeredSpriteCounter_ = 0;

	/// skinning用リストクリア
	skinningDataList_ = {};

	/// インスタンスリストクリア
	instanceCounterDL_ = 0;
	instanceCounter2D_ = 0;
	instanceCounter3D_ = 0;
	instanceCounterParticleC_ = 0;
}

void DrawDataCollector::EndCollect() {

	/// ネールスキップ
	if (opaqueBucket2D_.empty() && transparentBucket2D_.empty() &&
		opaqueBucket3D_.empty() && transparentBucket3D_.empty())
		return;

	/// 実際のインスタンスリスト作成
	BuildInstanceList2D();
	BuildInstanceList3D();

}


#pragma region /// ================================== DebugLine関連 ================================= ///


TransformationMatrix DrawDataCollector::DLWVPAdjustment(DebugLine* debugLine) {
	Camera* cam = cameraManager_->GetActiveCamera();
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();

	Matrix4x4 worldMatrix = Identity(); // 因為點已經是世界座標

	TransformationMatrix result{};
	result.WVP = worldMatrix * viewMatrix * projectionMatrix;
	result.world = worldMatrix;
	result.WorldInverseTranspose = worldMatrix.Inverse().Transpose();
	return result;
}

void DrawDataCollector::CollectDebugLine(DebugLine* debugLine) {

	/// lineDataをGPU用の構造体に変換
	DebugLineVertexGPU newData[2];
	newData[0].startPoint = debugLine->startPoint;
	newData[0].color = debugLine->color;
	newData[1].startPoint = debugLine->endPoint;
	newData[1].color = debugLine->color;
	for (auto& point : newData) {
		debugLinesVertexBucket_.push_back(point);
	}

	/// TransformMatrixを作るためのデータ収集
	TransformationMatrix DLTM = DLWVPAdjustment(debugLine);
	instancingListDL_[instanceCounterDL_].world = DLTM.world;
	instancingListDL_[instanceCounterDL_].WVP = DLTM.WVP;
	instancingListDL_[instanceCounterDL_].WorldInverseTranspose = DLTM.WorldInverseTranspose;
	instanceCounterDL_++;
}


#pragma region /// ===================================== 2D関連 ===================================== ///

void DrawDataCollector::Collect2D(SpriteData* sprite) {
	/// nullチェック
	if (!sprite)return;

	for (auto& object : sprite->objectParts_) {

		/// ========================================  RenderData作成  ========================================///

		/// metaData
		DirectX::TexMetadata metaData = ResourceManager::GetInstance()->GetTextureMetaData(object.materialConfig->textureHandle);

		/// スプライトメッシュのリサイズ
		ResourceManager::GetInstance()->ResizeSimpleSpriteMesh(
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
		renderData.mesh = ResourceManager::GetInstance()->simpleSpriteMeshList_[simpleSpriteCounter_].get();

		/// マテリアル設定
		object.materialConfig->MakeUVMatrix();
		renderData.materialID = ResourceManager::GetInstance()->InputMaterialConfig(object.materialConfig);

		/// 変換行列設定
		renderData.transformData = SpriteWVPAdjustment2D(*sprite, object);

		/// PSO設定
		renderData.psoKey = PSODecision(*object.materialConfig);

		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddSpriteToBucket2D(renderData, simpleSpriteCounter_);

		/// スプライトカウンターインクリメント
		simpleSpriteCounter_++;
	}
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

Matrix4x4 DrawDataCollector::MakeFollowObjectMatrix2D(SpriteData* sprite) {

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

TransformationMatrix DrawDataCollector::SpriteWVPAdjustment2D(SpriteData& sprite, SpritePart& part) {

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

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix2D(&sprite);

	Matrix4x4 localMatrix = MakeAffineMatrix(
		part.transform.scale,
		part.transform.rotate,
		part.transform.translate
	);

	Matrix4x4 worldMatrix = localMatrix * followWorldMatrix;

	float spriteTransform = part.transform.translate.z;
	worldMatrix.m[3][2] = SpriteLayerManagement(spriteTransform);

	TransformationMatrix result{};
	result.world = worldMatrix;
	result.WVP = worldMatrix * viewMatrix * projectionMatrix;
	result.WorldInverseTranspose = worldMatrix.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddSpriteToBucket2D(RenderData& renderData, int meshID) {

	auto checker = ResourceManager::GetInstance()->idToIndex_.find(renderData.materialID);

	if (checker == ResourceManager::GetInstance()->idToIndex_.end()) {
		Logger::Log("[kError]DDC:MaterialID not found in ResourceManager!");
		return;
	} else {
		MaterialForGPU* material = ResourceManager::GetInstance()->materialList_[checker->second].cpuMaterial.get();

		if (material->color.w < 1.0f) {

			float z = renderData.transformData.WVP.m[3][2];

			auto checker2 = std::find_if(
				transparentBucket2D_.begin(),
				transparentBucket2D_.end(),
				[z](const RenderData& data) {
					float dataZ = data.transformData.WVP.m[3][2];
					return z > dataZ;
				}
			);

			if (checker2 == transparentBucket2D_.end()) {
				transparentBucket2D_.emplace_back(renderData);
			} else {
				std::size_t index = std::distance(transparentBucket2D_.begin(), checker2);
				transparentBucket2D_.insert(transparentBucket2D_.begin() + index, renderData);
			}
		} else {
			/// 不透明オブジェクトバケットへ追加
			opaqueBucket2D_[static_cast<PSOKey>(renderData.psoKey)][renderData.materialID][meshID].emplace_back(renderData);
		}
	}
}

#pragma endregion

#pragma region /// ===================================== 3D関連 ===================================== ///

void DrawDataCollector::Collect3D(ObjectData* object) {
	/// nullチェック
	if (!object)return;

	int modelCounter = 0;
	for (auto& objectPart : object->objectParts_) {

		/// ========================================  RenderData作成  ========================================///

		/// RenderData作成
		RenderData renderData;

		/// メッシュ設定
		Model* modelData = ResourceManager::GetInstance()->modelGroupList_[object->modelHandle_]->GetModel(modelCounter);
		renderData.mesh = modelData;

		/// マテリアル設定
		objectPart.materialConfig->MakeUVMatrix();
		renderData.materialID = ResourceManager::GetInstance()->InputMaterialConfig(objectPart.materialConfig);

		/// 変換行列設定
		renderData.transformData = ObjectWVPAdjustment3D(*object, objectPart, modelData->GetModelData().get(), modelCounter);

		/// PSO設定
		renderData.psoKey = PSODecision(*objectPart.materialConfig);

		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddObjectToBucket3D(renderData, object->modelHandle_);

		modelCounter++;
	}
}

Matrix4x4 DrawDataCollector::MakeFollowObjectMatrix3D(ObjectData* object) {

	DirtyEulerToQuat(object->mainPosition);
	Matrix4x4 objectMainMatrix = MakeAffineMatrix(
		object->mainPosition.transform.scale,
		object->mainPosition.transform.rotateQuat,
		object->mainPosition.transform.translate
	);

	Matrix4x4 parentMatrix = Identity();

	ObjectPart* parent = object->followObject_;
	while (parent != nullptr) {
		DirtyEulerToQuat(*parent);
		Matrix4x4 local = MakeAffineMatrix(
			parent->transform.scale,
			parent->transform.rotateQuat,
			parent->transform.translate
		);

		parentMatrix = local * parentMatrix;
		parent = parent->parentPart;
	}

	return objectMainMatrix * parentMatrix;
}

Matrix4x4 DrawDataCollector::MakeAnimationMatrix(ObjectPart& part) {

	Matrix4x4 result = Identity();
	if (part.transform.isAnimated) {
		result = MakeAffineMatrix(
			part.transform.aniScale,
			part.transform.aniRotate,
			part.transform.aniTranslate
		);
	}
	// アニメーション行列は毎フレーム更新されるため、使用後にフラグをリセット
	part.transform.isAnimated = false;
	return result;
}

TransformationMatrix DrawDataCollector::ObjectWVPAdjustment3D(ObjectData& object, ObjectPart& part, ModelData* modelData, int modelCounter) {
	Camera* cam = cameraManager_->GetActiveCamera();
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();


	// Billboard 子物件：XY 旋轉を0にする
	if (object.isBillboard_) {
		Vector3 camRot = cam->GetTransform().rotate;
		part.transform.rotate.x = camRot.x;
		part.transform.rotate.y = camRot.y;
		part.transform.rotate.z = 0.0f;
		DirtyEulerToQuat(part);
	}

	Matrix4x4 localMatrix = MakeAffineMatrix(
		part.transform.scale,
		part.transform.rotateQuat,
		part.transform.translate
	);

	int animationIt = animationManager_->GetControlObjectPartHandle(&object);

	auto animationPartIt = std::find_if(
		object.objectParts_.begin(),
		object.objectParts_.end(),
		[&](const ObjectPart& p) {
			return &p == &part;
		}
	);

	int animationPartInt = (int)std::distance(object.objectParts_.begin(), animationPartIt);

	Matrix4x4 animationMatrix;
	if (animationIt == -1) {
		animationMatrix = Identity();
	} else {
		auto object = animationManager_->GetInstanceObjectByUnitHandle(animationIt);
		Vector3 aniScale = object->objectParts_[animationPartInt].transform.aniScale;
		Quaternion aniRotate = object->objectParts_[animationPartInt].transform.aniRotate;
		Vector3 aniTranslate = object->objectParts_[animationPartInt].transform.aniTranslate;

		animationMatrix = MakeAffineMatrix(
			aniScale,
			aniRotate,
			aniTranslate
		);
	}

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix3D(&object);

	Matrix4x4 worldMatrix = localMatrix * animationMatrix * followWorldMatrix;

	Matrix4x4 nodeMatrix = Identity();

	if (modelData != nullptr) {
		if (modelCounter >= 0 && modelCounter < modelData->meshDataList.size()) {

			uint32_t nodeIndex = modelData->meshDataList[modelCounter].nodeIndex;

			if (nodeIndex < modelData->nodeList.size()) {
				nodeMatrix = modelData->nodeList[nodeIndex].globalMatrix;
			}
		}
	}
	Matrix4x4 finalWorld = nodeMatrix * worldMatrix;

	TransformationMatrix result{};
	result.WVP = (finalWorld * viewMatrix * projectionMatrix);
	result.world = finalWorld;
	result.WorldInverseTranspose = finalWorld.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddObjectToBucket3D(RenderData& renderData, int meshID) {

	auto checker = ResourceManager::GetInstance()->idToIndex_.find(renderData.materialID);

	if (checker == ResourceManager::GetInstance()->idToIndex_.end()) {
		Logger::Log("[kError]DDC:MaterialID not found in ResourceManager!");
		return;
	} else {
		MaterialForGPU* material = ResourceManager::GetInstance()->materialList_[checker->second].cpuMaterial.get();

		if (material->color.w < 1.0f) {

			float z = renderData.transformData.world.m[3][2];

			auto checker2 = std::find_if(
				transparentBucket3D_.begin(),
				transparentBucket3D_.end(),
				[z](const RenderData& data) {
					float dataZ = data.transformData.world.m[3][2];
					return z > dataZ;
				}
			);

			if (checker2 == transparentBucket3D_.end()) {
				transparentBucket3D_.emplace_back(renderData);
			} else {
				std::size_t index = std::distance(transparentBucket3D_.begin(), checker2);
				transparentBucket3D_.insert(transparentBucket3D_.begin() + index, renderData);
			}
		} else {
			/// 不透明オブジェクトバケットへ追加
			opaqueBucket3D_[static_cast<PSOKey>(renderData.psoKey)][renderData.materialID][meshID].emplace_back(renderData);
		}
	}
}

void DrawDataCollector::DirtyEulerToQuat(ObjectPart& part) {
	if (part.transform.CheckRotateDirty()) {
		part.transform.UpdateRotateQuat();
	}
}

void DrawDataCollector::DirtyEulerToQuat(ObjectData& part) {
	if (part.mainPosition.transform.CheckRotateDirty()) {
		part.mainPosition.transform.UpdateRotateQuat();
	}
}

#pragma endregion



#pragma region /// ================================= skinning関連 ================================== ///


int DrawDataCollector::SetSkinningData(WellForGPU* mappedPalette, int mappedNum, VertexInfluence* influenceSpan, int VertexNum) {

	/// VertexInfluenceがnullの場合はエラー
	if (!influenceSpan) {
		Logger::Log("[kEngine]DDC:VertexInfluence pointer is null!");
		return -1;
	}

	/// MappedPaletteがnullの場合はエラー
	if (!mappedPalette) {
		Logger::Log("[kEngine]DDC:WellForGPU pointer is null!");
		return -1;
	}

	/// 頂点数が0以下の場合はエラー
	if (VertexNum <= 0) {
		Logger::Log("[kEngine]DDC:VertexNum must be greater than 0!");
		return -1;
	}

	/// MappedNumが0以下の場合はエラー
	if (mappedNum <= 0) {
		Logger::Log("[kEngine]DDC:MappedNum must be greater than 0!");
		return -1;
	}

	/// Dataの追加
	skinningDataList_.emplace_back(
		std::span<VertexInfluence>(influenceSpan, VertexNum),
		std::span<WellForGPU>(mappedPalette, mappedNum)
	);

	/// mappedPaletteすべての行列をIdentityで埋める
	std::generate(
		skinningDataList_.back().mappedPalette.begin(),
		skinningDataList_.back().mappedPalette.end(), 
		[](){return WellForGPU{ Identity(),Identity() }; }
	);

	return int(skinningDataList_.size() - 1);
}

void DrawDataCollector::ClearSkinningData(int index) {

	skinningDataList_[index] = SkinningData{};

}

#pragma endregion



#pragma region /// ================================= パーティクル関連 ================================== ///

void DrawDataCollector::CollectParticleC(ObjectData* object) {
	/// nullチェック
	if (!object)return;

	int modelCounter = 0;
	for (auto& objectPart : object->objectParts_) {

		/// ========================================  RenderData作成  ========================================///

		/// RenderData作成
		RenderData renderData;

		/// メッシュ設定
		Model* modelData = ResourceManager::GetInstance()->modelGroupList_[object->modelHandle_]->GetModel(modelCounter);
		renderData.mesh = modelData;

		/// マテリアル設定
		objectPart.materialConfig->MakeUVMatrix();
		renderData.materialID = ResourceManager::GetInstance()->InputMaterialConfig(objectPart.materialConfig);

		/// 変換行列設定
		renderData.transformData = ObjectWVPAdjustmentPC(*object, objectPart, *modelData->GetModelData().get(), modelCounter);

		/// PSO設定
		renderData.psoKey = PSODecision(*objectPart.materialConfig);

		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddObjectToBucketPC(renderData);

		modelCounter++;
	}
}


TransformationMatrix DrawDataCollector::ObjectWVPAdjustmentPC(ObjectData& object, ObjectPart& part, ModelData& modelData, int modelCounter) {
	Camera* cam = cameraManager_->GetActiveCamera();
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix3D(&object);

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

	uint32_t nodeIndex = modelData.meshDataList[modelCounter].nodeIndex;
	Matrix4x4 nodeMatrix = modelData.nodeList[nodeIndex].globalMatrix;
	Matrix4x4 finalWorld = nodeMatrix * worldMatrix;

	TransformationMatrix result{};
	result.WVP = (finalWorld * viewMatrix * projectionMatrix);
	result.world = finalWorld;
	result.WorldInverseTranspose = finalWorld.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddObjectToBucketPC(RenderData& renderData) {

	auto checker = ResourceManager::GetInstance()->idToIndex_.find(renderData.materialID);

	if (checker == ResourceManager::GetInstance()->idToIndex_.end()) {
		Logger::Log("[kError]DDC:MaterialID not found in ResourceManager!");
		return;
	} else {
		bucketParticleC_.emplace_back(renderData);
	}

	auto& t = renderData.transformData;
}

#pragma endregion

#pragma region /// ========================== カメラ/マテリアル/ライティング関連 ========================= ///

void DrawDataCollector::BuildInstanceList2D() {

	if (opaqueBucket2D_.empty() && transparentBucket2D_.empty())return;

	/// 不透明物件
	for (auto& [psoID, materialBuckets] : opaqueBucket2D_) {
		for (auto& [materialID, RenderDataGroup] : materialBuckets) {
			if (RenderDataGroup.empty()) continue;
			for (auto& [meshBuffer, RenderData] : RenderDataGroup) {
				/// WVP計算
				for (auto& object : RenderData) {
					instancingList2D_[instanceCounter2D_].WVP = object.transformData.WVP;;
					instancingList2D_[instanceCounter2D_].world = object.transformData.world;
					instancingList2D_[instanceCounter2D_].WorldInverseTranspose = object.transformData.WorldInverseTranspose;
					instanceCounter2D_++;
				}
			}
		}
	}

	/// 透明物件
	for (auto& object : transparentBucket2D_) {
		instancingList2D_[instanceCounter2D_].WVP = object.transformData.WVP;;
		instancingList2D_[instanceCounter2D_].world = object.transformData.world;
		instancingList2D_[instanceCounter2D_].WorldInverseTranspose = object.transformData.WorldInverseTranspose;
		instanceCounter2D_++;
	}
}

void DrawDataCollector::BuildInstanceList3D() {
	if (opaqueBucket3D_.empty() && transparentBucket3D_.empty())return;

	/// 不透明物件
	for (auto& [psoID, materialBuckets] : opaqueBucket3D_) {
		for (auto& [materialID, RenderDataGroup] : materialBuckets) {
			if (RenderDataGroup.empty()) continue;
			for (auto& [meshBuffer, RenderData] : RenderDataGroup) {
				/// WVP計算
				for (auto& object : RenderData) {
					instancingList3D_[instanceCounter3D_].WVP = object.transformData.WVP;;
					instancingList3D_[instanceCounter3D_].world = object.transformData.world;
					instancingList3D_[instanceCounter3D_].WorldInverseTranspose = object.transformData.WorldInverseTranspose;
					instanceCounter3D_++;
				}
			}
		}
	}

	/// 透明物件
	for (auto& object : transparentBucket3D_) {
		instancingList3D_[instanceCounter3D_].WVP = object.transformData.WVP;;
		instancingList3D_[instanceCounter3D_].world = object.transformData.world;
		instancingList3D_[instanceCounter3D_].WorldInverseTranspose = object.transformData.WorldInverseTranspose;
		instanceCounter3D_++;
	}
}

#pragma endregion

#pragma region /// ========================== カメラ/マテリアル/ライティング関連 ========================= ///

Vector3 DrawDataCollector::GetCameraPosition() const {
	return cameraManager_->GetActiveCamera()->GetTransform().translate;
}

PSOKey DrawDataCollector::PSODecision(MaterialConfig& material) {

	/// 新しいkeyを準備
	PSOKey key{};

	/// PSOKeyの各要素をMaterialConfigから設定
	key = {
		.lightModelType = material.lightModelType,
		.renderModelType = material.renderModelType,
		.blendModeType = material.blendModeType,
		.rasterizerMode = material.rasterizerMode,
		.depthStencilType = material.depthStencilType,
	};

	/// PrimitiveTypeの決定
	switch (material.renderModelType) {

	case RenderModelType::DebugLine:
		key.primitiveType = PrimitiveType::LINE;
		break;

	case RenderModelType::Sprite2D:
	case RenderModelType::SkyCube:
	case RenderModelType::Static:
	case RenderModelType::Skinned:
	case RenderModelType::FlameNeonGlow:
		key.primitiveType = PrimitiveType::TRIANGLE;
		break;

	default:
		key.primitiveType = PrimitiveType::TRIANGLE;
		break;
	}

	return key;
}


void DrawDataCollector::UpdateLightData() {
	if (lightManager_)lightManager_->TurnDataToGPUData();
}

std::vector<LightGPU> DrawDataCollector::GetLightGPUBuffer() {
	return lightManager_->GetGPUBuffer();
}

uint32_t DrawDataCollector::GetLightCount() {
	return (uint32_t)lightManager_->GetLightCount();
}

#pragma endregion