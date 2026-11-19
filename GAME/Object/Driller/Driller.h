#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "BaseScene.h"
#include "Data/Geometry/Corner/Corner.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include "Math/Kinematics/Gravity.h"
#include "GAME/Setting/GameConfig.h"
#include "Tool/MovementObject/MovementObject.h"
#include "Utility/randomMaker.h"


/// リソースパーズ
// GAME/resources/Object/driller/
// driller.gltf

static const float Driller_MaxSpeed = 0.1f;
static const float Driller_DrillMaxSpeed = 0.1f;
static const float Driller_OnOffTime = 0.1f;

/// ===== 落下・ドリル・ドア ===== ///
static inline const float kDrillerCallHeight = 10.0f;		// 生成点の何マス上から降りるか
static inline const float kDrillerCallRangeCell = 5.0f;     // プレイヤーが何マス以内で呼ばれるか
static inline const float kDrillerLandOffsetY = -0.5f;		// 生成点（マスの中心）→ 地面の上
static inline const float kDrillerTipOffset = 0.375f;		// 本体の底 → ドリルの先
static inline const float kDrillerDrillSpinSpeed = 3.0f;    // ドリル回転速度（rad/s）
static inline const float kDrillerDoorTime = 0.6f;			// ドア開閉時間
static inline const float kDrillerDoorOpenAngle = 0.85f;    // ドアの開く角度（rad）
static inline const int	kDrillerDrillHalfWidthCell = 1;		// ドリルの幅：中心から左右に何マス（1 → 左中右の 3 マス）


class Driller :public Object
{
public:
	/// コンストラクタ
	Driller(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });

	/// タイトルムーブメント
	void IsTitle(bool isTitle) { drillerState_ = isTitle ? (int)DrillerState::kTitleMovement : (int)DrillerState::kNormal; }

	/// 基本アップデート
	void Update(Camera* camera) override;
	void Draw() override;
	// title用特別なアップデート
	void TitleMovement();

	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	bool IsOnGround() { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

	/// ================= 操作 ================= ///
	/// 下へ掘り続ける（止まらない）
	void DrillDown();
	/// targetY（本体の底）まで掘って止まる
	void DrillDownTo(float targetY);


	/// 停止点（CollisionManager が見る）
	bool HasStopPoint() const { return hasStopPoint_; }
	float GetStopY() const { return stopY_; }

	/// ドリルの先
	Vector3 GetTipPoint() const;
	/// ブロックを壊した：速度 0 から落ち直す
	void OnDrillHit() { velocity_.y = 0.0f; }
	/// ② で停止点に着いた瞬間を 1 回だけ受け取る
	bool ConsumeArrived() { const bool r = justArrived_; justArrived_ = false; return r; }

	/// 落下を止める（着地・衝突時）
	void StopFall();
	/// ドリルだけ
	void StartDrill() { isDrilling_ = true; }
	void StopDrill() { isDrilling_ = false; }
	/// ドア（アニメーションは自分で再生）
	void OpenDoor();
	void CloseDoor();
	/// 表示
	void Show() { isVisible_ = true; }
	void Hide() { isVisible_ = false; }

	/// ================= 生成点 ================= ///
	void SetSpawnPoint(const Vector3& point) { spawnPoint_ = point; hasSpawnPoint_ = true; }
	const Vector3& GetSpawnPoint() const { return spawnPoint_; }
	bool HasSpawnPoint() const { return hasSpawnPoint_; }
	/// 生成点の上から降りてくる（一回だけ）
	void Call();

	/// CollisionManager が書く
	void SetIsPlayerNear(bool isNear) { isPlayerNear_ = isNear; }
	bool GetIsPlayerNear() const { return isPlayerNear_; }

	bool IsCalled() const { return isCalled_; }
	bool IsLanded() const { return isCalled_ && !isFalling_; }

	/// ================= 状態 ================= ///
	bool IsVisible() const { return isVisible_; }
	bool IsFalling() const { return isFalling_; }
	bool IsDrilling() const { return isDrilling_; }
	bool IsDoorOpen() const { return doorState_ == DoorState::kOpen; }
	bool IsDoorClosed() const { return doorState_ == DoorState::kClosed; }


#ifdef USE_IMGUI
	void ImGuiPart();
#endif

private:

	std::vector<std::function<void()>> function;

	bool floorHit = false;   // 床に当の当たり判定

private:

	void RunDrill();
	void MoveTrack(float UVTransform);

	void MoveUpdate();
	void DoorUpdate();

	void SetDoorAngle(float t);

private:

	std::unique_ptr<Object> shakeObject_;

private:

	///サントハンドル
	int SH_OpenCloseDoor_ = 0;
	int SH_RunningDrill_ = 0;

private:

	/// State
	enum class DrillerState {

		kNone = 0,
		kTitleMovement = 1 << 0,
		kNormal = 1 << 1,

		numStates = 2,///<<< numStatesの値の更新を忘れなく
	};
	int drillerState_ = (int)DrillerState::kNormal;

	/// 移動・ドアの状態
	enum class DoorState { kClosed, kOpening, kOpen, kClosing };
	DoorState doorState_ = DoorState::kClosed;

	Timer checkTimer_;

	/// 動作フラグ
	bool isVisible_ = true;
	bool isFalling_ = false;
	bool isDrilling_ = false;
	bool hasStopPoint_ = false;    // 止まるポイントがあるかどうか
	bool justArrived_ = false;
	float stopY_ = 0.0f;

	/// 生成点
	Vector3 spawnPoint_{};
	bool hasSpawnPoint_ = false;
	bool isCalled_ = false;
	bool isPlayerNear_ = false;

	Timer doorAnimationTimer_;

private:

	//////// プレイヤーデータ
	/// 移動速度
	Vector3 velocity_ = {};
	/// 接地状態フラグ
	bool onGround_ = true;

	std::unique_ptr<MovementObject> sharkObject_;

private:
	// 計算用
	float totelDeltaTime_{};
	std::unique_ptr<RandomMaker> randomMaker_;

	float shakingScale_ = 0.005f;
	bool isShake_ = false;

private:
	/// 当たり判定
	AABB hitBox_{};
	DebugLine debugLine;

private:
	/// Timer

	Timer drillOnOffTimer_;

};

