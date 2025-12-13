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


	/// セットカメラ
	void SetRotate(Vector3 rotate) { cameraTransform_.rotate = rotate; }
	void SetTranslate(Vector3 translate) { cameraTransform_.translate = translate; }
	void SetFovY(float fovY) { fovY_ = fovY; }
	void SetAspect(float aspect) { aspect_ = aspect; }
	void SetNearClip(float nearClip) { nearClip_ = nearClip; }
	void SetFarClip(float farClip) { farClip_ = farClip; }

	Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }
	Matrix4x4 GetViewMatrix() const { return viewMatrix_; }
	Matrix4x4 GetProjectionMatrix() const { return projectionMatrix_; }
	Transform GetTransform() const { return cameraTransform_; }

	void SetDefaultTransform(Transform defaultTransform) { defaultTransform_ = defaultTransform; }
	Transform GetDefaultTransform() const { return defaultTransform_; }

	void ResetCamera(); 

private:

	/// 水平方向視野角
	float fovY_;
	/// アスペクト比
	float aspect_ ;
	/// ニアクリップ距離
	float nearClip_;
	/// ファークリップ距離
	float farClip_;

	/// 累積回転行列
	Matrix4x4 matRot_;

	/// カメラ変数を作る
	Transform cameraTransform_;

	/// リセット用カメラ変数
	Transform defaultTransform_;

	/// WorldMatrix用
	Matrix4x4 worldMatrix_{};

	/// ProjectionMatrix
	Matrix4x4 projectionMatrix_{};

	/// ビュー行列
	Matrix4x4 viewMatrix_{};

};

