#include "CollisionManager.h"
#include "Player/Player.h"
#include "tool/MapChip/MapChipField.h"
#include "Bullet/Bullet.h"
#include "BackPoint/BackPoint.h"
#include "GameObject/Enemy/Enemy.h"
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

void CollisionManager::MapCollisionCircle() {

}

bool CollisionManager::PlayerMapCollision() {

	if (player_ == nullptr || mapChipField_ == nullptr) { return false; }

	AABB box = player_->GetMapChipAABB();
	Vector3 velocity = player_->GetVelocity();
	Vector3 move = velocity * deltaTime_;
	AABB predicted = box;
	predicted.min += move;
	predicted.max += move;

	MapChipField::IndexSet minIdx = mapChipField_->GetMapChipIndexByPosition({ box.min.x, box.min.y, box.min.z });
	MapChipField::IndexSet maxIdx = mapChipField_->GetMapChipIndexByPosition({ box.max.x, box.max.y, box.max.z });

	// 夾到範圍後重新排序，避免 min>max 迴圈不執行
	minIdx.xIndex = std::clamp(minIdx.xIndex, 0, (int)mapChipField_->GetNumBlockHorizontal() - 1);
	maxIdx.xIndex = std::clamp(maxIdx.xIndex, 0, (int)mapChipField_->GetNumBlockHorizontal() - 1);
	minIdx.yIndex = std::clamp(minIdx.yIndex, 0, (int)mapChipField_->GetNumBlockVirtical() - 1);
	maxIdx.yIndex = std::clamp(maxIdx.yIndex, 0, (int)mapChipField_->GetNumBlockVirtical() - 1);

	int xBegin = std::min<int>(minIdx.xIndex, maxIdx.xIndex);
	int xEnd   = std::max<int>(minIdx.xIndex, maxIdx.xIndex);
	int yBegin = std::min<int>(minIdx.yIndex, maxIdx.yIndex);
	int yEnd   = std::max<int>(minIdx.yIndex, maxIdx.yIndex);

	Vector3 push{};
	bool hit = false;
	bool landed = false;

	for (int y = yBegin; y <= yEnd; ++y) {
		for (int x = xBegin; x <= xEnd; ++x) {

			MapChipType checker = mapChipField_->GetMapChipTypeByIndex((uint32_t)x, (uint32_t)y);
			if (CheckIsNotWall(checker))continue;

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(x, y);

			float overlapX = std::min<float>(box.max.x, rect.right)  - std::max<float>(box.min.x, rect.left);
			float overlapY = std::min<float>(box.max.y, rect.top)    - std::max<float>(box.min.y, rect.bottom);
			if (overlapX <= 0.0f || overlapY <= 0.0f) { continue; }

			hit = true;

			if (overlapX < overlapY) {
				float dirX = (velocity.x != 0.0f)
					? (velocity.x > 0.0f ? -1.0f : 1.0f)
					: (((box.min.x + box.max.x) * 0.5f < (rect.left + rect.right) * 0.5f) ? -1.0f : 1.0f);
				push.x += dirX * overlapX;
				velocity.x = 0.0f;
			} else {
				float dirY = (velocity.y != 0.0f)
					? (velocity.y > 0.0f ? -1.0f : 1.0f)
					: (((box.min.y + box.max.y) * 0.5f < (rect.bottom + rect.top) * 0.5f) ? -1.0f : 1.0f);
				push.y += dirY * overlapY;
				velocity.y = 0.0f;
				if (push.y > 0.0f) { landed = true; } // 從上撞到方塊
			}

			box.min.x += push.x; box.max.x += push.x;
			box.min.y += push.y; box.max.y += push.y;
			predicted.min += push; predicted.max += push;
		}
	}

	// 4-point check using predicted position to curb tunneling
	Vector3 corners[4] = {
		{predicted.min.x, predicted.min.y, predicted.min.z},
		{predicted.max.x, predicted.min.y, predicted.min.z},
		{predicted.min.x, predicted.max.y, predicted.min.z},
		{predicted.max.x, predicted.max.y, predicted.min.z}
	};
	for (auto& c : corners) {
		MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexByPosition(c);
		if (idx.xIndex < 0 || idx.yIndex < 0 || idx.xIndex >= (int)mapChipField_->GetNumBlockHorizontal() || idx.yIndex >= (int)mapChipField_->GetNumBlockVirtical()) {
			continue;
		}
		MapChipType checker = mapChipField_->GetMapChipTypeByIndex((uint32_t)idx.xIndex, (uint32_t)idx.yIndex);
		if (CheckIsNotWall(checker)) continue;
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(idx.xIndex, idx.yIndex);
		float overlapX = std::min<float>(predicted.max.x, rect.right) - std::max<float>(predicted.min.x, rect.left);
		float overlapY = std::min<float>(predicted.max.y, rect.top)   - std::max<float>(predicted.min.y, rect.bottom);
		if (overlapX <= 0.0f || overlapY <= 0.0f) { continue; }
		hit = true;
		if (overlapX < overlapY) {
			float dirX = (move.x != 0.0f)
				? (move.x > 0.0f ? -1.0f : 1.0f)
				: (((predicted.min.x + predicted.max.x) * 0.5f < (rect.left + rect.right) * 0.5f) ? -1.0f : 1.0f);
			push.x += dirX * overlapX;
			velocity.x = 0.0f;
		} else {
			float dirY = (move.y != 0.0f)
				? (move.y > 0.0f ? -1.0f : 1.0f)
				: (((predicted.min.y + predicted.max.y) * 0.5f < (rect.bottom + rect.top) * 0.5f) ? -1.0f : 1.0f);
			push.y += dirY * overlapY;
			velocity.y = 0.0f;
			if (push.y > 0.0f) { landed = true; }
		}
		box.min += push; box.max += push;
		predicted.min += push; predicted.max += push;
	}

	if (hit) {
		Vector3 pos = player_->GetPosition();
		pos += push;
		player_->SetPosition(pos);
		player_->SetVelocity(velocity);
		player_->SetOnGround(landed);
		return true;
	} else {
		player_->SetOnGround(false);
		return false;
	}
}

