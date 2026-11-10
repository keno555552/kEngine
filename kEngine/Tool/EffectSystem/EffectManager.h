#pragma once
#include <string>
#include <vector>
#include <memory>
#include "Tool/EffectSystem/ParticleSystem/ParticleManager.h"
#include "Tool/EffectSystem/ParticleSystem/EffectData.h"

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

	/// --- Effect の登録 --- ///
	/// json ファイルを読んで登録。同じパスは2回目から同じハンドル。失敗したら -1
	int LoadEffect(const std::string& path);
	/// メモリ上のデータで登録（エディタのプレビュー用など）
	/// 同じ key がもうあれば中身を更新：同名の線は SetPrototype、無くなった線は引退、Link は同名上書き・無くなった物は削除
	/// ほかの Effect には一切触らない
	int RegisterEffect(const std::string& key, const EffectData& data);
	/// ファイルから読んだ Effect を全部読み直す（エディタで保存した後に呼ぶ → ゲーム側にすぐ反映）
	void ReloadAll();
	/// Effect を外す：線は引退（飛んでいる粒子は最後まで）、Link は削除。ハンドルはもう使えない
	void UnloadEffect(int handle);

	/// --- 使う --- ///
	/// 指定した位置で発射（isRoot の線を撃つ。後は Link が繋ぐ）
	void Play(int handle, const Vector3& position);
	/// この Effect の線だけ一時停止
	void PauseEffect(int handle, bool pause);
	/// Effect の中の線の id（個別に止める・消すなど用）。無ければ -1
	int GetLine(int handle, const std::string& particleName) const;
	bool HasRoot(int handle) const;

	/// --- エフェクトマネージャーをとる --- ///
	ParticleManager* GetParticleManager() { return particleManager_.get(); }

private:

	/// 登録した Effect 1つ分
	struct LoadedEffect {
		bool isActive = false;
		std::string key;                                    // LoadEffect ならパス
		std::string path;                                   // ファイルから読んだ時だけ（ReloadAll 用）
		std::string prefix;                                 // 線・Link の名前の頭（"0:HitRock/"）
		std::vector<std::pair<std::string, int>> lines;     // 元の Particle 名, 線の id
		std::vector<std::string> linkNames;                 // ParticleManager に入れた Link の名前（prefix 付き）
		std::vector<int> rootIds;                           // Play で撃つ線
		bool isPaused = false;
	};

	/// 中身を ParticleManager に反映（新規・更新どちらも）
	void ApplyEffect(LoadedEffect& effect, const EffectData& data);
	int FindByKey(const std::string& key) const;
	bool IsValid(int handle) const;

	/// 借りのDevice
	kEngine* system_ = nullptr;

	/// 持ってるドライブ
	std::unique_ptr<ParticleManager> particleManager_;

	/// handle = index（Unload しても詰めない → ほかのハンドルがずれない）
	std::vector<LoadedEffect> effects_;
};