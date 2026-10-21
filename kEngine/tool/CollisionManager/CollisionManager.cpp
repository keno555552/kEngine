#include "CollisionManager.h"
#include "GAME/Object/Player/Player.h"
#include "Tool/MapBulider/MapChipManager.h"
#include "GAME/Object/Bullet/Bullet.h"
#include "GAME/Object/BackPoint/BackPoint.h"
#include "GAME/Object/Enemy/Enemy.h"
#include <algorithm>

void CollisionManager::Update(float deltaTime) {
	totelDeltaTime_ = deltaTime;

	PlayerMapCollision();
	PlayerEnemyCollision();
	PlayerBackPointCollision();
	BulletMapCollision();
	BulletPlayerCollision();
	BulletEnemyCollision();
	EnemyMapCollision();
}

bool CollisionManager::PlayerMapCollision() {

	if (!player_ || !mapChipManager_) return false;

	AABB box = player_->GetAABB();
	Vector3 velocity = player_->GetVelocity();

	// MapChipField内での当たり判定を計算
	CollisionResult result = mapChipManager_->GetMapCollisionCorrection(
		box,
		velocity,
		totelDeltaTime_
	);

	// 当たると速度を0にする
	if (result.xIsMainAxis) {
		velocity.x = 0.0f;
	}
	if (result.yIsMainAxis) {
		velocity.y = 0.0f;
	}

	player_->SetVelocity(velocity);
	player_->SetOnGround(result.landed);

	Vector3 velAfter = player_->GetVelocity();
	// Logger::Log("PlayerVelocity: (%f, %f, %f)", velAfter.x, velAfter.y, velAfter.z);

	return true;
}

void CollisionManager::BulletMapCollision() {

	if (bulletList_ == nullptr || mapChipManager_ == nullptr) { return; }

	for (auto* bullet : *bulletList_) {
		if (bullet == nullptr || !bullet->IsAlive()) { continue; }

		const Vector3 pos = bullet->GetPosition();
		// マップ外 → 削除
		if (!mapChipManager_->IsInsideAt(pos)) {
			bullet->SetAlive(false);
			continue;
		}
		if (mapChipManager_->IsWallAt(pos)) {
			bullet->HitRockEffect();
			bullet->SetAlive(false);
		}
	}
}

void CollisionManager::BulletPlayerCollision() {
	if (bulletList_ == nullptr || player_ == nullptr) { return; }

	const float kMaxDistSq = 8.0f * 8.0f;
	Vector3 playerPos = player_->GetPosition();

	for (auto* bullet : *bulletList_) {
		if (bullet == nullptr || !bullet->IsAlive()) { continue; }

		Vector3 d = bullet->GetPosition() - playerPos;
		float distSq = d.x * d.x + d.y * d.y + d.z * d.z;
		if (distSq > kMaxDistSq) {
			bullet->SetAlive(false);
		}
	}
}

void CollisionManager::BulletEnemyCollision() {
	if (bulletList_ == nullptr || enemyList_ == nullptr) { return; }

	auto isOverlap = [](const AABB& a, const AABB& b) {
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
			(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
			(a.min.z <= b.max.z && a.max.z >= b.min.z);
		};

	for (auto* bullet : *bulletList_) {
		if (bullet == nullptr || !bullet->IsAlive()) { continue; }
		AABB bBox = bullet->GetAABB(bullet);
		for (auto* enemy : *enemyList_) {
			if (enemy == nullptr || !enemy->IsAlive()) { continue; }
			AABB eBox = enemy->GetAABB();
			if (isOverlap(bBox, eBox)) {
				bullet->SetAlive(false);
				enemy->GetDamage(1);
				break;
			}
		}
	}
}


void CollisionManager::PlayerEnemyCollision() {
	if (player_ == nullptr || enemyList_ == nullptr) { return; }
	if (player_->IsDisableDamage()) return;

	AABB pBox = player_->GetAABB();

	auto isOverlap = [](const AABB& a, const AABB& b) {
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
			(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
			(a.min.z <= b.max.z && a.max.z >= b.min.z);
		};

	for (auto* enemy : *enemyList_) {
		if (enemy == nullptr || !enemy->IsAlive()) { continue; }
		AABB eBox = enemy->GetAABB();
		if (isOverlap(pBox, eBox)) {
			// damage
			player_->GetDamage(1.0f);

			// knockback: apply velocity impulse instead of teleport
			Vector3 dir = player_->GetPosition() - enemy->GetPosition();
			dir.y = 0.0f; // horizontal direction for push
			float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
			if (lenSq < 0.0001f) {
				dir = { 1.0f, 0.0f, 0.0f }; // fallback
			} else {
				dir = Normalize(dir);
			}
			const float kKnockbackSpeed = 4.0f; // adjust as needed
			Vector3 knockbackVel = dir * kKnockbackSpeed;
			knockbackVel.y = 0.5f; // small lift
			player_->SetVelocity(knockbackVel);

			// refresh player box for further collisions this frame
			pBox = player_->GetAABB();
		}
	}
}

void CollisionManager::PlayerBackPointCollision() {
	if (player_ == nullptr || backPoint_ == nullptr) { return; }

	AABB pBox = player_->GetAABB();
	AABB bBox = backPoint_->GetAABB();
	auto isOverlap = [](const AABB& a, const AABB& b) {
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
			(a.min.y <= b.max.y && a.max.y >= b.min.y) &&
			(a.min.z <= b.max.z && a.max.z >= b.min.z);
		};
	if (isOverlap(pBox, bBox)) {
		backPoint_->SetIsBack(true);
	} else {
		backPoint_->SetIsBack(false);
	}
}

void CollisionManager::EnemyMapCollision() {
	if (mapChipManager_ == nullptr || enemyList_ == nullptr) { return; }

	const Vector2 blockSize = mapChipManager_->GetBlockSize();
	for (auto* e : *enemyList_) {
		if (e == nullptr || !e->IsAlive()) { continue; }   // ★return → continue

		// 正面の 1 マス先（向いている方向に半マス）
		Vector3 probe = e->GetPosition();
		probe.x += e->IsFaceRight() ? (blockSize.x * 0.5f) : -(blockSize.x * 0.5f);
		if (!mapChipManager_->IsInsideAt(probe)) { continue; }   // ★return → continue

		const bool frontWall = mapChipManager_->IsWallAt(probe);
		// 正面の 1 マス下に地面があるか
		Vector3 probeDown = probe;
		probeDown.y -= blockSize.y;
		const bool cliffAhead = !mapChipManager_->IsInsideAt(probeDown) || !mapChipManager_->IsWallAt(probeDown);

		if (frontWall || cliffAhead) {
			e->SetFaceRight(!e->IsFaceRight());
		}
	}
}

void CollisionManager::InputPlayer(Player* player) {
	player_ = player;
}

void CollisionManager::InputMapChip(MapChipManager* mapChipManager) {
	mapChipManager_ = mapChipManager;
}

void CollisionManager::InputBackPoint(BackPoint* backPoint) {
	backPoint_ = backPoint;
}

void CollisionManager::InputBulletList(std::vector<Bullet*>* bulletList) {
	bulletList_ = bulletList;
}

void CollisionManager::InputEnemyList(std::vector<Enemy*>* enemyList) {
	enemyList_ = enemyList;
}
