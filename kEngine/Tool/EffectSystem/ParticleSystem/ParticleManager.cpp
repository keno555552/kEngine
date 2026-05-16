#include "ParticleManager.h"
#include "kEngine.h"

ParticleManager::ParticleManager(kEngine* system):
	system_(system)
{}

ParticleManager::~ParticleManager() {}

void ParticleManager::Update() {

	for (auto& [id, emitter] : emitterList_) {
		emitter->Update();
	}

}

void ParticleManager::Draw() {

	for (auto& [id, emitter] : emitterList_) {
		emitter->Draw();
	}

}

int ParticleManager::CreateEmitter(const ParticlePrototype& proto, int maxParticles) {
	int id = nextEmitterId_++;
	emitterList_[id] = std::make_unique<ParticleEmitter>(system_, proto, maxParticles);
	return id;
}

void ParticleManager::ClearEmitter(int emitterId) {

	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		emitterList_.erase(emitterId);
	} else {
		Logger::Log("[kError] ParticleManager::ClearEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}

void ParticleManager::ShootEmitter(int emitterId, int count) {
	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		it->second->Emit(count);
	} else {
		Logger::Log("[kError] ParticleManager::ShootEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}

void ParticleManager::SwitchEmitter(int emitterId, bool isEmitting) {
	auto it = emitterList_.find(emitterId);
	if (it != emitterList_.end()) {
		it->second->SwitchEmitter(isEmitting);
	} else {
		Logger::Log("[kError] ParticleManager::SwitchEmitter() Emitter ID not found: " + std::to_string(emitterId));
	}
}


