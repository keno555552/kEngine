#include "Camera.h"

Transform CameraDefaultTransform() {
	return { { 1.0f, 1.0f, 1.0f },   // scale
			{ 0.0f, 0.0f, 0.0f },   // rotate
			{ 0.0f, 0.0f, -5.0f } // translate
	};
}

Camera::Camera() {
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(config::GetClientWidth()) / float(config::GetClientHeight()), 0.1f, 100.0f);
	cameraTransform_ = CameraDefaultTransform();
	Update();
	matRot_ = MakeRotateMatrix4x4(cameraTransform_.rotate);
}

void Camera::Update() {
	/// カメラMatrix更新
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	/// ビュー行列更新
	viewMatrix_ = Inverse(cameraMatrix);
}

TransformationMatrix Camera::transformationMatrixTransform(Transform objTransform) {

	Matrix4x4 objWorldMatrix = MakeAffineMatrix(objTransform.scale, objTransform.rotate, objTransform.translate);
	Matrix4x4 worldViewProjectionMatrix = objWorldMatrix * (viewMatrix_ * projectionMatrix_);

	return { worldViewProjectionMatrix, objWorldMatrix };
}

void Camera::SetCamera(Transform cameraTransform) {
	cameraTransform_ = cameraTransform;
	Update();
}


///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<以降フレームの更新方法が変わったら、ここも変えよう
void Camera::Move(Vector3 speed) {
	Matrix4x4 handle = MakeTranslateMatrix4x4(speed) * MakeRotateMatrix4x4(cameraTransform_.rotate);
	cameraTransform_.translate = cameraTransform_.translate + MakeTranslateVector3(handle);
}

void Camera::Rotate(Vector3 Theta) {
	cameraTransform_.rotate = cameraTransform_.rotate + Theta;
}


