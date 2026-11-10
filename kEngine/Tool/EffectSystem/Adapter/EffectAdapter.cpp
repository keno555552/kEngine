#include "EffectAdapter.h"
#include <filesystem>
#include <set>
#include <algorithm>

#include "kEngine.h"
#include "Tool/JsonHelper/JsonHelper.h"
#include "Tool/FileManager/FileManager.h"
#include "Utility/EnumNameFinder/EnumNameFinder.h"

namespace {
	bool WriteJsonFile(const std::string& path, const nlohmann::json& j) {
		std::ofstream out(path);
		if (!out.is_open()) return false;
		out << j.dump(4);
		return true;
	}

	/// 同じファイルに2つ書かないように。使用済みなら _1, _2 ... を付ける
	std::filesystem::path MakeUniquePath(std::set<std::string>& used, const std::filesystem::path& path) {
		std::filesystem::path result = path;
		for (int n = 1; used.count(result.lexically_normal().generic_string()); ++n) {
			result = path.parent_path() / (path.stem().string() + "_" + std::to_string(n) + path.extension().string());
		}
		used.insert(result.lexically_normal().generic_string());
		return result;
	}

	/// 内蔵モデルのハンドル → 種類（パーティクルはデフォルトの内蔵モデルしか使わないので、ハンドルで判定できる）
	EngineModelType FindEngineModelType(int handle) {
		if (handle == config::default_Plane_MeshBufferHandle_)    return EngineModelType::Plane;
		if (handle == config::default_Triangle_MeshBufferHandle_) return EngineModelType::Triangle;
		if (handle == config::default_Cube_MeshBufferHandle_)     return EngineModelType::Cube;
		if (handle == config::default_Sphere_MeshBufferHandle_)   return EngineModelType::Sphere;
		if (handle == config::default_Ring_MeshBufferHandle_)     return EngineModelType::Ring;
		if (handle == config::default_Cylinder_MeshBufferHandle_) return EngineModelType::Cylinder;
		return EngineModelType::None;
	}

	bool ReadJsonFile(const std::string& path, nlohmann::json& out) {
		std::ifstream in(path);
		if (!in.is_open()) return false;
		out = nlohmann::json::parse(in, nullptr, false);   // 壊れた JSON でも例外を投げない
		return !out.is_discarded();
	}

	void AddLog(std::string* log, const std::string& line) {
		if (log) *log += line + "\n";
	}

	/// 新しい形式のファイル → 警告だけ（読めるところは読む）
	void CheckVersion(const nlohmann::json& j, int currentVersion, const std::string& path, std::string* log) {
		if (j.value("version", 0) > currentVersion) {
			AddLog(log, "Load Warning: newer file version " + path);
		}
	}

	/// 保存されたパス → 実際に使うパス
	/// 相対なら baseDir から。作業フォルダの中なら "./kEngine/..." の形に揃える（エディタが読む時と同じ文字列 → 二重読み込みしない）
	std::string ResolveLoadPath(const std::string& raw, const std::string& baseDir) {
		if (raw.empty()) return raw;
		return FileManager::ToRelativePath(FileManager::ResolveFrom(raw, baseDir));
	}

	/// 内蔵モデルの種類 → ハンドル
	int EngineModelHandle(EngineModelType type) {
		switch (type) {
		case EngineModelType::Plane:    return config::default_Plane_MeshBufferHandle_;
		case EngineModelType::Triangle: return config::default_Triangle_MeshBufferHandle_;
		case EngineModelType::Cube:     return config::default_Cube_MeshBufferHandle_;
		case EngineModelType::Sphere:   return config::default_Sphere_MeshBufferHandle_;
		case EngineModelType::Ring:     return config::default_Ring_MeshBufferHandle_;
		case EngineModelType::Cylinder: return config::default_Cylinder_MeshBufferHandle_;
		default:                        return config::default_Plane_MeshBufferHandle_;
		}
	}
}

/// ======================================= 読み込み ======================================= ///

