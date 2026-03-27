#include "Quaternion.h"

#include <math.h>
#include "MathsIncluder.h"

#ifdef USE_NOVICE
#include "Novice.h"
#endif


Quaternion Quaternion::operator+(const Quaternion& target) {
	return { x + target.x, y + target.y, z + target.z , w + target.w };
}

Quaternion Quaternion::operator-(const Quaternion& target) {
	return { x - target.x, y - target.y, z - target.z , w - target.w };
}

Quaternion Quaternion::operator*(const Quaternion& target) {
	return {
		w * target.x + x * target.w + y * target.z - z * target.y,
		w * target.y - x * target.z + y * target.w + z * target.x,
		w * target.z + x * target.y - y * target.x + z * target.w,
		w * target.w - x * target.x - y * target.y - z * target.z
	};
}

Quaternion Quaternion::operator*(const float& target) {
	return { x * target, y * target, z * target , w * target };
}

Quaternion Quaternion::operator/(const Quaternion& target) {
	return { x / target.x, y / target.y, z / target.z , w / target.w };
}

Quaternion Quaternion::operator-() const {
	return { -x, -y, -z, -w };
}

Quaternion Quaternion::operator=(const Quaternion& target) {
	if (this != &target) {
		x = target.x;
		y = target.y;
		z = target.z;
		w = target.w;
	}
	return *this;
}

float Dot(const Quaternion& q1, const Quaternion& q2) {
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

Quaternion Multiply(const Quaternion& m1, const Quaternion& m2) {
	Quaternion result = m1;
	return result * m2;
}

#ifdef USE_NOVICE
void QuaternionScreenPrintf(int x, int y, Quaternion& quaternion, const char* name) {

	if (name == nullptr) {
		Novice::ScreenPrintf(x, y, "%.02f   %.02f   %.02f   %.02f", quaternion.x, quaternion.y, quaternion.z, quaternion.w);
	} else {
		Novice::ScreenPrintf(x, y, "%.02f   %.02f   %.02f   %.02f   %s", quaternion.x, quaternion.y, quaternion.z, quaternion.w, name);
	}
}
#endif // 

Quaternion IdentityQuaternion() {
	return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Quaternion Conjugate(const Quaternion& target) {
	return { -target.x,-target.y,-target.z,target.w };
}

float Norm(const Quaternion& target) {
	return sqrtf(target.x * target.x + target.y * target.y + target.z * target.z + target.w * target.w);
}

Quaternion Normalize(const Quaternion& target) {
	float norm = Norm(target);
	if (norm != 0) {
		return { target.x / norm, target.y / norm, target.z / norm, target.w / norm };
	}
	return { 0,0,0,0 };
}

Quaternion Inverse(const Quaternion& target) {
	float sqNorm = target.x * target.x + target.y * target.y + target.z * target.z + target.w * target.w;

	if (sqNorm <= 1e-12f) {
		return { 0,0,0,0 };
	}

	Quaternion c = Conjugate(target);
	float invSq = 1.0f / sqNorm;
	return { c.x * invSq, c.y * invSq, c.z * invSq, c.w * invSq };
}

Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t) {
	Quaternion q1n = Normalize(q1);
	Quaternion q2n = Normalize(q2);

	float dot = Dot(q1, q2);

	if (dot < 0) {
		q1n = -q1n;
		dot = -dot;
	}

	const float EPSILON = 1e-6f;
	if (dot > 1.0f - EPSILON) {
		// 線形補間の後に正規化
		Quaternion result = q1n * (1.0f - t) + q2n * t;
		return Normalize(result);
	}

	float theta = acosf(dot);
	float sinTheta = sinf(theta);

	float w1 = sinf((1.0f - t) * theta) / sinTheta;
	float w2 = sinf(t * theta) / sinTheta;

	return q1n * w1 + q2n * w2;
}

Quaternion MakeRotateAxisAngleQuaternion(const Vector3& axis, float angle) {
	Vector3 nAxis = Normalize(axis);
	if (nAxis.x == 0 && nAxis.y == 0 && nAxis.z == 0) {
		return IdentityQuaternion();
	}

	float halfAngle = angle * 0.5f;
	float sinHalfAngle = sinf(halfAngle);
	float cosHalfAngle = cosf(halfAngle);
	return {
		nAxis.x * sinHalfAngle,
		nAxis.y * sinHalfAngle,
		nAxis.z * sinHalfAngle,
		cosHalfAngle
	};
}

Vector3 RotateVector(const Vector3& vector, const Quaternion& quaternion) {
	Quaternion q = Normalize(quaternion); 
	Quaternion p = { vector.x, vector.y, vector.z, 0.0f };
	Quaternion qInv = Conjugate(q); 

	Quaternion result = q * p * qInv;

	return { result.x, result.y, result.z };


}

Quaternion EulerToQuaternion(const Vector3& euler) {

	float cx = cosf(euler.x * 0.5f);
	float sx = sinf(euler.x * 0.5f);
	float cy = cosf(euler.y * 0.5f);
	float sy = sinf(euler.y * 0.5f);
	float cz = cosf(euler.z * 0.5f);
	float sz = sinf(euler.z * 0.5f);

	Quaternion q;
	q.w = cx * cy * cz + sx * sy * sz;
	q.x = sx * cy * cz - cx * sy * sz;
	q.y = cx * sy * cz + sx * cy * sz;
	q.z = cx * cy * sz - sx * sy * cz;
	return q;
}

Vector3 QuaternionToEuler(const Quaternion& quaternion) {
	Vector3 euler;

	float x = quaternion.x;
	float y = quaternion.y;
	float z = quaternion.z;
	float w = quaternion.w;

	// pitch (x-axis rotation)
	float sinp = 2.0f * (w * x + y * z);
	float cosp = 1.0f - 2.0f * (x * x + y * y);
	euler.x = atan2f(sinp, cosp);

	// yaw (y-axis rotation)
	float siny = 2.0f * (w * y - z * x);
	siny = siny > 1.0f ? 1.0f : siny;
	siny = siny < -1.0f ? -1.0f : siny;
	euler.y = asinf(siny);

	// roll (z-axis rotation)
	float sinr = 2.0f * (w * z + x * y);
	float cosr = 1.0f - 2.0f * (y * y + z * z);
	euler.z = atan2f(sinr, cosr);

	return euler;
}


Matrix4x4 MakeRotateMatrix(const Quaternion& quaternion) {
	float x = quaternion.x;
	float y = quaternion.y;
	float z = quaternion.z;
	float w = quaternion.w;

	return {
		x * x - y * y - z * z + w * w,        2.0f * (x * y + z * w),	     2.0f * (x * z - y * w),			0.0f,
			  2.0f * (x * y - z * w),  w * w - x * x + y * y - z * z,	     2.0f * (y * z + x * w),			0.0f,
			  2.0f * (x * z + y * w),		 2.0f * (y * z - x * w),  w * w - x * x - y * y + z * z ,	        0.0f,
								0.0f,						   0.0f,						   0.0f,			1.0f
	};
}

