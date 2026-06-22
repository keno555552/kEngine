#include "Bullet.h"
#include "kEngine.h"
#include "core/config.h"
#include <algorithm>
#include <cmath>
#include "ImguiManager.h"
#include "EngineAssets/Particle/HitSparkWithFellRock.h"

// 定義 pi 常數
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Bullet::Bullet(kEngine* system, const Vector3& position, const Vector3& direction) {

	IntObject(system);
	MH_bullet_ = system_->SetModelObj("GAME/resources/object/bullet/bullet.obj");
	whiteTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/white5x5.png");
	clicleTextureHandle_ = system_->LoadTexture("./kEngine/EngineAssets/TemplateResource/texture/circle_withAlpha.png");
	effectTextureHandle_ = system_->LoadTexture("./GAME/resources/texture/gradationLine.png");

	CreateModelData(MH_bullet_);
	mainPosition.transform.translate = position;
	mainPosition.transform.scale = { 0.4f,0.4f,0.4f };
	objectParts_[0].materialConfig->lightModelType = LightModelType::FlameNeonGlow;
	objectParts_[0].materialConfig->renderModelType = RenderModelType::FlameNeonGlow;
	objectParts_[0].materialConfig->textureColor = ColorTo1({ 224,238,8,255 });
	objectParts_[0].materialConfig->textureColor2 = ColorTo1({ 225,0,0,255 });
	objectParts_[0].materialConfig->textureColor3 = ColorTo1({ 225,248,248,255 });
	objectParts_[0].materialConfig->intensity = 1.587f;
	objectParts_[0].materialConfig->heightScale = 1.783f;
	direction_ = Normalize(direction);

	// orient bullet in 2D: forward (+Y) rotates only around Z to face move direction
	if (!(direction_.x == 0.0f && direction_.y == 0.0f)) {
		float yaw = std::atan2(direction_.x, direction_.y); // zero when facing +Y
		mainPosition.transform.rotate.z = yaw;
		mainPosition.transform.rotate.x = 0.0f;
		mainPosition.transform.rotate.y = 0.0f;
	}


}

Bullet::~Bullet() {
}

void Bullet::Update(Camera* camera) {
	float dt = system_->GetDeltaTime();
	mainPosition.transform.translate += direction_ * speed_ * dt;
}

void Bullet::HitRockEffect() {

	// エフェクトの準備
	HitSpark3 hitSpark;
	hitSpark.startPosition = mainPosition.transform.translate;
	hitSpark.objectList[0].objectParts_[0].materialConfig->textureHandle = clicleTextureHandle_;
	particleHandle_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitSpark, 0);

	HitImpact3 hitImpact;
	hitImpact.startPosition = mainPosition.transform.translate;
	hitImpact.objectList[0].objectParts_[0].materialConfig->textureHandle = effectTextureHandle_;
	int particleHandle2_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitImpact, 1);

	HitRock hitRock;
	hitRock.startPosition = mainPosition.transform.translate;
	float velocityScale = 2.0f;
	hitRock.startVelocity = {-direction_.x * velocityScale,
							 -direction_.y * velocityScale * 1.5f,
							 -direction_.z * velocityScale};
	hitRock.objectList[0].objectParts_[0].materialConfig->textureHandle = whiteTextureHandle_;
	int particleHandle3_ = system_->GetEffectManager()->GetParticleManager()->CreateEmitter(hitRock, 0);

	HitSpackImpactLink3 linkData;
	linkData.sourceId = particleHandle_;
	linkData.targetId = particleHandle2_;
	system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData);

	HitSpackImpactLink3_2 linkData2;
	linkData2.sourceId = particleHandle_;
	linkData2.targetId = particleHandle3_;
	system_->GetEffectManager()->GetParticleManager()->LinkEmitterToEmitter(linkData2);

	system_->GetEffectManager()->GetParticleManager()->ShootEmitter(particleHandle_, 1);

}