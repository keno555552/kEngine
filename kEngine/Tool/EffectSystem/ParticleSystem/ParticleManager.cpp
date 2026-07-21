#include "ParticleManager.h"
#include "kEngine.h"

ParticleManager::ParticleManager(kEngine* system) :
	system_(system) {}

ParticleManager::~ParticleManager() {}

void ParticleManager::Update() {
	/// ポーズ中は更新しない
	if (isPaused_)return;

	/// Emitterの更新
	for (auto& [id, emitter] : emitterList_) {
		emitter->Update();
	}

	/// Emitter同士のリンクを更新
	UpdateEmitterLinks();
	
	/// Emitterの寿命が終わったら削除する
	for (auto it = emitterList_.begin(); it != emitterList_.end(); ) {
		auto& emitter = it->second;

		emitter->ClearEmittingData();
		emitter->ClearDexpiredData();;

		if (emitter->GetIsEnd() && emitter->GetIsFinished()) {
			it = emitterList_.erase(it);
		} else {
			++it;
		}
	}
}

void ParticleManager::Draw() {

	for (auto& [id, emitter] : emitterList_) {
		emitter->Draw();
	}

}

int ParticleManager::CreateEmitter(const ParticlePrototype& proto, int maxParticles) {

	/// Emitterの名前がすでに存在する場合、既存のEmitterIdを返す
	if (emitterNameToId_.find(proto.name) != emitterNameToId_.end()) {
		int id = emitterNameToId_[proto.name];
		return id;
	}

	/// 新しいEmitterを作成
	int id = nextEmitterId_++;
	emitterList_[id] = std::make_unique<ParticleEmitter>(system_, proto, maxParticles);
	emitterNameToId_[proto.name] = id;
	return id;
}

void ParticleManager::ClearEmitter(int emitterId) {

	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		emitterList_.erase(emitterId);
		emitterNameToId_.erase(it->second->GetPrototype().name);
	} else {
		Logger::Log("[kError] ParticleManager::ClearEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}

void ParticleManager::ShootEmitter(int emitterId, int count) {
	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		it->second->Emit(count);
	} else {
		Logger::Log("[kError] ParticleManager::ShootEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}

void ParticleManager::SwitchEmitter(int emitterId, bool isEmitting) {
	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		it->second->SwitchEmitter(isEmitting);
	} else {
		Logger::Log("[kError] ParticleManager::SwitchEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}

void ParticleManager::LinkEmitterToEmitter(EmitterLink& linkData) {

	/// リンクデータを保存
	EmitterLink emitterLinks{ linkData };

	/// リンクデータをリストに追加
	emitterLinks_.push_back(emitterLinks);
}

void ParticleManager::SetEmitterEnd(int emitterId, bool isEnd) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) { 
		Logger::Log("[kEngine] ParticleManager::SetEmitterEnd() Emitter ID not found: " + std::to_string(emitterId));
		return;
	}

}

void ParticleManager::SetEmitterDead(int emitterId) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) {
		Logger::Log("[kEngine] ParticleManager::SetEmitterEnd() Emitter ID not found: " + std::to_string(emitterId));
		return;
	}

}

int ParticleManager::GetEmitterParticleCount(int emitterId) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) { return -1; }

	/// 存在する場合、Emitterの粒子数を返す
	return emitterList_[emitterId]->GetParticleCount();
}

int ParticleManager::GetEmitterIdByName(const std::string& name) const {

	/// 名前からEmitterIdを取得
	auto it = emitterNameToId_.find(name);
	if (it != emitterNameToId_.end()) {
		return it->second;
	}
	return -1;
}

int ParticleManager::GetAllParticleCount() {
	int totalCount = 0;
	for (const auto& [id, emitter] : emitterList_) {
		totalCount += emitter->GetParticleCount();
	}
	return totalCount;

}

