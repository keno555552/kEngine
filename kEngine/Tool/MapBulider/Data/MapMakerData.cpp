#include "MapMakerData.h"
#include <fstream>
#include <iostream>

void MapMaker::Compile() {
	tokens_ = Tokenize(script_);
	sentences_ = BuildSentences(tokens_);

	nodes_ = ParseProgram(sentences_, nullptr);
}

void MapMaker::BuildJson() {
	nlohmann::json out;

	out["name"] = mapName_;
	out["context"] = {
		{"maxRow", 128},
		{"maxCol", 128},
		{"seed",   12345}
	};

	out["script"] = MapProgramAdapter::BodyToJson(nodes_);
	nlohmann::json varsJson = nlohmann::json::array();

	// 你現在的變數系統是單一 list，不再有 unitLists
	nlohmann::json unitJson;
	unitJson["unitName"] = "Variables";
	unitJson["units"] = nlohmann::json::array();

	for (auto& v : generatorVariables_.variableList) {
		nlohmann::json jv;
		jv["name"] = v.name;
		jv["type"] = VarTypeToString(v.type);

		switch (v.type) {
		case VarType::Int: jv["value"] = std::get<int>(v.value); break;
		case VarType::Float: jv["value"] = std::get<float>(v.value); break;
		case VarType::Double: jv["value"] = std::get<double>(v.value); break;
		case VarType::Bool: jv["value"] = std::get<bool>(v.value); break;
		case VarType::String: jv["value"] = std::get<std::string>(v.value); break;
		case VarType::Vector2: jv["value"] = { std::get<Vector2>(v.value).x, std::get<Vector2>(v.value).y }; break;
		case VarType::Vector3: jv["value"] = { std::get<Vector3>(v.value).x, std::get<Vector3>(v.value).y, std::get<Vector3>(v.value).z }; break;
		case VarType::Vector4: jv["value"] = { std::get<Vector4>(v.value).x, std::get<Vector4>(v.value).y, std::get<Vector4>(v.value).z, std::get<Vector4>(v.value).w }; break;
		}

		unitJson["units"].push_back(jv);
	}

	varsJson.push_back(unitJson);
	out["variables"] = varsJson;
	mapBuilderJson_ = out.dump(4);
}


bool MapMaker::ExportMapMakerToJson(const std::string& outPath) {

	std::string finalOutPath = CheckSavedPathEmpty(outPath);

	if (finalOutPath.empty()) {
		return false;
	}

	std::ofstream out(finalOutPath);
	if (!out.is_open()) {
		std::cout << "ExportMapMakerToJson failed: " << finalOutPath << std::endl;
		return false;
	}

	out << mapBuilderJson_;
	out.close();

	isSave_ = true; // 成功儲存 MapBuilder.json
	return true;
}

bool MapMaker::ExportMapDataToJson(const std::string& outPath) {
	std::string finalOutPath = CheckSavedPathEmpty(outPath);

	if (finalOutPath.empty()) {
		return false;
	}

	std::ofstream out(finalOutPath);
	if (!out.is_open()) {
		std::cout << "ExportMapDataToJson failed: " << finalOutPath << std::endl;
		return false;
	}

	out << mapDataJson_;
	out.close();

	isSave_ = true; // 成功儲存 MapDetail.json
	return true;
}

TileMapData MapMaker::GenerateMap(MapData& mapData) {
	TileMapData tileMap;
	tileMap.name = mapData.dataName;

	int width = mapData.xSize;
	int height = mapData.ySize;

	// 1. 初始化腳本輸出緩衝區
	std::vector<std::vector<int>> scriptResult_;
	scriptResult_.resize(height, std::vector<int>(width, 0));

	//// 2. 綁定 result[][] 給腳本
	//BindScriptOutput();
	//
	//// 3. 執行腳本（你已經有 Compile + LoadScript）
	//ProgramAdapter::Execute(script_);   // 或你自己的 ExecuteScript()

	// 4. 把腳本輸出寫入 TileMapData
	tileMap.Column.resize(width);
	tileMap.Row.resize(height);

	for (int y = 0; y < height; y++) {
		tileMap.Row[y].resize(width);
		for (int x = 0; x < width; x++) {
			tileMap.Row[y][x] = scriptResult_[y][x];
		}
	}

	return tileMap;
}



std::string MapMaker::CheckSavedPathEmpty(std::string outPath) {

	std::string finalOutPath;

	if (outPath.empty()) {
		if (mapMakerPath_.empty()) {
			debugText_.clear();
			debugText_ = "ExportMapDataToJson failed: mapMakerPath_ is empty. Please set the project path before exporting.";
			return "";
		}
		finalOutPath = mapMakerPath_ + "/" + mapName_ + ".json";
	} else {
		finalOutPath = outPath + "/" + mapName_ + ".json";
	}
	return finalOutPath;
}
