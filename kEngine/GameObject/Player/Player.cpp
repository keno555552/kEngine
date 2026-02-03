#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Player.h"
#include <algorithm>
#include <numbers>
#include "Bullet/Bullet.h"

Player::Player(kEngine* system, const Vector3& position) {
	Object::IntObject(system);
	Object::CreateDefaultData();
	mainPosition.transform.translate = position;
	objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

	mainPosition.transform.rotate.y = 0.0f;

	shootCD_.Init0(kShootInterval, system_->GetTimeManager());
	damageEffectTimer_.InitM(kPlayerDamageEffectTime, system_->GetTimeManager());

	MH_playerModel_ = system_->SetModelObj("resources/object/player/player.obj");
	MH_pickaxe_ = system_->SetModelObj("resources/object/pickaxe/pickaxe.obj");

	TH_playerTexture_ = system_->LoadTexture("resources/object/player/playerUV.png");
	TH_White5x5_ = system_->LoadTexture("resources/TemplateResource/texture/white5x5.png");

	SH_JUMP_ = system_->SoundLoadSE("resources/sound/SE/player_jump.wav");
	SH_SHOOT_ = system_->SoundLoadSE("resources/sound/SE/player_attack.wav");
	SH_DAMAGE_ = system_->SoundLoadSE("resources/sound/SE/playerDamage.wav");

	nearAttackChangeTimer_.InitM(kNearAttackChangeTime, system_->GetTimeManager());
	nearAttackEffectTimer_.InitM(kNearAttackEffectTime, system_->GetTimeManager());
}

void Player::Update(Camera* camera) {
	deltaTime_ = system_->GetDeltaTime();
	BehaviorRootUpdate();

	DamageAndEffectPart();

	Object::Update(camera);
}

AABB& Player::GetAABB() {

	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = kPlayerWidth * std::abs(s.x) * 0.5f;
	float halfHeight = kPlayerHeight * std::abs(s.y) * 0.5f;
	float halfDepth = kPlayerWidth * std::abs(s.z) * 0.5f;

	hitBox_.min = mainPosition.transform.translate - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = mainPosition.transform.translate + Vector3(halfWidth, halfHeight, halfDepth);

	return hitBox_;
}

AABB& Player::GetMapChipAABB() {

	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = kPlayerWidth * s.x * kPlayerHitBoxScale;
	float halfHeight = kPlayerHeight * s.y * kPlayerHitBoxScale;
	float halfDepth = kPlayerWidth * s.z * kPlayerHitBoxScale;

	hitBox_.min = mainPosition.transform.translate - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = mainPosition.transform.translate + Vector3(halfWidth, halfHeight, halfDepth);

	return hitBox_;
}

void Player::GetDamage(float damage) {
	disableDamage_ = true;
	damageEffectTimer_.Reset0();
	HP_ -= damage;

	/// 音再生
	system_->SoundPlaySE(SH_DAMAGE_);
}

void Player::Shoot(Vector3 mousePos) {

	/// 出来ない条件に該当したら終了
	if (bulletList_ == nullptr)return;
	if (playerState_ == PlayerState::kAttark || playerState_ == PlayerState::kDamage) return;
	if (shootCD_.parameter_ != shootCD_.maxTime_)return;

	/// 弾の生成
	Vector3 shootDirection = mousePos - mainPosition.transform.translate;
	Bullet* newBullet = new Bullet(system_, mainPosition.transform.translate, Normalize(shootDirection));
	bulletList_->emplace_back(newBullet);
	shootCD_.Reset0();

	/// 音再生
	system_->SoundPlaySE(SH_SHOOT_);
}

void Player::BehaviorRootUpdate() {
	/// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	/// 移動量に速度の値をコピー
	collisionMapInfo.moveVector = velocity_;

	/// 移動関連
	Move();
	MapCollisionDecideDown(collisionMapInfo);
	MovePlayerByResult(collisionMapInfo);
	OnGroundChanger(collisionMapInfo);

	///射撃CD更新
	ShootUpdate();

	/// 
}

Vector3 Player::CornerPosition(const Vector3& center, Corner4 corner) {
	Vector3 offsetTable[(int)Corner4::kNumCorner] = {
		{+kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0}, // kRightBottom
		{-kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0}, // kLeftBottom
		{+kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0}, // kRightTop
		{-kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0}, // kLeftTop
	};

	Vector3 result;
	result.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;
	result.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;
	result.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;

	return result;
}