void ParticleManager::UpdateEmitterLinks() {

	for (int i = 0; i < emitterLinks_.size(); i++) {

		auto& linkData = emitterLinks_[i];

		auto sourceIt = find_if(
			emitterList_.begin(),
			emitterList_.end(),
			[&linkData](const auto& pair) {
			return pair.second->GetPrototype().name == linkData.sourceName;
		});
		auto targetIt = find_if(
			emitterList_.begin(),
			emitterList_.end(),
			[&linkData](const auto& pair) {
				return pair.second->GetPrototype().name == linkData.targetName;
			});

		if (sourceIt == emitterList_.end()) {
			Logger::Log("[kError] ParticleMkanager::UpdateEmitterLinks() SourceParticle Name not found: " + linkData.sourceName);
			continue;
		}
		if (targetIt == emitterList_.end()) {
			Logger::Log("[kError] ParticleManager::UpdateEmitterLinks() TargetParticle Name not found: " + linkData.targetName);
			continue;
		}

		/// 連動処理
		ParticleEmitter* sourceEmitter = sourceIt->second.get();

		std::vector<ParticleInstance> emittingData;
		if (linkData.emitterTiming != EmitterTiming::SourceEnd) {
			emittingData = sourceEmitter->GetEmittingData();
			if (emittingData.size() != 0) {
				Logger::Log("[kInfo] ParticleManager::UpdateEmitterLinks() Found " + std::to_string(emittingData.size()) + " emitting particles in source emitter Name: " + linkData.sourceName);
			}
		} else {
			emittingData = sourceEmitter->GetDexpiredData();
			if (emittingData.size() != 0) {
				Logger::Log("[kInfo] ParticleManager::UpdateEmitterLinks() Found " + std::to_string(emittingData.size()) + " Dexpired particles in source emitter Name: " + linkData.sourceName);
			}
		}

		/// 発射するデータがない場合はスキップ
		if (emittingData.empty())
			continue;


		/// 発射したデータがある場合は、delayDataList_に追加する
		std::vector<const ParticleInstance*> eventUnits;

		if (linkData.linkMode == LinkMode::PerBurst) {
			eventUnits.push_back(&emittingData[0]);
		} else {
			for (const auto& p : emittingData) {
				eventUnits.push_back(&p);
			}
		}

		std::vector<delayData> newDelayDataList;

		/// 発射したらデータはあるのでチェック
		for (auto* p : eventUnits) {

			delayData d;
			d.emitterLinkIndex = i;
			d.sourceEmitterName = linkData.sourceName;
			d.targetEmitterName = linkData.targetName;

			if (linkData.linkFollow == LinkFollow::Emitter) {
				d.followParticle = false;
				d.particleId = -1;

				d.positionOffset = sourceEmitter->GetPosition();
				d.rotationOffset = sourceEmitter->GetRotation();
				d.scaleOffset = sourceEmitter->GetScale();
			} else {
				d.followParticle = true;
				d.particleId = p->particleId;
				d.positionOffset = p->nowTranslate;
				d.rotationOffset = p->nowRotate;
				d.scaleOffset = p->nowScale;
			}

			if (linkData.emitterTiming == EmitterTiming::SourceEmit ||
				linkData.emitterTiming == EmitterTiming::SourceEnd) {
				d.activeNow = true;
				d.timer.Init0(0.0f, system_->GetTimeManager());
			} else {
				d.activeNow = false;
				d.timer.Init0(linkData.delayTime, system_->GetTimeManager());
			}

			d.emittyCount = linkData.emitCount;

			newDelayDataList.push_back(d);
		}

		// newDelayDataListをdelayDataList_に追加
		delayDataList_.insert(
			delayDataList_.end(),
			newDelayDataList.begin(),
			newDelayDataList.end()
		);
	}

	/// delayDataList_内のデータにより、Emitterを発射や生成する
	for (auto& delayData : delayDataList_) {

		/// タイマーが動いてて、タイマーがMixになるかactiveNowであるか、次のParticleをEmittingする
		bool ready =
			(delayData.activeNow) ||        // 立即觸發
			(delayData.timer.ToMix());      // 延遲觸發

		if (!ready) continue;

		auto& linkData = emitterLinks_[delayData.emitterLinkIndex];
		int targetId = GetEmitterIdByName(linkData.targetName);
		int sourceId = GetEmitterIdByName(linkData.sourceName);
		if (targetId == -1) {
			Logger::Log("[kError] ParticleManager::UpdateEmitterLinks() Invalid emitter name: " + linkData.targetName);
			continue;
		}
		if (sourceId == -1) {
			Logger::Log("[kError] ParticleManager::UpdateEmitterLinks() Invalid emitter name: " + linkData.sourceName);
			continue;
		}

		auto& targetEmitter = emitterList_[targetId];
		auto& sourceEmitter = emitterList_[sourceId];

		ParticlePrototypeOverride protoOverride{};
		protoOverride.hasStartPosition = true;
		if (linkData.followScale)protoOverride.hasStartScale = true;
		if (linkData.followRotation)protoOverride.hasStartRotation = true;

		auto particle = sourceEmitter->FindParticleById(delayData.particleId);
		if (linkData.followPosition) {
			protoOverride.startPosition = (particle) ?
				protoOverride.startPosition = particle->nowTranslate :
				protoOverride.startPosition = delayData.positionOffset;
		} else {
			protoOverride.startPosition = delayData.positionOffset;
		}

		if (linkData.followScale) {
			protoOverride.startScale = (particle) ?
				protoOverride.startScale = particle->nowScale :
				protoOverride.startScale = delayData.scaleOffset;
		} else {
			protoOverride.startScale = delayData.scaleOffset;
		}
		if (linkData.followRotation) {
			protoOverride.startRotation = (particle) ?
				protoOverride.startRotation = particle->nowRotate :
				protoOverride.startRotation = delayData.rotationOffset;
		} else {
			protoOverride.startRotation = delayData.rotationOffset;
		}

		targetEmitter->Emit(delayData.emittyCount, &protoOverride);

		delayData.hasEmitted = true;
	}

	/// 無序刪除
	for (int i = (int)delayDataList_.size() - 1; i >= 0; --i) {
		if (delayDataList_[i].hasEmitted) {
			delayDataList_[i] = std::move(delayDataList_.back());
			delayDataList_.pop_back();
		}
	}
}

bool ParticleManager::EmitterIDCheckMiss(int emitterId) {

	if (emitterList_.find(emitterId) == emitterList_.end()) {
		return false;
	}
	Logger::Log("[EffectManager] ParticleManager::EmitterIDCheckMiss Invalid emitter ID: " + std::to_string(emitterId));
	return true;
}

