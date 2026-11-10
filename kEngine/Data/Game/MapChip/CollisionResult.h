#pragma once
#include "MathsIncluder.h"

struct CollisionResult {
	Vector3 correction;
	bool xIsMainAxis = false;
	bool yIsMainAxis = false;
	bool landed = false;
	bool ceilingHit = false;
};