bool EffectAdapter::LoadEffect(const std::string& effectPath, EffectData& out, std::string* log) {
	namespace fs = std::filesystem;

	nlohmann::json j;
	if (!ReadJsonFile(effectPath, j)) {
		AddLog(log, "Load Error: cannot read " + effectPath);
		return false;
	}
	if (j.value("kind", std::string()) != kKindEffect) {
		AddLog(log, "Load Error: not an Effect file (kind = " + j.value("kind", std::string("?")) + ") " + effectPath);
		return false;
	}
	CheckVersion(j, kFileVersion, effectPath, log);

	const std::string effectDir = fs::path(effectPath).parent_path().string();

	EffectData effect;                       // 一旦別の入れ物に（失敗しても out を壊さない）
	effect.name = j.value("name", fs::path(effectPath).stem().string());
	bool missing = false;

	std::vector<bool> hasRootFlag;   // prototypes と同じ順番。false = 古い形式（後で自動判定）

	for (const auto& f : j.value("particles", nlohmann::json::array())) {
		/// 古い形式："Particle/xxx.json"  新しい形式：{ "path": ..., "isRoot": ... }
		std::string rel;
		bool hasFlag = false;
		bool isRoot = true;
		if (f.is_string()) {
			rel = f.get<std::string>();
		} else if (f.is_object()) {
			rel = f.value("path", std::string());
			if (f.contains("isRoot")) {
				hasFlag = true;
				isRoot = f.value("isRoot", true);
			}
		} else {
			continue;
		}

		const std::string full = FileManager::ResolveFrom(rel, effectDir);
		ParticlePrototype proto;
		if (LoadParticle(full, proto, log)) {
			proto.isRoot = isRoot;
			effect.prototypes.push_back(std::move(proto));
			hasRootFlag.push_back(hasFlag);
		} else {
			missing = true;
		}
	}

	for (const auto& f : j.value("links", nlohmann::json::array())) {
		if (!f.is_string()) continue;
		const std::string full = FileManager::ResolveFrom(f.get<std::string>(), effectDir);
		EmitterLink link;
		if (LoadEmitterLink(full, link, log)) effect.links.push_back(std::move(link));
		else missing = true;
	}

	/// 古い形式で isRoot が無かった Particle → 「どの Link の target にもなってない」なら Root
	for (size_t i = 0; i < effect.prototypes.size(); i++) {
		if (hasRootFlag[i]) continue;
		const std::string& name = effect.prototypes[i].name;
		const bool isTarget = std::any_of(effect.links.begin(), effect.links.end(),
			[&](const EmitterLink& l) { return l.targetName == name; });
		effect.prototypes[i].isRoot = !isTarget;
	}

	effect.savePath = effectPath;
	/// 読めなかったものがある → そのまま保存すると参照が消えるので「未保存」にして気づかせる
	effect.isSaved = !missing;
	if (missing) AddLog(log, "Load Warning: some files were missing in " + effect.name);

	out = std::move(effect);
	return true;
}

bool EffectAdapter::LoadParticle(const std::string& path, ParticlePrototype& out, std::string* log) {
	nlohmann::json j;
	if (!ReadJsonFile(path, j)) {
		AddLog(log, "Load Error: cannot read " + path);
		return false;
	}
	if (j.value("kind", std::string()) != kKindParticle) {
		AddLog(log, "Load Error: not a Particle file " + path);
		return false;
	}
	CheckVersion(j, kFileVersion, path, log);

	out = JsonToParticle(j, std::filesystem::path(path).parent_path().string(), log);
	out.savePath = path;
	out.isSaved = true;
	return true;
}

bool EffectAdapter::LoadEmitterLink(const std::string& path, EmitterLink& out, std::string* log) {
	nlohmann::json j;
	if (!ReadJsonFile(path, j)) {
		AddLog(log, "Load Error: cannot read " + path);
		return false;
	}
	if (j.value("kind", std::string()) != kKindLink) {
		AddLog(log, "Load Error: not an EmitterLink file " + path);
		return false;
	}
	CheckVersion(j, kFileVersion, path, log);

	out = JsonToEmitterLink(j);
	out.savePath = path;
	out.isSaved = true;
	return true;
}

/// ======================================= 保存 ======================================= ///


EffectSavePlan EffectAdapter::PlanEffect(const EffectData& effect, const std::string& effectPath) const {
	namespace fs = std::filesystem;

	EffectSavePlan plan;
	plan.effectPath = effectPath;

	const fs::path effectDir = fs::path(effectPath).parent_path();
	std::set<std::string> usedFiles;
	usedFiles.insert(fs::path(effectPath).lexically_normal().generic_string());

	for (const auto& proto : effect.prototypes) {
		fs::path full = proto.savePath.empty()
			? effectDir / "Particle" / (FileManager::ToSafeFileName(proto.name) + ".json")   // 新規 → Effectフォルダの下
			: fs::path(proto.savePath);                                                    // 既にある → その場所（共用フォルダも）
		plan.particlePaths.push_back(MakeUniquePath(usedFiles, full).string());
	}

	for (const auto& link : effect.links) {
		fs::path full = link.savePath.empty()
			? effectDir / "EmitterLink" / (FileManager::ToSafeFileName(link.name) + ".json")
			: fs::path(link.savePath);
		plan.linkPaths.push_back(MakeUniquePath(usedFiles, full).string());
	}

	return plan;
}

