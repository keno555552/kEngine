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
	MovementObject(kEngine *system);
	void Update(Camera* camera) override;

	void SetShake(bool on, float scale = 0.05f) {
		shakingScale_ = scale;
		state_ = on ? (state_ | (int)MovementObjectState::kShake)
					: (state_ & ~(int)MovementObjectState::kShake);
	}

	void TakeAppartObject(Object* object);
	void ReleaseObject();

private:
	// 計算用
	float deltaTime_{};
	std::unique_ptr<RandomMaker> randomMaker_;
	Object* targetFollow_ = nullptr;

	float shakingScale_ = 0.005f;

	int state_ = (int)MovementObjectState::kNormal;
};

