#pragma once
#include <vector>
#include <map>
#include "config.h"
#include "DirectXCore.h"
#include "ParticleEmitter.h"

class kEngine;
class ParticleManager
{
public:
	ParticleManager(kEngine* system);
	~ParticleManager();

	/// --- 基本関数 --- ///


	void Update();
	void Draw();

	/// --- エミッタ管理 --- ///
	int CreateEmitter(const ParticlePrototype& proto, int maxParticles);
	void ClearEmitter(int emitterId);

	/// 手動発射
	void ShootEmitter(int emitterId, int count);
	void SwitchEmitter(int emitterId, bool isEmitting);

private:

	/// 借りのDevice
	kEngine* system_ = nullptr;

	/// 粒子生成器
	std::map<int, std::unique_ptr<ParticleEmitter>> emitterList_;
	int nextEmitterId_ = 0;


};