bool EffectAdapter::WriteEffect(EffectData& effect, const EffectSavePlan& plan, bool useRelativePath, std::string* error) {
	namespace fs = std::filesystem;

	auto fail = [&](const std::string& msg) {
		if (error) *error = "[EffectAdapter] " + msg;
		return false;
		};

	if (plan.effectPath.empty()) return fail("Save path is empty.");
	/// 計画した後に Particle / Link が増減していたら書かない
	if (plan.particlePaths.size() != effect.prototypes.size() || plan.linkPaths.size() != effect.links.size()) {
		return fail("Effect changed after planning. Please save again.");
	}

	useRelativePath_ = useRelativePath;
	const fs::path effectDir = fs::path(plan.effectPath).parent_path();
	std::error_code ec;

	nlohmann::json j;
	j["kind"] = kKindEffect;
	j["version"] = kFileVersion;
	j["name"] = effect.name;
	j["relativePaths"] = useRelativePath;
	j["particles"] = nlohmann::json::array();
	j["links"] = nlohmann::json::array();

	/// --- Particle：1つ1ファイル ---
	for (size_t i = 0; i < effect.prototypes.size(); i++) {
		auto& proto = effect.prototypes[i];
		const std::string& full = plan.particlePaths[i];
		if (!WriteParticle(proto, full, useRelativePath, error)) return false;
		proto.savePath = full;

		/// Effectファイルには「Effectファイルから見たパス」と「起点かどうか」を書く
		nlohmann::json entry;
		entry["path"] = ToSavePath(full, effectDir.string());
		entry["isRoot"] = proto.isRoot;
		j["particles"].push_back(entry);
	}

	/// --- EmitterLink：1つ1ファイル ---
	for (size_t i = 0; i < effect.links.size(); i++) {
		auto& link = effect.links[i];
		const std::string& full = plan.linkPaths[i];
		if (!WriteEmitterLink(link, full, error)) return false;
		link.savePath = full;
		j["links"].push_back(ToSavePath(full, effectDir.string()));
	}

	/// 最後にEffectファイル（中身が全部書けてから）
	fs::create_directories(effectDir, ec);
	if (!WriteJsonFile(plan.effectPath, j)) return fail("Failed to write: " + plan.effectPath);
	return true;
}

bool EffectAdapter::WriteParticle(const ParticlePrototype& proto, const std::string& path, bool useRelativePath, std::string* error) {
	namespace fs = std::filesystem;
	if (path.empty()) { if (error) *error = "[EffectAdapter] Save path is empty."; return false; }

	useRelativePath_ = useRelativePath;
	const fs::path full = path;

	std::error_code ec;
	fs::create_directories(full.parent_path(), ec);
	if (ec) { if (error) *error = "[EffectAdapter] Cannot create folder: " + full.parent_path().string(); return false; }

	/// モデル・テクスチャの相対パスは「このファイルの場所」基準
	if (!WriteJsonFile(path, ParticleToJson(proto, full.parent_path().string()))) {
		if (error) *error = "[EffectAdapter] Failed to write: " + path;
		return false;
	}
	return true;
}

bool EffectAdapter::WriteEmitterLink(const EmitterLink& link, const std::string& path, std::string* error) {
	namespace fs = std::filesystem;
	if (path.empty()) { if (error) *error = "[EffectAdapter] Save path is empty."; return false; }

	std::error_code ec;
	fs::create_directories(fs::path(path).parent_path(), ec);
	if (ec) { if (error) *error = "[EffectAdapter] Cannot create folder: " + fs::path(path).parent_path().string(); return false; }

	if (!WriteJsonFile(path, EmitterLinkToJson(link))) {
		if (error) *error = "[EffectAdapter] Failed to write: " + path;
		return false;
	}
	return true;
}


