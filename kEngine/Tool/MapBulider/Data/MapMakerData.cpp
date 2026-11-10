#include "MapMakerData.h"
#include <fstream>
#include <iostream>

void MapMaker::Compile() {
	for (auto* n : nodes_) delete n;   // 前回の AST を消す
	nodes_.clear();

	tokens_ = Tokenize(script_);
	sentences_ = BuildSentences(tokens_);

	nodes_ = ParseProgram(sentences_, nullptr);
}

void MapMaker::BuildJson() {
	nlohmann::json out;

	out["name"] = mapName_;
	out["code"] = codingText_;
	out["context"] = {
		{ "targetMap", generateSettings_.targetMapName },
		{ "width",     generateSettings_.width },
		{ "height",    generateSettings_.height },
		{ "seed",      generateSettings_.seed },
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

bool MapMaker::ImportMapMakerFromJson(const std::string& filePath) {
	std::ifstream in(filePath);
	if (!in.is_open()) {
		debugText_ = "Import failed: cannot open " + filePath;
		return false;
	}
	nlohmann::json file = nlohmann::json::parse(in, nullptr, false);   // 壊れた JSON でも例外を投げない
	if (file.is_discarded() || !file.contains("script")) {
		debugText_ = "Import failed: not a MapMaker json " + filePath;
		return false;
	}

	mapName_ = file.value("name", mapName_);
	codingText_ = file.value("code", std::string());
	/// 生成設定（古いファイルは maxCol / maxRow）
	const nlohmann::json ctx = file.value("context", nlohmann::json::object());
	generateSettings_.targetMapName = ctx.value("targetMap", std::string());
	generateSettings_.width = ctx.value("width", ctx.value("maxCol", 32));
	generateSettings_.height = ctx.value("height", ctx.value("maxRow", 32));
	generateSettings_.seed = ctx.value("seed", 0);
	script_ = codingText_;

	/// 変数：BuildJson は variables[ { unitName, units:[...] } ] の形で書いている
	generatorVariables_.variableList.clear();
	generatorVariables_ = MapProgramAdapter::JsonToVariables(file);

	if (codingText_.empty()) {
		debugText_ = "Import warning: no \"code\" in file (old format). Script text cannot be restored.";
	} else {
		Compile();         // nodes_ を作り直す
	}
	BuildJson();           // mapBuilderJson_ を最新に（Apply / プレビュー用）
	isSave_ = true;
	return true;
}

TileMapData MapMaker::GenerateMap(int blankId) {
	BuildJson();
	const nlohmann::json file = nlohmann::json::parse(mapBuilderJson_, nullptr, false);
	if (file.is_discarded()) return TileMapData();

	std::vector<std::string> errors;
	TileMapData map = MapProgramAdapter::GenerateFromJson(file, blankId, -1, nullptr, &errors);
	debugText_.clear();
	for (const auto& e : errors) debugText_ += "Script Error: " + e + "\n";
	return map;
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
