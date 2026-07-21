#include "EffectAdapter.h"

#include "kEngine.h"
#include "Tool/JsonHelper/JsonHelper.h"


EffectData EffectAdapter::LoadEffect(const std::string& filePath) {

	/// まずDataを作る
	EffectData readData;

	/// ファイルを開く
	std::ifstream inputFile(filePath);





	return readData;
}

void EffectAdapter::SaveEffect(const EffectData& effectData, const std::string& filePath, const std::string& name) {

	std::string  correctPath = filePath + name + ".json";

	/// 新しいファイルを作成し、初期内容を書き込む
	nlohmann::json file;
	file["EffectData"] = nlohmann::json::array();

	/// EffectDataのデータ
	nlohmann::json j;

	j = EffectToJson(effectData);

	/// EffectDataを追加
	std::ofstream ofs(correctPath);
	if (!ofs.is_open()) {
		std::cerr << "[EffectAdapter] Failed to save file: " << correctPath << std::endl;
		return;
	}

	ofs << j.dump(4); // 美化輸出
	ofs.close();
}

void EffectAdapter::SaveParticle(const ParticlePrototype& effectData, const std::string& filePath, const std::string& name) {

	/// pathを組み立てる
	std::string correctPath = filePath + name;

	// .json検査及び修正
	if (correctPath.ends_with(".json.json")) {
		correctPath = correctPath.substr(0, correctPath.length() - 5);
	} else if (correctPath.ends_with(".json") == false) {
		correctPath += ".json";
	}

	// 如果有傳 name，就覆蓋掉 link.name（可選）
	ParticlePrototype particleCopy = effectData;

	// 轉成 JSON
	nlohmann::json j = ParticlePrototypeToJson(particleCopy);

	// 寫入檔案
	std::ofstream ofs(correctPath);
	if (!ofs.is_open()) {
		std::cerr << "[EffectAdapter] Failed to save file: " << correctPath << std::endl;
		return;
	}

	ofs << j.dump(4); // 美化輸出
	ofs.close();

}

void EffectAdapter::SaveEmitterLink(const EmitterLink& emitterLink, const std::string& filePath, const EffectData& effectData, const std::string& name) {

	/// pathを組み立てる
	std::string correctPath = filePath + name;

	// .json検査及び修正
	if (correctPath.ends_with(".json.json")) {
		correctPath = correctPath.substr(0, correctPath.length() - 5);
	} else if (correctPath.ends_with(".json") == false) {
		correctPath += ".json";
	}

	// 如果有傳 name，就覆蓋掉 link.name（可選）
	EmitterLink linkCopy = emitterLink;

	// 轉成 JSON
	nlohmann::json j = EmitterLinkToJson(linkCopy, effectData);

	// 寫入檔案
	std::ofstream ofs(correctPath);
	if (!ofs.is_open()) {
		std::cerr << "[EffectAdapter] Failed to save file: " << correctPath << std::endl;
		return;
	}

	ofs << j.dump(4); // 美化輸出
	ofs.close();
}

nlohmann::json EffectAdapter::EffectToJson(const EffectData& effectData) {

	nlohmann::json effectProto;

	effectProto["name"] = effectData.name;

	effectProto["ParticlePrototypes"] = nlohmann::json::array();
	for (const auto& proto : effectData.prototypes) {
		effectProto["ParticlePrototypes"].push_back(ParticlePrototypeToJson(proto));
	}

	effectProto["EmitterLinks"] = nlohmann::json::array();
	for (const auto& link : effectData.links) {
		effectProto["EmitterLinks"].push_back(EmitterLinkToJson(link, effectData));
	}

	return effectProto;
}

