#include "Particle.h"


void Particle::IntParticele(kEngine* system) {
	Object::IntObject(system);
	commonMaterialConfig = std::make_shared<MaterialConfig>();
	InitMaterialConfig(commonMaterialConfig.get());

	createTimer.Init0(defaultParticleInterval_, system_->GetTimeManager());
}

Particle::~Particle() {
	commonMaterialConfig.reset();
}

void Particle::Update(Camera* camera) {
	Object::Update(camera);
}

void Particle::Draw() {
	for (auto& obj : particleObjectList_) {
		obj->Draw();
	}
}

void Particle::SetCommonMaterialConfig(const MaterialConfig& material) {
	*commonMaterialConfig = material;
}

void Particle::SetRoot(Vector3 pos) {
	mainPosition.transform.translate = pos;
}

void Particle::SetSeed(unsigned int seed) {
	randomMaker_->SetSeed(seed);
}

void Particle::SetTimer(float maxTime) {
	createTimer.maxTime_ = maxTime;
	if (createTimer.parameter_ > maxTime)createTimer.parameter_ = maxTime;
}
