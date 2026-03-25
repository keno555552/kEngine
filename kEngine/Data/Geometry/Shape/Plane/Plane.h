#pragma once
#include "Vector3.h"

struct Plane {
	Vector3 normal; //<<< 法線ベクトル
	float distance; //<<< 平面の方程式のdistance
};