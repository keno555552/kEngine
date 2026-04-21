#include "ObjReader.h"
#include "filesystem"
#include "AssimpReader.h"
#include "Data/Render/CPUData/ModelData.h"

ModelData ReadObj(const std::string& filePath) {
	return ReadAssimp(filePath);
}
