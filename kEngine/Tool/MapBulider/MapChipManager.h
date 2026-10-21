#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <optional>
#include "Data/Game/MapChip/MapChipField.h"
#include "Tool/MapBulider/Reader/MapReader.h"

class kEngine;
class Object;

/// 全マップ（レイヤー）の窓口。MapChipField / MapReader は外に出さない
///  - 読み込み：MapData（共有キャッシュ）＋ csv → レイヤー
///  - 生成　　：登録された関数を呼ぶだけ（物は呼ばれた側が持つ）
///  - 読み書き：マス単位の ID（破壊・設置は MapBlockData から）
///  - 問い合わせ：ワールド座標だけで答える（当たり判定用）
class MapChipManager {
public:
	explicit MapChipManager(kEngine* system);

	/// ================= 読み込み ================= ///
	/// csv を 1 枚読んでレイヤーに追加（MapData が見つからなければ失敗）
	bool AddLayer(const std::string& layerName, const std::string& csvPath, const std::string& mapDataPath = "");
	/// 空のレイヤー（エディタ用）
	bool CreateLayer(const std::string& layerName, const std::string& mapDataPath, int width, int height);
	void RemoveLayer(const std::string& layerName);
	bool HasLayer(const std::string& layerName) const { return Find(layerName) != nullptr; }
	void Clear();

	/// ================= 生成の登録（シーン側のルール） ================= ///
	void Register(const std::string& unitName, MapReader::SpawnFunc func) { reader_->Register(unitName, std::move(func)); }
	void Register(int id, MapReader::SpawnFunc func) { reader_->Register(id, std::move(func)); }
	/// 登録に無い物（ブロック等）
	void SetDefault(MapReader::SpawnFunc func) { reader_->SetDefault(std::move(func)); }
	void ClearRegistry() { reader_->ClearRegistry(); }

	/// ================= 生成 ================= ///
	/// 登録に従って全マスの生成関数を呼ぶ（物は呼ばれた側が持つ）
	MapReader::SpawnReport Generate(const std::string& layerName);
	void GenerateAll();
	/// MapUnit の見た目で Object を作る（new したものを返す。delete は呼んだ側）
	Object* BuildObject(const MapUnit& unit, const Vector3& position) const { return reader_->BuildObject(unit, position); }

	/// ================= マス単位の読み書き（破壊・設置用） ================= ///
	using MapIndex = MapChipField::MapIndex;
	/// ワールド座標 → マス（範囲外なら nullopt）
	std::optional<MapIndex> GetIndexAt(const std::string& layerName, const Vector3& position) const;
	/// マスの中心のワールド座標
	Vector3 GetCellCenter(const std::string& layerName, MapIndex index) const;
	int GetId(const std::string& layerName, MapIndex index) const;
	void SetId(const std::string& layerName, MapIndex index, int id);
	/// マスの物件（空白なら nullptr）
	const MapUnit* GetUnit(const std::string& layerName, MapIndex index) const;
	/// レイヤーの MapData から名前で探す（設置するときに使う）
	const MapUnit* FindUnit(const std::string& layerName, const std::string& unitName) const;
	int GetBlankId(const std::string& layerName) const;
	/// {横の数, 縦の数}
	std::pair<int, int> GetMapSize(const std::string& layerName) const;

	/// ================= 問い合わせ（ワールド座標） ================= ///
	/// collision レイヤーのどれかが壁なら true
	bool IsWallAt(const Vector3& position) const;
	/// collision レイヤーのどれかの範囲内なら true
	bool IsInsideAt(const Vector3& position) const;
	/// 押し戻し（今はメインの collision レイヤー 1 枚）
	CollisionResult GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime);
	/// メインの collision レイヤーのマスの大きさ
	Vector2 GetBlockSize() const;

	/// ================= デバッグ ================= ///
	const std::string& GetLog() const { return reader_->GetLog(); }

private:
	struct Layer {
		std::string name;
		std::string csvPath;
		std::unique_ptr<MapChipField> field;
		bool collision = true;   // 当たり判定に使うか
		bool visible = true;     // 描画するか
	};

	Layer* Find(const std::string& name);
	const Layer* Find(const std::string& name) const;
	const Layer* GetCollisionLayer() const;
	Layer& PushLayer(const std::string& name, const MapGrid& grid, const MapData* data);
	const MapData* LoadMapData(const std::string& jsonPath);
	static std::string NormalizePath(const std::string& path);

	std::unique_ptr<MapReader> reader_;
	/// ★宣言順に注意：data を先に → 解放は layers_ が先（field は data を借りている）
	std::map<std::string, std::unique_ptr<MapData>> dataStore_;   // key = 正規化した絶対パス
	std::vector<Layer> layers_;                                     // 順番 = 追加順
};