#include "filesystem"
#include "GLTFReader.h"
#include "AssimpReader.h"
#include "Data/Render/CPUData/ModelData.h"

ModelData ReadGltf(const std::string& filePath) {
	return ReadAssimp(filePath);
}
