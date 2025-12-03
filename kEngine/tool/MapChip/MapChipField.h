#pragma once
#include <vector>
#include "MathsIncluder.h"
#include <string>
#include "MapChipData.h"
#include "../../GameObject/Object/Corner.h"

struct MapChipData {
	std::vector<std::vector<MapChipType>> data;
};

class MapChipField {
public:
	struct IndexSet {
		int xIndex;
		int yIndex;
	};


public:

	/// リセット
	void ResetMapChipData();

	/// 読み込み
	void LoadMapChipCsv(const std::string& filePath);

	void SetBlockSize(Vector2 size);

	/// マップチップ種類を取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	/// マップチップ座標を取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	/// 座標からマップチップの番号を計算
	IndexSet GetMapChipIndexByPosition(const Vector3& position);

	/// ブロックの範囲取得
	Rect GetRectByIndex(int xIndex, int yIndex);

	uint32_t GetNumBlockVirtical() { return kNumBlockVirtical; }
	uint32_t GetNumBlockHorizontal() { return kNumBlockHorizontal; }

private:
	// 1ブロックのサイズ
	float kBlockWidth = 2.0f;
	float kBlockHeight = 2.0f;
	// ブロックの個数
	uint32_t kNumBlockVirtical = 35;
	uint32_t kNumBlockHorizontal = 21;

	MapChipData mapChipData_;
};

// void GenerateBlocks(std::vector<std::vector<WorldTransform*>>& worldTransformBlocks);