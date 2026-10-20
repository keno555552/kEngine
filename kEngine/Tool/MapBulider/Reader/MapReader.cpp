#include "MapReader.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include "kEngine.h"
#include "Renderer/Resource/ResourceManager.h"
#include "Tool/JsonHelper/JsonHelper.h"
#include "Tool/FileManager/FileManager.h"

namespace {
	EngineModelParams JsonToEngineParams(const nlohmann::json& j) {
		EngineModelParams p{};
		p.sphereLat = j.value("sphereLat", p.sphereLat);
		p.sphereLong = j.value("sphereLong", p.sphereLong);
		p.ringDivide = j.value("ringDivide", p.ringDivide);
		p.ringOuter = j.value("ringOuter", p.ringOuter);
		p.ringInner = j.value("ringInner", p.ringInner);
		p.cylinderDivide = j.value("cylinderDivide", p.cylinderDivide);
		p.cylinderTop = j.value("cylinderTop", p.cylinderTop);
		p.cylinderBottom = j.value("cylinderBottom", p.cylinderBottom);
		p.cylinderHeight = j.value("cylinderHeight", p.cylinderHeight);
		p.cylinderReverseY = j.value("cylinderReverseY", p.cylinderReverseY);
		return p;
	}
}

/// ============================== 読み込み ============================== ///

bool MapReader::LoadMap(const std::string& csvPath, const std::string& mapDataPath, MapPackage& out) {
	namespace fs = std::filesystem;

	if (!ReadMapGrid(csvPath, out.grid)) {
		log_ += "MapReader: cannot read csv " + csvPath + "\n";
		return false;
	}

	/// MapData の場所：指定が無ければ csv の「#mapData」の名前で、よくある場所を探す
	std::string dataPath = mapDataPath;
	if (dataPath.empty()) {
		if (out.grid.mapDataName.empty()) {
			log_ += "MapReader: no #mapData in " + csvPath + " and no MapData path given\n";
			return false;
		}
		const fs::path dir = fs::path(csvPath).parent_path();
		const std::string file = out.grid.mapDataName + ".json";
		for (const fs::path& candidate : { dir / file, dir / "MapData" / file, dir.parent_path() / "MapData" / file }) {
			if (fs::exists(candidate)) { dataPath = candidate.generic_string(); break; }
		}
		if (dataPath.empty()) {
			log_ += "MapReader: MapData '" + out.grid.mapDataName + "' not found near " + csvPath + "\n";
			return false;
		}
	}

	if (!ReadMapData(dataPath, out.data)) return false;

	if (!out.grid.mapDataName.empty() && out.grid.mapDataName != out.data.dataName) {
		log_ += "MapReader: warning: csv wants '" + out.grid.mapDataName + "' but loaded '" + out.data.dataName + "'\n";
	}
	return true;
}

bool MapReader::ReadMapData(const std::string& jsonPath, MapData& out) {
	std::ifstream in(jsonPath);
	if (!in.is_open()) {
		log_ += "MapReader: cannot open " + jsonPath + "\n";
		return false;
	}
	nlohmann::json j = nlohmann::json::parse(in, nullptr, false);
	if (j.is_discarded() || j.value("kind", std::string()) != "MapData") {
		log_ += "MapReader: not a MapData file " + jsonPath + "\n";
		return false;
	}

	const std::string baseDir = std::filesystem::path(jsonPath).parent_path().string();

	out = MapData{};
	out.dataName = j.value("name", std::string("MapData"));
	out.xSize = j.value("xSize", 0);
	out.ySize = j.value("ySize", 0);
	out.startX = j.value("startX", 0);
	out.startY = j.value("startY", 0);
	out.blankId = j.value("blankId", 0);
	if (j.contains("cellSpacing") && j["cellSpacing"].size() >= 2) {
		out.cellSpacing = { j["cellSpacing"][0].get<float>(), j["cellSpacing"][1].get<float>() };
	}
	for (const auto& uj : j.value("units", nlohmann::json::array())) {
		out.units.push_back(JsonToUnit(uj, baseDir));
	}
	out.savePath = jsonPath;
	out.isSaved = true;
	return true;
}

