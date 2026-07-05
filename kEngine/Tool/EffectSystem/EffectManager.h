#pragma once
#include "Tool/EffectSystem/ParticleSystem/ParticleManager.h"
#include "Tool/EffectSystem/ParticleSystem/EffectInstance.h"

class kEngine;
class EffectManager
{
public:

	/// --- コンストラクタ・デストラクタ --- ///
	EffectManager() = default;
	~EffectManager() = default;

	/// --- システム管理 --- ///
	void Initialize(kEngine* system);
	void Finalize();

	/// --- 基本行為 --- ///
	void Update();
	void Draw();

	/// --- Effect管理 ---///
	/// 追加する。もし同じEffectDataが存在する場合はそのハンドルを返す。
	int AddEffect(const EffectData& data);
	/// エフェクトの内容を更新する
	std::string UpdateEffect(int handle,EffectData& data);
	/// エフェクトの内容をクリアする
	void ResetEffect(int handle);
	/// エフェクトの発射状態を全てリセットする
	void ClearEffect(int handle);
	/// エフェクトを削除する,発射したパーティクルは残る
	void RemoveEffect(int handle);

	/// --- エフェクトマネージャーをとる --- ///
	ParticleManager* GetParticleManager() { return particleManager_.get(); }

private:

	/// 借りのDevice
	kEngine* system_ = nullptr;

	/// 持ってるドライブ
	std::unique_ptr<ParticleManager> particleManager_;

	/// エフェクトインスタンスのリスト
	std::map<int, EffectInstance> effectInstances_;
	std::vector<int> effectHandles_;
	int counter_ = 0;

private:

	/// --- 内部関数 --- ///
	/// Dataがすでに存在するかを確認する。存在する場合はそのハンドルを返す。存在しない場合は-1を返す。
	int CheckHaveData(const EffectData& data);

};

