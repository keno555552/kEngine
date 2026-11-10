#include "ParticleManager.h"
#include "kEngine.h"


delayData InputLinkData(
	const EmitterLink& linkData,
	const ParticleInstance& targetParticle,
	const ParticleEmitter& sourceEmitter,
	TimeManager* timeManager,
	int linkIndex) {

	delayData d;
	d.emitterLinkIndex = linkIndex;
	d.sourceEmitterName = linkData.sourceName;
	d.targetEmitterName = linkData.targetName;

	if (linkData.linkFollow == LinkFollow::Emitter) {
		d.followParticle = false;
		d.particleId = -1;

		/// 「その粒子が撃たれた時の位置」（線の今の位置だと、同じフレームに別の場所で撃った時にずれる）
		d.positionOffset = targetParticle.startTranslate;
		d.rotationOffset = sourceEmitter.GetRotation();
		d.scaleOffset = sourceEmitter.GetScale();
	} else {
		d.followParticle = true;
		d.particleId = targetParticle.particleId;
		d.positionOffset = targetParticle.nowTranslate;
		d.rotationOffset = targetParticle.nowRotate;
		d.scaleOffset = targetParticle.nowScale;
	}

	if (linkData.emitterTiming == EmitterTiming::SourceEmit ||
		linkData.emitterTiming == EmitterTiming::SourceEnd) {
		d.activeNow = true;
		d.timer.Init0(0.0f, timeManager);
	} else {
		d.activeNow = false;
		d.timer.Init0(linkData.delayTime, timeManager);
	}

	d.emittyCount = linkData.emitCount;

	return d;
}


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

	/// 先に消すものを集めて、後でまとめて消す（ループ中に map を変えない）
	std::vector<int> removeList;
	for (auto& [id, emitter] : emitterList_) {
		emitter->ClearEmittingData();
		emitter->ClearDexpiredData();

		/// 明示的に終わらせて、粒子も全部死んだ
		if (emitter->GetIsEnd() && emitter->GetIsFinished()) {
			removeList.push_back(id);
		}
	}
	for (int id : removeList) RemoveEmitter(id);
}

void ParticleManager::Draw() {

	for (auto& [id, emitter] : emitterList_) {
		emitter->Draw();
	}

}

int ParticleManager::CreateEmitter(const ParticlePrototype& proto, int maxParticles) {

	/// 同じ名前の線がある → Prototype だけ差し替える（飛んでいる粒子はそのまま）
	const int existing = GetEmitterIdByName(proto.name);
	if (existing != -1) {
		auto& emitter = emitterList_[existing];
		emitter->SetPrototype(proto);
		emitter->SwitchEmitter(true);    // 引退中だった場合に戻す
		emitter->SetIsEnd(false);
		return existing;
	}

	/// 新しい線
	const int id = nextEmitterId_++;
	emitterList_[id] = std::make_unique<ParticleEmitter>(system_, proto, &EMParticleID, maxParticles);
	return id;
}

void ParticleManager::ClearEmitter(int emitterId) {
	if (EmitterIDCheckMiss(emitterId)) return;
	RemoveEmitter(emitterId);
}

void ParticleManager::ClearEmitterLinks() {
	emitterLinks_.clear();
	delayDataList_.clear();
	warnedLinks_.clear();
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

	/// 同じ名前の Link がある → 中身を差し替える（増えない）
	for (auto& link : emitterLinks_) {
		if (link.name == linkData.name) {
			link = linkData;
			return;
		}
	}

	/// 無ければ追加
	emitterLinks_.push_back(linkData);
}

void ParticleManager::RemoveEmitter(int id) {
	emitterList_.erase(id);
}

void ParticleManager::RetireEmitter(int emitterId) {
	if (EmitterIDCheckMiss(emitterId)) return;
	emitterList_[emitterId]->SwitchEmitter(false);   // Emit() が何もしなくなる（Link からも撃てない）
	emitterList_[emitterId]->SetIsEnd(true);         // 粒子が全部死んだら Update で削除
}

void ParticleManager::SetEmitterEnd(int emitterId, bool isEnd) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) return;
	emitterList_[emitterId]->SetIsEnd(isEnd);
}

void ParticleManager::SetEmitterDead(int emitterId) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) return;
	emitterList_[emitterId]->SetIsDead();
}

void ParticleManager::UpdatePrototype(int emitterId, const ParticlePrototype& proto, int maxParticles) {
	if (EmitterIDCheckMiss(emitterId)) return;
	emitterList_[emitterId]->SetPrototype(proto);
}

int ParticleManager::GetEmitterParticleCount(int emitterId) {

	/// EmitterIdが存在するか確認
	if (EmitterIDCheckMiss(emitterId)) { return -1; }

	/// 存在する場合、Emitterの粒子数を返す
	return emitterList_[emitterId]->GetParticleCount();
}

