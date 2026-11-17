#include "MapMakerStore.h"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <set>

namespace fs = std::filesystem;

namespace {
	/// ファイル名に使えない文字を _ に
	std::string SafeFileName(const std::string& name) {
		std::string out = name.empty() ? std::string("Unnamed") : name;
		for (char& c : out) {
			if (std::string("\\/:*?\"<>|").find(c) != std::string::npos) c = '_';
		}
		return out;
	}

	/// handle が範囲内で、まだ消されていないか
	template<typename T>
	T* At(std::vector<std::unique_ptr<T>>& list, int handle) {
		return (handle >= 0 && handle < (int)list.size()) ? list[handle].get() : nullptr;
	}

	template<typename T>
	std::vector<int> Alive(const std::vector<std::unique_ptr<T>>& list) {
		std::vector<int> out;
		for (int i = 0; i < (int)list.size(); ++i) if (list[i]) out.push_back(i);
		return out;
	}

	/// path が「base.json」か「base_数字.json」か（= もう名前どおりのファイル名）
	bool IsNameOfPath(const std::string& path, const std::string& base) {
		if (path == base + ".json") return true;
		const std::string head = base + "_";
		if (path.rfind(head, 0) != 0 || path.size() <= head.size() + 5) return false;
		const std::string num = path.substr(head.size(), path.size() - head.size() - 5);
		return path.compare(path.size() - 5, 5, ".json") == 0 &&
			std::all_of(num.begin(), num.end(), [](unsigned char ch) { return std::isdigit(ch); });
	}

	template<typename T>
	int FindByName(const std::vector<std::unique_ptr<T>>& list, const std::string& name) {
		for (int i = 0; i < (int)list.size(); ++i) if (list[i] && list[i]->name == name) return i;
		return -1;
	}

	/// MapMaker の Variables の名前（= ファイル名）
	std::string VariablesNameOf(const std::string& makerName) { return makerName + "_variables"; }
	/// 保存先フォルダ：共用 Variables だけ Shared の下
	const char* FolderFor(const MapVariablesAsset& a, const char* def) { return a.shared ? MapMakerStore::kSharedFolder : def; }
	template<class T> const char* FolderFor(const T&, const char* def) { return def; }
}

/// ============================== プロジェクト ============================== ///

void MapMakerStore::Clear() {
	makers_.clear();
	functions_.clear();
	variables_.clear();
	makerPathMap_.clear();
	functionPathMap_.clear();
	variablesPathMap_.clear();
	legacyFunctionVars_.clear();
	log_.clear();
}

bool MapMakerStore::LoadProject(const std::string& mmprojPath) {
	nlohmann::json project;
	if (!ReadJson(mmprojPath, project)) {
		log_.push_back("LoadProject failed: " + mmprojPath);
		return false;
	}
	Clear();
	root_ = fs::path(mmprojPath).parent_path().string();
	ReadFromProject(project);
	return true;
}

void MapMakerStore::ReadFromProject(const nlohmann::json& project) {

	/// 共用 Variables（どの MapMaker にも掛けていない物も残す）
	for (const auto& p : project.value("sharedVariables", nlohmann::json::array())) if (p.is_string()) LoadSharedVariables(p.get<std::string>());
	/// Function → MapMaker の順（MapMaker が自分の Variables を読む。"variables" の一覧は確認用なので読まない）
	for (const auto& p : project.value("functions", nlohmann::json::array())) if (p.is_string()) LoadFunction(p.get<std::string>());
	for (const auto& p : project.value("mapMaker", nlohmann::json::array())) if (p.is_string()) LoadMaker(p.get<std::string>());
}