bool MapReader::ReadMapGrid(const std::string& csvPath, MapGrid& out, int fillId) {
	std::ifstream in(csvPath);
	if (!in.is_open()) return false;

	out = MapGrid{};
	out.name = std::filesystem::path(csvPath).stem().string();

	std::string line;
	size_t maxWidth = 0;
	while (std::getline(in, line)) {
		if (!line.empty() && line.back() == '\r') line.pop_back();
		if (line.empty()) continue;

		/// '#' の行は付加情報
		if (line[0] == '#') {
			if (line.rfind("#mapData,", 0) == 0) {
				const std::string name = line.substr(9);
				out.mapDataName = name.substr(0, name.find(','));    // Excel が付ける ",,,," を落とす
			}
			continue;
		}

		std::vector<int> row;
		std::stringstream ss(line);
		std::string cell;
		while (std::getline(ss, cell, ',')) {
			try { row.push_back(cell.empty() ? fillId : std::stoi(cell)); } catch (...) { row.push_back(fillId); }                   // 数字でない → 空白扱い
		}
		maxWidth = (std::max)(maxWidth, row.size());
		out.ids.push_back(std::move(row));
	}
	for (auto& row : out.ids) row.resize(maxWidth, fillId);       // 短い行を揃える
	return !out.ids.empty();
}

/// ============================== 登録器 ============================== ///

void MapReader::Register(const std::string& name, SpawnFunc func) { nameRegistry_[name] = std::move(func); }
void MapReader::Register(int id, SpawnFunc func) { idRegistry_[id] = std::move(func); }

void MapReader::ClearRegistry() {
	nameRegistry_.clear();
	idRegistry_.clear();
	defaultFunc_ = nullptr;
}

/// ============================== 適配器 ============================== ///

MapReader::SpawnReport MapReader::Spawn(MapChipField& field) const {
	SpawnReport report;
	const MapData* data = field.GetMapData();
	const int blank = field.GetBlankId();

	for (int y = 0; y < field.GetNumBlockVirtical(); ++y) {
		for (int x = 0; x < field.GetNumBlockHorizontal(); ++x) {
			const MapChipField::MapIndex index{ x, y };
			const int id = field.GetIdByMap(index);
			if (id == blank) continue;

			SpawnInfo info;
			info.id = id;
			info.unit = data ? data->FindUnit(id) : nullptr;
			info.index = index;
			info.position = field.GetWorldPosFromMapByMapIndex(index);

			/// 優先順：ID → 名前 → Default
			const SpawnFunc* func = nullptr;
			if (auto it = idRegistry_.find(id); it != idRegistry_.end()) {
				func = &it->second;
			} else if (info.unit) {
				if (auto it2 = nameRegistry_.find(info.unit->name); it2 != nameRegistry_.end()) func = &it2->second;
			}
			if (!func && defaultFunc_) func = &defaultFunc_;

			if (func) { (*func)(info); ++report.spawned; } else ++report.unhandled;
		}
	}
	return report;
}

Object* MapReader::BuildObject(const MapUnit& u, const Vector3& position) const {
	if (u.modelHandle < 0) return nullptr;

	Object* obj = new Object();
	obj->IntObject(system_);
	if (u.engineModel == EngineModelType::None) {
		obj->CreateModelData(u.modelHandle);
	} else {
		obj->CreateDefaultData();                 // 内蔵モデルは ModelData が無い
		obj->modelHandle_ = u.modelHandle;
	}

	/// 位置 = マスの中心 + MapData の Translate（オフセット）。回転・スケールは MapData のまま
	obj->mainPosition.transform = u.transform;
	Vector3 pos = position;
	obj->mainPosition.transform.translate = pos + u.transform.translate;

	for (int p = 0; p < (int)obj->objectParts_.size(); ++p) {
		MaterialConfig& mat = *obj->objectParts_[p].materialConfig;
		mat = u.material;
		if (u.material.useModelTexture && u.engineModel == EngineModelType::None) {
			mat.textureHandle = system_->GetModelTextureHandle(u.modelHandle, p);
		}
	}
	return obj;
}

/// ============================== 内部 ============================== ///

