#pragma once
#include <vector>
#include <map>
#include <unordered_set>
#include "DirectXCore.h"
#include "ParticlePrototype.h"
#include "ParticleInstance.h"
#include "Utility/RandomMaker.h"
#include "Math/Geometry/Interpolation.h"

class kEngine;
class ParticleEmitter
{
public:

	/// --- コンストラクタ・デストラクタ --- ///
	ParticleEmitter(kEngine* system,
					const ParticlePrototype particlePrototype,
					int maxParticles = 1000);
	~ParticleEmitter();

	void SetPosition(const Vector3& pos) { emitterPosition = pos; }
	void SetRotation(const Vector3& rot) { emitterRotation = rot; }


	/// --- 基本行為 --- ///
	void Update();
	void Emit(int count = 1); // 手動発射
	void SwitchEmitter(bool isEmitting);

	void Draw();

private:

	/// 借りのDevice
	kEngine* system_ = nullptr;

	/// 粒子のプロトタイプ
	ParticlePrototype prototype_{};
	int maxParticles_{};

	/// 発射された粒子のインスタンス
	std::vector<ParticleInstance> particles_;

	/// 発射するもののタイプ
	ParticleRenderType isObjectType_ = ParticleRenderType::Object; // true = Object, false = Sprite

	/// 発射器の設定
	Timer emitTimer_;
	Vector3 emitterPosition = { 0,0,0 };
	Vector3 emitterScale = { 1,1,1 };
	Vector3 emitterRotation = { 0,0,0 };

	/// 乱数生成器
	std::shared_ptr<RandomMaker> randomMaker_ = std::make_unique<RandomMaker>();

	/// 更新速度
	float timeScale_ = 1.0f;

	/// 粒子のスウィッチ
	bool isEmitting_ = true;
	bool isPaused_ = false;

};