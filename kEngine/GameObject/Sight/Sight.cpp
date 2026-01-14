#include "Sight.h"
#include "Player/Player.h"
#include "tool/mathsUnit/Matrix4x4.h"
#include "tool/mathsUnit/Vector4.h"
#include "tool/Camera/Camera.h"
#include "core/driver/config.h"
#include <numbers>
#include <cmath>

Sight::Sight(kEngine* system, Player* player) :
	player_(player)
{
	system_ = system;
}

Sight::~Sight() {
}

void Sight::Update(Camera* camera) {

	if (player_ != nullptr) {
		mainPosition.transform.translate = player_->mainPosition.transform.translate;
	}

	if (camera && player_) {
		// 1) 取得螢幕座標 -> NDC
		float w = static_cast<float>(config::GetClientWidth());
		float h = static_cast<float>(config::GetClientHeight());
		float xNdc = (2.0f * static_cast<float>(system_->GetMousePosX()) / w) - 1.0f;
		float yNdc = 1.0f - (2.0f * static_cast<float>(system_->GetMousePosY()) / h);

		// 2) 反變換到世界座標（投射在玩家高度的平面上）
		Matrix4x4 vp = camera->GetViewMatrix() * camera->GetProjectionMatrix();
		Matrix4x4 invVP = Inverse(vp);

		Vector4 nearClip{ xNdc, yNdc, 0.0f, 1.0f };
		Vector4 farClip{ xNdc, yNdc, 1.0f, 1.0f };

		Vector4 nearWorld = Multiply(nearClip, invVP);
		Vector4 farWorld = Multiply(farClip, invVP);
		if (nearWorld.w != 0.0f) nearWorld = nearWorld /= nearWorld.w;
		if (farWorld.w != 0.0f) farWorld = farWorld /= farWorld.w;

		Vector3 dir{ farWorld.x - nearWorld.x, farWorld.y - nearWorld.y, farWorld.z - nearWorld.z };
		// 交會到玩家所在的Z平面 (Z 為上軸)
		float planeZ = player_->mainPosition.transform.translate.z;
		if (std::abs(dir.z) > 1e-4f) {
			float t = (planeZ - nearWorld.z) / dir.z;
			Vector3 hit{
				nearWorld.x + dir.x * t,
				nearWorld.y + dir.y * t,
				planeZ
			};
			mouseOnPlane_ = hit;
			Vector3 toHit = hit - player_->mainPosition.transform.translate;
			if (std::abs(toHit.x) > 1e-4f || std::abs(toHit.y) > 1e-4f) {
				mainPosition.transform.rotate.z = std::atan2(toHit.y, toHit.x) - std::numbers::pi_v<float> / 2.0f;
			}
		}
	}

	Object::Update(camera);
}
