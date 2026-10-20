#pragma once
#include <functional>
#include <unordered_map>
#include <map>
#include <string>
#include "externals/nlohmann/json.hpp"
#include "../Data/MapDatas.h"
#include "Data/Game/MapChip/MapChipField.h"

class kEngine;
class Object;

/// ============================================================
/// MapMakingEditor が書いたファイルを読むだけのクラス
///  - 読んだ結果を返すだけで、マップの今の状態は持たない（それは MapChipField）
///  - 登録器：名前 / ID → 生成関数
///  - 適配器：マスを登録した関数に渡す / MapUnit → Object
/// ============================================================
class MapReader {
public:
	/// 1マス分の生成情報（登録した関数に渡される）
	struct SpawnInfo {
		int id = 0;
		const MapUnit* unit = nullptr;              // MapData に無い番号なら nullptr
		MapChipField::MapIndex index{};              // csv 上の位置（y = 0 が一番上）
		Vector3 position{};                          // マスの中心（ワールド）
	};
	using SpawnFunc = std::function<void(const SpawnInfo&)>;

	struct SpawnReport {
		int spawned = 0;                             // 関数で処理したマス
		int unhandled = 0;                           // 誰も処理しなかったマス（空白は数えない）
	};

	explicit MapReader(kEngine* system) : system_(system) {}

	/// ================= 読み込み ================= ///
	/// まとめて：csv → MapData。mapDataPath が空なら csv の「#mapData」の名前で探す
	bool LoadMap(const std::string& csvPath, const std::string& mapDataPath, MapPackage& out);
	/// MapData の json だけ（モデル・texture もここでエンジンに読み込む）
	bool ReadMapData(const std::string& jsonPath, MapData& out);
	/// csv だけ（エンジンを使わないので static）
	static bool ReadMapGrid(const std::string& csvPath, MapGrid& out, int fillId = 0);

	/// ================= 登録器 ================= ///
	void Register(const std::string& name, SpawnFunc func);
	void Register(int id, SpawnFunc func);
	/// どれにも当たらなかった時（ブロック等）
	void SetDefault(SpawnFunc func) { defaultFunc_ = std::move(func); }
	void ClearRegistry();

	/// ================= 適配器 ================= ///
	/// 全マスを走査して、ID → 名前 → Default の順で登録した関数を呼ぶ
	SpawnReport Spawn(MapChipField& field) const;
	/// MapUnit の見た目で Object を作る（new したものを返す。delete は呼んだ側）
	Object* BuildObject(const MapUnit& unit, const Vector3& position) const;

	/// 警告・エラーの記録
	const std::string& GetLog() const { return log_; }
	void ClearLog() { log_.clear(); }

private:
	MapUnit JsonToUnit(const nlohmann::json& j, const std::string& baseDir);
	int GetEngineModelHandle(EngineModelType type, const EngineModelParams& p);

	kEngine* system_ = nullptr;                                 // 借り
	std::unordered_map<std::string, SpawnFunc> nameRegistry_;
	std::unordered_map<int, SpawnFunc> idRegistry_;
	SpawnFunc defaultFunc_;
	std::map<std::string, int> engineModelCache_;               // 同じ数値の内蔵モデルを二重に作らない
	std::string log_;
};