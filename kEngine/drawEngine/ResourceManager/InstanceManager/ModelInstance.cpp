#include "ModelInstance.h"

bool ModelInstance::CheckSame(const ModelInstance target) {
	if (WVP != target.WVP)return false;
	if (world != target.world)return false;
	if (materialConfigIndex != target.materialConfigIndex)return false;
	if (drawState != target.drawState)return false;
	return true;
}