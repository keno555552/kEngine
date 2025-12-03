#include "DebugCamera.h"

DebugCamera::DebugCamera(kEngine* system) {
	system_ = system;
}

DebugCamera::~DebugCamera() {
}

void DebugCamera::Update() {
	Camera::Update();
	MouseControlUpdate();
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
	if (system_->GetMouseIsPush(2) && isMouseM_) {
		cameraTransform.translate.x += (float)system_->GetMousePosXIns() * -0.005f;
		cameraTransform.translate.y += (float)system_->GetMousePosYIns() * 0.005f;
	}
	//右鍵視覚移動
	if (system_->GetMouseIsPush(1) && isMouseR_) {
		{
			cameraTransform.rotate.x = ((float)system_->GetMousePosYIns() * scale);
			cameraTransform.rotate.y = ((float)system_->GetMousePosXIns() * scale);
		}
	}

	/// GamePad操作
	if (config::GetKeyboardState() && isGamePad_) {
		if (system_->GetGamepadLStick01X() != 0)				cameraTransform.translate.x += system_->GetGamepadLStick01X() / 50.0f;
		if (system_->GetGamepadLStick01Y() != 0)				cameraTransform.translate.z += system_->GetGamepadLStick01Y() / 50.0f;
		if (system_->GetGamepadIsPush(VK_PAD_LSHOULDER) != 0)	cameraTransform.translate.y += 0.01f;
		if (system_->GetGamepadIsPush(VK_PAD_RSHOULDER) != 0)	cameraTransform.translate.y -= 0.01f;
		if (system_->GetGamepadRStick01Y() != 0)				cameraTransform.rotate.x -= system_->GetGamepadRStick01Y() / 100.0f;
		if (system_->GetGamepadRStick01X() != 0)				cameraTransform.rotate.y += system_->GetGamepadRStick01X() / 100.0f;
	}

	///元の位置に戻る
	if (system_->GetIsPush(DIK_R) && isR_) {
		SetCamera(CameraDefaultTransform());
	}
	if (system_->GetGamepadIsPush(VK_PAD_BACK) && isGamePad_) {
		SetCamera(CameraDefaultTransform());
	}

	Move(cameraTransform.translate);
	Rotate(cameraTransform.rotate);
}

