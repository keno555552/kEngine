#pragma once
#include "Model.h"
#include <vector>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


class ModelGroup
{
public:
	~ModelGroup();

	Model* GetModel(int handle){return modelGroup_[handle].get();}
	void PushModel(std::shared_ptr<Model> model);

	std::string GetFilePath();

	void SetModelGroupData(std::shared_ptr<ModelData> modelData);
	void PushMeshHandle(int handle) { meshHandle_.push_back(handle); }

	int GetMeshHandle(int handle) { return meshHandle_[handle]; }
	int GetModelNum() { return (int)modelGroup_.size();}

	void SwapModel(int modelIndex, std::shared_ptr<Model> newModel, int newMeshHandle);

	bool HasSkinClusterData();

private:
	/// ModelDataを保存する
	std::weak_ptr<ModelData> modelData_ = {};

	/// Modelを保存するどころ
	std::vector<std::shared_ptr<Model>> modelGroup_;

	/// ResourceManagerからもらうMeshハンドルを保存するどころ
	std::vector<int> meshHandle_;

};

