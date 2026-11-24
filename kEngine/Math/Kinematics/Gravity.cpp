#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Gravity.h"
#include <algorithm>

float GravityCalc(float velocityY, float gravity, float limitFallSpeed, float dt, float scale) {
	float result = velocityY - gravity * scale * dt;
	result = std::max(result, -limitFallSpeed * scale);
	return result;
}