#include "MapMakerData.h"
#include <fstream>
#include <iostream>

const std::string& MapMaker::GetCodingText() const {
	static const std::string kEmpty;
	const MakerFunction* f = CurrentFunction();
	return f ? f->code : kEmpty;
}

void MapMaker::BuildJson() {
	nlohmann::json out;

	out["name"] = mapName_;
	/// 函數ごとに 腳本 → AST → json（AST はここだけで使ってすぐ消す）
	nlohmann::json funcs = nlohmann::json::array();
	for (const auto& f : functions_) {
		std::vector<Node*> nodes = ParseProgram(BuildSentences(Tokenize(f.code)), nullptr);
		funcs.push_back({
			{ "name", f.name },{ "enabled", f.enabled },{ "code", f.code },
			{ "script", MapProgramAdapter::BodyToJson(nodes) },
			});
		for (auto* n : nodes) delete n;
	}
	out["functions"] = funcs;
	out["context"] = {
		{ "targetMap", generateSettings_.targetMapName },
		{ "width",     generateSettings_.width },
		{ "height",    generateSettings_.height },
		{ "seed",      generateSettings_.seed },
	};

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
	if (file.is_discarded() || (!file.contains("script") && !file.contains("functions"))) {
		debugText_ = "Import failed: not a MapMaker json " + filePath;
		return false;
	}

	mapName_ = file.value("name", mapName_);

	/// 函數（古い形式は "code" 1 つ → 函數 1 つ）
	functions_.clear();
	if (file.contains("functions")) {
		for (const auto& f : file["functions"]) {
			functions_.push_back({ f.value("name", std::string("Function")), f.value("enabled", true), f.value("code", std::string()) });
		}
	} else {
		functions_.push_back({ "Main", true, file.value("code", std::string()) });
	}
	if (functions_.empty()) functions_.push_back(MakerFunction{});
	currentFunction_ = 0;
	/// 生成設定（古いファイルは maxCol / maxRow）
	const nlohmann::json ctx = file.value("context", nlohmann::json::object());
	generateSettings_.targetMapName = ctx.value("targetMap", std::string());
	generateSettings_.width = ctx.value("width", ctx.value("maxCol", 32));
	generateSettings_.height = ctx.value("height", ctx.value("maxRow", 32));
	generateSettings_.seed = ctx.value("seed", 0);

	/// 変数：BuildJson は variables[ { unitName, units:[...] } ] の形で書いている
	generatorVariables_.variableList.clear();
	generatorVariables_ = MapProgramAdapter::JsonToVariables(file);

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

MapMaker::MakerFunction* MapMaker::CurrentFunction() {
	return (currentFunction_ >= 0 && currentFunction_ < (int)functions_.size()) ? &functions_[currentFunction_] : nullptr;
}
const MapMaker::MakerFunction* MapMaker::CurrentFunction() const {
	return (currentFunction_ >= 0 && currentFunction_ < (int)functions_.size()) ? &functions_[currentFunction_] : nullptr;
}

void MapMaker::SelectFunction(int index) {
	currentFunction_ = (index >= 0 && index < (int)functions_.size()) ? index : -1;
}

void MapMaker::AddFunction(const std::string& name) {
	MakerFunction f;
	f.name = name;
	/// 選んでいる函數のすぐ下に入れる（無ければ最後）
	const int at = (currentFunction_ >= 0) ? currentFunction_ + 1 : (int)functions_.size();
	functions_.insert(functions_.begin() + at, f);
	currentFunction_ = at;
}

void MapMaker::DuplicateFunction(int index) {
	if (index < 0 || index >= (int)functions_.size()) return;
	MakerFunction copy = functions_[index];
	copy.name += "_copy";
	functions_.insert(functions_.begin() + index + 1, copy);
	currentFunction_ = index + 1;
}

void MapMaker::RemoveFunction(int index) {
	if (index < 0 || index >= (int)functions_.size()) return;
	functions_.erase(functions_.begin() + index);
	if (functions_.empty()) functions_.push_back(MakerFunction{});   
	currentFunction_ = (std::min)(index, (int)functions_.size() - 1);
}

void MapMaker::MoveFunction(int index, int dir) {
	const int to = index + dir;
	if (index < 0 || index >= (int)functions_.size() || to < 0 || to >= (int)functions_.size()) return;
	std::swap(functions_[index], functions_[to]);
	currentFunction_ = to;                                           
}