nlohmann::json EffectAdapter::ParticleToJson(const ParticlePrototype& proto, const std::string& baseDir) const {
	nlohmann::json j;
	j["kind"] = kKindParticle;
	j["version"] = kFileVersion;

	j["name"] = proto.name;
	j["lifetime"] = proto.lifetime;
	j["lifetimeRandomness"] = proto.lifetimeRandomness;

	j["startPosition"] = JsonHelper::V3ToJson(proto.startPosition);
	j["startPositionRandom"] = JsonHelper::V3ToJson(proto.startPositionRandom);

	j["startVelocity"] = JsonHelper::V3ToJson(proto.startVelocity);
	j["startVelocityRandom"] = JsonHelper::V3ToJson(proto.startVelocityRandom);
	j["startSpeedRandom"] = JsonHelper::V3ToJson(proto.startSpeedRandom);

	j["startScale"] = JsonHelper::V3ToJson(proto.startScale);
	j["endScale"] = JsonHelper::V3ToJson(proto.endScale);
	j["startScaleRandom"] = JsonHelper::V3ToJson(proto.startScaleRandom);
	j["endScaleRandom"] = JsonHelper::V3ToJson(proto.endScaleRandom);
	j["isConstantScale"] = proto.isConstantScale;

	j["startRotation"] = JsonHelper::V3ToJson(proto.startRotation);
	j["endRotation"] = JsonHelper::V3ToJson(proto.endRotation);
	j["startRotationRandom"] = JsonHelper::V3ToJson(proto.startRotationRandom);
	j["endRotationRandom"] = JsonHelper::V3ToJson(proto.endRotationRandom);
	j["isConstantRotation"] = proto.isConstantRotation;

	j["startColor"] = JsonHelper::V4ToJson(proto.startColor);
	j["endColor"] = JsonHelper::V4ToJson(proto.endColor);
	j["startColorRandom"] = JsonHelper::V4ToJson(proto.startColorRandom);
	j["endColorRandom"] = JsonHelper::V4ToJson(proto.endColorRandom);
	j["isConstantColor"] = proto.isConstantColor;

	j["gravity"] = JsonHelper::V3ToJson(proto.gravity);
	j["damping"] = proto.damping;
	j["dampingRandom"] = proto.dampingRandom;

	j["emitRate"] = proto.emitRate;
	j["emitIntervalRandom"] = proto.emitIntervalRandom;
	j["burstCount"] = proto.burstCount;
	j["emitNumRandom"] = proto.emitNumRandom;

	j["renderType"] = (int)proto.renderType;

	/// 見た目（モデル・マテリアル）
	j["objectList"] = nlohmann::json::array();
	for (const auto& obj : proto.objectList) {
		j["objectList"].push_back(ObjectToJson(obj, baseDir));
	}
	/// spriteList はまだ使えないので書かない

	return j;
}

nlohmann::json EffectAdapter::EmitterLinkToJson(const EmitterLink& link) const {
	nlohmann::json j;
	j["kind"] = kKindLink;
	j["version"] = kFileVersion;

	j["name"] = link.name;
	j["sourceEmitter"] = link.sourceName;
	j["targetEmitter"] = link.targetName;
	j["emitCount"] = link.emitCount;
	j["delayTime"] = link.delayTime;
	j["followPosition"] = link.followPosition;
	j["followRotation"] = link.followRotation;
	j["followScale"] = link.followScale;
	j["linkMode"] = (int)link.linkMode;
	j["linkFollow"] = (int)link.linkFollow;
	j["emitterTiming"] = (int)link.emitterTiming;
	return j;
}

nlohmann::json EffectAdapter::ObjectToJson(const ObjectData& obj, const std::string& baseDir) const {
	nlohmann::json j;

	nlohmann::json model = ModelToJson(obj.modelHandle_, baseDir);
	const bool isFileModel = (model["type"] == "file");
	j["model"] = model;

	j["isBillboard"] = obj.isBillboard_;
	j["mainTransform"] = JsonHelper::TransformToJson(obj.mainPosition.transform);

	j["parts"] = nlohmann::json::array();
	for (const auto& part : obj.objectParts_) {
		nlohmann::json pj;
		pj["name"] = part.name;
		pj["transform"] = JsonHelper::TransformToJson(part.transform);

		if (part.materialConfig) {
			nlohmann::json mat = JsonHelper::MaterialConfigToJson(*part.materialConfig, system_);
			if (isFileModel && part.materialConfig->useModelTexture) {
				/// ファイルモデル付属のテクスチャ → パスは持たない（読み込み時にモデルから取り直す）
				mat["textureHandle"] = "";
			} else {
				/// 内蔵モデル or 手で指定したテクスチャ → パスを保存
				mat["textureHandle"] = ToSavePath(mat.value("textureHandle", std::string()), baseDir);
			}
			pj["material"] = mat;
		} else {
			pj["material"] = nullptr;
		}

		j["parts"].push_back(pj);
	}
	return j;
}

