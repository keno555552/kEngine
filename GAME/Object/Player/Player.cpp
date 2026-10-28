#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Player.h"
#include <algorithm>
#include <numbers>
#include "GAME/Object/Bullet/Bullet.h"
#include "GAME/Object/Sight/Sight.h"

Player::Player(kEngine* system, const Vector3& position) {
	Object::IntObject(system);
	Object::CreateDefaultData();
	mainPosition.transform.translate = position;
	objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

	mainPosition.transform.rotate.y = 0.0f;

	shootCD_.Init0(kShootInterval, system_->GetTimeManager());
	damageEffectTimer_.InitM(kPlayerDamageEffectTime, system_->GetTimeManager());

	MH_playerModel_ = system_->SetModelObj("GAME/resources/Object/player/player.obj");
	MH_pickaxe_ = system_->SetModelObj("GAME/resources/Object/pickaxe/pickaxe.obj");

	TH_playerTexture_ = system_->LoadTexture("GAME/resources/Object/player/playerUV.png");
	TH_playerTexture_damaged_ = system_->LoadTexture("GAME/resources/Object/player/playerUV_damage.png");
	TH_playerTexture_dead_ = system_->LoadTexture("GAME/resources/Object/player/playerUV_dead.png");

	TH_White5x5_ = system_->LoadTexture("kEngine/EngineAssets/TemplateResource/texture/white5x5.png");

	SH_JUMP_ = system_->SoundLoadSE("GAME/resources/sound/SE/player_jump.wav");
	SH_SHOOT_ = system_->SoundLoadSE("GAME/resources/sound/SE/player_attack.wav");
	SH_DAMAGE_ = system_->SoundLoadSE("GAME/resources/sound/SE/playerDamage.wav");

	nearAttackChangeTimer_.InitM(kNearAttackChangeTime, system_->GetTimeManager());
	nearAttackEffectTimer_.InitM(kNearAttackEffectTime, system_->GetTimeManager());

	/// 照準（★最後に追加）
	MH_sight_ = system_->SetModelObj("GAME/resources/Object/targeter/targeter.obj");
	sight_ = std::make_unique<Sight>(system_, this);
	sight_->IntObject(system_);
	sight_->CreateModelData(MH_sight_);
	sight_->mainPosition.transform.scale = { 0.5f, 0.5f, 0.5f };
	sight_->objectParts_[0].materialConfig->textureColor = { 0.8f,0.2f,0.2f,1.0f };

	/// Playerデータ設定
	HP_ = kPlayerMaxHP;

	/// bugCheck
	checkTimer_.Init0(1.0f, system_->GetTimeManager());
}

Player::~Player() = default;

void Player::Update(Camera* camera) {
	totelDeltaTime_ = system_->GetDeltaTime();

	if (isDead_) {          // ★ 入力・旋回・点滅 全部止める
		DeathUpdate();
		return;
	}
	BitMapBugCheck();
	BehaviorRootUpdate();
	DamageAndEffectPart();
	UpdataState();

	sight_->Update(camera);

}

void Player::Draw() {
	Object::Draw();
	if (!isDead_) sight_->Draw();
}


AABB& Player::GetAABB() {

	const Vector3& s = mainPosition.transform.scale;
	float halfWidth = kPlayerWidth * kPlayerHitScale * 0.5f;
	float halfHeight = kPlayerHeight * kPlayerHitScale * 0.5f;
	float halfDepth = kPlayerWidth * kPlayerHitScale * 0.5f;

	Vector3 center = mainPosition.transform.translate;

	hitBox_.min = center - Vector3(halfWidth, halfHeight, halfDepth);
	hitBox_.max = center + Vector3(halfWidth, halfHeight, halfDepth);

	return hitBox_;
}

void Player::SetCollisionResult(const CollisionResult& result) {

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.ceilingHit = result.ceilingHit;
	collisionMapInfo.floorHit = result.landed;
	collisionMapInfo.wallHit = result.xIsMainAxis;
	collisionMapInfo.moveVector = result.correction;

	MovePlayerByResult(collisionMapInfo);
	OnGroundChanger(collisionMapInfo);
}

