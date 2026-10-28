#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "BaseScene.h"
#include "Data/Geometry/Corner/Corner.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include "Data/Game/MapChip/CollisionResult.h"

/// ================== プレイヤーデータ関連
static inline const float kPlayerMaxHP = 1.0f;

/// 移動速度
static inline const float kAcceleration = 10.0f;
/// 移動減衰
static inline const float kAttenuation = 12.0f;
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
static inline const float kPlayerHitScale = 0.9f;
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

/// ================== 死亡演出関連
static inline const float kDeathKnockbackX = 3.0f;        // 後ろへの吹っ飛び
static inline const float kDeathKnockbackY = 2.5f;        // 浮き上がり
static inline const float kDeathFallTime = 0.45f;         // 倒れきるまで
static inline const float kDeathFallPower = 3.0f;         // 大きいほど最後に一気に「バタン」
static inline const float kDeathFallAngle = 1.5707963f;  // 後ろ倒れ（逆なら符号反転）
static inline const float kDeathBounceTime = 0.2f;        // 着地の跳ね返り
static inline const float kDeathBounceRatio = 0.15f;      // 跳ね返りで起き上がる割合
static inline const float kDeathAttenuation = 6.0f;       // 着地後の滑り減衰


class Bullet;
class Sight;
class Player :public Object
{
public:
	/// コンストラクタ
	Player(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });
	~Player();

	/// ムーブメント
	void MoveLeft() { moveLeft_ = true; }
	void MoveRight() { moveRight_ = true; }
	void MoveJump() { moveJump_ = true; }

	void StopLeft() { moveLeft_ = false; }
	void StopRight() { moveRight_ = false; }
	void StopJump() { moveJump_ = false; }

	/// =============== ステージ用 =============== ///
	/// ヴァーチャル関数
	void Update(Camera* camera) override;
	void Draw() override;

	/// 当たり判定
	AABB& GetAABB();
	void SetCollisionResult(const CollisionResult& result);
	//AABB& GetMapChipAABB();

	/// 位置・移動関連
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	bool IsOnGround() { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

	/// 射撃関連
	Vector3 GetAimPosition() const;
	void Shoot(Vector3 mousePos);
	void Shoot() { Shoot(GetAimPosition()); }

	/// 弾丸管理
	void InputBulletList(std::vector<Bullet*>* bulletList) { bulletList_ = bulletList; };

	/// HP管理
	int GetHP() const { return static_cast<int>(HP_); }
	void SetHP(int hp) { HP_ = static_cast<float>(hp); }
	void GetDamage(float damage, const Vector3& attackerPos);

	bool IsDisableDamage() const { return disableDamage_; }

	/// 状態管理
	bool IsDead() const { return isDead_; }
	bool IsDeathAnimEnd() const { return deathPhase_ == DeathPhase::kLie; }

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

	/// 死亡動画フェイス用
	enum class DeathPhase { 
		kNone, 
		kFall, 
		kBounce, 
		kLie };
	DeathPhase deathPhase_ = DeathPhase::kNone;
	Timer deathTimer_;

private:
	///モデルハンドル
	int MH_playerModel_{};
	int MH_pickaxe_{};
	int MH_sight_{};

	/// テキスチャハンドル
	int TH_playerTexture_{};
	int TH_playerTexture_damaged_{};
	int TH_playerTexture_dead_{};
	int TH_White5x5_{};

	///サントハンドル
	int SH_JUMP_{};
	int SH_SHOOT_{};
	int SH_DAMAGE_{};
private:
	/////// State
	/// HP
	float HP_ = 3.0f;

	/////// CD
	/// dushCD
	float dushCD_ = 1.0f; ///(秒)
	float dushCDParameter_ = dushCD_ * 60.0f;

	/// NearAttack
	float nearAttackCD_ = 1.0f; ///(秒)
	float nearAttackParameter_ = nearAttackCD_ * 60.0f;

	/// Bullet
	float kBulletCD_ = 0.25; ///(秒)
	float kBulletParameter_ = kBulletCD_ * 60.0f;

	/// DamageCD
	float kDamageCD_ = 1.0f; ///(秒)
	float kDamageParameter_ = kDamageCD_ * 60.0f;

	/// State
	enum class PlayerState {

		kNone = 0,
		kMove = 1 << 0,
		kDamage = 1 << 1,
		kShoot = 1 << 2,
		kNearAttack = 1 << 3,
		kDush = 1 << 4,
		kDead = 1 << 5,

		numStates = 6,///<<< numStatesの値の更新を忘れなく
	};
	PlayerState playerState_ = PlayerState::kNone;

	// 行動可能フラグ
	int cantShoot = static_cast<int>(PlayerState::kDamage) |
					static_cast<int>(PlayerState::kNearAttack) |
					static_cast<int>(PlayerState::kDead);

	Timer checkTimer_;

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
	float totelDeltaTime_{};

private:
	AABB hitBox_{};
	DebugLine debugLine;

	bool isDamage_ = false;
	// デスフラグ
	bool isDead_ = false;

private:
	Timer damageEffectTimer_;
	bool disableDamage_ = false;

private:
	/// ================ 攻撃関連 ================ ///
	/// 照準
	std::unique_ptr<Sight> sight_;

	/// ========== 射撃関連
	std::vector<Bullet*>* bulletList_ = nullptr;
	Timer shootCD_;

	bool isGun_ = true; // 仮、後で武器により改変

	/// ========== 近接関連

private:
	/// move
	bool moveLeft_ = false;
	bool moveRight_ = false;
	bool moveJump_ = false;
	bool moveDush_ = false;

	bool isNearAttack_ = false;
	bool isNearAttackEffect_ = false;

	AABB nearAttackBox_{};
	Timer nearAttackChangeTimer_;
	Timer nearAttackEffectTimer_;


private:

	void BehaviorRootUpdate();
	Vector3 CornerPosition(const Vector3& center, Corner4 corner);
	void Move();
	void ResetMove();
	void OnGroundChanger(const CollisionMapInfo& info);
	void MovePlayerByResult(const CollisionMapInfo& info);

	void BitMapBugCheck();
	void ShootUpdate();
	void NearAttackUpdate();

	void UpdataState();

	void StartDeath(const Vector3& attackerPos);
	void DeathUpdate();
	void SetFaceTexture(int handle);

	void DamageAndEffectPart();
};

#pragma region TODO（アイデアメモ）
// - ジャンプ中のブランク時間（コヨーテタイム？）：旧 kBlank = 0.0f
// - 行動キュー：std::vector<std::function<void()>> で入力バッファ？
// - 武器切替：isGun_ → 武器 enum に
// - Pickaxe：MH_pickaxe_ はロード済み、Object 生成まだ
#pragma endregion