#pragma once
#include <vector>
#include <memory>
#include <string>
#include "MathsIncluder.h"
#include "Data/Geometry/Shape/Quad/AABB.h"
#include "Tool/MapBulider/Data/MapDatas.h"   // Object も入っている
#include "CollisionResult.h"

struct MapIndex { 
	int x; 
	int y; 
};
struct TileInfo { 
	bool isWall; 
	AABB aabb; 
};

/// ============================================================
/// MapChipField：1 枚分のマスのデータ（ID・座標・当たり判定）
///  - マスの番号は MapIndex だけ（y = 0 が csv の一番上）
///  - ワールド：一番左下のマスの左下が (0,0)、y は上が正
/// ============================================================
class MapChipField {
public:

	/// ================= 準備 ================= ///
	/// MapData は参照だけ持つ → こちらより先に消さないこと
	void Setup(const MapGrid& grid, const MapData* data);
	/// 今の大きさのまま全部空白に
	void ResetMapChipData();

	/// ================= マス ================= ///
	bool InRange(MapIndex m) const {
		return m.x >= 0 && m.x < numBlockHorizontal_ && m.y >= 0 && m.y < numBlockVirtical_;
	}
	int GetIdByMap(MapIndex m) const;
	void SetIdByMap(MapIndex m, int id);
	/// そのマスの物件（空白 / MapData に無い番号なら nullptr）
	const MapUnit* GetUnitByMap(MapIndex m) const;
	/// 壁か（今は「isDirectTile = 壁」。後で isSolid に変える）
	bool IsWallByMap(MapIndex m) const;

	/// ================= 座標 ================= ///
	/// ワールド座標 → マス（範囲外もそのまま返す → InRange で確認）
	MapIndex GetMapIndexByPosition(const Vector3& position) const;
	/// マスの中心
	Vector3 GetWorldPosFromMapByMapIndex(MapIndex m) const;
	AABB GetAABBByMapIndex(MapIndex m) const;

	/// ================= 当たり判定 ================= ///
	CollisionResult GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime) const;

	/// ================= 大きさ ================= ///
	Vector2 GetBlockSize() const { return { blockWidth_, blockHeight_ }; }
	int GetNumBlockVirtical() const { return numBlockVirtical_; }
	int GetNumBlockHorizontal() const { return numBlockHorizontal_; }
	/// 地図全体の大きさ（ワールド、(0,0) から）
	Vector2 GetWorldSize() const { return { blockWidth_ * numBlockHorizontal_, blockHeight_ * numBlockVirtical_ }; }

	const MapData* GetMapData() const { return mapData_; }
	int GetBlankId() const { return mapData_ ? mapData_->blankId : 0; }

private:
	std::vector<TileInfo> GetTilesOverlapping(const AABB& box) const;

	float blockWidth_ = 1.0f;
	float blockHeight_ = 1.0f;
	int numBlockVirtical_ = 0;
	int numBlockHorizontal_ = 0;

	std::vector<std::vector<int>> ids_;     // [y][x]、y = 0 が一番上
	const MapData* mapData_ = nullptr;      // 借り
};


/// ============================================================
/// MapLayer：Field（データ）＋ ブロックの見た目
///  - 壊す・置く時は ID と見た目を必ず一緒に変える
///  - Object を作るのは Manager（MapReader が要るので）
/// ============================================================

struct MapLayerObject {
	std::vector<std::vector<std::unique_ptr<Object>>> data_;
};

class MapLayer {
public:
	MapLayer(const MapGrid& grid, const MapData* data);

	/// ================= ブロック ================= ///
	/// ID と見た目を置く（生成・設置で使う）
	void SetBlock(MapIndex m, int id, std::unique_ptr<Object> obj);
	/// 壊す（ブロックが無ければ false）
	bool Break(MapIndex m);
	bool HasBlock(MapIndex m) const { return field_.InRange(m) && blocks_.data_[m.y][m.x] != nullptr; }

	void UpdateBlocks();
	void DrawBlocks(const Vector3& center, float drawRadius);

	/// ================= 取得 ================= ///
	MapChipField& GetField() { return field_; }
	const MapChipField& GetField() const { return field_; }
	const std::string& GetName() const { return name_; }

	bool collision = true;   // 当たり判定に使うか
	bool visible = true;     // 描画するか

private:
	std::string name_;                                           // csv のファイル名
	MapChipField field_;
	MapLayerObject blocks_;   // [y][x]
};