#define NOMINMAX
#include "GameCamera.h"
#include "kEngine.h"
#include <algorithm>
#include <cmath>

GameCamera::GameCamera(kEngine* system, ConstructorKey)
	: Camera(ConstructorKey()), system_(system) {
	TimeManager* tm = system_->GetTimeManager();
	shakeTimer_.Init0(1.0f, tm);
	zoomTimer_.Init0(1.0f, tm);
	focusTimer_.Init0(1.0f, tm);
}

void GameCamera::UpdateFollow(const Vector3& playerPos, float moveDir) {
	const float dt = system_->GetDeltaTime();

	ZoomUpdate();
	if (isFocus_) {
		FocusUpdate();
	} else {
		FollowUpdate(playerPos, moveDir, dt);
	}
	currentPos_ = ClampToBounds(currentPos_);

	/// 中心 + z のずれ + 揺れ → カメラへ
	Transform t = CreateDefaultTransform();
	t.translate = currentPos_ + Vector3{ 0.0f, 0.0f, offsetZ_ } + ShakeUpdate();
	SetCamera(t);
}

void GameCamera::SnapTo(const Vector3& playerPos) {
	leadX_ = 0.0f;
	currentPos_ = targetPos_ = ClampToBounds(Vector3{ lookOffset_.x, lookOffset_.y, 0.0f } + playerPos);
}

/// ============================== 更新 ============================== ///
void GameCamera::FollowUpdate(const Vector3& playerPos, float moveDir, float dt) {
	/// ① 入力方向で決める（手を離したフレームから戻り始める）
	float leadGoal = 0.0f;
	float k = returnK_;
	if (moveDir > 0.0f) { leadGoal = +leadDistance_; k = leadK_; }   // 右入力 → 右を見る
	else if (moveDir < 0.0f) { leadGoal = -leadDistance_; k = leadK_; }   // 左入力 → 左を見る

	/// ② 先読み量をだんだん近づける（反対方向も 0 を通ってなめらかに）
	leadX_ += (leadGoal - leadX_) * (1.0f - std::exp(-k * dt));

	/// ③ 目標 = プレイヤー + オフセット + 先読み
	targetPos_ = Vector3{ lookOffset_.x + leadX_, lookOffset_.y, 0.0f } + playerPos;
	targetPos_ = ClampToBounds(targetPos_);

	/// ④ 中心を目標へ（指数平滑）
	currentPos_ += (targetPos_ - currentPos_) * (1.0f - std::exp(-followK_ * dt));
}

/// ============================== 邊界 ============================== ///
Vector2 GameCamera::GetViewHalfSize() const {
	const float halfH = std::abs(offsetZ_) * std::tan(fovY_ * 0.5f);
	return { halfH * aspect_, halfH };
}

Vector3 GameCamera::ClampToBounds(const Vector3& center) const {
	if (!hasBounds_) return center;
	const Vector2 half = GetViewHalfSize();
	Vector3 c = center;

	/// 地図が画面より小さい軸は真ん中に固定
	auto clampAxis = [](float v, float min, float max, float h) {
		if (max - min <= h * 2.0f) return (min + max) * 0.5f;
		return std::clamp(v, min + h, max - h);
		};
	c.x = clampAxis(c.x, boundsMin_.x, boundsMax_.x, half.x);
	c.y = clampAxis(c.y, boundsMin_.y, boundsMax_.y, half.y);
	return c;
}

/// ============================== 演出 ============================== ///
void GameCamera::Shake(float power, float time) {
	shakePower_ = power;
	shakeTimer_.Init0(time, system_->GetTimeManager());
	isShaking_ = true;
}

Vector3 GameCamera::ShakeUpdate() {
	if (!isShaking_) return {};
	const bool isEnd = shakeTimer_.ToMix();
	const float power = shakePower_ * (1.0f - shakeTimer_.linearity());   // だんだん弱く
	if (isEnd) isShaking_ = false;

	std::uniform_real_distribution<float> d(-1.0f, 1.0f);
	return { d(rng_) * power, d(rng_) * power, 0.0f };
}

void GameCamera::ZoomTo(float z, float time) {
	zoomFrom_ = offsetZ_;
	zoomTo_ = z;
	zoomTimer_.Init0(time, system_->GetTimeManager());
	isZooming_ = true;
}

void GameCamera::ZoomUpdate() {
	if (!isZooming_) return;
	const bool isEnd = zoomTimer_.ToMix();
	offsetZ_ = zoomFrom_ + (zoomTo_ - zoomFrom_) * SmoothStep(zoomTimer_.linearity());
	if (isEnd) isZooming_ = false;
}

void GameCamera::FocusTo(const Vector3& point, float time) {
	focusFrom_ = currentPos_;
	focusTo_ = point;
	focusTimer_.Init0(time, system_->GetTimeManager());
	isFocus_ = true;
}

void GameCamera::FocusUpdate() {
	focusTimer_.ToMix();
	currentPos_ = focusFrom_ + (focusTo_ - focusFrom_) * SmoothStep(focusTimer_.linearity());
}

void GameCamera::ReleaseFocus() {
	isFocus_ = false;
	targetPos_ = currentPos_;     // ここから平滑でプレイヤーへ戻る
}

#ifdef USE_IMGUI
void GameCamera::ImGuiPart() {
	ImGui::Begin("GameCamera");
	ImGui::DragFloat2("LookOffset", &lookOffset_.x, 0.05f);
	ImGui::DragFloat("Distance", &offsetZ_, 0.1f);
	ImGui::DragFloat("LeadDistance", &leadDistance_, 0.05f, 0.0f, 20.0f);
	ImGui::DragFloat("LeadK", &leadK_, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("ReturnK", &returnK_, 0.1f, 0.0f, 50.0f);
	ImGui::DragFloat("FollowK", &followK_, 0.1f, 0.0f, 50.0f);
	ImGui::Text("Lead %.2f", leadX_);
	ImGui::Text("Center (%.2f, %.2f)  Target (%.2f, %.2f)", currentPos_.x, currentPos_.y, targetPos_.x, targetPos_.y);
	if (ImGui::Button("Shake")) Shake(0.2f, 0.3f);
	ImGui::End();
}
#endif