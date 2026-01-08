#include "MapChipField.h"
#include <fstream>
#include <map>
#include <sstream>
#include <string>

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
	uint32_t newVertical = static_cast<uint32_t>(lines.size());
	uint32_t newHorizontal = 0;
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
	for (uint32_t i = 0; i < kNumBlockVirtical && i < lines.size(); ++i) {
		std::istringstream line_stream(lines[i]);
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			std::string word;
			getline(line_stream, word, ',');
			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {
	if ((xIndex < 0) || (kNumBlockHorizontal - 1 < xIndex)) {
		return MapChipType::kBlank;
	}
	if ((yIndex < 0) || (kNumBlockVirtical - 1 < yIndex)) {
		return MapChipType::kBlank;
	}
	return mapChipData_.data[yIndex][xIndex];
}

void MapChipField::SetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex, MapChipType type) {
	if ((xIndex < 0) || (kNumBlockHorizontal - 1 < xIndex)) {
		return;
	}
	if ((yIndex < 0) || (kNumBlockVirtical - 1 < yIndex)) {
		return;
	}
	mapChipData_.data[yIndex][xIndex] = type;
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) { return Vector3(kBlockWidth * xIndex, kBlockHeight * (kNumBlockVirtical - 1 - yIndex), 0); }

MapChipField::IndexSet MapChipField::GetMapChipIndexByPosition(const Vector3& position) {
	IndexSet indexSet = {};
	indexSet.xIndex = static_cast<uint32_t>((position.x + kBlockWidth / 2) / kBlockWidth);
	//float posY = (position.y - kBlockHeight / 2.0f);
	//float indexY = (posY / kBlockHeight);
	//indexSet.yIndex = static_cast<uint32_t>(kNumBlockVirtical - 1 - indexY);
	indexSet.yIndex = static_cast<uint32_t>(kNumBlockVirtical - 1 - ((position.y - kBlockHeight / 2) / kBlockHeight));
	return indexSet;
}

MapChipField::Rect MapChipField::GetRectByIndex(int xIndex, int yIndex) { 
	Vector3 center = GetMapChipPositionByIndex(xIndex, yIndex);

	Rect rect;
	rect.left = center.x - (kBlockWidth / 2.0f);
	rect.right = center.x + (kBlockWidth / 2.0f);
	rect.top = center.y + (kBlockHeight / 2.0f);
	rect.bottom = center.y - (kBlockHeight / 2.0f);
	return rect;
}


void MapChipField::SetBlockWidth(float width) {
	kBlockWidth = width;
}

void MapChipField::SetBlockHeight(float height) {
	kBlockHeight = height;
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