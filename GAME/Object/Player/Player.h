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
#include "GAME/Object/Backpack/Backpack.h"
#include "Math/Kinematics/Gravity.h"
#include "GAME/Setting/GameConfig.h"

/// ================== プレイヤースディータス関連
/// HP設定
static inline const float kPlayerMaxHP = 3.0f;

/// ================== 速度関連
/// 移動速度
static inline const float kAcceleration = 10.0f;
/// 移動減衰
static inline const float kAttenuation = 12.0f;
/// 回転速度
static inline const float kTimeTurn = 0.15f;
/// 移動限界速度
static inline const float kLimitRunSpeed = 3.0f;
/// ジャンプ初速
static inline const float kJumpAcceleration = 7.0f;
/// 着地時の速度減衰率
static inline const float kAttenuationLanding = 0.00000001f;

/// ================== キャラクターの当たり判定サイズ
static inline const float kPlayerWidth = 1.0f;
static inline const float kPlayerHeight = 1.0f;
static inline const float kPlayerHitScale = 0.9f;
/// 被ダメージエフェクト
static inline const float kPlayerDamageEffectTime = 1.0f;

/// ================== CD
/// 射撃間隔
static inline const float kShootInterval = 1.0f / 2.0f;
/// 掘削関連
static inline const float kDigReach = 2.0f;       // 何マス先まで掘れるか
static inline const float kDigInterval = 0.25f;   // 掘る間隔（秒）
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

	/// 基本関数
	void Update(Camera* camera) override;
	void Draw() override;

	/// ========== 外部からの指令（受け取るだけ。実行は Update の中） ========== ///
	/// ムーブメント
	void MoveLeft() { input_.left = true; }
	void MoveRight() { input_.right = true; }
	void MoveJump() { input_.jump = true; }
	void PushDown() { input_.down = true; }

	/// 動きストッパー
	void StopLeft() { input_.left = false; }
	void StopRight() { input_.right = false; }
	void StopJump() { input_.jump = false; }
	void StopDown() { input_.down = false; }

	/// 手持ちもの使用
	void RequestUse(const Vector3& aimPos) { input_.use = true; input_.aimPos = aimPos; }
	void RequestUse() { RequestUse(GetAimPosition()); }

	/// ダメージを受ける
	void GetDamage(float damage, const Vector3& attackerPos);

	/// ================ 当たり判定（CollisionManager） ================= ///
	AABB& GetAABB();
	void SetCollisionResult(const CollisionResult& result);

	/// ======================= 位置・移動関連 ========================== ///
	/// 位置・移動関連
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	/// ========================= 情報交流 ============================= ///
	/// 掘るリクエストを受け取る（あれば true、Scene が Break する）
	bool ConsumeDigRequest(Vector3& outPos);
	/// 狙いところ
	Vector3 GetAimPosition() const;
	/// HP管理
	int GetHP() const { return static_cast<int>(HP_); }
	void SetHP(int hp) { HP_ = static_cast<float>(hp); }
	/// 手持ちもの関連
	BPItem& GetInventory() { return inventery_; }
	/// 状態管理
	bool IsDisableDamage() const { return disableDamage_; }
	bool IsDead() const { return playerState_ == PlayerState::kDead; }
	bool IsDeathAnimEnd() const { return deathPhase_ == DeathPhase::kLie; }

	/// ===================== Scene との受け渡し ======================== ///
	/// 弾丸管理
	void InputBulletList(std::vector<Bullet*>* bulletList) { bulletList_ = bulletList; };

	/// ========================= 未分類
	void Shoot() { Shoot(GetAimPosition()); }
	bool IsOnGround() { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

private:

	/// ================== 状態関連 ================== ///
	/// 方向
	enum class LRDirection { kRight, kLeft, None };

	/// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceilingHit = false; // 天井に当の当たり判定
		bool floorHit = false;   // 床に当の当たり判定
		bool wallHit = false;    // 壁に当の当たり判定
		Vector3 moveVector = {}; // 移動量
	};

	/// State
	enum class PlayerState {
		kNormal,
		kDamage,
		kDead,
		numStates
	};

	/// 死亡動画フェイス用
	enum class DeathPhase {
		kNone,
		kFall,
		kBounce,
		kLie
	}deathPhase_ = DeathPhase::kNone;
	Timer deathTimer_;

	/// 入力指令（毎フレーム Update の最後に消す）
	struct Input {
		bool left = false;
		bool right = false;
		bool jump = false;
		bool down = false;
		bool use = false;
		Vector3 aimPos{};
	} input_;

	/// 被弾指令（処理したら消す）
	struct DamageRequest {
		bool has = false;
		float amount = 0.0f;
		Vector3 from{};
	} damageRequest_;

private:
	/// ================== ステートパターン ================== ///
	/// 状態更新の関数配列
	std::array<std::function<void()>, (size_t)PlayerState::numStates> stateUpdateFunc_;
	/// 手持ち使用の関数配列
	std::array<std::function<void(const Vector3&)>, (size_t)BPItemID::kCount> useFunc_;
private:
	/// ================== ハンドル関連 ================== ///
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

	/// 持ち物関連
	std::unique_ptr<Object> pickaxe_ = nullptr;

	/// 照準
	std::unique_ptr<Sight> sight_;

	std::vector<Bullet*>* bulletList_ = nullptr;
private:
	/// ================== プレイヤースディータス ================== ///
	/// HP
	float HP_ = 3.0f;
	/// Player状態
	PlayerState playerState_ = PlayerState::kNormal;
	/// バックパック
	BPItem inventery_{};

	/// ========== 掘削
	Timer digCD_;
	bool hasDigRequest_ = false;
	Vector3 digRequestPos_{};

	/// debug用
	Timer checkTimer_;


	/// ================== 移動関連の ================== ///
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

private:
	// 計算用
	float totelDeltaTime_{};
	AABB hitBox_{};
	Timer damageEffectTimer_;
	bool disableDamage_ = false;

	

private:
	/// ================ 攻撃関連 ================ ///
	Timer shootCD_;

	/// ========== 動作スウイッチ
	bool isNearAttack_ = false;
	bool isNearAttackEffect_ = false;
	AABB nearAttackBox_{};
	Timer nearAttackChangeTimer_;
	Timer nearAttackEffectTimer_;

private:

	Vector3 CornerPosition(const Vector3& center, Corner4 corner);

	void BitMapBugCheck();
	void CDUpdate();
	void NearAttackUpdate();


	/// 状態の更新
	void StateUpdate();
	/// 状態ことの更新
	void NormalUpdate();
	void DamageUpdate();
	void DeathUpdate();
	void UseUpdate();

	/// ============================== 動作の更新
	/// 移動の更新
	void MoveUpdata();

	/// 下処理
	void OnGroundChanger(const CollisionMapInfo& info);
	void MovePlayerByResult(const CollisionMapInfo& info);

	/// 掘り処理
	void Dig(const Vector3& aimPos);
	void Shoot(const Vector3& mousePos);

	/// 射撃処理
	void StartDeath(const Vector3& attackerPos);
	void SetFaceTexture(int handle);

	void ApplyDamageRequest();
	void DamageAndEffectPart();
};

#pragma region TODO（アイデアメモ）
// - ジャンプ中のブランク時間（コヨーテタイム？）：旧 kBlank = 0.0f
// - 行動キュー：std::vector<std::function<void()>> で入力バッファ？
// - 武器切替：isGun_ → 武器 enum に
// - Pickaxe：MH_pickaxe_ はロード済み、Object 生成まだ
#pragma endregion