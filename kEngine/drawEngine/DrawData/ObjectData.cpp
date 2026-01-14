#include "ObjectData.h"

bool ObjectPart::operator==(const ObjectPart& target) {
	if (transform.translate != target.transform.translate) return false;
	if (transform.rotate != target.transform.rotate) return false;
	if (transform.scale != target.transform.scale) return false;
	if (transformationMatrix != target.transformationMatrix) return false;
	if (*materialConfig != *(target.materialConfig)) return false;
    return true;
}
