#pragma once
#include <vector>
#include "MathsIncluder.h"
#include "Data/Geometry/Shape/Quad/AABB.h"
#include "Tool/MapBulider/Data/MapDatas.h"
#include "CollisionResult.h"
#include <string>

class MapChipField {

public:
	/// マップの計算用の構造体(y軸は上が正、エンジンと一致する)
	struct WorldIndex { int x; int y; };
	/// マップのデータを指定するための構造体（y = 0 が一番上）
	struct MapIndex { int x; int y; };
	struct TileInfo { bool isWall; AABB aabb; };

public:
	/// ================= 準備 ================= ///
	/// MapReader で読んだ grid と MapData を渡す（MapData は参照だけ持つ → こちらより先に消さないこと）
	void Setup(const MapGrid& grid, const MapData* data);
	/// 今の大きさのまま全部空白に
	void ResetMapChipData();

	/// ================= マスの番号 ================= ///
	int GetIdByMap(MapIndex mapIndex) const;
	int GetIdByWorld(WorldIndex worldIndex) const;
	void SetIdByMap(MapIndex mapIndex, int id);
	void SetIdByWorld(WorldIndex worldIndex, int id);
	/// そのマスの物件（空白 / MapData に無い番号なら nullptr）
	const MapUnit* GetUnitByMap(MapIndex mapIndex) const;
	/// 壁か（今は「isDirectTile = 壁」。後で isSolid に変える）
	bool IsWallByMap(MapIndex mapIndex) const;
	bool IsWallByWorld(WorldIndex worldIndex) const;

	/// ================= 座標 ================= ///
	Vector3 GetWorldPosFromMapByMapIndex(MapIndex mapIndex) const;
	Vector3 GetWorldPosFromMapByWorldIndex(WorldIndex worldIndex) const;
	WorldIndex GetWorldIndexByPosition(const Vector3& position) const;
	MapIndex GetMapIndexByPosition(const Vector3& position) const;

	/// ================= 当たり判定 ================= ///
	CollisionResult GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime, bool& landed);
	CollisionResult GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime);
	std::vector<TileInfo> GetTilesOverlapping(const AABB& box);
	AABB GetAABBByMapIndex(MapIndex mapIndex) const;
	AABB GetAABBByWorldIndex(WorldIndex worldIndex) const;

	/// ================= 大きさ ================= ///
	Vector2 GetBlockSize() const { return { kBlockWidth, kBlockHeight }; }
	int GetNumBlockVirtical() const { return kNumBlockVirtical; }
	int GetNumBlockHorizontal() const { return kNumBlockHorizontal; }

	const MapData* GetMapData() const { return mapData_; }
	int GetBlankId() const { return mapData_ ? mapData_->blankId : 0; }

	/// ================= 変換 ================= ///
	WorldIndex ExchangeMap2WorldIndex(MapIndex mapSet) const;
	MapIndex ExchangeWorld2MapIndex(WorldIndex worldIndex) const;

private:
	bool InRange(MapIndex m) const {
		return m.x >= 0 && m.x < kNumBlockHorizontal && m.y >= 0 && m.y < kNumBlockVirtical;
	}

	// 1ブロックのサイズ（★static をやめた → マップごとに別）
	float kBlockWidth = 1.0f;
	float kBlockHeight = 1.0f;
	// ブロックの個数
	int kNumBlockVirtical = 0;
	int kNumBlockHorizontal = 0;

	std::vector<std::vector<int>> ids_;     // [y][x]、y = 0 が一番上
	const MapData* mapData_ = nullptr;      // 借り
};