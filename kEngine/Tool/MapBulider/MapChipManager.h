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
	//~MapChipManager();

	/// ================= 読み込み ================= ///
	int LoadLayer(const std::string& csvPath, const std::string& mapDataPath = "");
	/// 空のレイヤー（エディタ用）
	int CreateLayer(const std::string& mapDataPath, int width, int height);
	void UnloadLayer(int handle);
	void Clear();

	MapLayer* GetLayer(int handle);
	const MapLayer* GetLayer(int handle) const;

	/// ================= 生成の登録（シーン側のルール） ================= ///
	/// 複数：create で作った物を list に追加（vector<T*> / vector<unique_ptr<T>> 両方 OK）
	template<class Owner, class T, class Elem>
	void Register(const std::string& unitName, std::vector<Elem>* list,
		Owner* owner, T* (Owner::* create)(const MapReader::SpawnInfo&)) {
		reader_->Register(unitName, [list, owner, create](const MapReader::SpawnInfo& s) {   // ★reader_->
			list->emplace_back((owner->*create)(s));
			});
	}

	/// 単体：create で作った物を *slot に（既にあれば前のは delete → 最後の 1 個）
	template<class Owner, class T>
	void Register(const std::string& unitName, T** slot,
		Owner* owner, T* (Owner::* create)(const MapReader::SpawnInfo&)) {
		reader_->Register(unitName, [slot, owner, create](const MapReader::SpawnInfo& s) {   // ★reader_->
			delete* slot;
			*slot = (owner->*create)(s);
			});
	}

	/// 登録に無い物（戻り値は無視）
	template<class Owner, class R>
	void SetDefault(Owner* owner, R(Owner::* method)(const MapReader::SpawnInfo&)) {
		userDefault_ = [owner, method](const MapReader::SpawnInfo& s) { (owner->*method)(s); };
	}

	void ClearRegistry() { reader_->ClearRegistry(); userDefault_ = nullptr; }

	/// ================= 生成 ================= ///
	/// 登録に従って全マスの生成関数を呼ぶ（物は呼ばれた側が持つ）
	MapReader::SpawnReport Generate(int handle);
	void GenerateAll();
	/// MapUnit の見た目で Object を作る（new したものを返す。delete は呼んだ側）
	Object* BuildObject(const MapUnit& unit, const Vector3& position) const { return reader_->BuildObject(unit, position); }

	/// ================= マス単位の読み書き（破壊・設置用） ================= ///
	/// position のブロックを壊す（collision レイヤーから探す）
	bool Break(const Vector3& position);
	/// handle のレイヤーの position に unitName を置く
	bool Place(int handle, const Vector3& position, const std::string& unitName);
	void UpdateBlocks();
	void DrawBlocks(const Vector3& center, float drawRadius);

	/// ================= 問い合わせ（ワールド座標） ================= ///
	/// collision レイヤーのどれかが壁なら true
	bool IsWallAt(const Vector3& position) const;
	/// collision レイヤーのどれかの範囲内なら true
	bool IsInsideAt(const Vector3& position) const;
	/// 押し戻し（今はメインの collision レイヤー 1 枚）
	CollisionResult GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime) const;
	/// メインの collision レイヤーのマスの大きさ
	Vector2 GetBlockSize() const;

	/// ================= デバッグ ================= ///
	const std::string& GetLog() const { return reader_->GetLog(); }

private:
	int PushLayer(const MapGrid& grid, const MapData* data);
	MapReader::SpawnReport SpawnLayer(MapLayer& layer);
	const MapLayer* GetCollisionLayer() const;
	const MapData* LoadMapData(const std::string& jsonPath);
	static std::string NormalizePath(const std::string& path);

	std::unique_ptr<MapReader> reader_;
	MapReader::SpawnFunc userDefault_;
	/// ★宣言順に注意：data を先に → 解放は layers_ が先（field は data を借りている）
	std::map<std::string, std::unique_ptr<MapData>> dataStore_;
	std::vector<std::unique_ptr<MapLayer>> layers_;   // index = handle（Unload したら nullptr）
};