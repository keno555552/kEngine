#pragma once
#include <string>
#include <vector>
#include "Tool/EffectSystem/ParticleSystem/EffectData.h"

#include <fstream>
#include "externals/nlohmann/json.hpp"

class kEngine;
class EffectAdapter
{
public:
	EffectAdapter(kEngine* system) : system_(system) {}

	/// json形式のファイルからEffectDataを読み込む
	EffectData LoadEffect(const std::string& filePath);

	/// EffectDataを保存する、json形式で保存する
	void SaveEffect(const EffectData& effectData,
					const std::string& filePath,
					const std::string& name = "");
	
	void SaveParticle(const ParticlePrototype& effectData,
					  const std::string& filePath,
					  const std::string& name = "");

	void SaveEmitterLink(const EmitterLink& emitterLink,
						 const std::string& filePath,
						 const EffectData& effectData = {},
						 const std::string& name = "");

private:
	kEngine* system_ = nullptr;							// 借り
	
	nlohmann::json EffectToJson(const EffectData& effectData);
	nlohmann::json ParticlePrototypeToJson(const ParticlePrototype& proto);
	nlohmann::json EmitterLinkToJson(const EmitterLink& link, const EffectData& effectData = {});
};

