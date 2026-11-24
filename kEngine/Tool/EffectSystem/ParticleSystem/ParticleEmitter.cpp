#include "ParticleEmitter.h"
#include "kEngine.h"

ParticleEmitter::ParticleEmitter(kEngine* system,
	const ParticlePrototype particlePrototype,
	int* particleIdIP,
	int maxParticles) :
	system_(system),
	prototype_(std::make_shared<ParticlePrototype>(particlePrototype)),
	maxParticles_(maxParticles),
	particleIdIP_(particleIdIP) {
	DeepCopyMaterials(*prototype_);          // ← 加
	isObjectType_ = particlePrototype.renderType;
	emitTimer_.Init0(particlePrototype.emitRate, system_->GetTimeManager());
}

ParticleEmitter::~ParticleEmitter() {}

void ParticleEmitter::SetPrototype(const ParticlePrototype& proto) {
	/// 新しい Prototype を作るだけ。古い方は飛んでいる粒子が持っているので、その粒子が全部死ぬまで残る
	prototype_ = std::make_shared<ParticlePrototype>(proto);
	DeepCopyMaterials(*prototype_);
	isObjectType_ = proto.renderType;
	emitTimer_.Init0(proto.emitRate, system_->GetTimeManager());
}

void ParticleEmitter::Update() {

	/// 発射停止中は更新しない
	if (isPaused_)return;

	/// デルタタイムの取得
	float deltaTime = system_->GetDeltaTime() * timeScale_;

	/// 自動發射
	if (prototype_->emitRate > 0.0f) {
		emitTimer_.parameter_ += deltaTime;
		float interval = 1.0f / prototype_->emitRate;

		while (emitTimer_.parameter_ >= interval) {
			Emit(prototype_->burstCount);
			emitTimer_.parameter_ -= interval;
		}
	}

	/// 更新所有粒子
	for (auto& p : particles_) {

		p.life.parameter_ += deltaTime;
		if (p.life.parameter_ >= p.life.maxTime_) {
			p.isAlive = false;
			continue;
		}

		/// 重力
		p.velocity += p.prototype->gravity * deltaTime;

		/// 位置更新
		p.nowTranslate += p.velocity * deltaTime;

		/// 縮放插值
		p.nowScale = {
			Lerp(p.startScale.x, p.endScale.x, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startScale.y, p.endScale.y, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startScale.z, p.endScale.z, p.life.parameter_, p.life.maxTime_)
		};

		/// 旋轉插值
		p.nowRotate = {
			Lerp(p.startRotate.x, p.endRotate.x, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startRotate.y, p.endRotate.y, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startRotate.z, p.endRotate.z, p.life.parameter_, p.life.maxTime_)
		};

		/// 顏色插值
		p.nowColor = {
			std::clamp(Lerp(p.startColor.x, p.endColor.x, p.life.parameter_, p.life.maxTime_),0.0f,1.0f),
			std::clamp(Lerp(p.startColor.y, p.endColor.y, p.life.parameter_, p.life.maxTime_),0.0f,1.0f),
			std::clamp(Lerp(p.startColor.z, p.endColor.z, p.life.parameter_, p.life.maxTime_),0.0f,1.0f),
			std::clamp(Lerp(p.startColor.w, p.endColor.w, p.life.parameter_, p.life.maxTime_),0.0f,1.0f)
		};

		/// UV 插值
		p.nowUVTranslate = {
			Lerp(p.startUVTranslate.x, p.endUVTranslate.x, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startUVTranslate.y, p.endUVTranslate.y, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startUVTranslate.z, p.endUVTranslate.z, p.life.parameter_, p.life.maxTime_)
		};
		p.nowUVRotate = Lerp(p.startUVRotate, p.endUVRotate, p.life.parameter_, p.life.maxTime_);
		p.nowUVScale = {
			Lerp(p.startUVScale.x, p.endUVScale.x, p.life.parameter_, p.life.maxTime_),
			Lerp(p.startUVScale.y, p.endUVScale.y, p.life.parameter_, p.life.maxTime_)
		};

		/// 阻尼
		p.velocity *= std::exp(-p.prototype->damping * deltaTime);
	}

	/// 自動發射およびタイマー更新
	if (isEmitting_) {
		/// TimerとParameterも同じく0なら
		/// Timerが無効とし
		/// 発射しないし、更新もしない
		bool isParameter0 = (emitTimer_.parameter_ == 0.0f);
		bool isMaxTime0 = (emitTimer_.maxTime_ == 0.0f);
		if (!(isParameter0 || isMaxTime0)) {
			emitTimer_.ToMix();
			if (emitTimer_.GetIsMax()) {
				Emit(prototype_->burstCount);
				emitTimer_.Reset0();
			}
		}
	}

	/// 無序刪除
	for (int i = (int)particles_.size() - 1; i >= 0; --i) {
		if (!particles_[i].isAlive) {
			if (particles_[i].partIndex <= 0) dexpiredData_.push_back(particles_[i]);   // 死亡 Link は Part0 だけ
			particles_[i] = std::move(particles_.back());
			particles_.pop_back();
		}
	}
	if (particles_.empty()) {
		isFinished_ = true;
	}
}

