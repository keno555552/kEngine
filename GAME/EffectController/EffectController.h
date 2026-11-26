#pragma once
#include <array>
#include <random>
#include <vector>
#include "Vector3.h"

class kEngine;

/// このゲームで使うエフェクト（増えたらここと .cpp の kTable に追加）
enum class GameFx {
	HitRock,     // 弾が壁に当たった
	BreakDirt,   // ブロックが壊れた（3種類からランダム）
	BreakRock,   // ブロックが壊れた（3種類からランダム）
	Count
};

/// ゲーム全体のエフェクト窓口。パスはここだけが知っている
class EffectController
{
public:
	/// 全エフェクトを読み込む（各シーンの Initialize で呼んでよい：2回目からは同じハンドル）
	static void Initialize(kEngine* system, bool isMust = false);
	static void Finalize();
	/// 発射。複数登録されていればランダムで1つ
	static void Play(GameFx fx, const Vector3& position);

	static void Load(GameFx fx, const char* path);

private:
	static inline kEngine* system_ = nullptr;
	static inline std::array<std::vector<int>, (size_t)GameFx::Count> handles_{};
	static inline std::mt19937 rng_{ std::random_device{}() };
};