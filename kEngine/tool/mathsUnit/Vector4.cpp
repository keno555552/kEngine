#include "Vector4.h"

Vector4 Vector4::operator/=(const float& target) {
	return { x / target, y / target , z / target, w / target };
}

Vector4 Vector4::operator+(const Vector4& target) {
    return { x + target.x, y + target.y , z + target.z, w + target.w };
}

Vector4 Vector4::operator-(const Vector4& target) {
    return { x - target.x, y - target.y , z - target.z, w - target.w };
}

bool Vector4::operator==(const Vector4& target) const{
	if (x != target.x)return false;
	if (y != target.y)return false;
	if (z != target.z)return false;
	if (w != target.w)return false;
	return true;
}

bool Vector4::operator!=(const Vector4& target) const{
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

//Vector4 Vector4::ColorBy144() {
//	return { z * 144.0f, y * 144.0f, x * 144.0f, w * 144.0f };
//}
//
//Vector4 Vector4::ColorBy1() {
//	return { z / 144.0f, y / 144.0f, x / 144.0f, w / 144.0f };
//}
