#include "MapChipManager.h"
#include <filesystem>
#include "Object/Object.h"

MapChipManager::MapChipManager(kEngine* system)
	: reader_(std::make_unique<MapReader>(system)) {}

/// ============================== 読み込み ============================== ///

int MapChipManager::PushLayer(const MapGrid& grid, const MapData* data) {
	layers_.push_back(std::make_unique<MapLayer>(grid, data));
	return static_cast<int>(layers_.size()) - 1;
}

int MapChipManager::LoadLayer(const std::string& csvPath, const std::string& mapDataPath) {
	MapGrid grid;
	if (!MapReader::ReadMapGrid(csvPath, grid)) return -1;

	const std::string dataPath = mapDataPath.empty()
		? MapReader::FindMapDataPath(csvPath, grid.mapDataName)
		: mapDataPath;
	if (dataPath.empty()) return -1;

	const MapData* data = LoadMapData(dataPath);
	if (!data) return -1;
	return PushLayer(grid, data);
}

int MapChipManager::CreateLayer(const std::string& mapDataPath, int width, int height) {
	const MapData* data = LoadMapData(mapDataPath);
	if (!data) return -1;

	MapGrid grid;
	grid.mapDataName = data->dataName;
	grid.ids.assign(height, std::vector<int>(width, data->blankId));
	return PushLayer(grid, data);
}

void MapChipManager::UnloadLayer(int handle) {
	if (handle >= 0 && handle < (int)layers_.size()) layers_[handle].reset();   // 番号はずらさない
}

std::string MapChipManager::NormalizePath(const std::string& path) {
	std::error_code ec;
	auto p = std::filesystem::weakly_canonical(path, ec);
	return ec ? path : p.generic_string();
}

const MapData* MapChipManager::LoadMapData(const std::string& jsonPath) {
	const std::string key = NormalizePath(jsonPath);
	if (auto it = dataStore_.find(key); it != dataStore_.end()) return it->second.get();

	auto data = std::make_unique<MapData>();
	if (!reader_->ReadMapData(jsonPath, *data)) return nullptr;
	const MapData* ptr = data.get();
	dataStore_.emplace(key, std::move(data));
	return ptr;
}

void MapChipManager::Clear() {
	layers_.clear();      // ブロック・field が先
	dataStore_.clear();   // data は後
}

MapLayer* MapChipManager::GetLayer(int handle) {
	return (handle >= 0 && handle < (int)layers_.size()) ? layers_[handle].get() : nullptr;
}

const MapLayer* MapChipManager::GetLayer(int handle) const {
	return const_cast<MapChipManager*>(this)->GetLayer(handle);
}

const MapLayer* MapChipManager::GetCollisionLayer() const {
	for (const auto& l : layers_) if (l && l->collision) return l.get();
	return nullptr;
}

/// ============================== 生成 ============================== ///

MapReader::SpawnReport MapChipManager::SpawnLayer(MapLayer& layer) {
	/// 直接タイル → ブロック（レイヤーが持つ）／それ以外 → シーンの Default
	reader_->SetDefault([this, &layer](const MapReader::SpawnInfo& s) {
		if (s.unit && s.unit->isDirectTile) {
			layer.SetBlock(s.index, s.id, std::unique_ptr<Object>(BuildObject(*s.unit, s.position)));
		} else if (userDefault_) {
			userDefault_(s);
		}
		});
	return reader_->Spawn(layer.GetField());
}

MapReader::SpawnReport MapChipManager::Generate(int handle) {
	MapLayer* layer = GetLayer(handle);
	return layer ? SpawnLayer(*layer) : MapReader::SpawnReport{};
}

void MapChipManager::GenerateAll() {
	for (auto& l : layers_) if (l) SpawnLayer(*l);
}

/// ============================== ブロック ============================== ///

bool MapChipManager::Break(const Vector3& position) {
	for (auto& l : layers_) {
		if (!l || !l->collision) continue;
		if (l->Break(l->GetField().GetMapIndexByPosition(position))) return true;
	}
	return false;
}

bool MapChipManager::Place(int handle, const Vector3& position, const std::string& unitName) {
	MapLayer* l = GetLayer(handle);
	if (!l) return false;
	const MapChipField& f = l->GetField();

	const MapIndex m = f.GetMapIndexByPosition(position);
	if (!f.InRange(m) || f.GetIdByMap(m) != f.GetBlankId()) return false;   // 範囲外 / 埋まっている

	const MapUnit* unit = f.GetMapData() ? f.GetMapData()->FindUnit(unitName) : nullptr;
	if (!unit || !unit->isDirectTile) return false;

	l->SetBlock(m, unit->id, std::unique_ptr<Object>(BuildObject(*unit, f.GetWorldPosFromMapByMapIndex(m))));
	return true;
}

void MapChipManager::UpdateBlocks() {
	for (auto& l : layers_) if (l) l->UpdateBlocks();
}

void MapChipManager::DrawBlocks(const Vector3& center, float drawRadius) {
	for (auto& l : layers_) if (l) l->DrawBlocks(center, drawRadius);
}

/// ============================== 問い合わせ ============================== ///

bool MapChipManager::IsWallAt(const Vector3& position) const {
	for (const auto& l : layers_) {
		if (!l || !l->collision) continue;
		const MapChipField& f = l->GetField();
		if (f.IsWallByMap(f.GetMapIndexByPosition(position))) return true;
	}
	return false;
}

bool MapChipManager::IsInsideAt(const Vector3& position) const {
	for (const auto& l : layers_) {
		if (!l || !l->collision) continue;
		const MapChipField& f = l->GetField();
		if (f.InRange(f.GetMapIndexByPosition(position))) return true;
	}
	return false;
}

CollisionResult MapChipManager::GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime) const {
	const MapLayer* l = GetCollisionLayer();
	return l ? l->GetField().GetMapCollisionCorrection(box, velocity, deltaTime) : CollisionResult{};
}

Vector2 MapChipManager::GetBlockSize() const {
	const MapLayer* l = GetCollisionLayer();
	return l ? l->GetField().GetBlockSize() : Vector2{ 1.0f, 1.0f };
}