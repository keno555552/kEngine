#include "EffectManager.h"
#include <filesystem>
#include <algorithm>
#include "kEngine.h"
#include "Tool/EffectSystem/Adapter/EffectAdapter.h"

namespace {
	constexpr int kMaxParticles = 1000;
}

void EffectManager::Initialize(kEngine* system) {
	system_ = system;
	particleManager_ = std::make_unique<ParticleManager>(system);
}

void EffectManager::Finalize() {}

void EffectManager::Update() {
	particleManager_->Update();
}

void EffectManager::Draw() {
	particleManager_->Draw();
}

/// ======================================= 登録 ======================================= ///

int EffectManager::LoadEffect(const std::string& path) {
	namespace fs = std::filesystem;

	/// 同じファイル → 同じハンドル
	for (int i = 0; i < (int)effects_.size(); i++) {
		if (!effects_[i].isActive || effects_[i].path.empty()) continue;
		std::error_code ec;
		if (fs::equivalent(effects_[i].path, path, ec)) return i;
	}

	EffectAdapter adapter(system_);
	EffectData data;
	std::string log;
	const bool ok = adapter.LoadEffect(path, data, &log);
	if (!log.empty()) Logger::Log("[EffectManager] " + log);
	if (!ok) return -1;

	const int handle = RegisterEffect(path, data);
	effects_[handle].path = path;
	return handle;
}

int EffectManager::RegisterEffect(const std::string& key, const EffectData& data) {

	/// 同じ key → 中身だけ更新
	int handle = FindByKey(key);
	if (handle != -1) {
		ApplyEffect(effects_[handle], data);
		return handle;
	}

	/// 新規
	handle = (int)effects_.size();
	LoadedEffect e;
	e.isActive = true;
	e.key = key;
	/// 頭にハンドル番号を付ける（違うファイルに同じ名前の Effect があってもぶつからない）
	e.prefix = std::to_string(handle) + ":" + data.name + "/";
	effects_.push_back(e);

	ApplyEffect(effects_[handle], data);
	return handle;
}

void EffectManager::ApplyEffect(LoadedEffect& e, const EffectData& data) {

	/// --- 線：同名は SetPrototype（飛んでいる粒子はそのまま）、新しい名前は新規 ---
	std::vector<std::pair<std::string, int>> newLines;
	std::vector<int> newRoots;
	for (auto proto : data.prototypes) {               // コピーして名前だけ変える
		const std::string original = proto.name;
		const bool isRoot = proto.isRoot;
		proto.name = e.prefix + original;
		const int id = particleManager_->CreateEmitter(proto, kMaxParticles);
		particleManager_->PauseEmitter(id, e.isPaused);  // 一時停止中なら新しい線も止める

		newLines.push_back({ original, id });
		if (isRoot) newRoots.push_back(id);
	}

	/// 前にあって今回無い線 → 引退（名前を変えた・消した）
	for (const auto& [name, oldId] : e.lines) {
		const bool stillUsed = std::any_of(newLines.begin(), newLines.end(),
			[&](const auto& l) { return l.second == oldId; });
		if (!stillUsed) particleManager_->RetireEmitter(oldId);
	}

	/// --- Link：同名は上書き、無くなった物は削除 ---
	std::vector<std::string> newLinks;
	for (auto link : data.links) {
		link.name = e.prefix + link.name;
		link.sourceName = e.prefix + link.sourceName;
		link.targetName = e.prefix + link.targetName;
		particleManager_->LinkEmitterToEmitter(link);
		newLinks.push_back(link.name);
	}
	for (const auto& oldName : e.linkNames) {
		if (std::find(newLinks.begin(), newLinks.end(), oldName) == newLinks.end()) {
			particleManager_->RemoveEmitterLink(oldName);
		}
	}

	e.lines = newLines;
	e.rootIds = newRoots;
	e.linkNames = newLinks;
}

void EffectManager::ReloadAll() {
	EffectAdapter adapter(system_);
	for (auto& e : effects_) {
		if (!e.isActive || e.path.empty()) continue;

		EffectData data;
		std::string log;
		const bool ok = adapter.LoadEffect(e.path, data, &log);
		if (!log.empty()) Logger::Log("[EffectManager] " + log);
		if (ok) ApplyEffect(e, data);   // 読めなかったら前のまま
	}
}

void EffectManager::UnloadEffect(int handle) {
	if (!IsValid(handle)) return;
	auto& e = effects_[handle];
	for (const auto& [name, id] : e.lines) particleManager_->RetireEmitter(id);
	for (const auto& linkName : e.linkNames) particleManager_->RemoveEmitterLink(linkName);
	e = LoadedEffect{};   // isActive = false
}

/// ======================================= 使う ======================================= ///

void EffectManager::Play(int handle, const Vector3& position) {
	if (!IsValid(handle)) {
		Logger::Log("[kError] EffectManager::Play() invalid handle: " + std::to_string(handle));
		return;
	}
	for (int id : effects_[handle].rootIds) {
		particleManager_->SetEmitterPosition(id, position);
		particleManager_->ShootEmitter(id, 1);
	}
}

void EffectManager::PauseEffect(int handle, bool pause) {
	if (!IsValid(handle)) return;
	auto& e = effects_[handle];
	e.isPaused = pause;
	for (const auto& [name, id] : e.lines) particleManager_->PauseEmitter(id, pause);
}

int EffectManager::GetLine(int handle, const std::string& particleName) const {
	if (!IsValid(handle)) return -1;
	for (const auto& [name, id] : effects_[handle].lines) {
		if (name == particleName) return id;
	}
	return -1;
}

bool EffectManager::HasRoot(int handle) const {
	return IsValid(handle) && !effects_[handle].rootIds.empty();
}

/// ======================================= 内部 ======================================= ///

int EffectManager::FindByKey(const std::string& key) const {
	for (int i = 0; i < (int)effects_.size(); i++) {
		if (effects_[i].isActive && effects_[i].key == key) return i;
	}
	return -1;
}

bool EffectManager::IsValid(int handle) const {
	return handle >= 0 && handle < (int)effects_.size() && effects_[handle].isActive;
}