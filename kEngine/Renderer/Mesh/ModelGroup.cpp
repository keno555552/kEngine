#include "ModelGroup.h"
#include "filesystem"

ModelGroup::~ModelGroup() {
	if (!modelGroup_.empty()) 		{
		for (auto model : modelGroup_) {
			if(model){
				model->ClearAllResource();
			}
		}
	}
	modelGroup_.clear();
}

void ModelGroup::PushModel(std::shared_ptr<Model> model) {
	modelGroup_.push_back(model);
}

std::string ModelGroup::GetFilePath() {

	if (modelData_.lock() == nullptr) { 
		Logger::Log("kEngine:: ModelGroup :: GetDirectoryPath :: ModelData is expired.");
		return std::string(); 
	}
	return modelData_.lock()->filePath;

}

void ModelGroup::SetModelGroupData(std::shared_ptr<ModelData> modelData) {
	modelData_ = modelData;
}

bool ModelGroup::HasSkinClusterData() {

	if (!modelData_.lock()) {
		Logger::Log("kEngine:: ModelGroup :: HasSkinClusterData :: ModelData is expired.");
		return false;
	}

	if (modelData_.lock()->skinListList.empty())return false;

	return true;
}