void Player::GetDamage(float damage, const Vector3& attackerPos) {
	if (isDead_) return;
	disableDamage_ = true;
	damageEffectTimer_.Reset0();
	HP_ -= damage;
	if (HP_ <= 0.0f) StartDeath(attackerPos);

	/// 音再生
	system_->SoundPlaySE(SH_DAMAGE_);
}

Vector3 Player::GetAimPosition() const {
	return sight_->GetMouseOnPlane();
}

void Player::Shoot(Vector3 mousePos) {

	/// 出来ない条件に該当したら終了
	// 弾のリストがセットされてない
	if (bulletList_ == nullptr)return;
	// 射撃CD中
	if (shootCD_.parameter_ != shootCD_.maxTime_)return;
	// 状態が射撃可能でない
	if (static_cast<int>(playerState_) & cantShoot) return;

	/// 弾の生成
	Vector3 shootDirection = mousePos - mainPosition.transform.translate;
	Bullet* newBullet = new Bullet(system_, mainPosition.transform.translate, Normalize(shootDirection));
	bulletList_->emplace_back(newBullet);
	shootCD_.Reset0();

	/// 音再生
	system_->SoundPlaySE(SH_SHOOT_);
}

void Player::BehaviorRootUpdate() {
	/// 移動関連
	Move();

	///射撃CD更新
	ShootUpdate();

	/// 
}

Vector3 Player::CornerPosition(const Vector3& center, Corner4 corner) {
	Vector3 offsetTable[(int)Corner4::kNumCorner] = {
		{ +kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0 }, // kRightBottom
		{ -kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0 }, // kLeftBottom
		{ +kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0 }, // kRightTop
		{ -kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0 }, // kLeftTop
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
	if (moveLeft_ || moveRight_) {

		Vector3 acceleration{};

		// 方向が変わったら、旋回タイマーをリセットする
		if (moveRight_ && !moveLeft_) {

			if (lrDirection_ != LRDirection::kRight) {
				lrDirection_ = LRDirection::kRight;
				turnFirstRotationY_ = mainPosition.transform.rotate.y;
				turnTimer_ = kTimeTurn;
			}

			acceleration.x = +kAcceleration;
		} else if (moveLeft_ && !moveRight_) {

			if (lrDirection_ != LRDirection::kLeft) {
				lrDirection_ = LRDirection::kLeft;
				turnFirstRotationY_ = mainPosition.transform.rotate.y;
				turnTimer_ = kTimeTurn;
			}

			acceleration.x = -kAcceleration;
		}

		// 計算しだした加速度を速度に加える
		velocity_.x += acceleration.x * totelDeltaTime_;

		// 入力がない場合速度を減衰をかける
		if (!moveRight_ && !moveLeft_) {
			velocity_.x *= std::exp(-kAttenuation * totelDeltaTime_);
			if (std::abs(velocity_.x) < 0.01f) {
				velocity_.x = 0.0f;
			}
		}

		// 速度上限
		velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);


	} else {
		// 非入力時は移動減衰をかける
		velocity_.x *= std::exp(-kAttenuation * totelDeltaTime_);
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
		turnTimer_ -= totelDeltaTime_;
		turnTimer_ = std::max(turnTimer_, 0.0f);

		float destinationRotationYTable[] = {
			std::numbers::pi_v<float> / 2.0f,                              // 右
			-std::numbers::pi_v<float> / 2.0f,                             // 左
			0.0f // なし
		};
		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];
		float ratio = turnTimer_ / kTimeTurn; // 1 -> 0
		// 自キャラの角度を設定する（線形補間）
		mainPosition.transform.rotate.y = turnFirstRotationY_ * ratio + destinationRotationY * (1.0f - ratio);
	}

	if (onGround_ && moveJump_) {
		velocity_.y += kJumpAcceleration;
		system_->SoundPlaySE(SH_JUMP_);
	}
	velocity_.y -= kGravityAcceleration * totelDeltaTime_;
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);


	ResetMove();
}

void Player::ResetMove() {
	moveLeft_ = false;
	moveRight_ = false;
	moveJump_ = false;
}

void Player::OnGroundChanger(const CollisionMapInfo& info) {
	if (info.wallHit)    velocity_.x = 0.0f;
	if (info.ceilingHit) velocity_.y = 0.0f;

	if (info.floorHit) {
		if (!onGround_) velocity_.x *= (1.0f - kAttenuationLanding);   // 着地した瞬間
		velocity_.y = 0.0f;
		onGround_ = true;
	} else {
		onGround_ = false;
	}
}

