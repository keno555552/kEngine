#include "CornerData.h"

CornerData CreateDefaultCornerData() {
	CornerData	coner = { { {  0,	 0 },
							{  0, 1 },
							{ 1,1 },
							{ 1,	 0 } } };
	return coner;
}