void ParticleEmitter::Emit(int count, ParticlePrototypeOverride* prototype) {

	/// 発射停止中は発射しない
	if (!isEmitting_) return;

	/// 粒子に持たせる Prototype：普段は今のものを共有（コピーしない）
	/// Link から上書きが来た時だけ、上書きしたコピーを1つ作る（Emit 1回につき1つ）
	std::shared_ptr<ParticlePrototype> proto = prototype_;
	if (prototype) {
		proto = std::make_shared<ParticlePrototype>(*prototype_);
		CopyPPbyActiveData(*proto, *prototype);
	}
	const ParticlePrototype& usedPrototype = *proto;

	/// Part ごとに飛ばすか（partTracks が空なら今まで通り：1 粒子で全 Part）
	const bool isSeparate = !usedPrototype.partTracks.empty() && !usedPrototype.objectList.empty();
	const int partCount = isSeparate ? (int)usedPrototype.objectList[0].objectParts_.size() : 1;

	for (int i = 0; i < count; i++) {

		float emitRate = randomMaker_->randomFloat(1.0f - usedPrototype.emitNumRandom, 1.0f + usedPrototype.emitNumRandom);
		int shootTime = (int)(usedPrototype.burstCount * emitRate);

		for (int s = 0; s < shootTime; s++) {

			/// 寿命は 1 回だけ決める（同じ物の破片は同時に消える）
			const float randLife = usedPrototype.lifetimeRandomness * usedPrototype.lifetime;
			const float life = usedPrototype.lifetime + randomMaker_->randomFloat(-randLife, randLife);

			for (int part = 0; part < partCount; part++) {
				const ParticlePartTrack& tr = isSeparate ? GetPartTrack(usedPrototype, part) : usedPrototype.mainTrack;

				ParticleInstance p = BuildParticle(usedPrototype, tr, prototype, life);
				p.prototype = proto;
				p.partIndex = isSeparate ? part : -1;

				particles_.push_back(p);
				/// Link は 1 つの物につき 1 回（Part0 だけ）
				if (part == 0) emittingData_.push_back(p);
			}
		}
	}

	if (isFinished_ == true) isFinished_ = false;
}
void ParticleEmitter::SwitchEmitter(bool isEmitting) {
	isEmitting_ = isEmitting;
}

void ParticleEmitter::Draw() {
	if (particles_.empty()) return;
	if (isObjectType_ == ParticleRenderType::Object) {
		system_->DrawParticle(particles_);
	}
}

std::vector<ParticleInstance>& ParticleEmitter::GetEmittingData() {
	return emittingData_;
}

std::vector<ParticleInstance>& ParticleEmitter::GetDexpiredData() {
	return dexpiredData_;
}

ParticleInstance* ParticleEmitter::FindParticleById(int id) {
	for (auto& p : particles_) {
		if (p.particleId == id) return &p;
	}
	return nullptr;
}

void ParticleEmitter::ClearEmittingData() {
	if (!emittingData_.empty()) {
		emittingData_.clear();
	}
}

void ParticleEmitter::ClearDexpiredData() {
	if (!dexpiredData_.empty()) {
		dexpiredData_.clear();
	}
}

