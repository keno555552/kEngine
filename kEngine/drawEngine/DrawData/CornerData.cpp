#include "CornerData.h"

CornerData CreateDefaultCornerData() {
	CornerData	coner = { { {  0,	 0 },
							{  0, 1 },
							{ 1,1 },
							{ 1,	 0 } } };
	return coner;
}

bool CheckCornerDataNull(const CornerData& target) {
	for (int i = 0; i < 4; i++) {
		if (target.coner[i].x != 0.0f || target.coner[i].y != 0.0f) {
			return false;
		}
	}
	return true;
}