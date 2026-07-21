#pragma once
#include <vector>
#include <string>

enum class LinkMode {
	PerBurst,
	PerParticle
};

enum class LinkFollow {
	Particle,
	Emitter
};

enum class EmitterTiming {
	SourceEmit,		// ソースエミット時
	TimeUp,			// ソースアップデート時
	SourceEnd 		// ソース終了時
};

struct EmitterLink {
	std::string name = "";
	std::string sourceName = "";									// A
	std::string targetName = "";
	int emitCount{ 1 };											// B
	float delayTime{};											// 遅延時間
	bool followPosition{ true };
	bool followRotation{ false };
	bool followScale{ false };

	LinkMode linkMode{ LinkMode::PerBurst };					// リンクの更新頻度
	LinkFollow linkFollow{ LinkFollow::Emitter };				// 追従対象
	EmitterTiming emitterTiming{ EmitterTiming::SourceEmit };	// 連動するタイミング

	// Editor用のデータ
	bool isSaved = false; // 保存済みかどうか
};