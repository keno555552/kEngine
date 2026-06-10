#include "PSORecorder.h"

void PSORecorder::Load(const std::string& path) {

	/// ロケーションを決める
	if (path.empty()) {
		psoCacheFilePath_ = config::GetPsoCacheFolderPath() + config::GetPsoCacheFolderName();
	} else {
		psoCacheFilePath_ = path;
	}

	/// 読み込み
	nlohmann::json file = findAndCreatePSOCache();

	auto& arr = file["pso_keys"];

	/// 開始の符号0を探す
	size_t startIndex = FindStartIndex(arr);
	if (startIndex == -1) return;

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

		if (flags.is_array() && flags.size() > 0) {
			/// 初期化
			key.featureMask = 0;
			/// 一つずつビットをチェックして featureMask にセットする
    		for (size_t i = 0; i < flags.size(); i++) {
    		    int bit = flags[i].get<int>();
    		    if (bit == 1) {
    		        key.featureMask |= (1ull << i);
    		    }
    		}
		}

		keylist_.push_back(key);
	}
}

void PSORecorder::AppendPSOKeyRecord(const PSOKey& key) {

	/// まずはListにいるかどうかを確認する
	for (const auto& existingKey : keylist_) {
		if(existingKey == key) {
			return;
		}
	}

	/// 読み込み
	nlohmann::json file = findAndCreatePSOCache();

	auto& arr = file["pso_keys"];

	/// 0の位置を探す 
	size_t startIndex = FindStartIndex(arr);

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
				psoFields[6] == (int)key.renderTargetFormatType;

			if (!same) continue;

            // flags array 比較
			int flagSameCount = 0;
            if (flags.is_array() && flags.size() == featureFlagsCount) {
                for (size_t b = 0; b < featureFlagsCount; b++) {
                    int bit = (key.featureMask & (1ull << b)) ? 1 : 0;
                    if (flags[b].get<int>() == bit) {
                        flagSameCount++;
                    } 
                }
            }
			if (flagSameCount == featureFlagsCount) return;
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
				(int)key.primitiveType,
				(int)key.renderTargetFormatType,
			},

			{
				(int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection)
			}
		});

	/// ファイルに保存
	std::ofstream output(psoCacheFilePath_);
	output << FormatPSOJson(file);

	keylist_.push_back(key);
}

void PSORecorder::CreateDefaultPSOcacheJson(std::string fullPath) {

	/// 新しいファイルを作成し、初期内容を書き込む
	nlohmann::json file;
	file["pso_keys"] = nlohmann::json::array();
	file["pso_keys"].push_back("OrderBy:");
	file["pso_keys"].push_back("LightModel,RenderModel,BlendMode,RasterizerMode,DepthStencil,PrimitiveType,RenderTargetFormatType");
	file["pso_keys"].push_back("EnvReflection");
	file["pso_keys"].push_back(0);

	std::ofstream output(fullPath);
	if (!output.is_open()) {
		throw std::runtime_error("[kEngine] Failed to create JSON file: " + fullPath);
	}

	output << file.dump(4);
	output.close();
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
		// 0の前は普通に出力
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
		// 0の後はフォーマットを並びなおす
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

nlohmann::json PSORecorder::findAndCreatePSOCache() {

	const std::string fullPath = psoCacheFilePath_;

	/// 読み込み
	nlohmann::json file;
	{
		/// PSOcacheを探す、なければ新規作成
		std::ifstream input(fullPath);
		if (!input.is_open()) {
			std::cout << "[kEngine] PSOManager: Cache file missing, creating new one.(weird)\n";
			CreateDefaultPSOcacheJson(fullPath);
			return nlohmann::json();
		}

		/// 試し読み、もし壊れていたら再生成
		try {
			input >> file;
		} catch (...) {
			std::cout << "[kEngine] PSOManager: Cache file corrupted, recreating.\n";
			CreateDefaultPSOcacheJson(fullPath);
			return nlohmann::json();
		}
	}
	return file;
}

size_t PSORecorder::FindStartIndex(const nlohmann::json& arr) {

	const std::string fullPath = psoCacheFilePath_;

	// 0まで探す
	size_t startIndex = arr.size();
	for (size_t i = 0; i < arr.size(); i++) {
		if (arr[i].is_number_integer() && arr[i].get<int>() == 0) {
			startIndex = i + 1;
			break;
		}
	}

	/// もし0が見つからなかったらエラー
	if (startIndex >= arr.size()) {
		std::cout << "[kEngine] PSOManager: No valid start index found in cache, recreating." << std::endl;
		CreateDefaultPSOcacheJson(fullPath);
		return -1; 
	}
	return startIndex;
}
