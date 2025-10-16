#include "SpriteInstance.h"

bool SpriteInstance::CheckSame(const SpriteInstance target) {
	if (position != target.position)return false;
	if (scale != target.scale)return false;
	if (rotate != target.rotate)return false;
	if (materialConfigIndex != target.materialConfigIndex)return false;
	if (resourceHandle != target.resourceHandle)return false;
	if (layer != target.layer)return false;
	return true;
}