bool MapMakerStore::WriteToProject(nlohmann::json& project, bool onlyUnsaved) {
	std::vector<std::string> oldFiles;

	/// 1. どの MapMaker も持っていない Variables（古い形式から変換した残り）→ 一覧から外して、ファイルも消す
	std::set<int> owned;
	for (const auto& m : makers_) if (m) owned.insert(m->variablesHandle);
	for (int h = 0; h < (int)variables_.size(); ++h) {
		if (!variables_[h] || owned.count(h) || variables_[h]->shared) continue;
		oldFiles.push_back(variables_[h]->path);
		variablesPathMap_.erase(variables_[h]->path);
		variables_[h].reset();
	}

	/// 2. ファイル名を名前・フォルダに合わせる
	SyncFileNames(oldFiles);

	std::error_code ec;
	for (const char* folder : { kMakerFolder, kFunctionFolder, kVariablesFolder, kSharedFolder }) fs::create_directories(fs::path(root_) / folder, ec);

	bool ok = true;
	project["variables"] = nlohmann::json::array();
	project["functions"] = nlohmann::json::array();
	project["mapMaker"] = nlohmann::json::array();

	/// 3. 書く → 一覧に入れる（どれも同じ流れ）
	auto save = [&](auto& list, const char* key, auto toJson) {
		for (auto& a : list) {
			if (!a) continue;
			if (!onlyUnsaved || !a->isSaved) {
				if (WriteJson(FullPath(a->path), (this->*toJson)(*a))) {
					a->isSaved = true;
				} else {
					log_.push_back("Save failed: " + a->path);
					ok = false;
				}
			}
			project[key].push_back(a->path);
		}
		};
	save(variables_, "variables", &MapMakerStore::VariablesToJson);
	save(functions_, "functions", &MapMakerStore::FunctionToJson);
	save(makers_, "mapMaker", &MapMakerStore::MakerToJson);

	project["sharedVariables"] = nlohmann::json::array();
	for (int h : SharedVariablesHandles()) project["sharedVariables"].push_back(variables_[h]->path);

	/// 4. 全部書けたら古いファイルを消す（今使っているパスは消さない）
	if (ok) {
		for (const auto& old : oldFiles) {
			if (makerPathMap_.count(old) || functionPathMap_.count(old) || variablesPathMap_.count(old)) continue;
			if (old.rfind("MapMakerData/", 0) != 0) continue;   // プロジェクトの外（Preset・別プロジェクト）は消さない＝コピー扱い
			fs::remove(FullPath(old), ec);
		}
	}
	return ok;
}

/// ============================== 読み込み ============================== ///

int MapMakerStore::LoadVariables(const std::string& relPath) {
	const std::string key = NormalizePath(relPath);
	if (key.empty()) return -1;
	if (auto it = variablesPathMap_.find(key); it != variablesPathMap_.end()) return it->second;   // 読み込み済み

	nlohmann::json file;
	if (!ReadJson(FullPath(key), file)) {
		log_.push_back("Variables not found: " + key);
		return -1;
	}
	auto a = std::make_unique<MapVariablesAsset>();
	a->path = key;
	a->name = file.value("name", fs::path(key).stem().string());
	a->vars = MapProgramAdapter::JsonToVariables(file);
	a->shared = file.value("shared", false);

	const int handle = (int)variables_.size();
	variables_.push_back(std::move(a));
	variablesPathMap_[key] = handle;
	return handle;
}

int MapMakerStore::LoadFunction(const std::string& relPath) {
	const std::string key = NormalizePath(relPath);
	if (key.empty()) return -1;
	if (auto it = functionPathMap_.find(key); it != functionPathMap_.end()) return it->second;

	nlohmann::json file;
	if (!ReadJson(FullPath(key), file)) {
		log_.push_back("Function not found: " + key);
		return -1;
	}
	auto a = std::make_unique<MapFunctionAsset>();
	a->path = key;
	a->name = file.value("name", fs::path(key).stem().string());
	a->code = file.value("code", std::string());
	a->script = file.value("script", nlohmann::json::array());

	const int handle = (int)functions_.size();
	functions_.push_back(std::move(a));
	functionPathMap_[key] = handle;

	/// 少し前の形式（Function が Variables を持っていた）→ 覚えておいて、MapMaker を読む時にまとめる
	const std::string varsPath = file.value("variables", std::string());
	if (!varsPath.empty()) {
		const int vh = LoadVariables(varsPath);
		if (vh >= 0) {
			legacyFunctionVars_[handle] = vh;
			functions_[handle]->isSaved = false;             // 次の保存で "variables" の無い形にする
		}
	}
	/// AST が無くて腳本だけある（手で書いたファイルなど）→ 作る
	if (functions_[handle]->script.empty() && !functions_[handle]->code.empty()) {
		const bool saved = functions_[handle]->isSaved;
		SetFunctionCode(handle, functions_[handle]->code);
		functions_[handle]->isSaved = saved;
	}
	return handle;
}

