#pragma once
#include <vector>
#include "Tool/EffectSystem/ParticleSystem/ParticlePrototype.h"
#include "Tool/EffectSystem/ParticleSystem/EmitterLink.h"


class EffectData{
public:
    std::string name;
    std::vector<ParticlePrototype> prototypes;
    std::vector<EmitterLink> links;

	// Editor用のデータ
	bool isSaved = false; // 保存済みかどうか
};