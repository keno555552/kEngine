#include "Vector4.h"

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
