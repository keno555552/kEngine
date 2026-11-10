#include "Sight.h"
#include "GAME/Object/Player/Player.h"
#include "Matrix4x4.h"
#include "Vector4.h"
#include "Camera/Camera.h"
#include "core/config.h"
#include <numbers>
#include <cmath>

Sight::Sight(kEngine* system, Player* player) :
	player_(player) {
	system_ = system;
}

Sight::~Sight() {
}

void Sight::Update(Camera* camera) {

    if (player_ != nullptr) {
        mainPosition.transform.translate = player_->mainPosition.transform.translate;
    }

    float mouseX = (float)system_->GetMousePosX();
    float mouseY = (float)system_->GetMousePosY();

    Ray ray = camera->ScreenPointToRay({ mouseX, mouseY });

	Vector3 origin = ray.origin;
	Vector3 dir = ray.direction;

	// 正確的 Ray 與 Z=0 平面交點
	float t = -origin.z / dir.z;
	mouseOnPlane_ = origin + dir * t;
	angleSightToTarget_ = mouseOnPlane_;

	// 只用 2D 平面
	Vector3 playerPos = mainPosition.transform.translate;
	Vector3 toHit = { mouseOnPlane_.x - playerPos.x, mouseOnPlane_.y - playerPos.y, 0.0f };

	float angle = atan2(-toHit.y, toHit.x);

	// 模型 forward 是 +Y → 補 90 度
	angle += std::numbers::pi_v<float> / 2.0f;

	mainPosition.transform.rotate.z = angle;

}

