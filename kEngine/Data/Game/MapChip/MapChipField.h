#pragma once
#include <vector>
#include "MathsIncluder.h"
#include "Data/Geometry/Shape/Quad/AABB.h"
#include <string>

enum class MapChipType {
	kBlank, // 空白
	kDirt, // ブロック
	kRock, // 岩
	kEnemy = 7,
	kPlayer = 8,
	kBackPoint = 9,
	NumOfTypes,
};

static std::vector<MapChipType> MapBlockType {
	MapChipType::kDirt,
	MapChipType::kRock,
};

static std::vector<MapChipType> MapEnemyType {
	MapChipType::kEnemy,
};

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	/// マップの計算用の構造体(y軸は上が正、エンジンと一致する)
	struct WorldIndex {
		int x;
		int y;
	};

	/// マップのデータを指定するための構造体
	struct MapIndex {
		int x;
		int y;
	};

	struct TileInfo {
		bool isWall;
		AABB aabb;
	};

public:

	/// リセット
	void ResetMapChipData();

	/// 読み込み
	void LoadMapChipCsv(const std::string& filePath);

	/// マップチップ種類を取得
	MapChipType GetMapChipTypeByMap(MapIndex mapIndex);
	MapChipType GetMapChipTypeByWorld(WorldIndex worldIndex);

	/// マップチップ種類を設定
	void SetMapChipTypeByMap(MapIndex mapIndex, MapChipType type);
	void SetMapChipTypeByWorld(WorldIndex worldIndex, MapChipType type);

	/// マップチップ座標を取得
	Vector3 GetWorldPosFromMapByMapIndex(MapIndex mapIndex);
	Vector3 GetWorldPosFromMapByWorldIndex(WorldIndex worldIndex);

	/// 座標からマップチップの番号を計算
	WorldIndex GetWorldIndexByPosition(const Vector3& position);
	MapIndex GetMapIndexByPosition(const Vector3& position);

	/// 当たり判定の修正量を取得
	Vector3 GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime, bool& landed);
	Vector3 GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime);

	/// 当たっているタイルの位置情報を取得
	std::vector<TileInfo> GetTilesOverlapping(const AABB& box);

	/// ブロックの範囲取得
	AABB GetAABBByMapIndex(MapIndex mapIndex);
	AABB GetAABBByWorldIndex(WorldIndex worldIndex);

	/// ブロックサイズ関連
	void SetBlockWidth(float width);
	void SetBlockHeight(float height);
	Vector2 GetBlockSize() { return {kBlockWidth, kBlockHeight}; }

	///　ブロック数取得
	int GetNumBlockVirtical() { return kNumBlockVirtical; }
	int GetNumBlockHorizontal() { return kNumBlockHorizontal; }

	/// マップチップデータ取得
	MapChipData& GetMapChipData() { return mapChipData_; }

	/// マップリーダー
	WorldIndex ExchangeMap2WorldIndex(MapIndex mapSet);
	MapIndex ExchangeWorld2MapIndex(WorldIndex worldIndex);

private:
	// 1ブロックのサイズ
	static inline float kBlockWidth = 2.0f;
	static inline float kBlockHeight = 2.0f;
	// ブロックの個数
	static inline int kNumBlockVirtical = 35;
	static inline int kNumBlockHorizontal = 21;

	MapChipData mapChipData_;
};

// void GenerateBlocks(std::vector<std::vector<WorldTransform*>>& worldTransformBlocks);