#include "MapChipField.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <algorithm>

namespace {
	std::map<std::string, MapChipType> mapChipTable = {
		{"0", MapChipType::kBlank},
		{"1", MapChipType::kDirt},
		{"2", MapChipType::kRock},
		{"7", MapChipType::kEnemy},
		{"8", MapChipType::kPlayer},
		{"9", MapChipType::kBackPoint},
	};
}

void MapChipField::ResetMapChipData() {
	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}

void MapChipField::LoadMapChipCsv(const std::string& filePath) {
	std::ifstream file(filePath);
	if (!file.is_open()) { return; }

	// 先讀入所有行以判斷列數/行數
	std::vector<std::string> lines;
	std::string line;
	while (getline(file, line)) {
		if (!line.empty()) { lines.push_back(line); }
	}
	file.close();
	if (lines.empty()) { return; }

	// 計算尺寸
	int newVertical = static_cast<int>(lines.size());
	int newHorizontal = 0;
	{
		std::istringstream ls(lines[0]);
		std::string word;
		while (getline(ls, word, ',')) { ++newHorizontal; }
	}
	if (newHorizontal == 0) { return; }

	// 更新尺寸並重置資料
	kNumBlockVirtical = newVertical;
	kNumBlockHorizontal = newHorizontal;
	ResetMapChipData();

	// 解析 CSV
	for (int i = 0; i < kNumBlockVirtical && i < static_cast<int>(lines.size()); ++i) {
		std::istringstream line_stream(lines[i]);
		for (int j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

/// マップチップ種類を取得
MapChipType MapChipField::GetMapChipTypeByMap(MapIndex mapIndex) {

	if ((mapIndex.x < 0) || (kNumBlockHorizontal - 1 < mapIndex.x)) {
		return MapChipType::kBlank;
	}
	if ((mapIndex.y < 0) || (kNumBlockVirtical - 1 < mapIndex.y)) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[mapIndex.y][mapIndex.x];
}

MapChipType MapChipField::GetMapChipTypeByWorld(WorldIndex worldIndex) {
	MapIndex mapIndex = ExchangeWorld2MapIndex(worldIndex);
	return GetMapChipTypeByMap(mapIndex);
}

/// マップチップ種類を設定
void MapChipField::SetMapChipTypeByMap(MapIndex mapIndex, MapChipType type) {
	if ((mapIndex.x < 0) || (kNumBlockHorizontal - 1 < mapIndex.x)) {
		return;
	}
	if ((mapIndex.y < 0) || (kNumBlockVirtical - 1 < mapIndex.y)) {
		return;
	}
	mapChipData_.data[mapIndex.y][mapIndex.x] = type;
}

void MapChipField::SetMapChipTypeByWorld(WorldIndex worldIndex, MapChipType type) {
	MapIndex mapIndex = ExchangeWorld2MapIndex(worldIndex);
	SetMapChipTypeByMap(mapIndex, type);
}

/// マップチップ座標を取得
Vector3 MapChipField::GetWorldPosFromMapByMapIndex(MapIndex mapIndex) {
	Vector3 leftBottom{ kBlockWidth * mapIndex.x,
						kBlockHeight * (kNumBlockVirtical - 1 - mapIndex.y),
						0 };

	Vector3 center{ leftBottom.x + kBlockWidth * 0.5f,
						leftBottom.y + kBlockHeight * 0.5f,
						0 };

	return center;
}

Vector3 MapChipField::GetWorldPosFromMapByWorldIndex(WorldIndex worldIndex) {
	MapIndex mapIndex = ExchangeWorld2MapIndex(worldIndex);
	return GetWorldPosFromMapByMapIndex(mapIndex);
}

/// 座標からマップチップの番号を計算
MapChipField::WorldIndex MapChipField::GetWorldIndexByPosition(const Vector3& position) {
	WorldIndex worldIndex = {};
	worldIndex.x = static_cast<int>(std::floor(position.x / kBlockWidth));
	worldIndex.y = static_cast<int>(std::floor(position.y / kBlockHeight));
	return worldIndex;
}

MapChipField::MapIndex MapChipField::GetMapIndexByPosition(const Vector3& position) {
	WorldIndex worldIndex = GetWorldIndexByPosition(position);
	return ExchangeWorld2MapIndex(worldIndex);
}

//CollisionResult MapChipField::GetMapCollisionCorrection(
//	const AABB& box, const Vector3& velocity, float deltaTime, bool& landed) {
//	
//	landed = false;
//	Vector3 unConstVelocity = velocity;
//	Vector3 move = unConstVelocity * deltaTime;
//	Vector3 finalMove = move;
//	const float EPS = 0.001f;
//
//	AABB current = box;
//
//	bool xIsMainAxis = false;
//	bool yIsMainAxis = false;
//
//	for (int step = 0; step < 2; ++step) {
//
//		AABB predicted = current;
//		predicted.min += finalMove;
//		predicted.max += finalMove;
//
//		auto tiles = GetTilesOverlapping(predicted);
//
//		float bestOverlapX = FLT_MAX;
//		float bestOverlapY = FLT_MAX;
//		float bestDirX = 0.0f;
//		float bestDirY = 0.0f;
//
//		bool hasHit = false;
//
//		for (auto& t : tiles) {
//			if (!t.isWall) continue;
//
//			const AABB& tile = t.aabb;
//
//			float overlapX =
//				std::min(predicted.max.x, tile.max.x) -
//				std::max(predicted.min.x, tile.min.x);
//
//			float overlapY =
//				std::min(predicted.max.y, tile.max.y) -
//				std::max(predicted.min.y, tile.min.y);
//
//			if (overlapX <= 0.0f || overlapY <= 0.0f) continue;
//
//			hasHit = true;
//
//			if (overlapX < bestOverlapX) {
//				bestOverlapX = overlapX;
//				bestDirX = (velocity.x > 0.0f ? -1.0f : 1.0f);
//			}
//
//			if (overlapY < bestOverlapY) {
//				bestOverlapY = overlapY;
//				bestDirY = (velocity.y > 0.0f ? -1.0f : 1.0f);
//			}
//		}
//
//		if (!hasHit) break;
//
//		Vector3 push{};
//		xIsMainAxis = false;
//		yIsMainAxis = false;
//
//		if (bestOverlapX < bestOverlapY) {
//			push.x = bestDirX * (bestOverlapX + EPS);
//			xIsMainAxis = true;
//		} else {
//			push.y = bestDirY * (bestOverlapY + EPS);
//			yIsMainAxis = true;
//			if (bestDirY > 0.0f) landed = true;
//		}
//
//		finalMove += push;
//		current.min += push;
//		current.max += push;
//	}
//
//	CollisionResult result;
//	result.correction = finalMove;
//	result.xIsMainAxis = xIsMainAxis;
//	result.yIsMainAxis = yIsMainAxis;
//	result.landed = landed;
//	return result;
//}

CollisionResult MapChipField::GetMapCollisionCorrection(
	const AABB& box, const Vector3& velocity, float deltaTime, bool& landed) {

	landed = false;
	Vector3 unConstVelocity = velocity;
	Vector3 move = unConstVelocity * deltaTime;
	Vector3 finalMove = move;
	const float EPS = 0.001f;

	AABB current = box;

	bool xIsMainAxis = false;
	bool yIsMainAxis = false;

	// ★★★ 最重要：逐 tile 累積推回，而不是只推一次 ★★★
	for (int step = 0; step < 2; ++step) {

		AABB predicted = current;
		predicted.min += finalMove;
		predicted.max += finalMove;

		auto tiles = GetTilesOverlapping(predicted);

		float bestOverlapX = FLT_MAX;
		float bestOverlapY = FLT_MAX;
		float bestDirX = 0.0f;
		float bestDirY = 0.0f;

		bool hasHit = false;

		for (auto& t : tiles) {
			if (!t.isWall) continue;

			const AABB& tile = t.aabb;

			float overlapX =
				std::min(predicted.max.x, tile.max.x) -
				std::max(predicted.min.x, tile.min.x);

			float overlapY =
				std::min(predicted.max.y, tile.max.y) -
				std::max(predicted.min.y, tile.min.y);

			if (overlapX <= 0.0f || overlapY <= 0.0f) continue;

			hasHit = true;

			// X 軸候選
			if (overlapX < bestOverlapX) {
				bestOverlapX = overlapX;

				// 用 move.x 判斷方向（比 velocity 更安全）
				if (move.x > 0.0f) bestDirX = -1.0f;
				else if (move.x < 0.0f) bestDirX = 1.0f;
				else {
					float cx = (current.min.x + current.max.x) * 0.5f;
					float tx = (tile.min.x + tile.max.x) * 0.5f;
					bestDirX = (cx < tx ? -1.0f : 1.0f);
				}
			}

			// Y 軸候選
			if (overlapY < bestOverlapY) {
				bestOverlapY = overlapY;

				if (move.y > 0.0f) bestDirY = -1.0f; // 撞頭 → 往下推
				else if (move.y < 0.0f) bestDirY = 1.0f; // 落地 → 往上推
				else {
					float cy = (current.min.y + current.max.y) * 0.5f;
					float ty = (tile.min.y + tile.max.y) * 0.5f;
					bestDirY = (cy < ty ? -1.0f : 1.0f);
				}
			}
		}

		if (!hasHit) break;

		Vector3 push{};
		//xIsMainAxis = false;
		//yIsMainAxis = false;

		// ★★★ overlap 決定主軸（舊版正確邏輯）★★★
		if (bestOverlapX < bestOverlapY) {
			push.x = bestDirX * (bestOverlapX + EPS);
			xIsMainAxis = true;
		} else {
			push.y = bestDirY * (bestOverlapY + EPS);
			yIsMainAxis = true;

			if (bestDirY > 0.0f) landed = true; // 往下推 → 落地
		}

		// ★★★ 逐 tile 累積推回（舊版最重要的行為）★★★
		finalMove += push;
		current.min += push;
		current.max += push;
	}

	CollisionResult result;
	result.correction = finalMove;
	result.xIsMainAxis = xIsMainAxis;
	result.yIsMainAxis = yIsMainAxis;
	result.landed = landed;
	return result;
}




CollisionResult MapChipField::GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime) {
	bool dummyLanded;
	return GetMapCollisionCorrection(box, velocity, deltaTime, dummyLanded);
}

std::vector<MapChipField::TileInfo> MapChipField::GetTilesOverlapping(const AABB& box) {

	std::vector<TileInfo> result;

	float eps = 0.001f;
	WorldIndex minIdx = GetWorldIndexByPosition({ box.min.x, box.min.y, box.min.z });
	WorldIndex maxIdx = GetWorldIndexByPosition({ box.max.x - eps, box.max.y - eps, box.max.z });

	// 2. clamp 避免越界
	minIdx.x = std::clamp(minIdx.x, 0, (int)GetNumBlockHorizontal() - 1);
	maxIdx.x = std::clamp(maxIdx.x, 0, (int)GetNumBlockHorizontal() - 1);
	minIdx.y = std::clamp(minIdx.y, 0, (int)GetNumBlockVirtical() - 1);
	maxIdx.y = std::clamp(maxIdx.y, 0, (int)GetNumBlockVirtical() - 1);

	int xBegin = std::min(minIdx.x, maxIdx.x);
	int xEnd = std::max(minIdx.x, maxIdx.x);
	int yBegin = std::min(minIdx.y, maxIdx.y);
	int yEnd = std::max(minIdx.y, maxIdx.y);

	// 3. 掃描所有 tile
	for (int y = yBegin; y <= yEnd; ++y) {
		for (int x = xBegin; x <= xEnd; ++x) {

			MapChipType type = GetMapChipTypeByWorld({ x, y });
			bool isWall = (type == MapChipType::kDirt || type == MapChipType::kRock);

			TileInfo info;
			info.isWall = isWall;
			info.aabb = GetAABBByWorldIndex({ x, y });

			result.push_back(info);
		}
	}

	return result;

}

AABB MapChipField::GetAABBByMapIndex(MapIndex mapIndex) {
	Vector3 center = GetWorldPosFromMapByMapIndex(mapIndex);

	float halfW = kBlockWidth * 0.5f;
	float halfH = kBlockHeight * 0.5f;

	AABB aabb;
	aabb.min = center - Vector3{ halfW, halfH, 0 };
	aabb.max = center + Vector3{ halfW, halfH, 0 };
	return aabb;
}

AABB MapChipField::GetAABBByWorldIndex(WorldIndex worldIndex) {
	return GetAABBByMapIndex(ExchangeWorld2MapIndex(worldIndex));
}


void MapChipField::SetBlockWidth(float width) {
	kBlockWidth = width;
}

void MapChipField::SetBlockHeight(float height) {
	kBlockHeight = height;
}

MapChipField::WorldIndex MapChipField::ExchangeMap2WorldIndex(MapIndex mapSet) {
	WorldIndex i;
	i.x = mapSet.x;
	i.y = (kNumBlockVirtical - 1 - mapSet.y); // 上下翻轉
	return i;
}

MapChipField::MapIndex MapChipField::ExchangeWorld2MapIndex(WorldIndex worldIndex) {
	MapIndex m;
	m.x = worldIndex.x;
	m.y = (kNumBlockVirtical - 1 - worldIndex.y); // 上下翻轉
	return m;
}

// void GenerateBlocks(std::vector<std::vector<WorldTransform*>>& worldTransformBlocks) {
//	/// ボックス生成
//	// 要素数
//	const uint32_t kNumBlockVertical = 10;
//	const uint32_t kNumBlockHorizontal = 20;
//	// ブロック1個分の横幅
//	const float kBlockWidth = 2.0f;
//	const float kBlockHeight = 2.0f;
//	// 要素数を変更する
//	// 列数を設定(縦方向のブロック数)
//	worldTransformBlocks.resize(kNumBlockVertical);
//	for (int i = 0; i < kNumBlockVertical; i++) {
//		// 列数を設定(横方向のブロック数)
//		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
//	}
//	// いざボックス生成
//	for (int i = 0; i < kNumBlockVertical; i++) {
//		for (int j = 0; j < kNumBlockHorizontal; j++) {
//			worldTransformBlocks_[i][j] = new WorldTransform();
//			worldTransformBlocks_[i][j]->Initialize();
//			worldTransformBlocks_[i][j]->translation_.x = kBlockWidth * j;
//			worldTransformBlocks_[i][j]->translation_.y = kBlockHeight * i;
//			if (i % 2 == 0) {
//				if (j % 2 == 1) {
//					worldTransformBlocks_[i][j] = nullptr;
//				}
//			} else {
//				if (j % 2 == 0) {
//					worldTransformBlocks_[i][j] = nullptr;
//				}
//			}
//		}
//	}
// }