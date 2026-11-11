#include "DrawDataCollector.h"
#include "Logger.h"
#include "CameraManager/CameraManager.h"
#include "LightManager/LightManager.h"
#include "Resource/ResourceManager.h"
#include "Tool/EffectSystem/ParticleSystem/ParticlePrototype.h"


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

	instancingListMaterialGPU_ = nullptr;
	instancingListDL_ = nullptr;
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
	bucketParticleC_.clear();

	memset(instancingListDL_, 0, sizeof(TransformationMatrix) * maxDebugLineInstance);
	debugLinesVertexBucket_.clear();

	/// レイヤードスプライト用リストクリア
	simpleSpriteCounter_ = 0;
	unlayeredSpriteCounter_ = 0;
	usedSpriteLayerKeys_.clear();

	/// skinning用リストクリア
	skinningDataList_ = {};

	/// インスタンスリストクリア
	instanceCounterMaterial_ = 0;
	instanceCounterDL_ = 0;
	instanceCounter2D_ = 0;
	instanceCounter3D_ = 0;
	instanceCounterParticleC_ = 0;
}

void DrawDataCollector::EndCollect() {

	/// ネールスキップ
	bool no2D = opaqueBucket2D_.empty() && transparentBucket2D_.empty();
	bool no3D = opaqueBucket3D_.empty() && transparentBucket3D_.empty();
	bool noParticle = bucketParticleC_.empty();
	if (no2D &&
		no3D &&
		noParticle)
		return;

	/// 実際のインスタンスリスト作成
	if (!no2D)			BuildInstanceList2D();
	if (!no3D)			BuildInstanceList3D();
	if (!noParticle)	BuildInstanceListParticle();

}

#ifdef USE_IMGUI
void DrawDataCollector::SetSoloTarget(const ObjectData* object, int partIndex) {
	soloObject_ = object;
	soloPartIndex_ = object ? partIndex : -1;
}
#endif

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

		/// ===== バケツ分け用資料
		/// メッシュ設定
		renderData.mesh = ResourceManager::GetInstance()->simpleSpriteMeshList_[simpleSpriteCounter_].get();

		/// TextureHandle設定
		renderData.textureHandle = object.materialConfig->textureHandle;

		/// PSOKey設定
		renderData.psoKey = PSODecision(*object.materialConfig);

		/// ===== 実際データ
		/// マテリアル設定
		object.materialConfig->MakeUVMatrix();
		MaterialForGPU newData;
		newData.inputMaterialConfig(*object.materialConfig);
		instancingListMaterialGPU_[instanceCounterMaterial_] = newData;
		/// MaterialID設定
		renderData.materialID = instanceCounterMaterial_++;


		/// 変換行列設定
		renderData.transformData = SpriteWVPAdjustment2D(*sprite, object);

		/// ===== まだ使えない
		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddSpriteToBucket2D(renderData);

		/// スプライトカウンターインクリメント
		simpleSpriteCounter_++;
	}
}

float DrawDataCollector::SpriteLayerManagement(float mainZ, float partZ) {

	const int mainLayer = (int)std::lround(mainZ);
	const int partLayer = (int)std::lround(partZ);

	/// ② どちらも指定なし → 自動で並べる（呼ばれた順に手前へ）
	if (mainLayer == 0 && partLayer == 0) {
		if (unlayeredSpriteCounter_ > 4000) {
			Logger::Log("[kWarning]DDC:unlayered Sprite meet max number!");
			return (float)unlayeredSpriteCounter_ * -layerDepth_Sprite;
		}
		float z = (float)unlayeredSpriteCounter_ * -layerDepth_Sprite;
		unlayeredSpriteCounter_++;
		return z;
	}

	/// ① ② ③ key を作る（大きいほど手前）
	int64_t key = 0;
	if (mainLayer != 0) {
		/// main 指定あり：main が上位、part は下 4 桁
		key = (int64_t)mainLayer * kSpriteSubLayerScale
			+ std::clamp(partLayer, 0, kSpriteSubLayerScale - 1);
	} else {
		/// main なし：part をレイヤーとして使う
		key = (int64_t)partLayer * kSpriteSubLayerScale;
	}

	/// ④ 重複 → 後から来た方を +1
	while (usedSpriteLayerKeys_.count(key)) {
		key++;
	}
	usedSpriteLayerKeys_.insert(key);

	/// 範囲チェック
	const int64_t maxKey = (int64_t)(kSpriteMaxMainLayer + 1) * kSpriteSubLayerScale - 1;
	if (key < 0 || key > maxKey) {
		Logger::Log("[kWarning]DDC:sprite layer out of range!");
		key = std::clamp<int64_t>(key, 0, maxKey);
	}

	return -layeredSpriteDepth_ - (float)(key * kLayeredKeyDepth);
}

