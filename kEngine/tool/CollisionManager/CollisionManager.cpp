#include "CollisionManager.h"
#include "GAME/Object/Player/Player.h"
#include "Data/Game/MapChip/MapChipField.h"
#include "GAME/Object/Bullet/Bullet.h"
#include "GAME/Object/BackPoint/BackPoint.h"
#include "GAME/Object/Enemy/Enemy.h"
#include <algorithm>

void CollisionManager::Update(float deltaTime) {
	deltaTime_ = deltaTime;

	PlayerMapCollision();
	PlayerEnemyCollision();
	PlayerBackPointCollision();
	BulletMapCollision();
	BulletPlayerCollision();
	BulletEnemyCollision();
	EnemyMapCollision();
}

bool CollisionManager::PlayerMapCollision() {
    if (player_ == nullptr || mapChipField_ == nullptr) {
        return false;
    }

    AABB box = player_->GetAABB();
    Vector3 velocity = player_->GetVelocity();

    bool landed = false;

    // 直接呼叫 MapChipField 的碰撞修正
    Vector3 correction = mapChipField_->GetMapCollisionCorrection(
        box,
        velocity,
        deltaTime_,
        landed
    );

    // 如果沒有修正量 → 沒撞到
	auto originalMove = velocity * deltaTime_;
	if (fabs(correction.x - originalMove.x) < 0.0001f &&
		fabs(correction.y - originalMove.y) < 0.0001f)
    {
        player_->SetOnGround(false);
        return false;
    }

    // 更新位置
    Vector3 pos = player_->GetPosition();
    pos += correction;
    player_->SetPosition(pos);

    // 如果落地 → Y 速度清零
    if (landed) {
        velocity.y = 0.0f;
    }

	// 2. 往上撞天花板
	if (correction.y < originalMove.y) {
		velocity.y = 0.0f;
	}

    // X 軸如果撞牆 → X 速度清零
	if (fabs(correction.x - velocity.x * deltaTime_) > 0.0001f) {
        velocity.x = 0.0f;
    }

    player_->SetVelocity(velocity);
    player_->SetOnGround(landed);

    return true;
}

void CollisionManager::BulletMapCollision() {

	if (bulletList_ == nullptr || mapChipField_ == nullptr) { return; }

	uint32_t maxX = mapChipField_->GetNumBlockHorizontal();
	uint32_t maxY = mapChipField_->GetNumBlockVirtical();

	for (auto* bullet : *bulletList_) {
		if (bullet == nullptr || !bullet->IsAlive()) { continue; }

		Vector3 pos = bullet->GetPosition();
		MapChipField::WorldIndex idx = mapChipField_->GetWorldIndexByPosition(pos);

		// out of bounds -> delete
		bool outOfBounds = (
			idx.x < 0 ||
			idx.y < 0 ||
			idx.x >= (int)maxX ||
			idx.y >= (int)maxY
			);

		if (outOfBounds) {
			bullet->SetAlive(false);
			continue;
		}

		MapChipType tile = mapChipField_->GetMapChipTypeByWorld({ idx.x, idx.y });
		if (!CheckIsNotWall(tile)) {
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
	if (mapChipField_ == nullptr || enemyList_ == nullptr) { return; }

	auto processEnemy = [&](Enemy* e) {
		if (e == nullptr || !e->IsAlive()) { return; }
		Vector3 pos = e->GetPosition();
		Vector2 blockSize = mapChipField_->GetBlockSize();
		// 探測正前方一個格子（沿 X 軸面向方向，一個 block 寬）
		Vector3 probe = pos;
		probe.x += e->IsFaceRight() ? (blockSize.x * 0.5f) : -(blockSize.x * 0.5f);

		MapChipField::WorldIndex idx = mapChipField_->GetWorldIndexByPosition(probe);
		bool out = (idx.x < 0 || idx.y < 0 || idx.x >= (int)mapChipField_->GetNumBlockHorizontal() || idx.y >= (int)mapChipField_->GetNumBlockVirtical());
		if (out) { return; }

		MapChipType frontTile = mapChipField_->GetMapChipTypeByWorld({idx.x, idx.y});
		// check ground one block below the front probe
		Vector3 probeDown = probe;
		probeDown.y -= blockSize.y;
		MapChipField::WorldIndex idxDown = mapChipField_->GetWorldIndexByPosition(probeDown);
		bool outDown = (idxDown.x < 0 || idxDown.y < 0 || idxDown.x >= (int)mapChipField_->GetNumBlockHorizontal() || idxDown.y >= (int)mapChipField_->GetNumBlockVirtical());
		bool cliffAhead = outDown;
		if (!cliffAhead) {
			MapChipType belowTile = mapChipField_->GetMapChipTypeByWorld({idxDown.x, idxDown.y});
			cliffAhead = CheckIsNotWall(belowTile); // no ground means cliff
		}

		if (CheckIsWall(frontTile) || cliffAhead) {
			e->SetFaceRight(!e->IsFaceRight());
		}
	};

	for (auto* e : *enemyList_) { processEnemy(e); }
}

bool CollisionManager::CheckIsNotWall(MapChipType type) {
	return !CheckIsWall(type);
}

bool CollisionManager::CheckIsWall(MapChipType type) {
	switch (type) {
	case MapChipType::kBlank:
	case MapChipType::kEnemy:
	case MapChipType::kPlayer:
	case MapChipType::kBackPoint:
		return false;
	case MapChipType::kDirt:
	case MapChipType::kRock:
		return true;
	default:
		return false;
	}
}

void CollisionManager::InputPlayer(Player* player) {
	player_ = player;
}

void CollisionManager::InputMapChip(MapChipField* mapChipField) {
	mapChipField_ = mapChipField;
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
