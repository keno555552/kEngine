#include "MapChipManager.h"
#include <filesystem>
#include "Object/Object.h"

MapChipManager::MapChipManager(kEngine* system)
	: reader_(std::make_unique<MapReader>(system)) {}

/// ============================== 読み込み ============================== ///

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

MapChipManager::Layer& MapChipManager::PushLayer(const std::string& name, const MapGrid& grid, const MapData* data) {
	RemoveLayer(name);   // 同名は置き換え
	Layer layer;
	layer.name = name;
	layer.field = std::make_unique<MapChipField>();
	layer.field->Setup(grid, data);
	layers_.push_back(std::move(layer));
	return layers_.back();
}

bool MapChipManager::AddLayer(const std::string& layerName, const std::string& csvPath, const std::string& mapDataPath) {
	MapGrid grid;
	if (!MapReader::ReadMapGrid(csvPath, grid)) return false;

	const std::string dataPath = mapDataPath.empty()
		? MapReader::FindMapDataPath(csvPath, grid.mapDataName)
		: mapDataPath;
	if (dataPath.empty()) return false;

	const MapData* data = LoadMapData(dataPath);
	if (!data) return false;

	PushLayer(layerName, grid, data).csvPath = csvPath;
	return true;
}

bool MapChipManager::CreateLayer(const std::string& layerName, const std::string& mapDataPath, int width, int height) {
	const MapData* data = LoadMapData(mapDataPath);
	if (!data) return false;

	MapGrid grid;
	grid.name = layerName;
	grid.mapDataName = data->dataName;
	grid.ids.assign(height, std::vector<int>(width, data->blankId));
	PushLayer(layerName, grid, data);
	return true;
}

void MapChipManager::RemoveLayer(const std::string& layerName) {
	std::erase_if(layers_, [&](const Layer& l) { return l.name == layerName; });
}

void MapChipManager::Clear() {
	layers_.clear();      // ブロック・field が先
	dataStore_.clear();   // data は後
}

MapChipManager::Layer* MapChipManager::Find(const std::string& name) {
	for (auto& l : layers_) if (l.name == name) return &l;
	return nullptr;
}
const MapChipManager::Layer* MapChipManager::Find(const std::string& name) const {
	return const_cast<MapChipManager*>(this)->Find(name);
}
const MapChipManager::Layer* MapChipManager::GetCollisionLayer() const {
	for (const auto& l : layers_) if (l.collision) return &l;
	return nullptr;
}

/// ============================== 生成 ============================== ///

MapReader::SpawnReport MapChipManager::Generate(const std::string& layerName) {
	const Layer* layer = Find(layerName);
	if (!layer) return {};
	return reader_->Spawn(*layer->field);
}
void MapChipManager::GenerateAll() {
	for (auto& l : layers_) reader_->Spawn(*l.field);
}

/// ============================== マス単位 ============================== ///

std::optional<MapChipManager::MapIndex> MapChipManager::GetIndexAt(const std::string& layerName, const Vector3& position) const {
	const Layer* l = Find(layerName);
	if (!l) return std::nullopt;
	const MapIndex m = l->field->GetMapIndexByPosition(position);
	if (m.x < 0 || m.y < 0 || m.x >= l->field->GetNumBlockHorizontal() || m.y >= l->field->GetNumBlockVirtical()) return std::nullopt;
	return m;
}

Vector3 MapChipManager::GetCellCenter(const std::string& layerName, MapIndex index) const {
	const Layer* l = Find(layerName);
	return l ? l->field->GetWorldPosFromMapByMapIndex(index) : Vector3{};
}

int MapChipManager::GetId(const std::string& layerName, MapIndex index) const {
	const Layer* l = Find(layerName);
	return l ? l->field->GetIdByMap(index) : 0;
}

void MapChipManager::SetId(const std::string& layerName, MapIndex index, int id) {
	if (Layer* l = Find(layerName)) l->field->SetIdByMap(index, id);
}

const MapUnit* MapChipManager::GetUnit(const std::string& layerName, MapIndex index) const {
	const Layer* l = Find(layerName);
	return l ? l->field->GetUnitByMap(index) : nullptr;
}

const MapUnit* MapChipManager::FindUnit(const std::string& layerName, const std::string& unitName) const {
	const Layer* l = Find(layerName);
	const MapData* data = l ? l->field->GetMapData() : nullptr;
	return data ? data->FindUnit(unitName) : nullptr;
}

int MapChipManager::GetBlankId(const std::string& layerName) const {
	const Layer* l = Find(layerName);
	return l ? l->field->GetBlankId() : 0;
}

std::pair<int, int> MapChipManager::GetMapSize(const std::string& layerName) const {
	const Layer* l = Find(layerName);
	return l ? std::pair{ l->field->GetNumBlockHorizontal(), l->field->GetNumBlockVirtical() } : std::pair{ 0, 0 };
}

/// ============================== 問い合わせ ============================== ///

bool MapChipManager::IsWallAt(const Vector3& position) const {
	for (const auto& l : layers_) {
		if (!l.collision) continue;
		if (l.field->IsWallByWorld(l.field->GetWorldIndexByPosition(position))) return true;
	}
	return false;
}

bool MapChipManager::IsInsideAt(const Vector3& position) const {
	for (const auto& l : layers_) {
		if (!l.collision) continue;
		const auto idx = l.field->GetWorldIndexByPosition(position);
		if (idx.x >= 0 && idx.y >= 0 &&
			idx.x < l.field->GetNumBlockHorizontal() && idx.y < l.field->GetNumBlockVirtical()) return true;
	}
	return false;
}

CollisionResult MapChipManager::GetMapCollisionCorrection(const AABB& box, const Vector3& velocity, float deltaTime) {
	const Layer* l = GetCollisionLayer();
	if (!l) return {};
	return l->field->GetMapCollisionCorrection(box, velocity, deltaTime);
}

Vector2 MapChipManager::GetBlockSize() const {
	const Layer* l = GetCollisionLayer();
	return l ? l->field->GetBlockSize() : Vector2{ 1.0f, 1.0f };
}