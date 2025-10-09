#pragma once
#include "Vector2.h"
#include "Vector3.h"

struct SpriteInstance {
	Vector2 position;
	Vector2 scale;
	Vector3 rotate;
	int materialConfigIndex;
	int layer;
	int drawState;

	bool CheckSame(const SpriteInstance target);
};