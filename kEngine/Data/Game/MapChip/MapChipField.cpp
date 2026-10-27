#include "MapChipField.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <algorithm>

void MapChipField::Setup(const MapGrid& grid, const MapData* data) {
	mapData_ = data;
	ids_ = grid.ids;
	kNumBlockVirtical = grid.Height();
	kNumBlockHorizontal = grid.Width();
	if (data) {
		kBlockWidth = data->cellSpacing.x;
		kBlockHeight = data->cellSpacing.y;
	}
}

void MapChipField::ResetMapChipData() {
	ids_.assign(kNumBlockVirtical, std::vector<int>(kNumBlockHorizontal, GetBlankId()));
}

int MapChipField::GetIdByMap(MapIndex m) const {
	return InRange(m) ? ids_[m.y][m.x] : GetBlankId();     // 範囲外は空白
}
int MapChipField::GetIdByWorld(WorldIndex w) const {
	return GetIdByMap(ExchangeWorld2MapIndex(w));
}

void MapChipField::SetIdByMap(MapIndex m, int id) {
	if (InRange(m)) ids_[m.y][m.x] = id;
}
void MapChipField::SetIdByWorld(WorldIndex w, int id) {
	SetIdByMap(ExchangeWorld2MapIndex(w), id);
}

const MapUnit* MapChipField::GetUnitByMap(MapIndex m) const {
	if (!mapData_) return nullptr;
	const int id = GetIdByMap(m);
	return (id == mapData_->blankId) ? nullptr : mapData_->FindUnit(id);
}

bool MapChipField::IsWallByMap(MapIndex m) const {
	if (!InRange(m)) return false;
	const MapUnit* u = GetUnitByMap(m);
	return u && u->isDirectTile;
}

bool MapChipField::IsWallByWorld(WorldIndex w) const {
	return IsWallByMap(ExchangeWorld2MapIndex(w));
}

/// マップチップ座標を取得
Vector3 MapChipField::GetWorldPosFromMapByMapIndex(MapIndex mapIndex) const {
	Vector3 leftBottom{ kBlockWidth * mapIndex.x,
						kBlockHeight * (kNumBlockVirtical - 1 - mapIndex.y),
						0 };

	Vector3 center{ leftBottom.x + kBlockWidth * 0.5f,
						leftBottom.y + kBlockHeight * 0.5f,
						0 };

	return center;
}

Vector3 MapChipField::GetWorldPosFromMapByWorldIndex(WorldIndex worldIndex) const {
	MapIndex mapIndex = ExchangeWorld2MapIndex(worldIndex);
	return GetWorldPosFromMapByMapIndex(mapIndex);
}

/// 座標からマップチップの番号を計算
MapChipField::WorldIndex MapChipField::GetWorldIndexByPosition(const Vector3& position) const {
	WorldIndex worldIndex = {};
	worldIndex.x = static_cast<int>(std::floor(position.x / kBlockWidth));
	worldIndex.y = static_cast<int>(std::floor(position.y / kBlockHeight));
	return worldIndex;
}

MapChipField::MapIndex MapChipField::GetMapIndexByPosition(const Vector3& position) const {
	WorldIndex worldIndex = GetWorldIndexByPosition(position);
	return ExchangeWorld2MapIndex(worldIndex);
}



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
	bool ceilingHit = false;

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
				(std::min)(predicted.max.x, tile.max.x) -
				(std::max)(predicted.min.x, tile.min.x);

			float overlapY =
				(std::min)(predicted.max.y, tile.max.y) -
				(std::max)(predicted.min.y, tile.min.y);

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

			if (bestDirY > 0.0f) landed = true;        // 上に押し戻す → 床
			else                 ceilingHit = true;    // ★下に押し戻す → 天井
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
	result.ceilingHit = ceilingHit;
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

	int xBegin = (std::min)(minIdx.x, maxIdx.x);
	int xEnd = (std::max)(minIdx.x, maxIdx.x);
	int yBegin = (std::min)(minIdx.y, maxIdx.y);
	int yEnd = (std::max)(minIdx.y, maxIdx.y);

	// 3. 掃描所有 tile
	for (int y = yBegin; y <= yEnd; ++y) {
		for (int x = xBegin; x <= xEnd; ++x) {

			TileInfo info;
			info.isWall = IsWallByWorld({ x, y });
			info.aabb = GetAABBByWorldIndex({ x, y });

			result.push_back(info);
		}
	}

	return result;

}

AABB MapChipField::GetAABBByMapIndex(MapIndex mapIndex) const {
	Vector3 center = GetWorldPosFromMapByMapIndex(mapIndex);

	float halfW = kBlockWidth * 0.5f;
	float halfH = kBlockHeight * 0.5f;

	AABB aabb;
	aabb.min = center - Vector3{ halfW, halfH, 0 };
	aabb.max = center + Vector3{ halfW, halfH, 0 };
	return aabb;
}

AABB MapChipField::GetAABBByWorldIndex(WorldIndex worldIndex) const {
	return GetAABBByMapIndex(ExchangeWorld2MapIndex(worldIndex));
}


MapChipField::WorldIndex MapChipField::ExchangeMap2WorldIndex(MapIndex mapSet) const {
	WorldIndex i;
	i.x = mapSet.x;
	i.y = (kNumBlockVirtical - 1 - mapSet.y); // 上下翻轉
	return i;
}

MapChipField::MapIndex MapChipField::ExchangeWorld2MapIndex(WorldIndex worldIndex) const {
	MapIndex m;
	m.x = worldIndex.x;
	m.y = (kNumBlockVirtical - 1 - worldIndex.y); // 上下翻轉
	return m;
}