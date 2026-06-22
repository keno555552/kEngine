#pragma once
#include "Tool/EffectSystem/ParticleSystem/EffectData.h"

static float hitRockScale = 2.0f;

struct HitSpark3 : ParticlePrototype
{
	HitSpark3() {
		/// 生存時間
		lifetime = 1.0f;

		/// 移動しないから設定要らない

		/// スケール設定、その後変わらない
		startScale = { 0.05f * hitRockScale, 1.0f * hitRockScale, 1.0f * hitRockScale };
		endScale = { 0.05f * hitRockScale, 1.0f * hitRockScale, 1.0f * hitRockScale };
		isConstantScale = true;

		/// 回転はランダム
		startRotationRandom = { 0.0f, 0.0f, 3.2f };
		isConstantRotation = true;

		/// 色は元の色から透明へ
		startColor = { 0.3f, 0.3f, 1.0f, 1.0f };
		endColor = { 0.3f, 0.3f, 1.0f, 0.0f };

		/// 行為設定
		burstCount = 5;
		emitNumRandom = 0.5f;

		/// 見た目設定
		ObjectData obj;
		obj.isBillboard_ = true;
		obj.modelHandle_ = config::config::default_Plane_MeshBufferHandle_;

		ObjectPart newObjectPart;
		newObjectPart.name = "HitSpark3";
		newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
		newObjectPart.materialConfig->MakePSOParticle();
		newObjectPart.materialConfig->enableLighting = false;
		newObjectPart.materialConfig->uvTranslate = { 1.0f, 1.0f };
		newObjectPart.materialConfig->uvScale = { 1.0f, 1.0f };
		newObjectPart.materialConfig->textureColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		InitMaterialConfig(newObjectPart.materialConfig.get());
		obj.objectParts_.push_back(newObjectPart);

		objectList.push_back(obj);
	}
};

struct HitImpact3 : ParticlePrototype
{
	HitImpact3() {
		/// 生存時間
		lifetime = 0.5f;
		/// 移動しないから設定要らない
		/// スケール設定、その後変わらない
		startScale = { 0.2f * hitRockScale, 0.2f * hitRockScale, 1.0f * hitRockScale };
		endScale = { 0.3f * hitRockScale, 0.3f * hitRockScale, 1.0f * hitRockScale };
		isConstantScale = false;
		/// 回転はランダム
		startRotationRandom = { 0.0f, 0.0f, 3.2f };
		isConstantRotation = true;
		/// 色は元の色から透明へ
		startColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		endColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		/// 行為設定
		burstCount = 1;
		/// 見た目設定
		ObjectData obj;
		obj.isBillboard_ = true;
		obj.modelHandle_ = config::config::default_Ring_MeshBufferHandle_;
		ObjectPart newObjectPart;
		newObjectPart.name = "HitImpact3";
		newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
		newObjectPart.materialConfig->MakePSOParticle();
		newObjectPart.materialConfig->enableLighting = false;
		newObjectPart.materialConfig->textureColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		InitMaterialConfig(newObjectPart.materialConfig.get());
		obj.objectParts_.push_back(newObjectPart);
		objectList.push_back(obj);
	}
};

struct HitRock : ParticlePrototype
{
	HitRock() {
		/// 生存時間
		lifetime = 2.0f;
		/// 移動しないから設定要らない
		/// スケール設定、その後変わらない
		float scale = 0.05f * hitRockScale;
		startScale = { scale, scale, scale };
		endScale = { scale, scale, scale };
		isConstantScale = false;
		/// 色は元の色から透明へ
		Vector3 color = { 0.63f, 0.55f, 0.42f};
		startColor = { color.x, color.y, color.z, 1.0f };
		endColor = { color.x, color.y, color.z, 0.0f };
		/// 移動設定
		startVelocity = { 0.0f, 3.5f, 0.0f };
		startVelocityRandom = { 0.5f, 0.5f, 0.0f };
		startSpeedRandom = { 0.2f,0.2f,0.0f };

		gravity = { 0.0f, -9.8f, 0.0f };

		/// 行為設定
		burstCount = 5;
		emitNumRandom = 0.3f;

		/// 見た目設定
		ObjectData obj;
		obj.modelHandle_ = config::config::default_Sphere_MeshBufferHandle_;
		ObjectPart newObjectPart;
		newObjectPart.name = "HitRock";
		newObjectPart.materialConfig = std::make_shared<MaterialConfig>();
		newObjectPart.materialConfig->MakePSOParticle();
		InitMaterialConfig(newObjectPart.materialConfig.get());
		obj.objectParts_.push_back(newObjectPart);
		objectList.push_back(obj);
	}
};

struct HitSpackImpactLink3 : EmitterLink
{
	HitSpackImpactLink3() {
		emitCount = 1;
		delayTime = 0.0f;
		linkMode = LinkMode::PerBurst;
		linkFollow = LinkFollow::Emitter;
		emitterTiming = EmitterTiming::SourceEmit;
	}
};

struct HitSpackImpactLink3_2 : EmitterLink
{
	HitSpackImpactLink3_2() {
		emitCount = 1;
		delayTime = 0.0f;
		linkMode = LinkMode::PerBurst;
		linkFollow = LinkFollow::Emitter;
		emitterTiming = EmitterTiming::SourceEmit;
	}
};

class Effect_HitSpark3 : public EffectData
{
public:
	Effect_HitSpark3() {
		prototypes.push_back(HitSpark3());
		prototypes.push_back(HitImpact3());
		prototypes.push_back(HitRock());
		links.push_back(HitSpackImpactLink3());
		links.push_back(HitSpackImpactLink3_2());
	};
};