/// ============================== 共用 Variables ============================== ///

int MapMakerStore::CreateSharedVariables(const std::string& name) {
	auto a = std::make_unique<MapVariablesAsset>();
	a->path = MakeUniquePath(kSharedFolder, name);
	a->name = name;
	a->shared = true;
	a->isSaved = false;
	const int handle = (int)variables_.size();
	variablesPathMap_[a->path] = handle;
	variables_.push_back(std::move(a));
	return handle;
}

int MapMakerStore::LoadSharedVariables(const std::string& relPath) {
	const int h = LoadVariables(relPath);           // 同じファイルなら同じ handle
	if (MapVariablesAsset* v = GetVariables(h)) v->shared = true;
	return h;
}

void MapMakerStore::RemoveSharedVariables(int handle) {
	MapVariablesAsset* v = GetSharedVariables(handle);
	if (!v) return;
	for (auto& m : makers_) if (m) {
		auto& s = m->sharedVariables;
		const size_t before = s.size();
		s.erase(std::remove(s.begin(), s.end(), handle), s.end());
		if (s.size() != before) m->isSaved = false;
	}
	variablesPathMap_.erase(v->path);
	variables_[handle].reset();
}

void MapMakerStore::AttachShared(int makerHandle, int varsHandle) {
	MapMakerAsset* m = GetMaker(makerHandle);
	if (!m || !GetSharedVariables(varsHandle)) return;
	if (std::find(m->sharedVariables.begin(), m->sharedVariables.end(), varsHandle) != m->sharedVariables.end()) return;   // 二重に掛けない
	m->sharedVariables.push_back(varsHandle);
	m->isSaved = false;
}

void MapMakerStore::DetachShared(int makerHandle, int varsHandle) {
	MapMakerAsset* m = GetMaker(makerHandle);
	if (!m) return;
	auto& s = m->sharedVariables;
	s.erase(std::remove(s.begin(), s.end(), varsHandle), s.end());
	m->isSaved = false;
}

std::vector<int> MapMakerStore::SharedVariablesHandles() const {
	std::vector<int> out;
	for (int i = 0; i < (int)variables_.size(); ++i) if (variables_[i] && variables_[i]->shared) out.push_back(i);
	return out;
}

std::vector<int> MapMakerStore::MakersUsingVariables(int varsHandle) const {
	std::vector<int> out;
	for (int i = 0; i < (int)makers_.size(); ++i) {
		if (!makers_[i]) continue;
		const auto& s = makers_[i]->sharedVariables;
		if (std::find(s.begin(), s.end(), varsHandle) != s.end()) out.push_back(i);
	}
	return out;
}

MapVariablesAsset* MapMakerStore::GetSharedVariables(int handle) {
	MapVariablesAsset* v = GetVariables(handle);
	return (v && v->shared) ? v : nullptr;
}

GeneratorVariable MapMakerStore::BuildRunVariables(int makerHandle) {
	GeneratorVariable run;
	MapMakerAsset* m = GetMaker(makerHandle);
	if (!m) return run;
	if (MapVariablesAsset* own = GetVariables(m->variablesHandle)) run = own->vars;   // 自分が一番優先
	for (int h : m->sharedVariables) {
		if (MapVariablesAsset* s = GetSharedVariables(h)) MergeVariables(run, s->vars); // 同じ名前は先にある方を残す
	}
	return run;
}