void CollisionManager::BulletMapCollision() {

	if (bulletList_ == nullptr || mapChipField_ == nullptr) { return; }

	uint32_t maxX = mapChipField_->GetNumBlockHorizontal();
	uint32_t maxY = mapChipField_->GetNumBlockVirtical();

	for (auto* bullet : *bulletList_) {
		if (bullet == nullptr || !bullet->IsAlive()) { continue; }

		Vector3 pos = bullet->GetPosition();
		MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexByPosition(pos);

		// out of bounds -> delete
		bool outOfBounds = (idx.xIndex < 0 || idx.yIndex < 0 || idx.xIndex >= (int)maxX || idx.yIndex >= (int)maxY);
		if (outOfBounds) {
			bullet->SetAlive(false);
			continue;
		}

		MapChipType tile = mapChipField_->GetMapChipTypeByIndex((uint32_t)idx.xIndex, (uint32_t)idx.yIndex);
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

		MapChipField::IndexSet idx = mapChipField_->GetMapChipIndexByPosition(probe);
		bool out = (idx.xIndex < 0 || idx.yIndex < 0 || idx.xIndex >= (int)mapChipField_->GetNumBlockHorizontal() || idx.yIndex >= (int)mapChipField_->GetNumBlockVirtical());
		if (out) { return; }

		MapChipType frontTile = mapChipField_->GetMapChipTypeByIndex((uint32_t)idx.xIndex, (uint32_t)idx.yIndex);
		// check ground one block below the front probe
		Vector3 probeDown = probe;
		probeDown.y -= blockSize.y;
		MapChipField::IndexSet idxDown = mapChipField_->GetMapChipIndexByPosition(probeDown);
		bool outDown = (idxDown.xIndex < 0 || idxDown.yIndex < 0 || idxDown.xIndex >= (int)mapChipField_->GetNumBlockHorizontal() || idxDown.yIndex >= (int)mapChipField_->GetNumBlockVirtical());
		bool cliffAhead = outDown;
		if (!cliffAhead) {
			MapChipType belowTile = mapChipField_->GetMapChipTypeByIndex((uint32_t)idxDown.xIndex, (uint32_t)idxDown.yIndex);
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