nlohmann::json EffectAdapter::ModelToJson(int modelHandle, const std::string& baseDir) const {
	nlohmann::json model;   // handle は保存しない（起動ごとに変わる）

	const EngineModelType engineType = FindEngineModelType(modelHandle);
	if (engineType != EngineModelType::None) {
		model["type"] = "engine";
		model["engine"] = GetEnumName(engineType);
		model["params"] = JsonHelper::EngineParamsToJson(EngineModelParams{});   // デフォルトモデルなので初期値
		return model;
	}

	const std::string path = GetModelPath(modelHandle);
	if (!path.empty()) {
		model["type"] = "file";
		model["path"] = ToSavePath(path, baseDir);
	} else {
		model["type"] = "none";
	}
	return model;
}

std::string EffectAdapter::GetModelPath(int modelHandle) const {
	auto& groups = system_->GetResourceManager()->modelGroupList_;
	if (modelHandle < 0 || modelHandle >= (int)groups.size()) return "";
	return groups[modelHandle]->GetFilePath();
}

std::string EffectAdapter::ToSavePath(const std::string& path, const std::string& baseDir) const {
	if (path.empty()) return path;
	return useRelativePath_ ? FileManager::MakeRelativeTo(path, baseDir)
		: FileManager::ToAbsolutePath(path);
}

ParticlePrototype EffectAdapter::JsonToParticle(const nlohmann::json& j, const std::string& baseDir, std::string* log) {
	ParticlePrototype p{};

	/// 無いキーは今の値（＝初期値）のまま
	auto v3 = [&](const char* key, const Vector3& def) { return j.contains(key) ? JsonHelper::JsonToV3(j[key]) : def; };
	auto v4 = [&](const char* key, const Vector4& def) { return j.contains(key) ? JsonHelper::JsonToV4(j[key]) : def; };

	p.name = j.value("name", std::string("new_particle"));
	p.lifetime = j.value("lifetime", p.lifetime);
	p.lifetimeRandomness = j.value("lifetimeRandomness", p.lifetimeRandomness);

	p.startPosition = v3("startPosition", p.startPosition);
	p.startPositionRandom = v3("startPositionRandom", p.startPositionRandom);

	p.startVelocity = v3("startVelocity", p.startVelocity);
	p.startVelocityRandom = v3("startVelocityRandom", p.startVelocityRandom);
	p.startSpeedRandom = v3("startSpeedRandom", p.startSpeedRandom);

	p.startScale = v3("startScale", p.startScale);
	p.endScale = v3("endScale", p.endScale);
	p.startScaleRandom = v3("startScaleRandom", p.startScaleRandom);
	p.endScaleRandom = v3("endScaleRandom", p.endScaleRandom);
	p.isConstantScale = j.value("isConstantScale", p.isConstantScale);

	p.startRotation = v3("startRotation", p.startRotation);
	p.endRotation = v3("endRotation", p.endRotation);
	p.startRotationRandom = v3("startRotationRandom", p.startRotationRandom);
	p.endRotationRandom = v3("endRotationRandom", p.endRotationRandom);
	p.isConstantRotation = j.value("isConstantRotation", p.isConstantRotation);

	p.startColor = v4("startColor", p.startColor);
	p.endColor = v4("endColor", p.endColor);
	p.startColorRandom = v4("startColorRandom", p.startColorRandom);
	p.endColorRandom = v4("endColorRandom", p.endColorRandom);
	p.isConstantColor = j.value("isConstantColor", p.isConstantColor);

	p.gravity = v3("gravity", p.gravity);
	p.damping = j.value("damping", p.damping);
	p.dampingRandom = j.value("dampingRandom", p.dampingRandom);

	p.emitRate = j.value("emitRate", p.emitRate);
	p.emitIntervalRandom = j.value("emitIntervalRandom", p.emitIntervalRandom);
	p.burstCount = j.value("burstCount", p.burstCount);
	p.emitNumRandom = j.value("emitNumRandom", p.emitNumRandom);

	p.renderType = (ParticleRenderType)j.value("renderType", (int)p.renderType);

	for (const auto& oj : j.value("objectList", nlohmann::json::array())) {
		p.objectList.push_back(JsonToObject(oj, baseDir, log));
	}
	return p;
}

