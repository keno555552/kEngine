#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "BaseScene.h"
#include "GameObject/Object/Corner.h"
#include "crashDecision/crashDecision.h"


/// 移動速度
static inline const float kAcceleration = 3.0f;
/// 移動減衰
static inline const float kAttenuation = 8.0f;
/// 回転速度
static inline const float kTimeTurn = 0.15f;
/// 移動限界速度
static inline const float kLimitRunSpeed = 3.0f;
/// 重力加速度
static inline const float kGravityAcceleration = 10.0f;
/// 最大落下速度
static inline const float kLimitFallSpeed = 10.0f;
/// ジャンプ初速
static inline const float kJumpAcceleration = 7.0f;
/// キャラクターの当たり判定サイズ
static inline const float kPlayerWidth = 1.0f;
static inline const float kPlayerHeight = 1.0f;
/// 着地時の速度減衰率
static inline const float kAttenuationLanding = 0.00000001f;

/// 射撃間隔
static inline const float kShootInterval = 0.25f;

/// 被ダメージエフェクト
static inline const float kPlayerDamageEffectTime = 1.0f;
/// プレイヤーのヒットボックス拡大率
static inline const float kPlayerHitBoxScale = 0.85f;


/// ================== 近接攻撃関連
/// 近接攻撃判定ボックスサイズ
static inline const float kNearAttackBoxWidth = 1.5f;
static inline const float kNearAttackBoxHeight = 1.2f;
/// 近接攻撃モーション時間
static inline const float kNearAttackChangeTime = 0.2f;
static inline const float kNearAttackEffectTime = 0.5f;


class Bullet;
class Player :public Object
{
public:
	Player(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });

	void Update(Camera* camera) override;
	AABB& GetAABB();
	AABB& GetMapChipAABB();
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	bool IsOnGround() { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

	void InputBulletList(std::vector<Bullet*>* bulletList) { bulletList_ = bulletList; };

	int GetHP() const { return static_cast<int>(HP_); }
	void SetHP(int hp) { HP_ = static_cast<float>(hp); }
	void GetDamage(float damage);

	bool IsDisableDamage() const { return disableDamage_; }

	void Shoot(Vector3 mousePos);

private:

	enum class LRDirection { kRight, kLeft, None };

private:

	std::vector<std::function<void()>> function;

private:

	/// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceilingHit = false; // 天井に当の当たり判定
		bool floorHit = false;   // 床に当の当たり判定
		bool wallHit = false;    // 壁に当の当たり判定
		Vector3 moveVector = {}; // 移動量
	};

private:

	void BehaviorRootUpdate();
	Vector3 CornerPosition(const Vector3& center, Corner4 corner);
	void Move();
	void OnGroundChanger(const CollisionMapInfo& info);
	void MapCollisionDecideDown(CollisionMapInfo& info);
	void MovePlayerByResult(const CollisionMapInfo& info);

	void ShootUpdate();
	void NearAttackUpdate();

	void DamageAndEffectPart();

private:
	///モデルハンドル
	int MH_playerModel_{};
	int MH_pickaxe_{};

	/// テキスチャハンドル
	int TH_playerTexture_{};
	int TH_White5x5_{};

	///サントハンドル
	int SH_JUMP_ = 0;
	int SH_SHOOT_ = 0;
	int SH_DAMAGE_ = 0;
private:
	/////// State
	/// HP
	float HP_ = 3.0f;

	/////// CD
	/// dushCD
	float dushCD_ = 1.0f; ///(秒)
	float dushCDParameter_ = dushCD_ * 60.0f;

	/// Bullet
	float kBulletCD_ = 0.25; ///(秒)
	float kBulletParameter_ = kBulletCD_ * 60.0f;

	/// DamageCD
	float kDamageCD_ = 1.0f; ///(秒)
	float kDamageParameter_ = kDamageCD_ * 60.0f;

	/// State
	enum class PlayerState {
		kNormal,
		kAttark,
		kDush,
		kDamage,
	};
	PlayerState playerState_ = PlayerState::kNormal;

private:



	/// オブジェクト
	Object* pickaxe_ = nullptr;

	//////// プレイヤーデータ
	/// 移動速度
	Vector3 velocity_ = {};
	/// 方向/開始時の方向
	LRDirection lrDirection_ = LRDirection::kRight;
	/// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	/// 旋回タイマー
	float turnTimer_{};
	/// 接地状態フラグ
	bool onGround_ = true;

	float kBlank = 0.0f; // ジャンプ中のブランク時間

private:
	// 計算用
	float deltaTime_{};

private:
	AABB hitBox_{};

	bool isDamage_ = false;
	// デスフラグ
	bool isDead_ = false;

private:
	Timer damageEffectTimer_;
	bool disableDamage_ = false;

private:
	std::vector<Bullet*>* bulletList_ = nullptr;

	Timer shootCD_;

private:
	bool isNearAttack_ = false;
	bool isNearAttackEffect_ = false;

	AABB nearAttackBox_{};
	Timer nearAttackChangeTimer_;
	Timer nearAttackEffectTimer_;

};

