#pragma once
#include "Vector2.h"
#include "Vector3.h"
#include "DrawData/CornerData.h"

struct SpriteInstance {
	Vector3 position;
	Vector2 scale;
	Vector3 rotate;

	Vector2 anchorPoint;
	CornerData cornerData;
	int materialConfigIndex;
	int resourceHandle;
	int layer;
	int drawState;

	bool CheckSame(const SpriteInstance target);
};