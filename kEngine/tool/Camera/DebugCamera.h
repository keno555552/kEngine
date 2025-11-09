#pragma once
#include "Camera.h"
#include "kEngine.h"

class DebugCamera {
public:
  /// 初期化
  DebugCamera(kEngine *system, float WindowWidth, float WindowHeight);
  ~DebugCamera();

  /// 更新
  void Update();

  /// スクリーン座標系転換
  TransformationMatrix transformationMatrixTransform(Transform objTransform);

  /// 直接座標指定
  void SetCamera(Transform cameraTransform);

  /// Mouse移動
  void MouseControlUpdate();

  /// 平行移動
  void Move(Vector3 speed);
  void Rotate(Vector3 Theta);

  Matrix4x4 GetViewMatrix() { return camera_->GetViewMatrix(); }
  Matrix4x4 GetProjectionMatrix() { return camera_->GetProjectionMatrix(); }
  Transform GetTransform() { return camera_->GetTransform(); }

	/// 操作設定
	void isKeyQAvailable(bool available) { isQ_ = available; }
	void isKeyEAvailable(bool available) { isE_ = available; }
	void isKeyAAvailable(bool available) { isA_ = available; }
	void isKeyDAvailable(bool available) { isD_ = available; }
	void isKeyWAvailable(bool available) { isW_ = available; }
	void isKeySAvailable(bool available) { isS_ = available; }
	void isKeyZAvailable(bool available) { isZ_ = available; }
	void isKeyRAvailable(bool available) { isR_ = available; }
	void isMouseMiddleAvailable(bool available) { isQ_ = available; }
	void isMouseRightvailable(bool available) { isQ_ = available; }
	void isGamePadAvailable(bool available) { isQ_ = available; }

private:
  kEngine *system_ = nullptr;

  Camera * camera_ = nullptr;

private:
	/// 操作可能フラグ
	bool isQ_ = true;
	bool isE_ = true;
	bool isA_ = true;
	bool isD_ = true;
	bool isW_ = true;
	bool isS_ = true;
	bool isZ_ = true;
	bool isC_ = true;
	bool isR_ = true;
	bool isMouseM_ = true;
	bool isMouseR_ = true;
	bool isGamePad_ = true;
};
