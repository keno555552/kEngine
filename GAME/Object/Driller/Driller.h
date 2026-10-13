#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "BaseScene.h"
#include "Data/Geometry/Corner/Corner.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include "Utility/randomMaker.h"


/// リソースパーズ
// GAME/resources/Object/driller/
// driller.gltf

static const float Driller_MaxSpeed = 0.1f;

static const float Driller_DrillMaxSpeed = 0.1f;

static const float Driller_OnOffTime = 0.1f;

class Driller :public Object
{
public:
	/// コンストラクタ
	Driller(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });

	/// タイトルムーブメント
	void IsTitle(bool isTitle) { drillerState_ = isTitle ? (int)DrillerState::kTitleMovement : (int)DrillerState::kNormal; }
	/// ステージ用
	void Update(Camera* camera) override;
	// title用特別なアップデート
	void TitleMovement();
	AABB& GetAABB() {};
	//AABB& GetMapChipAABB();
	Vector3 GetVelocity() { return velocity_; }
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetVelocity(const Vector3& velocity) { velocity_ = velocity; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	bool IsOnGround() { return onGround_; }
	void SetOnGround(bool onGround) { onGround_ = onGround; }

#ifdef USE_IMGUI
	void ImGuiPart();
#endif

private:

	std::vector<std::function<void()>> function;

	bool floorHit = false;   // 床に当の当たり判定

private:

	void OpenDoor();
	void CloseDoor();
	void ShakeMovement();
	void RunDrill();

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

	Timer checkTimer_;

private:

	//////// プレイヤーデータ
	/// 移動速度
	Vector3 velocity_ = {};
	/// 接地状態フラグ
	bool onGround_ = true;

private:
	// 計算用
	float deltaTime_{};
	std::unique_ptr<RandomMaker> randomMaker_;

	float shakingScale_ = 0.005f;
	bool isShake_ = false;

private:
	AABB hitBox_{};
	DebugLine debugLine;


private:

	Timer drillOnOffTimer_;

};

