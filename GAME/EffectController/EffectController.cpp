#include "EffectController.h"
#include "kEngine.h"

namespace {
	struct FxEntry {
		GameFx fx;
		std::vector<const char*> paths;
	};

	/// エフェクト表：1つの GameFx に複数パス → ランダム
	const FxEntry kTable[] = {
		{ GameFx::HitRock,{ "./GAME/resources/Effect/HitRock/HitRock.json" } },
		{ GameFx::BreakRock,{ "./GAME/resources/Effect/BrokeRock/BrokeRock.json",
							   "./GAME/resources/Effect/BrokeRock2/BrokeRock2.json",
							   "./GAME/resources/Effect/BrokeRock3/BrokeRock3.json" } },
	};
}

void EffectController::Initialize(kEngine* system, bool isMust) {
	if (!system) return;
	system_ = system;
	auto* em = system_->GetEffectManager();
	for (auto& list : handles_) list.clear();

	/// --- ファイルから ---
	Load(GameFx::HitRock, "./GAME/resources/Effect/HitRock/HitRock.json");

	Load(GameFx::BreakRock, "./GAME/resources/Effect/BrokeRock/BrokeRock.json");
	Load(GameFx::BreakRock, "./GAME/resources/Effect/BrokeRock2/BrokeRock2.json");
	Load(GameFx::BreakRock, "./GAME/resources/Effect/BrokeRock3/BrokeRock3.json");

	/// --- 土：石の Prototype をもらってテクスチャだけ変え、新しい Effect にする ---
	const int dirtTex = system_->LoadTexture("./GAME/resources/Stage/1YearGame/MapResource/dirt.png");
	for (int src : handles_[(size_t)GameFx::BreakRock]) {
		EffectData data;
		if (!em->GetEffectData(src, data)) continue;

		for (auto& proto : data.prototypes) {
			for (auto& obj : proto.objectList) {
				for (auto& part : obj.objectParts_) {
					part.materialConfig->textureHandle = dirtTex;
					part.materialConfig->useModelTexture = false;
				}
			}
		}

		const int h = em->RegisterEffect("GameFx/BreakDirt/" + std::to_string(src), data);
		if (h >= 0) handles_[(size_t)GameFx::BreakDirt].push_back(h);
	}
}

void EffectController::Finalize() {
	if (!system_) return;
	auto* em = system_->GetEffectManager();

	for (auto& list : handles_) {
		for (int handle : list) em->UnloadEffect(handle);   // 同じハンドルが2回来ても IsValid で弾かれる
		list.clear();
	}
	system_ = nullptr;
}

void EffectController::Play(GameFx fx, const Vector3& position) {
	if (!system_) return;
	const auto& list = handles_[(size_t)fx];
	if (list.empty()) return;

	std::uniform_int_distribution<size_t> pick(0, list.size() - 1);
	system_->GetEffectManager()->Play(list[pick(rng_)], position);
}

void EffectController::Load(GameFx fx, const char* path) {
	const int handle = system_->GetEffectManager()->LoadEffect(path);
	if (handle >= 0) handles_[(size_t)fx].push_back(handle);
}