Matrix4x4 DrawDataCollector::MakeFollowObjectMatrix2D(SpriteData* sprite, SpritePart* part) {

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

	return resultMatrix;
}

TransformationMatrix DrawDataCollector::SpriteWVPAdjustment2D(SpriteData& sprite, SpritePart& part) {

	float zBuffer = SpriteLayerManagement(sprite.mainPosition.transform.translate.z, part.transform.translate.z);
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

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix2D(&sprite, &part);

	Matrix4x4 localMatrix = MakeAffineMatrix(
		part.transform.scale,
		part.transform.rotate,
		part.transform.translate
	);

	Matrix4x4 worldMatrix = localMatrix * followWorldMatrix;
	worldMatrix.m[3][2] = zBuffer;

	TransformationMatrix result{};
	result.world = worldMatrix;
	result.WVP = worldMatrix * viewMatrix * projectionMatrix;
	result.WorldInverseTranspose = worldMatrix.Inverse().Transpose();
	//result.WorldInverseTranspose.m[3][2] = zBuffer;
	return result;
}

void DrawDataCollector::AddSpriteToBucket2D(RenderData& renderData) {

	int materialID = (int)renderData.materialID;
	MaterialForGPU& material = instancingListMaterialGPU_[materialID];

	if (materialID < 0 || materialID >= instanceCounterMaterial_) {
		Logger::Log("[kEngine]DDC::AddObjectToBucket2D()MaterialID Error!");
		return;
	} else {
		/// 透明判定（マテリアルの alpha ＋ テクスチャの透明ピクセル）
		bool isTransparent = TextureManager::GetInstance()->HasAlpha(renderData.textureHandle);
		for (int i = 0; i < material.layerCount && !isTransparent; ++i) {
			if (material.layers[i].color.w < 1.0f) {
				isTransparent = true;
			}
		}

		if (isTransparent) {
			///　透明の場合、z値を比較して、z値が大きい方が手前に描画されるようにソートして追加
			float z = renderData.transformData.world.m[3][2];
			auto checker2 = std::find_if(
				transparentBucket2D_.begin(),
				transparentBucket2D_.end(),
				[z](const RenderData& data) {
					return z > data.transformData.world.m[3][2];
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
			opaqueBucket2D_
				[renderData.psoKey]
				[renderData.mesh]
				[renderData.textureHandle]
				.emplace_back(renderData);
		}
	}
}

#pragma endregion

#pragma region /// ===================================== 3D関連 ===================================== ///

void DrawDataCollector::Collect3D(ObjectData* object) {
	/// nullチェック
	if (!object)return;

#ifdef USE_IMGUI
	if (soloObject_ && object != soloObject_) return;
#endif

	int modelCounter = 0;
	for (auto& objectPart : object->objectParts_) {


#ifdef USE_IMGUI
		if (soloObject_ && soloPartIndex_ >= 0 && modelCounter != soloPartIndex_) {
			modelCounter++;      // 網格索引一定要照樣前進
			continue;
		}
#endif

		/// ========================================  RenderData作成  ========================================///

		/// RenderData作成
		RenderData renderData;

		/// ===== バケツ分け用資料
		/// メッシュ設定
		Model* modelData = ResourceManager::GetInstance()->modelGroupList_[object->modelHandle_]->GetModel(modelCounter);
		renderData.mesh = modelData;

		/// TextureHandle設定
		renderData.textureHandle = objectPart.materialConfig->textureHandle;

		/// PSOKey設定
		renderData.psoKey = PSODecision(*objectPart.materialConfig);
		//if (objectPart.GetWellHandle() != -1 && renderData.psoKey.renderModelType == RenderModelType::Static)
		//	renderData.psoKey.renderModelType = RenderModelType::Skinned;

		/// ===== 実際データ
		/// skinning設定
		if (objectPart.GetWellHandle() != -1) renderData.skinningPaletteIndex = objectPart.GetWellHandle();
		/// マテリアル設定
		objectPart.materialConfig->MakeUVMatrix();
		MaterialForGPU newData;
		newData.inputMaterialConfig(*objectPart.materialConfig);
		instancingListMaterialGPU_[instanceCounterMaterial_] = newData;
		/// MaterialID設定
		renderData.materialID = instanceCounterMaterial_++;

		/// 変換行列設定
		renderData.transformData = ObjectWVPAdjustment3D(*object, objectPart, modelData->GetModelData().get(), modelCounter);

		/// ===== まだ使えない
		/// サブメッシュインデックス設定
		renderData.subMeshIndex = 0;

		/// ========================================  バケット振り分け  ========================================///
		AddObjectToBucket3D(renderData);

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

		//parentMatrix = local * parentMatrix;
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
#ifdef USE_IMGUI
	Matrix4x4 viewMatrix = usePreviewView_ ? previewView_.view : cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = usePreviewView_ ? previewView_.proj : cam->GetProjectionMatrix();
#else
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();
#endif

	Matrix4x4 localMatrix;

	/// Billboard計算--カメラの回転を打ち消
	if (object.isBillboard_) {

		localMatrix = MakeAffineMatrix(
			part.transform.scale,
			Vector3{ 0, 0, part.transform.rotate.z },
			part.transform.translate
		);

		/// 回転の移動を行列を取り除く
		Matrix4x4 rot = viewMatrix;
		rot.m[3][0] = rot.m[3][1] = rot.m[3][2] = 0;
		rot = rot.Inverse();

		localMatrix = localMatrix * rot;
	} else {

		localMatrix = MakeAffineMatrix(
			part.transform.scale,
			part.transform.rotate,
			part.transform.translate
		);
	}

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

	Matrix4x4 followWorldMatrix;

	if (object.isBillboard_) {
		followWorldMatrix = Identity();
	} else {
		followWorldMatrix = MakeFollowObjectMatrix3D(&object);
	}

	localMatrix;

	Matrix4x4 worldMatrix = localMatrix * animationMatrix * followWorldMatrix;
	//Matrix4x4 worldMatrix = followWorldMatrix * animationMatrix * localMatrix;

	Matrix4x4 nodeMatrix = Identity();

	//if (modelData != nullptr) {
	//	if (modelCounter >= 0 && modelCounter < modelData->meshDataList.size()) {
	//
	//		uint32_t nodeIndex = modelData->meshDataList[modelCounter].nodeIndex;
	//
	//		if (nodeIndex < modelData->nodeList.size()) {
	//			nodeMatrix = modelData->nodeList[nodeIndex].globalMatrix;
	//		}
	//	}
	//}
	Matrix4x4 finalWorld = nodeMatrix * worldMatrix;

	TransformationMatrix result{};
	result.WVP = (finalWorld * viewMatrix * projectionMatrix);
	result.world = finalWorld;
	result.WorldInverseTranspose = finalWorld.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddObjectToBucket3D(RenderData& renderData) {

	int materialID = (int)renderData.materialID;
	MaterialForGPU& material = instancingListMaterialGPU_[materialID];

	if (materialID < 0 || materialID >= instanceCounterMaterial_) {
		Logger::Log("[kEngine]DDC::AddObjectToBucket3D() MaterialID Error!");
		return;
	} else {
		bool isTransparent = false;
		for (int i = 0; i < material.layerCount; ++i) {
			if (material.layers[i].color.w < 1.0f) {
				isTransparent = true;
				break;
			}
		}

		if (isTransparent) {

			/// 半透明のものは、カメラとの距離を計算して、遠い順にソートして追加する
			// ワールド座標を取得
			Vector3 pos{
				renderData.transformData.world.m[3][0],
				renderData.transformData.world.m[3][1],
				renderData.transformData.world.m[3][2]
			};

			// カメラの位置を取得
			Vector3 camPos = GetCameraPosition();

			// カメラとの距離を計算
			float dist = Length(pos - camPos);

			// 遠い順にソートして追加するためのラムダ関数を使用して、挿入位置を見つける
			auto it = std::find_if(
				transparentBucket3D_.begin(),
				transparentBucket3D_.end(),
				[&](const RenderData& data) {

					Vector3 pos2{
						data.transformData.world.m[3][0],
						data.transformData.world.m[3][1],
						data.transformData.world.m[3][2]
					};

					float dist2 = Length(pos2 - camPos);

					return dist > dist2; // 遠的先畫
				}
			);

			if (it == transparentBucket3D_.end()) {
				transparentBucket3D_.push_back(renderData);
			} else {
				transparentBucket3D_.insert(it, renderData);
			}
		} else {
			/// 不透明オブジェクトバケットへ追加
			opaqueBucket3D_
				[static_cast<PSOKey>(renderData.psoKey)]
				[renderData.mesh]
				[renderData.textureHandle]
				.emplace_back(renderData);
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
		[]() {return WellForGPU{ Identity(),Identity() }; }
	);

	return int(skinningDataList_.size() - 1);
}

void DrawDataCollector::ClearSkinningData(int index) {

	//skinningDataList_[index] = SkinningData{};

}

#pragma endregion



#pragma region /// ================================= パーティクル関連 ================================== ///

void DrawDataCollector::CollectParticle(std::vector<ObjectData>& objectList, std::vector<ParticleInstance>& instance) {

	/// nullチェック
	if (objectList.empty()) {
		Logger::Log("[kEngine]DDC:CollectParticlePrototype() ObjectList is null!");
		return;
	}

	for (int i = 0; i < instance.size(); ++i) {

		const auto& inst = instance[i];

		/// パーティクルのObjectIndexがRenderDataのインデックス範囲内かチェック
		if (inst.objectIndex < 0 || inst.objectIndex >= (int)objectList.size()) {
			Logger::Log("[kEngine]DDC:ParticleInstance objectIndex is out of range! Data will not be input.");
			Logger::Log("[kEngine]DDC:ParticleInstance ObjectIndex:%d", i);
			continue;
		}

		auto& targetObject = objectList[inst.objectIndex];

		for (int i = 0; i < targetObject.objectParts_.size(); ++i) {
			/// ================ RenderData作成
			/// RenderData作成
			ParticleInstanceForDDC data;

			/// メッシュ設定
			Model* modelData = ResourceManager::GetInstance()->modelGroupList_[targetObject.modelHandle_]->GetModel(i);
			data.mesh = modelData;

			/// Material制作, ID獲得
			MaterialConfig materialConfig = *targetObject.objectParts_[i].materialConfig;
			materialConfig.textureColor = inst.nowColor;
			materialConfig.MakeUVMatrix();
			MaterialForGPU newData;
			newData.inputMaterialConfig(materialConfig);
			instancingListMaterialGPU_[instanceCounterMaterial_] = newData;
			data.materialID = instanceCounterMaterial_++;

			/// TextureHandle設定
			data.textureHandle = materialConfig.textureHandle;

			/// PSO制作
			data.psoKey = PSODecision(materialConfig);

			/// 変換行列設定
			data.transMatrix = ObjectWVPAdjustmentPC(targetObject, *modelData->GetModelData().get(), i, inst);

			/// バケットに入れる
			AddObjectToBucketPC(data);
		}
	}
}

void DrawDataCollector::CollectParticle(std::vector<ParticleInstance>& instance) {

	for (int i = 0; i < (int)instance.size(); ++i) {

		const auto& inst = instance[i];

		/// 見た目は粒子が持っている自分の Prototype から取る
		if (!inst.prototype) continue;
		auto& objectList = inst.prototype->objectList;

		/// objectList 全部で「1つの物」：全部同じ粒子の変換で描く
		for (auto& targetObject : objectList) {

			for (int p = 0; p < (int)targetObject.objectParts_.size(); ++p) {
				if (!targetObject.objectParts_[p].materialConfig) continue;   // null 対策

				ParticleInstanceForDDC data;

				/// メッシュ設定
				Model* modelData = ResourceManager::GetInstance()->modelGroupList_[targetObject.modelHandle_]->GetModel(p);
				data.mesh = modelData;

				/// Material制作, ID獲得
				MaterialConfig materialConfig = *targetObject.objectParts_[p].materialConfig;
				materialConfig.textureColor = inst.nowColor;
				materialConfig.MakeUVMatrix();
				MaterialForGPU newData;
				newData.inputMaterialConfig(materialConfig);
				instancingListMaterialGPU_[instanceCounterMaterial_] = newData;
				data.materialID = instanceCounterMaterial_++;

				/// TextureHandle設定
				data.textureHandle = materialConfig.textureHandle;

				/// PSO制作
				data.psoKey = PSODecision(materialConfig);

				/// 変換行列設定
				data.transMatrix = ObjectWVPAdjustmentPC(targetObject, *modelData->GetModelData().get(), p, inst);

				/// バケットに入れる
				AddObjectToBucketPC(data);
			}
		}
	}
}

TransformationMatrix DrawDataCollector::ObjectWVPAdjustmentPC(ObjectData& object, ModelData& modelData, int modelCounter, ParticleInstance particleData) {
	Camera* cam = cameraManager_->GetActiveCamera();
	Matrix4x4 viewMatrix = cam->GetViewMatrix();
	Matrix4x4 projectionMatrix = cam->GetProjectionMatrix();

	Matrix4x4 followWorldMatrix = MakeFollowObjectMatrix3D(&object);

	auto& part = object.objectParts_[modelCounter];

	Matrix4x4 localMatrix;

	/// Billboard計算--カメラの回転を打ち消
	if (object.isBillboard_) {

		localMatrix = MakeAffineMatrix(
			particleData.nowScale,
			Vector3{ 0, 0, particleData.nowRotate.z },
			particleData.nowTranslate
		);

		/// 回転の移動を行列を取り除く
		Matrix4x4 rot = viewMatrix;
		rot.m[3][0] = rot.m[3][1] = rot.m[3][2] = 0;
		rot = rot.Inverse();

		/// 
		//Matrix4x4 roll = MakeRZMatrix4x4(part.transform.rotate.z);
		//Matrix4x4 finalRot = rot * roll;

		localMatrix = localMatrix * rot;
	} else {

		localMatrix = MakeAffineMatrix(
			particleData.nowScale,
			particleData.nowRotate,
			particleData.nowTranslate
		);
	}

	//Matrix4x4 worldMatrix = localMatrix * partParentMatrix * followWorldMatrix;
	Matrix4x4 worldMatrix = followWorldMatrix * localMatrix;

	Matrix4x4 finalWorld = worldMatrix;

	TransformationMatrix result{};
	result.WVP = (finalWorld * viewMatrix * projectionMatrix);
	result.world = finalWorld;
	result.WorldInverseTranspose = finalWorld.Inverse().Transpose();
	return result;
}

void DrawDataCollector::AddObjectToBucketPC(ParticleInstanceForDDC& renderData) {

	/// 分類収納
	bucketParticleC_[renderData.psoKey][renderData.mesh][renderData.textureHandle].emplace_back(renderData.materialID, renderData.transMatrix);
}

#pragma endregion

#pragma region /// ========================== カメラ/マテリアル/ライティング関連 ========================= ///

void DrawDataCollector::BuildInstanceList2D() {

	if (opaqueBucket2D_.empty() && transparentBucket2D_.empty())return;

	/// 不透明物件
	// PSOKey -> MeshBuffer -> TextureHandle -> RenderData循環
	for (auto& [psoKey, modelBuckets] : opaqueBucket2D_) {
		if (modelBuckets.empty()) continue;
		for (auto& [meshBuffer, TextureHandleGroup] : modelBuckets) {
			if (TextureHandleGroup.empty()) continue;
			for (auto& [TextureHandle, RenderDataGroup] : TextureHandleGroup) {
				if (RenderDataGroup.empty()) continue;
				for (auto& RenderData : RenderDataGroup) {
					/// WVP計算
					instancingList2D_[instanceCounter2D_].WVP = RenderData.transformData.WVP;;
					instancingList2D_[instanceCounter2D_].world = RenderData.transformData.world;
					instancingList2D_[instanceCounter2D_].WorldInverseTranspose = RenderData.transformData.WorldInverseTranspose;
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
	for (auto& [psoKey, modelBuckets] : opaqueBucket3D_) {
		if (modelBuckets.empty()) continue;
		for (auto& [meshBuffer, TextureHandleGroup] : modelBuckets) {
			if (TextureHandleGroup.empty()) continue;
			for (auto& [TextureHandle, RenderDataGroup] : TextureHandleGroup) {
				if (RenderDataGroup.empty()) continue;
				for (auto& RenderData : RenderDataGroup) {
					/// WVP計算
					instancingList3D_[instanceCounter3D_].WVP = RenderData.transformData.WVP;;
					instancingList3D_[instanceCounter3D_].world = RenderData.transformData.world;
					instancingList3D_[instanceCounter3D_].WorldInverseTranspose = RenderData.transformData.WorldInverseTranspose;
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

void DrawDataCollector::BuildInstanceListParticle() {
	for (auto& [PSOData, MeshBuckets] : bucketParticleC_) {
		for (auto& [mesh, InstanceBuckets] : MeshBuckets) {
			for (auto& [TextureHandle, instanceData] : InstanceBuckets) {
				for (auto& [materialID, transformMatrix] : instanceData) {
					instancingListParticleC_[instanceCounterParticleC_].WVP = transformMatrix.WVP;;
					instancingListParticleC_[instanceCounterParticleC_].world = transformMatrix.world;
					instancingListParticleC_[instanceCounterParticleC_].WorldInverseTranspose = transformMatrix.WorldInverseTranspose;
					instanceCounterParticleC_++;
				}
			}
		}
	}
}

#pragma endregion

#pragma region /// ========================== カメラ/マテリアル/ライティング関連 ========================= ///


Vector3 DrawDataCollector::GetCameraPosition() const {
#ifdef USE_IMGUI
	if (usePreviewView_) return previewView_.position;
#endif
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
	case RenderModelType::Environment:
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


#ifdef USE_IMGUI
#pragma region /// ================================= debug関連 ==================================== ///
void DrawDataCollector::BeginPreview(ObjectData* object, int partIndex, const PreviewView& view) {

	savedOpaqueBucket3D_.swap(opaqueBucket3D_);
	savedTransparentBucket3D_.swap(transparentBucket3D_);

	usePreviewView_ = true;
	previewView_ = view;

	const ObjectData* prevObject = soloObject_;
	int prevPart = soloPartIndex_;
	soloObject_ = object;
	soloPartIndex_ = partIndex;
	Collect3D(object);
	soloObject_ = prevObject;
	soloPartIndex_ = prevPart;

	BuildInstanceList3D();
}

void DrawDataCollector::BeginPreview(const std::vector<ObjectData*>& objects, const PreviewView& view) {
	savedOpaqueBucket3D_.swap(opaqueBucket3D_);
	savedTransparentBucket3D_.swap(transparentBucket3D_);

	usePreviewView_ = true;
	previewView_ = view;

	/// Collect3D は soloObject_ 以外を弾くので、1個ずつ solo にして集める
	const ObjectData* prevObject = soloObject_;
	int prevPart = soloPartIndex_;
	for (ObjectData* obj : objects) {
		soloObject_ = obj;
		soloPartIndex_ = -1;
		Collect3D(obj);
	}
	soloObject_ = prevObject;
	soloPartIndex_ = prevPart;

	BuildInstanceList3D();
}

void DrawDataCollector::EndPreview() {
	usePreviewView_ = false;
	opaqueBucket3D_.clear();
	transparentBucket3D_.clear();
	opaqueBucket3D_.swap(savedOpaqueBucket3D_);
	transparentBucket3D_.swap(savedTransparentBucket3D_);
}
#pragma endregion
#endif