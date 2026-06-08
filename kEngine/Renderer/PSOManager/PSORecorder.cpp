#include "PSORecorder.h"

#include <iostream>
#include <fstream>
#include "externals/nlohmann/json.hpp"

void PSORecorder::Load(const std::string& path) {

	/// ロケーションを決める
	if (path.empty()) {
		psoCacheFilePath_ = config::GetPsoCacheFolderPath() + config::GetPsoCacheFolderName();
	} else {
		psoCacheFilePath_ = path;
	}
	const std::string fullPath = psoCacheFilePath_;

	/// 読み込み
	nlohmann::json file;
	{
		/// PSOcacheを探す、なければ新規作成
		std::ifstream input(fullPath);
		if (!input.is_open()) {
			std::cout << "[kEngine] PSOManager: Cache file missing, creating new one.(weird)\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}

		/// 試し読み、もし壊れていたら再生成
		try {
			input >> file;
		} catch (...) {
			std::cout << "[kEngine] PSOManager: Cache file corrupted, recreating.\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}
	}

	auto& arr = file["pso_keys"];

	/// 開始の符号0を探す
	size_t startIndex = arr.size();
	for (size_t i = 0; i < arr.size(); i++) {
		if (arr[i].is_number_integer() && arr[i].get<int>() == 0) {
			startIndex = i + 1;
			break;
		}
	}

	/// 開始の符号0が見つからなけれ再生成
	if (startIndex >= arr.size()) {
		std::cout << "[kEngine] PSOManager: No valid start index found in cache, recreating." << std::endl;
		CreateDefaultPSOcacheJson(fullPath);
		return;
	}

	/// PSOKeyを読み、リストに記録する
	for (size_t i = startIndex; i < arr.size(); i++) {

		const auto& record = arr[i];

		// record 必須是 array，且大小為 2
		if (!record.is_array() || record.size() != 2) {
			std::cout << "[kEngine] PSOManager: Invalid record at index " << i << std::endl;
			continue;
		}

		const auto& psoFields = record[0];
		const auto& flags = record[1];

		if (!psoFields.is_array() || psoFields.size() != keyElementCount) {
			std::cout << "[kEngine] PSOManager: Invalid PSO fields at index " << i << std::endl;
			continue;
		}

		PSOKey key;
		key.lightModelType = (LightModelType)psoFields[0].get<int>();
		key.renderModelType = (RenderModelType)psoFields[1].get<int>();
		key.blendModeType = (BlendModeType)psoFields[2].get<int>();
		key.rasterizerMode = (RasterizerMode)psoFields[3].get<int>();
		key.depthStencilType = (DepthStencilType)psoFields[4].get<int>();
		key.primitiveType = (PrimitiveType)psoFields[5].get<int>();
		key.renderTargetFormatType = (RenderTargetFormatType)psoFields[6].get<int>();

		if (flags.is_array() && flags.size() > 0 && flags[0].get<int>() == 1) {
			key.featureMask |= FeatureFlags::EnvReflection;
		}

		keylist_.push_back(key);
	}
}

void PSORecorder::AddKey(const PSOKey& key) {

	/// まずは予定したところにPSOKeyを保存したファイルがあるか探す
	const std::string fullPath = psoCacheFilePath_;

	/// 読み込み
	nlohmann::json file;
	{
		std::ifstream input(fullPath);
		if (!input.is_open()) {
			std::cout << "[kEngine] PSOManager: Cache file missing, creating new one.(weird)\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}

		try {
			input >> file;
		} catch (...) {
			std::cout << "[kEngine] PSOManager: Cache file corrupted, recreating.\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}
	}

	auto& arr = file["pso_keys"];

	// 只從 0 之後開始檢查重複
	size_t startIndex = 0;
	for (size_t i = 0; i < arr.size(); i++) {
		if (arr[i].is_number_integer() && arr[i].get<int>() == 0) {
			startIndex = i + 1;
			break;
		}
	}

	/// 重複チェック
	for (size_t i = startIndex; i < arr.size(); i++) {
		if (arr[i].is_array() && arr[i].size() == 2) {
			auto& psoFields = arr[i][0];
			auto& flags = arr[i][1];

			bool same =
				psoFields[0] == (int)key.lightModelType &&
				psoFields[1] == (int)key.renderModelType &&
				psoFields[2] == (int)key.blendModeType &&
				psoFields[3] == (int)key.rasterizerMode &&
				psoFields[4] == (int)key.depthStencilType &&
				psoFields[5] == (int)key.primitiveType &&
				psoFields[6] == (int)key.renderTargetFormatType &&
				flags[0] == (int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection);

			if (same)return;
		}
	}

	/// 書き込み
	arr.push_back(
		{
			{
				(int)key.lightModelType,
				(int)key.renderModelType,
				(int)key.blendModeType,
				(int)key.rasterizerMode,
				(int)key.depthStencilType,
				(int)key.renderTargetFormatType,
				(int)key.primitiveType
			},

			{
				(int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection)
			}
		});

	/// ファイルに保存
	std::ofstream output(fullPath);
	output << FormatPSOJson(file);
}

void PSORecorder::CreateDefaultPSOcacheJson(std::string fullPath) {

	// 直接生成一個新的 JSON 檔案
	nlohmann::json file;
	file["pso_keys"] = nlohmann::json::array();
	file["pso_keys"].push_back("OrderBy:");
	file["pso_keys"].push_back("LightModel,RenderModel,BlendMode,RasterizerMode,DepthStencil,PrimitiveType");
	file["pso_keys"].push_back("EnvReflection");
	file["pso_keys"].push_back(0);

	std::ofstream output(fullPath);
	if (!output.is_open()) {
		throw std::runtime_error("[kEngine] Failed to create JSON file: " + fullPath);
	}

	output << file.dump(4);
	output.close();
}

void PSORecorder::AppendPSOKeyRecord(const PSOKey& key) {

	/// まずは予定したところにPSOKeyを保存したファイルがあるか探す
	const std::string fullPath = psoCacheFilePath_;

	/// 読み込み
	nlohmann::json file;
	{
		std::ifstream input(fullPath);
		if (!input.is_open()) {
			std::cout << "[kEngine] PSOManager: Cache file missing, creating new one.(weird)\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}

		try {
			input >> file;
		} catch (...) {
			std::cout << "[kEngine] PSOManager: Cache file corrupted, recreating.\n";
			CreateDefaultPSOcacheJson(fullPath);
			return;
		}
	}

	auto& arr = file["pso_keys"];

	// 只從 0 之後開始檢查重複
	size_t startIndex = 0;
	for (size_t i = 0; i < arr.size(); i++) {
		if (arr[i].is_number_integer() && arr[i].get<int>() == 0) {
			startIndex = i + 1;
			break;
		}
	}

	/// 重複チェック
	for (size_t i = startIndex; i < arr.size(); i++) {
		if (arr[i].is_array() && arr[i].size() == 2) {
			auto& psoFields = arr[i][0];
			auto& flags = arr[i][1];

			bool same =
				psoFields[0] == (int)key.lightModelType &&
				psoFields[1] == (int)key.renderModelType &&
				psoFields[2] == (int)key.blendModeType &&
				psoFields[3] == (int)key.rasterizerMode &&
				psoFields[4] == (int)key.depthStencilType &&
				psoFields[5] == (int)key.primitiveType &&
				psoFields[6] == (int)key.renderTargetFormatType &&
				flags[0] == (int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection);

			if (same)return;
		}
	}

	/// 書き込み
	arr.push_back(
		{
			{
				(int)key.lightModelType,
				(int)key.renderModelType,
				(int)key.blendModeType,
				(int)key.rasterizerMode,
				(int)key.depthStencilType,
				(int)key.renderTargetFormatType,
				(int)key.primitiveType
			},

			{
				(int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection)
			}
		});

	/// ファイルに保存
	std::ofstream output(fullPath);
	output << FormatPSOJson(file);
}

std::string PSORecorder::FormatPSOJson(const nlohmann::json& file) {

	std::string s = file.dump(4);
	std::string out;
	out.reserve(s.size());

	bool afterZero = false;
	int bracketDepth = -1;
	bool compressingRecord = false;

	for (size_t i = 0; i < s.size(); ++i) {
		char c = s[i];

		// ============================
		// 0 之前不壓縮
		// ============================
		if (!afterZero) {
			out += c;

			if (c == '0') {
				bool isZero = true;
				if (i > 0 && std::isdigit(s[i - 1])) isZero = false;
				if (i + 1 < s.size() && std::isdigit(s[i + 1])) isZero = false;
				if (isZero) afterZero = true;
				bracketDepth = 0;
			}
			continue;
		}

		// ============================
		// 0 之後開始壓縮 PSOKey
		// ============================

		if (c == '[') {
			bracketDepth++;
		}


		if (c == ']') {
			bracketDepth--;
		}

		// 壓縮 record：只保留數字與逗號
		if (bracketDepth > 0) {
			if (!(c == ' ' || c == '\n')) {
				out += c;
			}
			continue;
		}

		// 其他正常輸出
		out += c;
	}

	return out;
}
