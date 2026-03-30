#pragma once
#include <vector>
#include "kEngine.h"
#include "Object/Object.h"
#include "Data/Animation/Animation.h"
#include "Data/Animation/AnimationNodeData.h"
#include "AnimationUnit.h"


class AnimationSystem
{
public:

	AnimationSystem(kEngine* system);
	~AnimationSystem();
	
	/// ====================== アニメーションシステムの基本的な流れ ========================///
	/// アニメーションデータを読み込む
	static Animation LoadAnimationData(const std::string& filePath);

	/// アニメーションデータを読み込む
	void ReadAnimationData(Animation* animation);

	/// Objectにアニメーションの計算結果を反映させる
	void ControlleObject();

	/// 時間を調整する
	void SetTime(float time);

	/// ======================= 操作関連 ========================///

	void TakeControlObject(Object* object);

	void Update();

private:
	kEngine* system_{};

	Animation* animationData_{}; // 借り
	float allMaxTime_{};
	float allStartTime_{};

	Object* controlledObject_{};             // 借り

	/// 計算用アニメーションのオブジェクト
	std::unique_ptr <Object> instanceObject_{};

	float nowTime_{};

private:
	 
	/// ====================== 読み込関連 ======================== ///
	static Animation LoadAnimationDataFromGltf(const std::string& filePath);
	static Animation LoadAnimationDataFromJson(const std::string& filePath);


	/// ====================== プレイ関連 ======================== ///

	void ResetTimer();
	void UpdateInstanceObject();

	/// ======================= ツール関数 ======================= ///
	float ChangeEasing(AnimationType type, float t, float rate = 0);


	/// 補間関連
	template<class T>
	T LerpValue(const T& a, const T& b, float t) {
		return a * (1.0f - t) + b * t;
	}

	// Vector3 專用
	template<>
	Vector3 LerpValue<Vector3>(const Vector3& a, const Vector3& b, float t) {
		return {
			linearity(a.x, b.x, t),
			linearity(a.y, b.y, t),
			linearity(a.z, b.z, t)
		};
	}

	// Quaternion 專用
	template<>
	Quaternion LerpValue<Quaternion>(const Quaternion& a, const Quaternion& b, float t) {
		return Slerp(a, b, t);
	}

	template<class T>
	T MakeTimeValue(const std::vector<KeyFrame<T>>& keys, float nowTime) {

		if (keys.empty()) {
			return T{};
		}

		if (keys.size() == 1) {
			return keys[0].value_;
		}

		if (nowTime <= keys.front().time_) {
			return keys.front().value_;
		}

		if (nowTime >= keys.back().time_) {
			return keys.back().value_;
		}

		for (size_t i = 0; i + 1 < keys.size(); i++) {
			if (keys[i].time_ <= nowTime && keys[i + 1].time_ >= nowTime) {

				float t = (nowTime - keys[i].time_) / (keys[i + 1].time_ - keys[i].time_);
				float easingT = ChangeEasing(keys[i].animationType_, t, keys[i].easeRate_);

				return LerpValue(keys[i].value_, keys[i + 1].value_, easingT);
			}
		}

		return keys.back().value_; // fallback
	}


};

