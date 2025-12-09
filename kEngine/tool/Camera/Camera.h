#pragma once
#include "Transform.h"
#include "TransformationMatrix.h"
#include "MathsIncluder.h"
#include "config.h"


Transform CameraDefaultTransform();

class Camera {
public:
	/// 初期化
	Camera();

	/// 更新
	virtual void Update();

	/// スクリーン座標系転換
	TransformationMatrix transformationMatrixTransform(const Transform objTransform);
	TransformationMatrix transformationMatrixTransform(const Matrix4x4 worldMatrix);

	/// 直接座標指定
	void SetCamera(Transform cameraTransform);

	/// 平行移動
	void Move(Vector3 speed);
	void Rotate(Vector3 Theta);

	Matrix4x4 GetViewMatrix() const { return viewMatrix_; }
	Matrix4x4 GetProjectionMatrix() const { return projectionMatrix_; }
	Transform GetTransform() const { return cameraTransform_; }

	/// リセット
	void SetDefaultTransform(Transform defaultTransform) { defaultTransform_ = defaultTransform; }
	Transform GetDefaultTransform() const { return defaultTransform_; }
	void ResetCamera(); 

private:

	/// 累積回転行列
	Matrix4x4 matRot_;

	/// カメラ変数を作る
	Transform cameraTransform_;

	/// リセット用カメラ変数
	Transform defaultTransform_;

	/// ProjectionMatrix
	Matrix4x4 projectionMatrix_{};
	/// ビュー行列
	Matrix4x4 viewMatrix_{};

};