int MapMakerStore::LoadMaker(const std::string& relPath) {
	const std::string key = NormalizePath(relPath);
	if (key.empty()) return -1;
	if (auto it = makerPathMap_.find(key); it != makerPathMap_.end()) return it->second;

	nlohmann::json file;
	if (!ReadJson(FullPath(key), file)) {
		log_.push_back("MapMaker not found: " + key);
		return -1;
	}

	/// 古い形式（腳本・変数がファイルの中にある）→ 分けて作り直す
	const nlohmann::json funcs = file.value("functions", nlohmann::json::array());
	const bool embedded = file.contains("script") || file.contains("code") ||
		std::any_of(funcs.begin(), funcs.end(), [](const nlohmann::json& f) { return !f.contains("file"); });
	if (embedded) return ImportOldMaker(file, key);

	auto a = std::make_unique<MapMakerAsset>();
	a->path = key;
	a->name = file.value("name", fs::path(key).stem().string());
	const nlohmann::json ctx = file.value("context", nlohmann::json::object());
	a->targetMapName = ctx.value("targetMap", std::string());
	a->width = ctx.value("width", 32);
	a->height = ctx.value("height", 32);
	a->seed = ctx.value("seed", 0);
	/// MapMaker → Function（ここで一緒に読む）
	for (const auto& f : funcs) {
		MapMakerAsset::Entry e;
		e.functionHandle = LoadFunction(f.value("file", std::string()));
		e.enabled = f.value("enabled", true);
		if (e.functionHandle >= 0) a->functions.push_back(e);
	}

	/// MapMaker → 自分の Variables
	a->variablesHandle = LoadVariables(file.value("variables", std::string()));
	if (a->variablesHandle < 0) {
		/// 少し前の形式：使っている函數が持っていた Variables をまとめて、この MapMaker 専用にする
		a->variablesHandle = CreateVariables(VariablesNameOf(a->name));
		for (const auto& e : a->functions) {
			if (auto it = legacyFunctionVars_.find(e.functionHandle); it != legacyFunctionVars_.end()) {
				if (MapVariablesAsset* old = GetVariables(it->second)) MergeVariables(variables_[a->variablesHandle]->vars, old->vars);
			}
		}
		a->isSaved = false;
	}

	/// MapMaker → 共用 Variables
	for (const auto& p : file.value("sharedVariables", nlohmann::json::array())) {
		if (!p.is_string()) continue;
		const int sh = LoadSharedVariables(p.get<std::string>());
		if (sh >= 0) a->sharedVariables.push_back(sh);
	}

	const int handle = (int)makers_.size();
	makers_.push_back(std::move(a));
	makerPathMap_[key] = handle;
	return handle;
}

int MapMakerStore::ImportOldMaker(const nlohmann::json& file, const std::string& relPath) {
	const std::string name = file.value("name", fs::path(relPath).stem().string());

	auto a = std::make_unique<MapMakerAsset>();
	a->path = relPath;                       // 次の保存で新しい場所・形式になる
	a->name = name;
	const nlohmann::json ctx = file.value("context", nlohmann::json::object());
	a->targetMapName = ctx.value("targetMap", std::string());
	a->width = ctx.value("width", ctx.value("maxCol", 32));
	a->height = ctx.value("height", ctx.value("maxRow", 32));
	a->seed = ctx.value("seed", 0);
	a->isSaved = false;

	/// 変数 → この MapMaker 専用の Variables
	a->variablesHandle = CreateVariables(VariablesNameOf(name));
	variables_[a->variablesHandle]->vars = MapProgramAdapter::JsonToVariables(file);

	/// 函數 → Function ファイル 1 つずつ（ファイル名 = 函數名。同じ名前があれば _1, _2 …）
	auto addFunction = [&](const std::string& fname, const std::string& code, const nlohmann::json& script, bool enabled) {
		const int fh = CreateFunction(fname);
		functions_[fh]->code = code;
		functions_[fh]->script = script;
		if (script.empty() && !code.empty()) SetFunctionCode(fh, code);
		a->functions.push_back({ fh, enabled });
		};
	if (file.contains("functions")) {
		for (const auto& f : file["functions"]) {
			addFunction(f.value("name", std::string("Function")), f.value("code", std::string()),
				f.value("script", nlohmann::json::array()), f.value("enabled", true));
		}
	} else {
		addFunction("Main", file.value("code", std::string()), file.value("script", nlohmann::json::array()), true);
	}

	const int handle = (int)makers_.size();
	makers_.push_back(std::move(a));
	makerPathMap_[relPath] = handle;
	log_.push_back("Converted old MapMaker: " + relPath);
	return handle;
}

