#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Tool/MapBulider/MapChipManager.h"

class Object;

/// 1 レイヤー分のブロック（シーンが持つ物）
///  - マップの ID は MapChipManager、見た目の Object はこちら
///  - 壊す・置く時は両方を同時に更新する（当たり判定は ID を見るので自動で追従）
class MapBlockData {
public:
	~MapBlockData() = default;

	/// マップの大きさで入れ物を用意
	void Init(MapChipManager* manager, const std::string& layerName);
	/// MapChipManager の SetDefault から呼ぶ（直接タイルなら Object を作る）
	bool OnSpawn(const MapReader::SpawnInfo& info);

	/// position のブロックを壊す（無ければ false）
	bool Break(const Vector3& position);
	/// position に unitName のブロックを置く（埋まっている / 直接タイルでないなら false）
	bool Place(const Vector3& position, const std::string& unitName);

	void Update();
	void Draw(const Vector3& center, float drawRadius);
	void Clear() { blocks_.clear(); }

private:
	MapChipManager* manager_ = nullptr;   // 借り
	std::string layer_;
	std::vector<std::vector<std::unique_ptr<Object>>> blocks_;   // [y][x]、y = 0 が一番上
};