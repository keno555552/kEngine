#pragma once
#include "kEngine.h"
#include "Object/Object.h"

enum class MovementObjectState {
	kNormal = 0,
	kShake = 1 << 0,
	kMax = 1,
};

class MovementObject :public Object
{
public:
	MovementObject(kEngine* system);
	void Update(Camera* camera) override;

	/// シェーク関連
	void SetShake(bool on, float scale = 0.05f) {
		shakingScale_ = scale;
		state_ = on ? (state_ | (int)MovementObjectState::kShake)
			: (state_ & ~(int)MovementObjectState::kShake);
	}
	float GetShakeScale() { return shakingScale_; }
	void SetAndResetShakeTime(float time);
	void SetShakeTime(float time);
	float GetShakeTime() { return shakingTime_.parameter_; }


	void TakeAppartObject(Object* object);
	void ReleaseObject();

	Object* GetTargetFollow() { return targetFollow_; }


private:
	// 計算用
	float totelDeltaTime_{};
	std::unique_ptr<RandomMaker> randomMaker_;
	Object* targetFollow_ = nullptr;

	/// シェーク関連
	float shakingScale_ = 0.005f;
	Timer shakingTime_;



	/// 起動してる状態
	int state_ = (int)MovementObjectState::kNormal;

private:
	void ShakeUpdate();
};



/// ここは主にカントタン,例えば 0 になると一回動くとか