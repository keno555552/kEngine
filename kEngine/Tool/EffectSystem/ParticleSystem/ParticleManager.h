#pragma once
#include <vector>
#include <map>
#include <string>
#include <set>
#include "config.h"
#include "DirectXCore.h"
#include "ParticleEmitter.h"
#include "EmitterLink.h"


struct delayData {
	bool hasEmitted{ false };
	bool activeNow{ false };
	bool followParticle{ false };
	int emittyCount{ 1 };
	int emitterLinkIndex{ -1 };
	std::string sourceEmitterName{};
	std::string targetEmitterName{};
	int particleId{ -1 };
	Vector3 positionOffset{};
	Vector3 rotationOffset{};
	Vector3 scaleOffset{};
	Timer timer{};

};

delayData InputLinkData(
	const EmitterLink& linkData,
	const ParticleInstance& targetParticle,
	const ParticleEmitter& sourceEmitter,
	TimeManager* timeManager,
	int linkIndex);

class kEngine;
class ParticleManager
{
public:
	ParticleManager(kEngine* system);
	~ParticleManager();

	/// --- 基本関数 --- ///

	void Update();
	void Draw();

	/// --- タイマー --- ///
	void Pause(bool pause) { isPaused_ = pause; }

	/// --- エミッタ管理 --- ///
	int CreateEmitter(const ParticlePrototype& proto, int maxParticles);
	void ClearEmitter(int emitterId);
	void ClearEmitterLinks();

	/// 手動発射
	void ShootEmitter(int emitterId, int count);
	void SwitchEmitter(int emitterId, bool isEmitting);

	/// リングパーティクル関連
	//void LinkParticleToObject(int emitterId, Object* object); //まだ準備できてない
	void LinkEmitterToEmitter(EmitterLink& linkData);

	/// データ管理
	void RemoveEmitter(int id);
	/// 線を引退させる：もう撃たない＋粒子が全部消えたら自動で削除（飛んでいる粒子は最後まで残る）
	void RetireEmitter(int emitterId);

	/// データセット
	void SetEmitterEnd(int emitterId, bool isEnd);
	void SetEmitterDead(int emitterId);
	void SetLinkLog(bool enable) { isLinkLogEnabled_ = enable; }
	void UpdatePrototype(int emitterId, const ParticlePrototype& proto, int maxParticles);


	/// データ取得
	int GetEmitterCount() const { return static_cast<int>(emitterList_.size()); }
	int GetEmitterParticleCount(int emitterId);
	int GetEmitterIdByName(const std::string& name) const;
	int GetAllParticleCount();
	bool GetLinkLog() const { return isLinkLogEnabled_; }

private:

	/// 借りのDevice
	kEngine* system_ = nullptr;

	/// 粒子生成器
	std::map<int, std::unique_ptr<ParticleEmitter>> emitterList_;

	int nextEmitterId_ = 0;

	/// Emitterの粒子ID生成用
	int EMParticleID = 0;

	/// Emitter同士のリンク管理
	std::vector<EmitterLink> emitterLinks_;
	std::vector<delayData> delayDataList_;

	/// ポーズ用
	bool isPaused_ = false;

	/// ======= デバッグ用
	/// Link の成功 Log を出すか（失敗は常に出す）
	bool isLinkLogEnabled_ = false;
	/// 失敗の Log を同じ Link で何度も出さないため（ClearEmitterLinks でリセット）
	std::set<std::string> warnedLinks_;

private:
	/// --- 内部関数 --- ///
	void UpdateEmitterLinks();

	///Checker,見つからなければtrueを返す
	bool EmitterIDCheckMiss(int emitterId);
};