void MapMakerStore::MergeVariables(GeneratorVariable& into, const GeneratorVariable& from) {
	for (const auto& v : from.variableList) {
		if (!into.FindVariable(v.name)) into.variableList.push_back(v);
	}
}

/// ============================== 作る ============================== ///

int MapMakerStore::CreateVariables(const std::string& name) {
	auto a = std::make_unique<MapVariablesAsset>();
	a->path = MakeUniquePath(kVariablesFolder, name);
	a->name = name;
	a->isSaved = false;
	const int handle = (int)variables_.size();
	variablesPathMap_[a->path] = handle;
	variables_.push_back(std::move(a));
	return handle;
}

int MapMakerStore::CreateFunction(const std::string& name) {
	auto a = std::make_unique<MapFunctionAsset>();
	a->path = MakeUniquePath(kFunctionFolder, name);
	a->name = name;
	a->isSaved = false;
	const int handle = (int)functions_.size();
	functionPathMap_[a->path] = handle;
	functions_.push_back(std::move(a));
	return handle;
}

int MapMakerStore::CreateMaker(const std::string& name) {
	auto a = std::make_unique<MapMakerAsset>();
	a->path = MakeUniquePath(kMakerFolder, name);
	a->name = name;
	a->isSaved = false;
	const int handle = (int)makers_.size();
	makerPathMap_[a->path] = handle;
	makers_.push_back(std::move(a));
	makers_[handle]->variablesHandle = CreateVariables(VariablesNameOf(name));   // 自分の Variables
	return handle;
}

int MapMakerStore::DuplicateMaker(int handle) {
	MapMakerAsset* src = GetMaker(handle);
	if (!src) return -1;
	const MapMakerAsset copy = *src;                          // 函數の並び・生成設定はそのまま（函數は共有）
	const int h = CreateMaker(src->name + "_copy");           // 新しいパス・新しい Variables
	MapMakerAsset& dst = *makers_[h];
	dst.functions = copy.functions;
	dst.targetMapName = copy.targetMapName;
	dst.width = copy.width;
	dst.height = copy.height;
	dst.seed = copy.seed;
	dst.sharedVariables = copy.sharedVariables;
	/// Variables は中身をコピー（別ファイル。直しても元には効かない）
	if (MapVariablesAsset* v = GetVariables(copy.variablesHandle)) variables_[dst.variablesHandle]->vars = v->vars;
	return h;
}

int MapMakerStore::DuplicateFunction(int handle) {
	MapFunctionAsset* src = GetFunction(handle);
	if (!src) return -1;
	const int h = CreateFunction(src->name + "_copy");
	functions_[h]->code = src->code;
	functions_[h]->script = src->script;
	return h;
}

/// ============================== 消す ============================== ///

void MapMakerStore::RemoveMaker(int handle) {
	MapMakerAsset* m = GetMaker(handle);
	if (!m) return;
	/// 自分の Variables も一緒に（他からは使われない）
	if (MapVariablesAsset* v = GetVariables(m->variablesHandle)) {
		variablesPathMap_.erase(v->path);
		variables_[m->variablesHandle].reset();
	}
	makerPathMap_.erase(m->path);
	makers_[handle].reset();
}

void MapMakerStore::RemoveFunction(int handle) {
	auto* f = GetFunction(handle);
	if (!f) return;
	/// 使っている MapMaker から外す
	for (auto& m : makers_) {
		if (!m) continue;
		const size_t before = m->functions.size();
		m->functions.erase(std::remove_if(m->functions.begin(), m->functions.end(),
			[&](const MapMakerAsset::Entry& e) { return e.functionHandle == handle; }), m->functions.end());
		if (m->functions.size() != before) m->isSaved = false;
	}
	functionPathMap_.erase(f->path);
	functions_[handle].reset();
}

std::vector<int> MapMakerStore::MakersUsingFunction(int functionHandle) const {
	std::vector<int> out;
	for (int i = 0; i < (int)makers_.size(); ++i) {
		if (!makers_[i]) continue;
		for (const auto& e : makers_[i]->functions) {
			if (e.functionHandle == functionHandle) { out.push_back(i); break; }
		}
	}
	return out;
}