ParticleInstance ParticleEmitter::BuildParticle(const ParticlePrototype& pr, const ParticlePartTrack& tr,
	const ParticlePrototypeOverride* ov, float life) {

	ParticleInstance p;
	p.particleId = (*particleIdIP_)++;
	p.isAlive = true;
	p.life.parameter_ = 0.0f;
	p.life.maxTime_ = life;
	p.objectIndex = 0;

	/// ============================== 初期状態設定 ============================== ///

	// 初速
	Vector3 rv = {
		randomMaker_->randomFloat(-tr.startVelocityRandom.x, tr.startVelocityRandom.x),
		randomMaker_->randomFloat(-tr.startVelocityRandom.y, tr.startVelocityRandom.y),
		randomMaker_->randomFloat(-tr.startVelocityRandom.z, tr.startVelocityRandom.z)
	};
	p.velocity = rv + tr.startVelocity;

	// 初期位置：Link から位置が来ていればそこ、なければ「線の位置 + startPosition（ずれ）」
	const Vector3 basePos = (ov && ov->hasStartPosition)
		? ov->startPosition
		: emitterPosition + tr.startPosition;
	Vector3 rp = {
		randomMaker_->randomFloat(-tr.startPositionRandom.x, tr.startPositionRandom.x),
		randomMaker_->randomFloat(-tr.startPositionRandom.y, tr.startPositionRandom.y),
		randomMaker_->randomFloat(-tr.startPositionRandom.z, tr.startPositionRandom.z)
	};
	p.startTranslate = rp + basePos;

	Vector3 ranScale = {
		randomMaker_->randomFloat(1.0f - tr.startScaleRandom.x, 1.0f + tr.startScaleRandom.x),
		randomMaker_->randomFloat(1.0f - tr.startScaleRandom.y, 1.0f + tr.startScaleRandom.y),
		randomMaker_->randomFloat(1.0f - tr.startScaleRandom.z, 1.0f + tr.startScaleRandom.z)
	};
	p.startScale = emitterScale * (ranScale * tr.startScale);

	Vector3 ranRotate = {
		randomMaker_->randomFloat(-tr.startRotationRandom.x, tr.startRotationRandom.x),
		randomMaker_->randomFloat(-tr.startRotationRandom.y, tr.startRotationRandom.y),
		randomMaker_->randomFloat(-tr.startRotationRandom.z, tr.startRotationRandom.z)
	};
	p.startRotate = emitterRotation + tr.startRotation + ranRotate;

	Vector4 ranColor = {
		randomMaker_->randomFloat(1.0f - pr.startColorRandom.x, 1.0f + pr.startColorRandom.x),
		randomMaker_->randomFloat(1.0f - pr.startColorRandom.y, 1.0f + pr.startColorRandom.y),
		randomMaker_->randomFloat(1.0f - pr.startColorRandom.z, 1.0f + pr.startColorRandom.z),
		randomMaker_->randomFloat(1.0f - pr.startColorRandom.w, 1.0f + pr.startColorRandom.w)
	};
	p.startColor = { pr.startColor.x * ranColor.x, pr.startColor.y * ranColor.y,
					 pr.startColor.z * ranColor.z, pr.startColor.w * ranColor.w };

	p.startUVTranslate = tr.uvTranslateStart;
	p.startUVRotate = tr.uvRotateStart;
	p.startUVScale = tr.uvScaleStart;

	/// ============================== 最終状態設定 ============================== ///

	if (tr.isConstantScale) {
		p.endScale = p.startScale;
	} else {
		ranScale = {
			randomMaker_->randomFloat(1.0f - tr.endScaleRandom.x, 1.0f + tr.endScaleRandom.x),
			randomMaker_->randomFloat(1.0f - tr.endScaleRandom.y, 1.0f + tr.endScaleRandom.y),
			randomMaker_->randomFloat(1.0f - tr.endScaleRandom.z, 1.0f + tr.endScaleRandom.z)
		};
		p.endScale = emitterScale * (ranScale * tr.endScale);
	}

	if (tr.isConstantRotation) {
		p.endRotate = p.startRotate;
	} else {
		ranRotate = {
			randomMaker_->randomFloat(-tr.endRotationRandom.x, tr.endRotationRandom.x),
			randomMaker_->randomFloat(-tr.endRotationRandom.y, tr.endRotationRandom.y),
			randomMaker_->randomFloat(-tr.endRotationRandom.z, tr.endRotationRandom.z)
		};
		p.endRotate = emitterRotation + tr.endRotation + ranRotate;
	}

	if (pr.isConstantColor) {
		p.endColor = p.startColor;
	} else {
		ranColor = {
			randomMaker_->randomFloat(1.0f - pr.endColorRandom.x, 1.0f + pr.endColorRandom.x),
			randomMaker_->randomFloat(1.0f - pr.endColorRandom.y, 1.0f + pr.endColorRandom.y),
			randomMaker_->randomFloat(1.0f - pr.endColorRandom.z, 1.0f + pr.endColorRandom.z),
			randomMaker_->randomFloat(1.0f - pr.endColorRandom.w, 1.0f + pr.endColorRandom.w)
		};
		p.endColor = { pr.endColor.x * ranColor.x, pr.endColor.y * ranColor.y,
					   pr.endColor.z * ranColor.z, pr.endColor.w * ranColor.w };
	}

	p.endUVTranslate = tr.uvTranslateEnd;
	p.endUVRotate = tr.uvRotateEnd;
	p.endUVScale = tr.uvScaleEnd;

	/// ============================== 現在状態設定 ============================== ///
	p.nowTranslate = p.startTranslate;
	p.nowRotate = p.startRotate;
	p.nowScale = p.startScale;
	p.nowColor = p.startColor;
	p.nowUVTranslate = p.startUVTranslate;
	p.nowUVRotate = p.startUVRotate;

	return p;
}