#include "Sight.h"
#include "Player/Player.h"
#include "tool/mathsUnit/Matrix4x4.h"
#include "tool/mathsUnit/Vector4.h"
#include "tool/Camera/Camera.h"
#include "core/driver/config.h"
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

	float w = config::GetClientWidth();
	float h = config::GetClientHeight();

	float ndcX = (mouseX / w) * 2.0f - 1.0f;
	float ndcY = 1.0f - (mouseY / h) * 2.0f;

	Vector3 dir =
		Vector3{ 1, 0, 0 } * ndcX +
		Vector3{ 0, 1, 0 } * ndcY +
		Vector3{ 0, 0, -1 } * 1.0f;   // 往前射

	dir = Normalize(dir);

	mouseOnPlane_ = camera->GetTransform().translate + dir * 10.0f; 
	mouseOnPlane_.z = 0.0f; // プレイヤーと同じ平面に投影

	Vector3 toHit = mouseOnPlane_ - mainPosition.transform.translate;
	angleSightToTarget_ = mouseOnPlane_;

	float angle = atan2(toHit.y, toHit.x);

	angle -= std::numbers::pi_v<float> / 2.0f;

	mainPosition.transform.rotate.z = angle;
}