nlohmann::json EffectAdapter::ParticlePrototypeToJson(const ParticlePrototype& proto) {

	nlohmann::json particleProto;

	particleProto["name"] = proto.name;
	particleProto["lifetime"] = proto.lifetime;
	particleProto["lifetimeRandomness"] = proto.lifetimeRandomness;

	particleProto["startPosition"] = JsonHelper::V3ToJson(proto.startPosition);
	particleProto["startPositionRandom"] = JsonHelper::V3ToJson(proto.startPositionRandom);

	particleProto["startVelocity"] = JsonHelper::V3ToJson(proto.startVelocity);
	particleProto["startVelocityRandom"] = JsonHelper::V3ToJson(proto.startVelocityRandom);
	particleProto["startSpeedRandom"] = JsonHelper::V3ToJson(proto.startSpeedRandom);

	particleProto["startScale"] = JsonHelper::V3ToJson(proto.startScale);
	particleProto["endScale"] = JsonHelper::V3ToJson(proto.endScale);
	particleProto["startScaleRandom"] = JsonHelper::V3ToJson(proto.startScaleRandom);
	particleProto["endScaleRandom"] = JsonHelper::V3ToJson(proto.endScaleRandom);
	particleProto["isConstantScale"] = proto.isConstantScale;

	particleProto["startRotation"] = JsonHelper::V3ToJson(proto.startRotation);
	particleProto["endRotation"] = JsonHelper::V3ToJson(proto.endRotation);
	particleProto["startRotationRandom"] = JsonHelper::V3ToJson(proto.startRotationRandom);
	particleProto["endRotationRandom"] = JsonHelper::V3ToJson(proto.endRotationRandom);
	particleProto["isConstantRotation"] = proto.isConstantRotation;

	particleProto["startColor"] = JsonHelper::V4ToJson(proto.startColor);
	particleProto["endColor"] = JsonHelper::V4ToJson(proto.endColor);
	particleProto["startColorRandom"] = JsonHelper::V4ToJson(proto.startColorRandom);
	particleProto["endColorRandom"] = JsonHelper::V4ToJson(proto.endColorRandom);
	particleProto["isConstantColor"] = proto.isConstantColor;

	particleProto["gravity"] = JsonHelper::V3ToJson(proto.gravity);
	particleProto["damping"] = proto.damping;
	particleProto["dampingRandom"] = proto.dampingRandom;

	particleProto["emitRate"] = proto.emitRate;
	particleProto["emitIntervalRandom"] = proto.emitIntervalRandom;

	particleProto["burstCount"] = proto.burstCount;
	particleProto["emitNumRandom"] = proto.emitNumRandom;

	particleProto["renderType"] = (int)proto.renderType;

	/// ====================== ObjectListのデータ
	nlohmann::json ObjectDataJson = nlohmann::json::array();

	int partIndex = 0;
	for (const auto& part : proto.objectList) {
		ObjectDataJson.push_back(JsonHelper::ObjectDataToJson(part, system_));
	}
	particleProto["objectList"] = ObjectDataJson;

	/// ====================== SpriteListのデータ
	/// まだ使えないから書かない

	return particleProto;
}

nlohmann::json EffectAdapter::EmitterLinkToJson(const EmitterLink& link, const EffectData& effectData) {

	bool haveEffectData = false;
	if (!effectData.prototypes.empty()) {
		auto finder = std::find_if(effectData.links.begin(),
			effectData.links.end(),
			[&link](const EmitterLink& l) { return l.name == link.name; });

		/// リンクが存在する場合
		if (finder != effectData.links.end()) {
			/// ソースとターゲットのプロトタイプが存在するか確認する
			bool sourceExists = std::find_if(
				effectData.prototypes.begin(),
				effectData.prototypes.end(),
				[&](const ParticlePrototype& p) {
					return p.name == link.sourceName;
				}
			) != effectData.prototypes.end();

			bool targetExists = std::find_if(
				effectData.prototypes.begin(),
				effectData.prototypes.end(),
				[&](const ParticlePrototype& p) {
					return p.name == link.targetName;
				}
			) != effectData.prototypes.end();

			if (sourceExists && targetExists) {
				haveEffectData = true;
			}
		}
	}

	nlohmann::json emitterLink;
	emitterLink["name"] = link.name;
	if (!haveEffectData) {
		emitterLink["sourceEmitter"] = "error";
		emitterLink["targetEmitter"] = "error";
	} else {
		emitterLink["sourceEmitter"] = link.sourceName;
		emitterLink["targetEmitter"] = link.targetName;
	}
	emitterLink["emissionRate"] = link.emitCount;
	emitterLink["delayTime"] = link.delayTime;
	emitterLink["followPosition"] = link.followPosition;
	emitterLink["followRotation"] = link.followRotation;
	emitterLink["followScale"] = link.followScale;
	emitterLink["linkMode"] = (int)link.linkMode;
	emitterLink["linkFollow"] = (int)link.linkFollow;
	emitterLink["emitterTiming"] = (int)link.emitterTiming;

	return emitterLink;
}