/// ============================== 取得 ============================== ///

MapMakerAsset* MapMakerStore::GetMaker(int handle) { return At(makers_, handle); }
MapFunctionAsset* MapMakerStore::GetFunction(int handle) { return At(functions_, handle); }
MapVariablesAsset* MapMakerStore::GetVariables(int handle) { return At(variables_, handle); }

MapVariablesAsset* MapMakerStore::GetMakerVariables(int makerHandle) {
	MapMakerAsset* m = GetMaker(makerHandle);
	if (!m) return nullptr;
	if (!GetVariables(m->variablesHandle)) {
		m->variablesHandle = CreateVariables(VariablesNameOf(m->name));   // 無ければ作る
		m->isSaved = false;
	}
	return GetVariables(m->variablesHandle);
}

std::vector<int> MapMakerStore::MakerHandles() const { return Alive(makers_); }
std::vector<int> MapMakerStore::FunctionHandles() const { return Alive(functions_); }

int MapMakerStore::FindMaker(const std::string& name) const { return FindByName(makers_, name); }
int MapMakerStore::FindFunction(const std::string& name) const { return FindByName(functions_, name); }

/// ============================== 編集 ============================== ///

void MapMakerStore::SetFunctionCode(int handle, const std::string& code) {
	auto* f = GetFunction(handle);
	if (!f) return;
	f->code = code;
	/// 腳本 → AST → json（AST はここだけで使ってすぐ消す）
	std::vector<Node*> nodes = ParseProgram(BuildSentences(Tokenize(code)), nullptr);
	f->script = MapProgramAdapter::BodyToJson(nodes);
	for (auto* n : nodes) delete n;
	f->isSaved = false;
}

bool MapMakerStore::HasUnsaved() const {
	for (const auto& m : makers_) if (m && !m->isSaved) return true;
	for (const auto& f : functions_) if (f && !f->isSaved) return true;
	for (const auto& v : variables_) if (v && !v->isSaved) return true;
	return false;
}

/// ============================== 生成 ============================== ///

TileMapData MapMakerStore::Generate(int makerHandle, int blankId, int seed, std::vector<std::string>* outErrors) {
	MapMakerAsset* m = GetMaker(makerHandle);
	if (!m) {
		if (outErrors) *outErrors = { "Generate: MapMaker not found (handle " + std::to_string(makerHandle) + ")" };
		return TileMapData();
	}

	std::vector<std::vector<Node*>> layers;
	std::vector<std::string> names;
	for (const auto& e : m->functions) {
		if (!e.enabled) continue;
		MapFunctionAsset* f = GetFunction(e.functionHandle);
		if (!f) continue;
		layers.push_back(MapProgramAdapter::LoadAstFromJson(f->script));
		names.push_back(f->name);
	}

	/// 変数は実行用のコピー（全部の函數で共有。元のファイルは変わらない）
	GeneratorVariable runVars = BuildRunVariables(makerHandle);

	TileMapData map = MapProgramAdapter::GenerateUnit(layers, names, m->width, m->height,
		seed >= 0 ? seed : m->seed, blankId, &runVars, outErrors);
	map.name = m->name;

	for (auto& layer : layers) for (auto* n : layer) delete n;
	return map;
}

/// ============================== json ============================== ///

nlohmann::json MapMakerStore::MakerToJson(const MapMakerAsset& m) const {
	nlohmann::json j;
	j["kind"] = "MapMaker";
	j["name"] = m.name;
	j["context"] = { { "targetMap", m.targetMapName },{ "width", m.width },{ "height", m.height },{ "seed", m.seed } };
	j["functions"] = nlohmann::json::array();
	for (const auto& e : m.functions) {
		if (e.functionHandle < 0 || e.functionHandle >= (int)functions_.size() || !functions_[e.functionHandle]) continue;
		j["functions"].push_back({ { "file", functions_[e.functionHandle]->path },{ "enabled", e.enabled } });
	}
	const bool hasVars = m.variablesHandle >= 0 && m.variablesHandle < (int)variables_.size() && variables_[m.variablesHandle];
	j["variables"] = hasVars ? variables_[m.variablesHandle]->path : std::string();
	j["sharedVariables"] = nlohmann::json::array();
	for (int h : m.sharedVariables) {
		if (h < 0 || h >= (int)variables_.size() || !variables_[h]) continue;
		j["sharedVariables"].push_back(variables_[h]->path);
	}
	return j;
}

