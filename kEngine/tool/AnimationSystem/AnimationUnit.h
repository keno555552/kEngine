#pragma once
#include <vector>
#include "kEngine.h"
#include "Transform.h"
#include "Geometry/Collision/crashDecision.h"
#include "Data/Animation/Animation.h"
#include "Data/Animation/AnimationNodeData.h"
#include "Object/Object.h"
#include "Data/Animation/Keyframe.h"

class AnimationUnit
{
public:
	AnimationUnit(kEngine* system);
	~AnimationUnit();

	void ReadAnimationData(AnimationNodeData* animationData);
	void SetTime(float time_);
	void TakeControlObject(Object* object);
	void RelistControlObject();

	void Update(Camera* camera);

	void KariDraw();

	float GetStartTime() const { return allStartTime_; }
	float GetMaxTime() const { return allMaxTime_; }

private:
	kEngine* system_{};
	
	std::weak_ptr<AnimationNodeData> animationData_;
	float allMaxTime_{};
	float allStartTime_{};

	std::weak_ptr <Object> controlledObject_{};
	std::unique_ptr<Object> instanceObject_{};

	int usingKeyFrameIndex_{};
	float usingStartTime_{};
	float usingEndTime_{};
	float nowTime_{};
	std::unique_ptr <Timer> time_{};

private:
	bool CheckObjectNumMeet(Object* target);

	void TunningTime() {};
	void ResetTimer();

	void UpdateInstanceObject();
	void ControlleObject(Camera* camera);

	float ChangeEasing(AnimationType type,float R = 0);



#ifdef USE_IMGUI
	void ImguiPart();
#endif
};