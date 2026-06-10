#include "Enemy.h"
#include <numbers>

Enemy::Enemy(kEngine* system, const Vector3& position) {
	Object::IntObject(system);
	Object::CreateDefaultData();
	mainPosition.transform.translate = position;

	TH_White5x5_ = system_->LoadTexture("kEngine/EngineAssets/TemplateResource/texture/white5x5.png");
	TH_Model_ = system_->LoadTexture("GAME/resources/Object/enemy/enemy.png");

	SH_Damage_ = system_->SoundLoadSE("GAME/resources/Sound/SE/enemyDamage.wav");

	/// Timer 初期化
	turnTimer_.Init0(kEnemyTurnTime, system_->GetTimeManager());
	damageEffectTimer_.InitM(kEnemyDamageEffectTime, system_->GetTimeManager());
	animationTimer_.Init0(kEnemyAnimationTime, system_->GetTimeManager());

}

Enemy::~Enemy() {
}

void Enemy::Update(Camera* camera) {

	if (!isAlive_)return;

	float deltaTime = system_->GetDeltaTime();

	// ======== 轉身控制（依 isFaceRight_ 與當前 Y 軸角度對比）======== //
	float targetYaw = isFaceRight_ ? 0.0f : std::numbers::pi_v<float>;
	float currentYaw = mainPosition.transform.rotate.y;
	// 將差值規範到 [-pi, pi]
	float diff = targetYaw - currentYaw;
	while (diff > std::numbers::pi_v<float>) diff -= 2.0f * std::numbers::pi_v<float>;
	while (diff < -std::numbers::pi_v<float>) diff += 2.0f * std::numbers::pi_v<float>;

	if (std::abs(diff) > 0.001f) {
		float turnSpeed = std::numbers::pi_v<float> / kEnemyTurnTime;
		float step = std::clamp(diff, -turnSpeed * deltaTime, turnSpeed * deltaTime);
		mainPosition.transform.rotate.y += step;
	} else {
		mainPosition.transform.rotate.y = targetYaw;
	}

	/// ======== 移動（向きにより）======== //
	if (isFaceRight_) {
		mainPosition.transform.translate.x += kEnemyAcceleration * deltaTime;
	} else {
		mainPosition.transform.translate.x -= kEnemyAcceleration * deltaTime;
	}

	/// ====== 被ダメージエフェクト制御 ====== ///
	if (damageEffectTimer_.parameter_ != damageEffectTimer_.maxTime_) {
		damageEffectTimer_.ToMix();
	} else {
		if (objectParts_[0].materialConfig->textureHandle == TH_White5x5_) {
			objectParts_[0].materialConfig->enableLighting = true;
			objectParts_[0].materialConfig->textureHandle = TH_Model_;
		}
	}

	/// ====== アニメーション制御 ====== ///

	if (isTimeUp_) {
		animationTimer_.ToMix();
		if (animationTimer_.parameter_ == animationTimer_.maxTime_)isTimeUp_ = false;
	} else {
		animationTimer_.ToZero();
		if (animationTimer_.parameter_ == 0.0f)isTimeUp_ = true;
	}

	objectParts_[0].transform.scale.x = kEnemyAnimationSmallSize + (kEnemyAnimationBigSize - kEnemyAnimationSmallSize) * animationTimer_.linearity();

	Object::Update(camera);
}

void Enemy::Draw() {
	if (!isAlive_)return;
	Object::Draw();
}

void Enemy::GetDamage(int damage) {

	hp_ -= damage;

	if (hp_ <= 0) {
		isAlive_ = false;
	}

	damageEffectTimer_.Reset0();

	objectParts_[0].materialConfig->enableLighting = false;
	objectParts_[0].materialConfig->textureHandle = TH_White5x5_;


	system_->SoundPlaySE(SH_Damage_,0.6f);
}

AABB& Enemy::GetAABB() {
	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = 0.5f * std::abs(s.x);
	float halfHeight = 0.3f * std::abs(s.y);
	float halfDepth = 0.5f * std::abs(s.z);
	hitBox_.min = mainPosition.transform.translate - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = mainPosition.transform.translate + Vector3(halfWidth, halfHeight, halfDepth);
	return hitBox_;
}



