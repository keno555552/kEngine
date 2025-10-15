#include "DebugCamera.h"

DebugCamera::DebugCamera(kEngine* system, float windowWidth, float windowHeight) {
	system_ = system;
	projectionMatrix_ = MakePerspectiveFovMatrix(0.45f, float(windowWidth) / float(windowHeight), 0.1f, 100.0f);
	Update();
	matRot_ = MakeRotateMatrix4x4(cameraTransform_.rotate);
}

void DebugCamera::Update() {
	/// カメラMatrix更新
	Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform_.scale, cameraTransform_.rotate, cameraTransform_.translate);
	/// ビュー行列更新
	viewMatrix_ = Inverse(cameraMatrix);
}

TransformationMatrix DebugCamera::transformationMatrixTransform(Transform objTransform) {

	Matrix4x4 objWorldMatrix = MakeAffineMatrix(objTransform.scale, objTransform.rotate, objTransform.translate);
	Matrix4x4 worldViewProjectionMatrix = objWorldMatrix * (viewMatrix_ * projectionMatrix_);

	return { worldViewProjectionMatrix, objWorldMatrix };;
}

void DebugCamera::SetCamera(Transform cameraTransform) {
	cameraTransform_ = cameraTransform;
	Update();
}

void DebugCamera::MouseControlUpdate() {
	float scale = 0.001f;

	///keyboard移動
	Transform cameraTransform = CreateDefaultTransform();
	if (system_->GetIsPush(DIK_Q) && isQ_) cameraTransform.translate.z += 0.01f;
	if (system_->GetIsPush(DIK_E) && isE_) cameraTransform.translate.z -= 0.01f;
	if (system_->GetIsPush(DIK_A) && isA_) cameraTransform.translate.x -= 0.01f;
	if (system_->GetIsPush(DIK_D) && isD_) cameraTransform.translate.x += 0.01f;
	if (system_->GetIsPush(DIK_W) && isW_) cameraTransform.translate.y += 0.01f;
	if (system_->GetIsPush(DIK_S) && isS_) cameraTransform.translate.y -= 0.01f;
	if (system_->GetIsPush(DIK_Z) && isZ_) cameraTransform.rotate.y += 0.01f; 
	if (system_->GetIsPush(DIK_C) && isC_) cameraTransform.rotate.y -= 0.01f; 
	cameraTransform.translate.z += system_->GetMouseScroll();

	///マウス操作
	//中鍵平行移動
	if (system_->GetMouseIsPush(2) && isMouseM_){
		cameraTransform.translate.x += (float)system_->GetMousePosXIns() * -0.005f;
		cameraTransform.translate.y += (float)system_->GetMousePosYIns() * 0.005f;
	}
	//右鍵視覚移動
	if (system_->GetMouseIsPush(1) && isMouseR_) {
		{
			cameraTransform_.rotate.x = cameraTransform_.rotate.x + ((float)system_->GetMousePosYIns() * scale);
			cameraTransform_.rotate.y = cameraTransform_.rotate.y + ((float)system_->GetMousePosXIns() * scale);
		}
	}

	/// GamePad操作
	if(config::GetKeyboardState() && isGamePad_){
		if (system_->GetGamepadLStick01X() != 0)				cameraTransform.translate.x += system_->GetGamepadLStick01X() / 50.0f;
		if (system_->GetGamepadLStick01Y() != 0)				cameraTransform.translate.z += system_->GetGamepadLStick01Y() / 50.0f;
		if (system_->GetGamepadIsPush(VK_PAD_LSHOULDER) != 0)	cameraTransform.translate.y += 0.01f;
		if (system_->GetGamepadIsPush(VK_PAD_RSHOULDER) != 0)	cameraTransform.translate.y -= 0.01f;
		if (system_->GetGamepadRStick01Y() != 0)				cameraTransform_.rotate.x = cameraTransform_.rotate.x - system_->GetGamepadRStick01Y() / 100.0f;
		if (system_->GetGamepadRStick01X() != 0)				cameraTransform_.rotate.y = cameraTransform_.rotate.y + system_->GetGamepadRStick01X() / 100.0f;
	}



	///元の位置に戻る
	if (system_->GetIsPush(DIK_R) && isR_) {
		SetCamera({ cameraDefaultTransform_ });
	}
	if (system_->GetGamepadIsPush(VK_PAD_BACK) && isGamePad_) {
		SetCamera({ cameraDefaultTransform_ });
	}

	Move(cameraTransform.translate);
}

///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<以降フレームの更新方法が変わったら、ここも変えよう
void DebugCamera::Move(Vector3 speed) {
	Matrix4x4 handle = MakeTranslateMatrix4x4(speed) * MakeRotateMatrix4x4(cameraTransform_.rotate);
	cameraTransform_.translate = cameraTransform_.translate + MakeTranslateVector3(handle);
}

void DebugCamera::Rotate(Vector3 Theta) {
	cameraTransform_.rotate = cameraTransform_.rotate + Theta;
}

