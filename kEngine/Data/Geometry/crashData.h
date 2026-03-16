#pragma once
#include "MathsIncluder.h"

/// レイ
struct Ray {
	Vector3 origin;
	Vector3 direction;
};

/// 線分
struct Segment{
	Vector3 origin; //<<< 始点
	Vector3 diff;   //<<< 終点への差分ベクトル
};

/// 球体
typedef struct Sphere {
	Vector3 center;
	float radius;
}Sphere;

///　ヒットボックス
struct HitBox final {
	Vector2 midPos;
	float width;
	float height;
};

///　AABB
struct AABB final {
	Vector3 min;
	Vector3 max;
};

/// OBB
struct OBB final {
	Vector3 oriertations[3];
	Vector3 center;
	Vector3 size;
};

/// カプセル
struct Capsule final {
	Vector3 start;
	Vector3 end;
	float radius;
};