//void PSORecorder::Load(const std::string& path) {
//
//	/// ロケーションを決める
//	if (path.empty()) {
//		psoCacheFilePath_ = config::GetPsoCacheFolderPath() + config::GetPsoCacheFolderName();
//	} else {
//		psoCacheFilePath_ = path;
//	}
//
//	/// 読み込み
//	nlohmann::json file = findAndCreatePSOCache();
//
//	auto& arr = file["pso_keys"];
//
//	/// 開始の符号0を探す
//	size_t startIndex = FindStartIndex(arr);
//	if (startIndex == -1) return;
//
//	/// PSOKeyを読み、リストに記録する
//	for (size_t i = startIndex; i < arr.size(); i++) {
//
//		const auto& record = arr[i];
//
//		// record 必須是 array，且大小為 2
//		if (!record.is_array() || record.size() != 2) {
//			std::cout << "[kEngine] PSOManager: Invalid record at index " << i << std::endl;
//			continue;
//		}
//
//		const auto& psoFields = record[0];
//		const auto& flags = record[1];
//
//		if (!psoFields.is_array() || psoFields.size() != keyElementCount) {
//			std::cout << "[kEngine] PSOManager: Invalid PSO fields at index " << i << std::endl;
//			continue;
//		}
//
//		PSOKey key;
//		key.lightModelType = (LightModelType)psoFields[0].get<int>();
//		key.renderModelType = (RenderModelType)psoFields[1].get<int>();
//		key.blendModeType = (BlendModeType)psoFields[2].get<int>();
//		key.rasterizerMode = (RasterizerMode)psoFields[3].get<int>();
//		key.depthStencilType = (DepthStencilType)psoFields[4].get<int>();
//		key.primitiveType = (PrimitiveType)psoFields[5].get<int>();
//		key.renderTargetFormatType = (RenderTargetFormatType)psoFields[6].get<int>();
//
//		if (flags.is_array() && flags.size() > 0) {
//			/// 初期化
//			key.featureMask = 0;
//			/// 一つずつビットをチェックして featureMask にセットする
//			for (size_t i = 0; i < flags.size(); i++) {
//				int bit = flags[i].get<int>();
//				if (bit == 1) {
//					key.featureMask |= (1ull << i);
//				}
//			}
//		}
//
//		keylist_.push_back(key);
//	}
//}
//
//void PSORecorder::AppendPSOKeyRecord(const PSOKey& key) {
//
//	/// まずはListにいるかどうかを確認する
//	for (const auto& existingKey : keylist_) {
//		if (existingKey == key) {
//			return;
//		}
//	}
//
//	/// 読み込み
//	nlohmann::json file = findAndCreatePSOCache();
//
//	auto& arr = file["pso_keys"];
//
//	/// 0の位置を探す 
//	size_t startIndex = FindStartIndex(arr);
//
//	/// 重複チェック
//	for (size_t i = startIndex; i < arr.size(); i++) {
//		if (arr[i].is_array() && arr[i].size() == 2) {
//			auto& psoFields = arr[i][0];
//			auto& flags = arr[i][1];
//
//			bool same =
//				psoFields[0] == (int)key.lightModelType &&
//				psoFields[1] == (int)key.renderModelType &&
//				psoFields[2] == (int)key.blendModeType &&
//				psoFields[3] == (int)key.rasterizerMode &&
//				psoFields[4] == (int)key.depthStencilType &&
//				psoFields[5] == (int)key.primitiveType &&
//				psoFields[6] == (int)key.renderTargetFormatType;
//
//			if (!same) continue;
//
//			// flags array 比較
//			int flagSameCount = 0;
//			if (flags.is_array() && flags.size() == featureFlagsCount) {
//				for (size_t b = 0; b < featureFlagsCount; b++) {
//					int bit = (key.featureMask & (1ull << b)) ? 1 : 0;
//					if (flags[b].get<int>() == bit) {
//						flagSameCount++;
//					}
//				}
//			}
//			if (flagSameCount == featureFlagsCount) return;
//		}
//	}
//
//	/// 書き込み
//	arr.push_back(
//		{
//			{
//				(int)key.lightModelType,
//				(int)key.renderModelType,
//				(int)key.blendModeType,
//				(int)key.rasterizerMode,
//				(int)key.depthStencilType,
//				(int)key.primitiveType,
//				(int)key.renderTargetFormatType,
//			},
//
//			{
//				(int)(key.featureMask & (uint64_t)FeatureFlags::EnvReflection)
//			}
//		});
//
//	/// ファイルに保存
//	std::ofstream output(psoCacheFilePath_);
//	output << FormatPSOJson(file);
//
//	keylist_.push_back(key);
//}
//
//void PSORecorder::CreateDefaultPSOcacheJson(std::string fullPath) {
//
//	/// 新しいファイルを作成し、初期内容を書き込む
//	nlohmann::json file;
//	file["pso_keys"] = nlohmann::json::array();
//	file["pso_keys"].push_back("OrderBy:");
//	file["pso_keys"].push_back("LightModel,RenderModel,BlendMode,RasterizerMode,DepthStencil,PrimitiveType,RenderTargetFormatType");
//	file["pso_keys"].push_back("EnvReflection");
//	file["pso_keys"].push_back(0);
//
//	std::ofstream output(fullPath);
//	if (!output.is_open()) {
//		throw std::runtime_error("[kEngine] Failed to create JSON file: " + fullPath);
//	}
//
//	output << file.dump(4);
//	output.close();
//}