nlohmann::json MapMakerStore::FunctionToJson(const MapFunctionAsset& f) const {
	nlohmann::json j;
	j["kind"] = "MapFunction";
	j["name"] = f.name;
	j["code"] = f.code;
	j["script"] = f.script;
	return j;
}

nlohmann::json MapMakerStore::VariablesToJson(const MapVariablesAsset& v) const {
	nlohmann::json j;
	j["kind"] = "MapVariables";
	j["name"] = v.name;
	j["variables"] = MapProgramAdapter::VariablesToJson(v.vars);
	j["shared"] = v.shared;
	return j;
}

/// ============================== パス ============================== ///

std::string MapMakerStore::FullPath(const std::string& relPath) const {
	return root_.empty() ? relPath : (fs::path(root_) / relPath).string();
}

std::string MapMakerStore::NormalizePath(const std::string& relPath) {
	std::string p = relPath;
	std::replace(p.begin(), p.end(), '\\', '/');
	while (p.rfind("./", 0) == 0) p.erase(0, 2);
	return p;
}

bool MapMakerStore::PathInUse(const std::string& relPath) const {
	if (makerPathMap_.count(relPath) || functionPathMap_.count(relPath) || variablesPathMap_.count(relPath)) return true;
	std::error_code ec;
	return !root_.empty() && fs::exists(FullPath(relPath), ec);   // 読んでいないファイルも上書きしない
}

std::string MapMakerStore::MakeUniquePath(const std::string& folder, const std::string& name) const {
	const std::string base = folder + "/" + SafeFileName(name);
	std::string path = base + ".json";
	for (int i = 1; PathInUse(path); ++i) path = base + "_" + std::to_string(i) + ".json";
	return path;
}

void MapMakerStore::SyncFileNames(std::vector<std::string>& oldFiles) {
	/// MapMaker の Variables の名前は「MapMaker名_variables」に揃える
	for (auto& m : makers_) {
		if (!m) continue;
		if (MapVariablesAsset* v = GetVariables(m->variablesHandle)) v->name = VariablesNameOf(m->name);
	}

	bool moved = false;
	auto sync = [&](auto& list, auto& pathMap, const char* folder) {
		for (int h = 0; h < (int)list.size(); ++h) {
			auto& a = list[h];
			if (!a) continue;

			const char* dir = FolderFor(*a, folder);              // 共用 Variables は Shared の下
			const std::string base = std::string(dir) + "/" + SafeFileName(a->name);
			if (IsNameOfPath(a->path, base)) continue;            // もう名前どおり
			const std::string newPath = MakeUniquePath(dir, a->name);

			pathMap.erase(a->path);
			oldFiles.push_back(a->path);
			a->path = newPath;
			pathMap[newPath] = h;
			a->isSaved = false;
			moved = true;
		}
		};
	sync(variables_, variablesPathMap_, kVariablesFolder);
	sync(functions_, functionPathMap_, kFunctionFolder);
	sync(makers_, makerPathMap_, kMakerFolder);

	/// 参照はメモリでは handle なので、書く時に新しいパスが自動で入る（参照している MapMaker も書き直す）
	if (moved) for (auto& m : makers_) if (m) m->isSaved = false;
}

bool MapMakerStore::ReadJson(const std::string& fullPath, nlohmann::json& out) {
	std::ifstream in(fullPath);
	if (!in.is_open()) return false;
	out = nlohmann::json::parse(in, nullptr, false);   // 壊れた json でも例外を投げない
	return !out.is_discarded();
}

bool MapMakerStore::WriteJson(const std::string& fullPath, const nlohmann::json& j) {
	std::ofstream out(fullPath);
	if (!out.is_open()) return false;
	out << j.dump(4);
	return true;
}