EmitterLink EffectAdapter::JsonToEmitterLink(const nlohmann::json& j) const {
	EmitterLink l{};
	l.name = j.value("name", std::string("new_link"));
	l.sourceName = j.value("sourceEmitter", std::string());
	l.targetName = j.value("targetEmitter", std::string());
	l.emitCount = j.value("emitCount", l.emitCount);
	l.delayTime = j.value("delayTime", l.delayTime);
	l.followPosition = j.value("followPosition", l.followPosition);
	l.followRotation = j.value("followRotation", l.followRotation);
	l.followScale = j.value("followScale", l.followScale);
	l.linkMode = (LinkMode)j.value("linkMode", (int)l.linkMode);
	l.linkFollow = (LinkFollow)j.value("linkFollow", (int)l.linkFollow);
	l.emitterTiming = (EmitterTiming)j.value("emitterTiming", (int)l.emitterTiming);
	return l;
}

ObjectData EffectAdapter::JsonToObject(const nlohmann::json& j, const std::string& baseDir, std::string* log) {
	ObjectData obj;

	bool isFileModel = false;
	obj.modelHandle_ = JsonToModel(j.value("model", nlohmann::json::object()), baseDir, &isFileModel, log);
	obj.isBillboard_ = j.value("isBillboard", false);
	if (j.contains("mainTransform")) obj.mainPosition.transform = JsonHelper::JsonToTransform(j["mainTransform"]);

	int partIndex = 0;
	for (const auto& pj : j.value("parts", nlohmann::json::array())) {
		ObjectPart part;
		part.name = pj.value("name", std::string());
		if (pj.contains("transform")) part.transform = JsonHelper::JsonToTransform(pj["transform"]);

		if (pj.contains("material") && pj["material"].is_object()) {
			nlohmann::json mat = pj["material"];
			const std::string rawTex = mat.value("textureHandle", std::string());

			/// テクスチャは JsonHelper に読ませない（パス変換・存在チェックをここでやる）
			mat["textureHandle"] = "";
			part.materialConfig = std::make_shared<MaterialConfig>(JsonHelper::JsonToMaterialConfig(mat, system_));

			if (isFileModel && part.materialConfig->useModelTexture && rawTex.empty()) {
				/// ファイルモデル付属のテクスチャ → モデルから取り直す
				const int modelParts = system_->GetMutiModelNum(obj.modelHandle_);
				part.materialConfig->textureHandle =
					system_->GetModelTextureHandle(obj.modelHandle_, partIndex < modelParts ? partIndex : 0);
			} else {
				part.materialConfig->textureHandle = LoadTextureFrom(rawTex, baseDir, log);
			}
		}

		obj.objectParts_.push_back(part);
		partIndex++;
	}
	return obj;
}

int EffectAdapter::JsonToModel(const nlohmann::json& model, const std::string& baseDir, bool* isFileModel, std::string* log) {
	*isFileModel = false;
	const std::string type = model.value("type", std::string("none"));

	if (type == "engine") {
		const std::string engineName = model.value("engine", std::string());
		for (const auto& [t, name] : EnumNameMap<EngineModelType>) {
			/// パーティクルはデフォルトの内蔵モデルだけ使う（params は今は見ない）
			if (name == engineName) return EngineModelHandle(t);
		}
		AddLog(log, "Load Warning: unknown engine model \"" + engineName + "\" -> Plane");
		return config::default_Plane_MeshBufferHandle_;
	}

	if (type == "file") {
		const std::string path = ResolveLoadPath(model.value("path", std::string()), baseDir);
		std::error_code ec;
		if (!path.empty() && std::filesystem::exists(path, ec)) {
			const int handle = system_->SetModelObj(path);
			if (system_->GetMutiModelNum(handle) > 0) {
				*isFileModel = true;
				return handle;
			}
		}
		AddLog(log, "Load Warning: cannot load model " + path + " -> Plane");
	}

	return config::default_Plane_MeshBufferHandle_;
}

int EffectAdapter::LoadTextureFrom(const std::string& rawPath, const std::string& baseDir, std::string* log) {
	if (rawPath.empty()) return config::default_White5x5_TextureHandle_;

	const std::string path = ResolveLoadPath(rawPath, baseDir);
	std::error_code ec;
	/// 無いファイルを読むと TextureManager が assert で止まる → 白で代用
	if (!std::filesystem::exists(path, ec)) {
		AddLog(log, "Load Warning: texture not found " + path);
		return config::default_White5x5_TextureHandle_;
	}
	return system_->LoadTexture(path);
}