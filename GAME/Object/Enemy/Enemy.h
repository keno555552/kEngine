#pragma once
#include "Object/Object.h"
#include "kEngine.h"
#include "Math\Geometry\Collision\crashDecision.h"

/// 移動速度
static inline const float kEnemyAcceleration = 1.0f;

/// 回転速度
static inline const float kEnemyTurnTime = 0.15f;

/// 敵被ダメージエフェクト
static inline const float kEnemyDamageEffectTime = 0.1f;

/// アニメーション速度
static inline const float kEnemyAnimationTime = 1.0f;
static inline const float kEnemyAnimationSmallSize = 0.7f;
static inline const float kEnemyAnimationBigSize = 1.0f;

class Enemy :public Object
{
	public:
	Enemy(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });
	~Enemy();
	void Update(Camera* camera) override;
	void Draw() override;
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }
	bool IsAlive() const { return isAlive_; }
	void SetAlive(bool isAlive) { isAlive_ = isAlive; }

	int GetHP() const { return hp_; }
	void SetHP(int hp) { hp_ = hp; }
	void GetDamage(int damage = 1);
	
	void SetFaceRight(bool isFaceRight) { isFaceRight_ = isFaceRight; }
	bool IsFaceRight() const { return isFaceRight_; }

	AABB& GetAABB();

private:

	/// ======== モデルハンドル ======== ///
	int TH_White5x5_{};
	int TH_Model_{};

	int SH_Damage_{};

	/// ========== 回転関連 ========== ///
	/// ターンレート
	float turningRate_ = 0.0f;
	/// ターンタイマー
	Timer turnTimer_;

	/// ========== 状態関連 ========== ///
	Timer damageEffectTimer_;

	/// ===== アニメーション関連 ===== ///

	Timer animationTimer_;
	bool isTimeUp_ = true;

private:

	/// 当たり判定
	AABB hitBox_{};

	/// HP
	int hp_ = 2;

	/// フラグ
	bool isAlive_ = true;
	/// Face right
	bool isFaceRight_ = true;
};

