#pragma once
#include <vector>
#include <string>
#include "Object\Object.h"
#include "config.h"

/// Mapに置く1種類の物の定義
struct MapUnit {
	int id = 1;                                             // csv に書かれる番号
	std::string name = "NewUnit";                           // 生成時に名前で見分ける（"Player" など）
	bool isDirectTile = true;                               // true : 置くだけ（石・土）/ false : ロジック物（生成は登録した関数で）
	int modelHandle = -1;
	Vector4 previewColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	EngineModelType engineModel = EngineModelType::None;
	EngineModelParams engineParams{};                       // engineModel が None 以外の時だけ意味がある

	Transform transform = CreateDefaultTransform();
	MaterialConfig material = InitMaterialConfig();
};

/// Mapの大定義
struct MapData {
	std::string dataName{};
	int xSize{};
	int ySize{};
	int startX{};
	int startY{};
	int blankId = 0;                                        // 空白マスの番号
	Vector2 cellSpacing = { 1.0f, 1.0f };                   // マスの間隔（x = 横 / y = 縦）
	std::vector<MapUnit> units{};

	bool isSaved = false;
	std::string savePath;

	/// 検索（見つからなければ nullptr）
	const MapUnit* FindUnit(const std::string& name) const;
	const MapUnit* FindUnit(int id) const;
	MapUnit* FindUnit(const std::string& name);
	MapUnit* FindUnit(int id);
	/// 次に使える番号（空白とも被らない）
	int NextUnitId() const;
	/// id を newId に。newId を使っている物（空白も含む）があれば入れ替え
	void ChangeId(int oldId, int newId);
};

/// Mapの資料(csvから読み込みの資料)
struct MapGrid {
	std::string name;                                       // ファイル名（拡張子なし）
	std::string mapDataName;                                // csv 最後の「#mapData,名前」（無ければ空）
	std::vector<std::vector<int>> ids;                      // [y][x]、y = 0 が一番上の行

	int Width() const { return ids.empty() ? 0 : (int)ids[0].size(); }
	int Height() const { return (int)ids.size(); }
};