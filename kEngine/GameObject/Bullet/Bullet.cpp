#include "Bullet.h"
#include "core/driver/config.h"
#include <algorithm>
#include <cmath>

// 定義 pi 常數
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Bullet::Bullet(kEngine* system, const Vector3& position, const Vector3& direction) {

	IntObject(system);
	MH_bullet_ = system_->SetModelObj("resources/object/bullet/bullet.obj");

	CreateModelData(MH_bullet_);
	mainPosition.transform.translate = position;
	mainPosition.transform.scale = { 0.4f,0.4f,0.4f };
	objectParts_[0].materialConfig->lightModelType = LightModelType::FlameNeonGlow;
	objectParts_[0].materialConfig->textureColor = ColorTo1({ 224,238,8,255 });
	objectParts_[0].materialConfig->textureColor2 = ColorTo1({ 225,0,0,255 });
	objectParts_[0].materialConfig->textureColor3 = ColorTo1({ 225,248,248,255 });
	objectParts_[0].materialConfig->intensity = 1.587f;
	objectParts_[0].materialConfig->heightScale = 1.783f;
	direction_ = Normalize(direction);

	// orient bullet in 2D: forward (+Y) rotates only around Z to face move direction
	if (!(direction_.x == 0.0f && direction_.y == 0.0f)) {
		float yaw = std::atan2(direction_.x, direction_.y); // zero when facing +Y
		mainPosition.transform.rotate.z = -yaw;
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