MapUnit MapReader::JsonToUnit(const nlohmann::json& j, const std::string& baseDir) {
	MapUnit u{};
	u.id = j.value("id", 1);
	u.name = j.value("name", std::string("NewUnit"));
	u.isDirectTile = j.value("isDirectTile", true);
	if (j.contains("previewColor")) u.previewColor = JsonHelper::JsonToV4(j["previewColor"]);
	if (j.contains("transform"))    u.transform = JsonHelper::JsonToTransform(j["transform"]);

	bool texturePathEmpty = true;
	if (j.contains("material")) {
		nlohmann::json mat = j["material"];
		std::string texPath = FileManager::ResolveFrom(mat.value("textureHandle", std::string()), baseDir);
		if (!texPath.empty() && !std::filesystem::exists(texPath)) {   // 無いファイルは TextureManager が assert で止まる
			log_ += "MapReader: texture not found " + texPath + "\n";
			texPath.clear();
		}
		mat["textureHandle"] = texPath;
		texturePathEmpty = texPath.empty();
		u.material = JsonHelper::JsonToMaterialConfig(mat, system_);
	}

	const nlohmann::json model = j.value("model", nlohmann::json::object());
	const std::string type = model.value("type", std::string("none"));
	if (type == "file") {
		const std::string path = FileManager::ResolveFrom(model.value("path", std::string()), baseDir);
		if (!path.empty()) {
			const int handle = system_->SetModelObj(path);
			if (system_->GetMutiModelNum(handle) > 0) u.modelHandle = handle;
			else log_ += "MapReader: cannot load model " + path + "\n";
		}
	} else if (type == "engine") {
		const std::string engineName = model.value("engine", std::string());
		for (const auto& [t, name] : EnumNameMap<EngineModelType>) {
			if (name == engineName) u.engineModel = t;
		}
		u.engineParams = JsonToEngineParams(model.value("params", nlohmann::json::object()));
		if (u.engineModel != EngineModelType::None) u.modelHandle = GetEngineModelHandle(u.engineModel, u.engineParams);
	}

	if (u.material.useModelTexture && u.engineModel == EngineModelType::None && u.modelHandle >= 0) {
		u.material.textureHandle = system_->GetModelTextureHandle(u.modelHandle, 0);
	} else if (texturePathEmpty) {
		u.material.textureHandle = config::default_White5x5_TextureHandle_;
	}
	return u;
}

int MapReader::GetEngineModelHandle(EngineModelType type, const EngineModelParams& p) {
	const EngineModelParams def{};
	switch (type) {
	case EngineModelType::Plane:    return config::default_Plane_MeshBufferHandle_;
	case EngineModelType::Triangle: return config::default_Triangle_MeshBufferHandle_;
	case EngineModelType::Cube:     return config::default_Cube_MeshBufferHandle_;
	case EngineModelType::Sphere: {
		if (p.sphereLat == def.sphereLat && p.sphereLong == def.sphereLong) return config::default_Sphere_MeshBufferHandle_;
		const std::string key = "Sphere_" + std::to_string(p.sphereLat) + "_" + std::to_string(p.sphereLong);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		SphereBuildMaterial m;
		m.LatitudeSegments = p.sphereLat;
		m.LongitudeSegments = p.sphereLong;
		return engineModelCache_[key] = system_->CreateEngineModel(m);
	}
	case EngineModelType::Ring: {
		if (p.ringDivide == def.ringDivide && p.ringOuter == def.ringOuter && p.ringInner == def.ringInner)
			return config::default_Ring_MeshBufferHandle_;
		const std::string key = "Ring_" + std::to_string(p.ringDivide) + "_" + std::to_string(p.ringOuter) + "_" + std::to_string(p.ringInner);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		RingBuildMaterial m;
		m.Subdivision = p.ringDivide;
		m.OuterRadius = p.ringOuter;
		m.InnerRadius = p.ringInner;
		return engineModelCache_[key] = system_->GetResourceManager()->CreateEngineModel(m);
	}
	case EngineModelType::Cylinder: {
		if (p.cylinderDivide == def.cylinderDivide && p.cylinderTop == def.cylinderTop &&
			p.cylinderBottom == def.cylinderBottom && p.cylinderHeight == def.cylinderHeight && !p.cylinderReverseY)
			return config::default_Cylinder_MeshBufferHandle_;
		const std::string key = "Cylinder_" + std::to_string(p.cylinderDivide) + "_" + std::to_string(p.cylinderTop) + "_" +
			std::to_string(p.cylinderBottom) + "_" + std::to_string(p.cylinderHeight) + "_" + std::to_string(p.cylinderReverseY);
		if (auto it = engineModelCache_.find(key); it != engineModelCache_.end()) return it->second;
		CylinderBuildMaterial m;
		m.Division = p.cylinderDivide;
		m.TopRadius = p.cylinderTop;
		m.BottomRadius = p.cylinderBottom;
		m.Height = p.cylinderHeight;
		m.isReverseY = p.cylinderReverseY;
		return engineModelCache_[key] = system_->CreateEngineModel(m);
	}
	default: return -1;
	}
}