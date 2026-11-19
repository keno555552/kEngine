#pragma once

/// ゲーム全体の設定値（データだけ。処理は持たない）
/// static inline なので .cpp に定義は要らない。ImGui からも直接いじれる
class GameConfig {
public:
	GameConfig() = delete;   // インスタンスは作らない

	/// ================= 物理 ================= ///
	static inline float gravity = 10.0f;          // 落下加速度
	static inline float limitFallSpeed = 10.0f;   // 最大落下速度
};