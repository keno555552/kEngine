#pragma once
#include <memory>
#include <random>
#include "Camera/Camera.h"
#include "Tool/TimeManager/TimeManager.h"

class kEngine;

/// ゲーム用カメラ（普通の Camera の更新方法を強化したもの）
///  - 中心(current) と 目標(target) の 2 つの位置
///  - 追従：進行方向を先読み ＋ 指数平滑 ／ 止まったらそのまま戻る
///  - 地図の邊界で clamp
///  - 演出：Shake / ZoomTo / FocusTo
class GameCamera : public Camera {
public:
	explicit GameCamera(kEngine* system, ConstructorKey);

	/// CameraManagerからのみ生成・破棄可能
	friend class CameraManager;
	friend class kEngine;

	/// 毎フレーム：追う位置と入力方向（-1 左 / 0 なし / +1 右）を渡して更新
	void UpdateFollow(const Vector3& playerPos, float moveDir);
	/// 即座に targetPos へ（シーン開始・ワープ時）
	void SnapTo(const Vector3& targetPos);

	/// ================= 設定 ================= ///
	void SetLookOffset(const Vector2& offset) { lookOffset_ = offset; }   // target = player + これ
	void SetDistance(float z) { offsetZ_ = z; }                           // カメラの z（-15 など）
	void SetLeadDistance(float d) { leadDistance_ = d; }   // 進行方向にどれだけ先を見るか
	void SetLeadSpeed(float k) { leadK_ = k; }             // 先を見に行く速さ（小さいほど「だんだん」）
	void SetReturnSpeed(float k) { returnK_ = k; }         // 止まった時に戻る速さ
	void SetFollowSpeed(float k) { followK_ = k; }         // 中心がターゲットに追いつく速さ

	/// 地図の範囲（ワールド座標）。画面がこの外を映さない
	void SetBounds(const Vector2& min, const Vector2& max) { boundsMin_ = min; boundsMax_ = max; hasBounds_ = true; }
	void ClearBounds() { hasBounds_ = false; }

	/// ================= 演出 ================= ///
	void Shake(float power, float time);
	void ZoomTo(float z, float time);
	/// 指定位置を見に行く（プレイヤー追従は止まる）
	void FocusTo(const Vector3& point, float time);
	/// プレイヤー追従に戻る（平滑で戻る）
	void ReleaseFocus();

	bool IsFocusing() const { return isFocus_; }
	bool IsFocusArrived() const { return isFocus_ && focusTimer_.GetIsMax(); }
	const Vector3& GetCenter() const { return currentPos_; }

#ifdef USE_IMGUI
	void ImGuiPart();
#endif

private:
	void FollowUpdate(const Vector3& playerPos, float moveDir, float dt);
	void FocusUpdate();
	void ZoomUpdate();
	Vector3 ShakeUpdate();
	Vector3 ClampToBounds(const Vector3& center) const;
	Vector2 GetViewHalfSize() const;

	static float SmoothStep(float t) { return t * t * (3.0f - 2.0f * t); }

private:
	kEngine* system_ = nullptr;          // 借り

	/// ===== 位置 ===== ///
	Vector3 currentPos_{};               // 今見ている中心
	Vector3 targetPos_{};                // 目標の中心
	Vector2 lookOffset_ = { 0.0f, 0.5f };
	float offsetZ_ = -15.0f;

	/// ===== 追従 ===== ///
	float leadDistance_ = 3.0f;			 // 先読み距離
	float leadK_ = 2.0f;				 // 先読みが伸びる速さ
	float returnK_ = 3.0f;				 // 止まった時に戻る速さ
	float followK_ = 10.0f;				 // 中心の平滑（y のジャンプがカクつかない程度）
	float leadX_ = 0.0f;				 // 今の先読み量（-leadDistance_ ～ +leadDistance_）

	/// ===== 邊界 ===== ///
	bool hasBounds_ = false;
	Vector2 boundsMin_{}, boundsMax_{};

	/// ===== 演出 ===== ///
	bool isShaking_ = false;
	float shakePower_ = 0.0f;
	Timer shakeTimer_;
	std::mt19937 rng_{ std::random_device{}() };

	bool isZooming_ = false;
	float zoomFrom_ = 0.0f, zoomTo_ = 0.0f;
	Timer zoomTimer_;

	bool isFocus_ = false;
	Vector3 focusFrom_{}, focusTo_{};
	Timer focusTimer_;
};