void Player::Move() {
	/// 移動
	// 左右移動捜索
	if (system_->GetIsPush(DIK_D) || system_->GetIsPush(DIK_A)) {
		Vector3 acceleration = {};
		if (system_->GetIsPush(DIK_D)) {
			if (velocity_.x < 0.0f) {
				// 速度と逆方向に入力中に急ブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x += kAcceleration;

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = mainPosition.transform.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		}
		if (system_->GetIsPush(DIK_A)) {
			if (velocity_.x > 0.0f) {
				// 速度と逆方向に入力中に急ブレーキ
				velocity_.x *= (1.0f - kAttenuation);
			}
			acceleration.x -= kAcceleration;
			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = mainPosition.transform.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		}

		if (system_->GetIsPush(DIK_A) && system_->GetIsPush(DIK_D)) {
			if (lrDirection_ != LRDirection::None) {
				lrDirection_ = LRDirection::None;
				turnFirstRotationY_ = mainPosition.transform.rotate.y;
				turnTimer_ = kTimeTurn;
			}
		}

		// 加速/減速
		velocity_.x += acceleration.x * deltaTime_;

		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);
	} else {
		// 非入力時は移動減衰をかける
		velocity_.x *= std::exp(-kAttenuation * deltaTime_);
		if (velocity_.x < 0.01f && velocity_.x > -0.01f) {
			velocity_.x = 0.0f;
		}
		if (lrDirection_ != LRDirection::None) {
			lrDirection_ = LRDirection::None;
			turnFirstRotationY_ = mainPosition.transform.rotate.y;
			turnTimer_ = kTimeTurn;
		}
	}

	// 回転制御
	if (turnTimer_ > 0.0f) {
		turnTimer_ -= deltaTime_;
		turnTimer_ = std::max(turnTimer_, 0.0f);

		float destinationRotationYTable[] = {
			-std::numbers::pi_v<float> / 2.0f,                            // 右
			std::numbers::pi_v<float> / 2.0f,                             // 左
			0.0f // なし
		};
		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float ratio = turnTimer_ / kTimeTurn; // 1 -> 0
		// 自キャラの角度を設定する（線形補間）
		mainPosition.transform.rotate.y = turnFirstRotationY_ * ratio + destinationRotationY * (1.0f - ratio);
	}

	if (onGround_) {
		// ジャンプ
		if (system_->GetIsPush(DIK_W)) {
			// ジャンプ初速を加える
			velocity_.y += kJumpAcceleration;

			/// 音再生
			system_->SoundPlaySE(SH_JUMP_);
		}
	} else {
		// 落下速度
		velocity_.y -= kGravityAcceleration * deltaTime_;
		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	}
}

void Player::OnGroundChanger(const CollisionMapInfo& info) {
	// CollisionManager 已判定 floorHit，這裡只根據 floorHit 切換狀態
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		}
	} else {
		if (info.floorHit) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	}
}

void Player::MapCollisionDecideDown(CollisionMapInfo& info) {
	/// 移動後の4つの角の座標
	std::vector<Vector3> positionsNew((int)Corner4::kNumCorner);

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		Vector3 translation_ = {};
		translation_.x = mainPosition.transform.translate.x + info.moveVector.x * deltaTime_;
		translation_.y = mainPosition.transform.translate.y + info.moveVector.y * deltaTime_;
		translation_.z = mainPosition.transform.translate.z + info.moveVector.z * deltaTime_;
		positionsNew[i] = CornerPosition(translation_, static_cast<Corner4>(i));
	}
	// 這裡不再做 y<=0 萠地判定，floorHit 由 CollisionManager 決定
	if (info.moveVector.y > 0) {
		return;
	}
}

void Player::MovePlayerByResult(const CollisionMapInfo& info) {
	mainPosition.transform.translate.x += info.moveVector.x * deltaTime_;
	mainPosition.transform.translate.y += info.moveVector.y * deltaTime_;
	mainPosition.transform.translate.z += info.moveVector.z * deltaTime_;
}

void Player::ShootUpdate() {

	if (shootCD_.parameter_ != shootCD_.maxTime_) shootCD_.ToMix();

}

void Player::NearAttackUpdate() {

	/// 近接攻撃触発
	if (system_->GetTriggerOn(DIK_SPACE)) {
		if (!isNearAttack_) {
			nearAttackChangeTimer_.Reset0();
			isNearAttack_ = true;

		}
	}



}

void Player::DamageAndEffectPart() {
	if (damageEffectTimer_.parameter_ == damageEffectTimer_.maxTime_) {
		return;
	}

	damageEffectTimer_.ToMix();

	const float kFlashPeriod = 0.2f; // seconds
	float t = std::fmod(damageEffectTimer_.parameter_, kFlashPeriod);
	bool flashOn = (t < kFlashPeriod * 0.5f);

	if (flashOn) {
		objectParts_[0].materialConfig->useModelTexture = false;
		objectParts_[0].materialConfig->enableLighting = false;
		objectParts_[0].materialConfig->textureHandle = TH_White5x5_;
	} else {
		objectParts_[0].materialConfig->useModelTexture = true;
		objectParts_[0].materialConfig->enableLighting = true;
		objectParts_[0].materialConfig->textureHandle = TH_playerTexture_;
	}

	if (damageEffectTimer_.parameter_ == damageEffectTimer_.maxTime_) {
		disableDamage_ = false;
		objectParts_[0].materialConfig->useModelTexture = true;
		objectParts_[0].materialConfig->enableLighting = true;
		objectParts_[0].materialConfig->textureHandle = TH_playerTexture_;
	}
}
