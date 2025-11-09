#include <algorithm>
#include "TimeManager.h"

void TimeManager::Update() {
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - lastUpdateTime;
	deltaTime_ = elapsed.count();
	lastUpdateTime = now;

	oneScondCounter_ += deltaTime_;

	if (oneScondCounter_ > 1.0f) {
		float sum = 0.0f;
		for (auto& ptr : fpsHistory_) {
			sum += ptr;
		}
		fpsPerSecond_ = sum / fpsHistory_.size();
		fpsHistory_.clear();
		oneScondCounter_ = 0.0f;
	} else {
		if (deltaTime_ > 0.0f)instantFps_ = 1.0f / deltaTime_;
		fpsHistory_.push_back(instantFps_);
	}

}

#pragma region Timer
Timer::Timer() {
	parameter_ = 0.0f;
	maxTime_ = 0.0f;
}

void Timer::Init0(float time, TimeManager* timeManager) {
	timeManager_ = timeManager;
	parameter_ = 0.0f;
	maxTime_ = time;
}

void Timer::InitM(float time, TimeManager* timeManager) {
	timeManager_ = timeManager;
	parameter_ = time;
	maxTime_ = time;
}

void Timer::Reset0() {
	parameter_ = 0.0f;
}

void Timer::ResetM() {
	parameter_ = maxTime_;
}

void Timer::ToMix() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ < maxTime_) {
		parameter_ += t;
	} else {
		parameter_ = maxTime_;
	}
}

void Timer::ToMixZero() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ < maxTime_ - 1 && parameter_ > 0) {
		parameter_ += t;
	} else {
		parameter_ = 0.0f;
	};
}

void Timer::ToZero() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ > 0) {
		parameter_ -= t;
	} else {
		parameter_ = 0.0f;
	}
}

void Timer::ToZeroMix() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ > 0 && parameter_ < maxTime_) {
		parameter_ -= t;
	} else {
		parameter_ = maxTime_;
	}
}

void Timer::foreverUp() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ < maxTime_) {
		parameter_ += t;
	} else {
		parameter_ = 0.0f;
	}
}

void Timer::foreverDown() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ > 0) {
		parameter_ -= t;
	} else {
		parameter_ = maxTime_;
	}
}

void Timer::AnimationF() {
	float t = timeManager_->getDeltaTime();
	if (parameter_ < maxTime_ - 1) {
		parameter_ += t;
	} else {
		parameter_ = 0.0f;
	}
}

int Timer::FrameChange() {
	return ((int)parameter_ / 5);
}

float Timer::linearity(int a, int b) {
	if (parameter_ <= 0.0f)return static_cast<float>(a);

	float T = std::clamp(parameter_ / maxTime_,0.0f,1.0f);
	return (1.0f - T) * a + (T)*b;
}

float Timer::easyIn(int a, int b, float r) {
	if (parameter_ <= 0.0f)return static_cast<float>(a);

	float T = std::clamp(parameter_ / maxTime_, 0.0f, 1.0f);
	float easedT = 1.0f - powf(1 - T, r);
	return (1.0f - easedT) * a + (easedT)*b;
}

float Timer::easyOut(int a, int b, float r) {
	if (parameter_ <= 0.0f)return static_cast<float>(a);

	float T = std::clamp(parameter_ / maxTime_, 0.0f, 1.0f);
	float easedT = 1 - (1.0f - powf(T, r));
	return (1.0f - easedT) * a + (easedT)*b;
}

float Timer::easyInOut(int a, int b, float r) {
	float T = float(parameter_ / maxTime_);
	float easedT = {};
	if (T <= 0.5) {
		easedT = (1.0f - sqrtf(1.0f - powf(2.0f * T, r))) / 2;
	} else {
		easedT = (sqrtf(1.0f - powf(-2 * T + 2, r)) + 1) / 2;
	}
	return (1.0f - easedT) * a + (easedT)*b;
}

float Timer::easyInBack(int a, int b, float r) {
	float T = float(parameter_ / maxTime_);
	float c1 = r;
	float c3 = c1 + 1;
	float easedT = c3 * powf(T, 4) - c1 * powf(T, 3);
	return (1.0f - easedT) * a + (easedT)*b;
}

float Timer::easyOutBack(int a, int b, float r) {
	float T = float(parameter_ / maxTime_);
	float c1 = r;
	float c3 = c1 + 1;
	float easedT = 1 + c3 * powf(T - 1, 3) + c1 * powf(T - 1, 2);
	return (1.0f - easedT) * a + (easedT)*b;
}

#pragma endregion 

#pragma region Easing
float linearity(int a, int b, int c, int t) {
	float time = float(c) / t;
	return (1.0f - time) * a + (time)*b;
}

float easyIn(int a, int b, int c, int t, float r) {
	float time = float(c) / t;
	float easedT = 1.0f - powf(1 - time, r);
	return (1.0f - easedT) * a + (easedT)*b;
}

float easyOut(int a, int b, int c, int t, float r) {
	float time = float(c) / t;
	float easedT = 1 - (1.0f - powf(time, r));
	return (1.0f - easedT) * a + (easedT)*b;
}

float easyInOut(int a, int b, int c, int t, float r) {
	float time = float(c) / t;
	float easedT = {};
	if (time <= 0.5) {
		easedT = (1.0f - sqrtf(1.0f - powf(2.0f * time, r))) / 2;
	} else {
		easedT = (sqrtf(1.0f - powf(-2 * time + 2, r)) + 1) / 2;
	}
	return (1.0f - easedT) * a + (easedT)*b;
}

float easyInBack(int a, int b, int c, int t, float r) {
	float time = float(c) / t;
	float c1 = r;
	float c3 = c1 + 1;
	float easedT = c3 * powf(time, 4) - c1 * powf(time, 3);
	return (1.0f - easedT) * a + (easedT)*b;
}

float easyOutBack(int a, int b, int c, int t, float r) {
	float time = float(c) / t;
	float c1 = r;
	float c3 = c1 + 1;
	float easedT = 1 + c3 * powf(time - 1, 3) + c1 * powf(time - 1, 2);
	return (1.0f - easedT) * a + (easedT)*b;
}

float smootherstep(float t) {
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

#pragma endregion 