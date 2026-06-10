#pragma once
#include <cmath>
static const float PI = 3.14159265f;

inline float gauss(float x, float y, float sigma) {
	float exponent = -(x * x + y * y) / (2.0f * sigma * sigma);
	float denominator = 2.0f * PI * sigma * sigma;
	return exp(exponent) * (1.0f / denominator);
}