int ParticleManager::GetEmitterIdByName(const std::string& name) const {
	/// 線の数は少ないので、そのまま全部見る
	for (const auto& [id, emitter] : emitterList_) {
		if (emitter->GetName() == name) return id;
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

	/// EmitterLinkが存在しない場合は処理をスキップ
	if (emitterLinks_.empty()) return;
	/// Emitterが存在しない場合は処理をスキップ
	if (emitterList_.empty()) return;

	std::vector < std::pair < std::string, int >> particleIdList; // <emitterID, < linkName, particleId>>

	for (auto& [emitterID, currentEmitter] : emitterList_) {

		const std::string& currectEmitterName = currentEmitter->GetPrototype().name;


		for (int linkID = 0; linkID < (int)emitterLinks_.size(); linkID++) {
			const auto& linkData = emitterLinks_[linkID];
			std::string currectLinkname = linkData.name;
			if (linkData.sourceName != currectEmitterName) continue;

			const auto& events = (linkData.emitterTiming == EmitterTiming::SourceEnd)
				? currentEmitter->GetDexpiredData()
				: currentEmitter->GetEmittingData();

			for (const auto& p : events) {

				// もし既に発動したらスキップする
				auto finder = std::find_if(
					particleIdList.begin(),
					particleIdList.end(),
					[currectLinkname, p](std::pair<std::string, int>& item) {
						return (item.first == currectLinkname && item.second == p.particleId);
					}
				);
				if (finder != particleIdList.end()) continue;

				delayData d = InputLinkData(linkData, p, *currentEmitter, system_->GetTimeManager(), linkID);
				delayDataList_.push_back(d);

				particleIdList.push_back(std::make_pair(currectLinkname, p.particleId));

				if (isLinkLogEnabled_) {
					Logger::Log("[Link] " + currectLinkname + " : " + currectEmitterName + " -> " + linkData.targetName
						+ " (particle " + std::to_string(p.particleId) + ")");
				}
			}
		}
	}



	/// delayDataList_内のデータにより、Emitterを発射や生成する
	for (auto& delayData : delayDataList_) {

		/// タイマーが動いてて、タイマーがMixになるかactiveNowであるか、次のParticleをEmittingする
		bool ready =
			(delayData.activeNow) ||        // 立即觸發
			(delayData.timer.ToMix());      // 延遲觸發

		if (!ready) continue;

		/// Link が消されていた → 捨てる
		if (delayData.emitterLinkIndex < 0 || delayData.emitterLinkIndex >= (int)emitterLinks_.size()) {
			delayData.hasEmitted = true;
			continue;
		}
		auto& linkData = emitterLinks_[delayData.emitterLinkIndex];

		/// 処理済みにする（成功でも失敗でも、この delayData は今回で終わり → 下で消される）
		delayData.hasEmitted = true;

		/// target が無い → 撃つ先が無いので捨てる（1回だけ Log）
		int targetId = GetEmitterIdByName(linkData.targetName);
		if (targetId == -1) {
			/// 同じ Link の失敗は1回だけ Log（連発中に毎回出さない）
			if (warnedLinks_.insert(linkData.name).second) {
				Logger::Log("[kError] Link \"" + linkData.name + "\" target not found: " + linkData.targetName + " (not fired)");
			}
			continue;
		}
		auto& targetEmitter = emitterList_[targetId];

		/// source は「追従する粒子を探す」ためだけに使う。無ければ保存してある位置を使う
		int sourceId = GetEmitterIdByName(linkData.sourceName);
		ParticleInstance* particle = (sourceId != -1)
			? emitterList_[sourceId]->FindParticleById(delayData.particleId)
			: nullptr;

		ParticlePrototypeOverride protoOverride{};
		protoOverride.hasStartPosition = true;
		if (linkData.followScale)protoOverride.hasStartScale = true;
		if (linkData.followRotation)protoOverride.hasStartRotation = true;

		if (linkData.followPosition) {
			protoOverride.startPosition = (particle) ? particle->nowTranslate : delayData.positionOffset;
		} else {
			protoOverride.startPosition = delayData.positionOffset;
		}

		if (linkData.followScale) {
			protoOverride.startScale = (particle) ? particle->nowScale : delayData.scaleOffset;
		} else {
			protoOverride.startScale = delayData.scaleOffset;
		}

		if (linkData.followRotation) {
			protoOverride.startRotation = (particle) ? particle->nowRotate : delayData.rotationOffset;
		} else {
			protoOverride.startRotation = delayData.rotationOffset;
		}

		targetEmitter->Emit(delayData.emittyCount, &protoOverride);
	}

	/// 無序刪除
	delayDataList_.erase(
		std::remove_if(delayDataList_.begin(), delayDataList_.end(),
			[](const delayData& d) { return d.hasEmitted; }),
		delayDataList_.end());
}

bool ParticleManager::EmitterIDCheckMiss(int emitterId) {

	if (emitterList_.find(emitterId) != emitterList_.end()) return false;
	Logger::Log("[EffectManager] ParticleManager::EmitterIDCheckMiss Invalid emitter ID: " + std::to_string(emitterId));
	return true;
}

