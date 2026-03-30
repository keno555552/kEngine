#pragma once
#include <vector>
#include "Data/Animation/AnimationNodeData.h"


struct Animation {

	int animationID{};

	float startTime{};

	/// アニメーションの長さ(秒)
	float duration{};

	std::vector<AnimationNodeData> nodeList;

};
