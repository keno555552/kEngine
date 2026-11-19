#include "MapChipField.h"
#include "Math/Geometry/Collision/crashDecision.h"
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <algorithm>

/// ================================================ MapChipField ================================================ ///

void MapChipField::Setup(const MapGrid& grid, const MapData* data) {
	mapData_ = data;
	ids_ = grid.ids;
	numBlockVirtical_ = grid.Height();
	numBlockHorizontal_ = grid.Width();
	if (data) {
		blockWidth_ = data->cellSpacing.x;
		blockHeight_ = data->cellSpacing.y;
	}
}

void MapChipField::ResetMapChipData() {
	ids_.assign(numBlockVirtical_, std::vector<int>(numBlockHorizontal_, GetBlankId()));
}

int MapChipField::GetIdByMap(MapIndex m) const {
	return InRange(m) ? ids_[m.y][m.x] : GetBlankId();     // 範囲外は空白
}

void MapChipField::SetIdByMap(MapIndex m, int id) {
	if (InRange(m)) ids_[m.y][m.x] = id;
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

/// マップチップ座標を取得
Vector3 MapChipField::GetWorldPosFromMapByMapIndex(MapIndex m) const {
	return {
		blockWidth_ * (m.x + 0.5f),
		blockHeight_ * (numBlockVirtical_ - 1 - m.y + 0.5f),
		0.0f
	};
}

MapIndex MapChipField::GetMapIndexByPosition(const Vector3& position) const {
	const int worldY = static_cast<int>(std::floor(position.y / blockHeight_));   // 下から数えた段
	return {
		static_cast<int>(std::floor(position.x / blockWidth_)),
		numBlockVirtical_ - 1 - worldY                                          // 上から数えた行
	};
}

CollisionResult MapChipField::GetMapCollisionCorrection(
	const AABB& box, const Vector3& velocity, float deltaTime) const {

	bool landed = false;
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

std::vector<TileInfo> MapChipField::GetTilesOverlapping(const AABB& box) const {

	std::vector<TileInfo> result;
	if (numBlockHorizontal_ <= 0 || numBlockVirtical_ <= 0) return result;

	const float eps = 0.001f;
	const MapIndex a = GetMapIndexByPosition(box.min);
	const MapIndex b = GetMapIndexByPosition({ box.max.x - eps, box.max.y - eps, box.max.z });

	/// y は上下反転しているので min / max で並べ直してから clamp
	const int xBegin = std::clamp((std::min)(a.x, b.x), 0, numBlockHorizontal_ - 1);
	const int xEnd = std::clamp((std::max)(a.x, b.x), 0, numBlockHorizontal_ - 1);
	const int yBegin = std::clamp((std::min)(a.y, b.y), 0, numBlockVirtical_ - 1);
	const int yEnd = std::clamp((std::max)(a.y, b.y), 0, numBlockVirtical_ - 1);

	for (int y = yBegin; y <= yEnd; ++y) {
		for (int x = xBegin; x <= xEnd; ++x) {
			result.push_back({ IsWallByMap({ x, y }), GetAABBByMapIndex({ x, y }) });
		}
	}
	return result;

}

AABB MapChipField::GetAABBByMapIndex(MapIndex m) const {
	const Vector3 center = GetWorldPosFromMapByMapIndex(m);
	const Vector3 half{ blockWidth_ * 0.5f, blockHeight_ * 0.5f, 0.0f };
	return { center - half, center + half };
}

/// ================================================== MapLayer ================================================== ///

MapLayer::MapLayer(const MapGrid& grid, const MapData* data) : name_(grid.name) {
	field_.Setup(grid, data);
	blocks_.data_.resize(field_.GetNumBlockVirtical());
	for (auto& row : blocks_.data_) row.resize(field_.GetNumBlockHorizontal());
}

void MapLayer::SetBlock(MapIndex m, int id, std::unique_ptr<Object> obj) {
	if (!field_.InRange(m)) return;
	field_.SetIdByMap(m, id);              // ID → 当たり判定
	blocks_.data_[m.y][m.x] = std::move(obj);    // 見た目
}

bool MapLayer::Break(MapIndex m) {
	if (!HasBlock(m)) return false;
	field_.SetIdByMap(m, field_.GetBlankId());
	blocks_.data_[m.y][m.x].reset();
	return true;
}

void MapLayer::UpdateBlocks() {
	for (auto& row : blocks_.data_)
		for (auto& block : row)
			if (block) block->Update(nullptr);
}

void MapLayer::DrawBlocks(const Vector3& center, float drawRadius) {
	if (!visible) return;
	const float radiusSq = drawRadius * drawRadius;
	for (auto& row : blocks_.data_) {
		for (auto& block : row) {
			if (!block) continue;
			const Vector3 d = block->mainPosition.transform.translate - center;
			if (d.x * d.x + d.y * d.y + d.z * d.z <= radiusSq) block->Draw();
		}
	}
}