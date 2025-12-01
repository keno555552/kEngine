#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "Scene.h"
#include "randomMaker.h"

class Particle :public Object
{
public:
	void IntParticele(kEngine* system);
	~Particle();

	void Update(Camera* camera) override;

	/// ========== 全体設定 ==========///
	void SetCommonMaterialConfig(const MaterialConfig& material);

	void SetRoot(Vector3 pos);


	/// ========= ランダム関連 ==========///
	void SetSeed(unsigned int seed);
	int GetSeedGenerator() { return randomMaker_->GetSeedGenerator(); }

	void SetTimer(float maxTime);

protected:
	/// ========= 共通関数 ==========///
	float defaultParticleInterval_ = 0.5f;

protected:

	/// ========== 共通マテリアル ==========///
	std::shared_ptr<MaterialConfig>commonMaterialConfig;

	/// =========== ランダム関連 ===========///
	RandomMaker* randomMaker_ = new RandomMaker();
	Timer createTimer{};

};

