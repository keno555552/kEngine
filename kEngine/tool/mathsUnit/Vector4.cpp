#include "Vector4.h"
#include "Matrix4x4.h"

Vector4 Vector4::operator/=(const float& target) {
	return { x / target, y / target , z / target, w / target };
}

Vector4 Vector4::operator+(const Vector4& target) {
    return { x + target.x, y + target.y , z + target.z, w + target.w };
}

Vector4 Vector4::operator-(const Vector4& target) {
    return { x - target.x, y - target.y , z - target.z, w - target.w };
}

bool Vector4::operator==(const Vector4& target) {
	if (x != target.x)return false;
	if (y != target.y)return false;
	if (z != target.z)return false;
	if (w != target.w)return false;
	return true;
}

bool Vector4::operator!=(const Vector4& target) {
	return !(*this == target);
}

void Vector4::ColorBy255() {
	x = x * 255.0f;
	y = y * 255.0f;
	z = z * 255.0f;
	w = w * 255.0f;
}

void Vector4::ColorBy1() {
	x = x / 255.0f;
	y = y / 255.0f;
	z = z / 255.0f;
	w = w / 255.0f;
}

// Row-vector (v * M) multiplication
Vector4 Multiply(const Vector4& v, const Matrix4x4& m) {
	return {
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + v.w * m.m[3][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + v.w * m.m[3][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + v.w * m.m[3][2],
		v.x * m.m[0][3] + v.y * m.m[1][3] + v.z * m.m[2][3] + v.w * m.m[3][3]
	};
}
 
//Vector4 Vector4::ColorBy144() {
//	return { z * 144.0f, y * 144.0f, x * 144.0f, w * 144.0f };
//}
//
//Vector4 Vector4::ColorBy1() {
//	return { z / 144.0f, y / 144.0f, x / 144.0f, w / 144.0f };
//}
