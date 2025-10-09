#include "Material.h"

bool Material::operator==(const Material target) {
	if (color != target.color)return false;
	if (enableLighting != target.enableLighting)return false;
	if (uvTransform != target.uvTransform)return false;
	return true;
}
bool Material::operator!=(const Material target) {
	return !(*this == target);
}

void Material::inputMaterialConfig(MaterialConfig target) {
	color = target.textureColor;
	enableLighting = target.enableLighting;
	uvTransform = target.uvTransformMatrix;
}
