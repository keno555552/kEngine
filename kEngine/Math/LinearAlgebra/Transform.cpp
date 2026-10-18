#include "Transform.h"

Transform CreateDefaultTransform() {
	Transform defaultTransform;
	defaultTransform.scale = { 1.0f,1.0f,1.0f };
	defaultTransform.rotate = { 0.0f,0.0f,0.0f };
	defaultTransform.translate = { 0.0f,0.0f,0.0f };

	return defaultTransform;
}

Transform LinearTransform(const Transform& a, const Transform& b, float t) {
	Transform result{};

	result.translate.x = (1.0f - t) * a.translate.x + (t)*b.translate.x;
	result.translate.y = (1.0f - t) * a.translate.y + (t)*b.translate.y;
	result.translate.z = (1.0f - t) * a.translate.z + (t)*b.translate.z;

	result.rotate.x = (1.0f - t) * a.rotate.x + (t)*b.rotate.x;
	result.rotate.y = (1.0f - t) * a.rotate.y + (t)*b.rotate.y;
	result.rotate.z = (1.0f - t) * a.rotate.z + (t)*b.rotate.z;

	result.scale.x = (1.0f - t) * a.scale.x + (t)*b.scale.x;
	result.scale.y = (1.0f - t) * a.scale.y + (t)*b.scale.y;
	result.scale.z = (1.0f - t) * a.scale.z + (t)*b.scale.z;

	return result;
}

bool Transform::CheckRotateDirty() {
	if (lastRotate != rotate) {
		lastRotate = rotate;
		eulerDirty = true;
		return true;
	}
	eulerDirty = false;
	return false;
}

void Transform::UpdateRotateQuat() {
	rotateQuat = EulerToQuaternion(rotate);
	eulerDirty = false;
}

void Transform::CopyAniTranFrom(Transform& target) {
	aniScale = target.aniScale;
	aniRotate = target.aniRotate;
	aniTranslate = target.aniTranslate;

	isAnimated = target.isAnimated;
}