void Player::MovePlayerByResult(const CollisionMapInfo& info) {
	mainPosition.transform.translate += info.moveVector;
}

void Player::BitMapBugCheck() {
	checkTimer_.ToMix();
	if (!checkTimer_.GetIsMax()) {
		return;
	}
	checkTimer_.Reset0();

	{
		std::vector<int> bugChecksList = {
			{ static_cast<int>(PlayerState::kDamage) | static_cast<int>(PlayerState::kMove) },
			{ static_cast<int>(PlayerState::kDamage) | static_cast<int>(PlayerState::kShoot) },
			{ static_cast<int>(PlayerState::kDamage) | static_cast<int>(PlayerState::kNearAttack) },
			{ static_cast<int>(PlayerState::kDamage) | static_cast<int>(PlayerState::kDush) },
		};

		for (auto& bugCheck : bugChecksList) {
			if ((static_cast<int>(playerState_) & bugCheck) == bugCheck) {
				Logger::Log("Player:: PlayerState Bug Detected:");
				// 状態をビットごとに出力
				for (int i = 0; i < static_cast<int>(PlayerState::numStates); i++) {
					if (static_cast<int>(playerState_) & (1 << i)) {
						Logger::Log("%d,", (1 << i));
					}
				}
			}
		}
	}
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
	///<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 未完成
}

void Player::UpdataState() {
}

void Player::StartDeath(const Vector3& attackerPos) {
	isDead_ = true;
	playerState_ = PlayerState::kDead;     // 射撃も自動で不可になる

	/// 攻撃者の方を向く（kRight = +π/2 と同じ）
	const float faceSign = (attackerPos.x >= mainPosition.transform.translate.x) ? 1.0f : -1.0f;
	mainPosition.transform.rotate.y = faceSign * std::numbers::pi_v<float> *0.5f;
	turnTimer_ = 0.0f;

	/// 後ろへ吹っ飛ぶ
	velocity_.x = -faceSign * kDeathKnockbackX;
	velocity_.y = kDeathKnockbackY;
	onGround_ = false;

	deathPhase_ = DeathPhase::kFall;
	deathTimer_.Init0(kDeathFallTime, system_->GetTimeManager());
	SetFaceTexture(TH_playerTexture_damaged_);
}

void Player::DeathUpdate() {
	/// 物理：重力だけ。着地したら滑りを止める
	velocity_.y -= kGravityAcceleration * totelDeltaTime_;
	velocity_.y = std::max(velocity_.y, -kLimitFallSpeed);
	if (onGround_) velocity_.x *= std::exp(-kDeathAttenuation * totelDeltaTime_);

	float& rotX = objectParts_[0].transform.rotate.x;

	switch (deathPhase_) {
	case DeathPhase::kFall:
		/// T^r：だんだん速く倒れる
		rotX = kDeathFallAngle * deathTimer_.easyOut(kDeathFallPower);
		deathTimer_.ToMix();

		/// 倒れきった ＆ 着地した瞬間 → バタン
		if (deathTimer_.GetIsMax() && onGround_) {
			rotX = kDeathFallAngle;
			SetFaceTexture(TH_playerTexture_dead_);
			//system_->SoundPlaySE(SH_LOSE_);
			deathPhase_ = DeathPhase::kBounce;
			deathTimer_.Init0(kDeathBounceTime, system_->GetTimeManager());
		}
		break;

	case DeathPhase::kBounce:
		/// 一回だけ少し起き上がって戻る
		rotX = kDeathFallAngle * (1.0f - kDeathBounceRatio * std::sin(std::numbers::pi_v<float> *deathTimer_.linearity()));
		if (deathTimer_.ToMix()) {
			rotX = kDeathFallAngle;
			deathPhase_ = DeathPhase::kLie;
		}
		break;

	case DeathPhase::kLie:
		break;
	}
}

void Player::SetFaceTexture(int handle) {
	auto& mc = objectParts_[0].materialConfig;
	mc->useModelTexture = false;
	mc->enableLighting = true;
	mc->textureHandle = handle;
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
